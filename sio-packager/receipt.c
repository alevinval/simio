#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sha256.h"
#include "receipt.h"
#include "block.h"
#include "dirnav.h"
#include "dir.h"
#include "util.h"

void recover_block(struct receipt *receipt, struct block *parity, int fd)
{
	int i;
	unsigned char *missing_buffer;
	unsigned char *block_buffer;
	struct block *block;
	struct block *missing_block;

	block_buffer = malloc(sizeof(unsigned char) * receipt->block_size);
	missing_buffer = malloc(sizeof(unsigned char) * receipt->block_size);

	memcpy(missing_buffer, parity->buffer, receipt->block_size);
	for (block = receipt->blocks->head; block != NULL; block = block->next) {
		if (block->corrupted != 1) {
			block->buffer = block_buffer;
			fetch_block_data(block);
			for (i = 0; i < block->size; i++) {
				missing_buffer[i] =
				    block->buffer[i] ^ missing_buffer[i];
			}
		}
	}
	missing_block = block_from_buffer(missing_buffer, receipt->block_size);
	store_block(missing_block);
	write(fd, missing_block->buffer, receipt->block_size);
}

void recover_original_file(struct receipt *receipt)
{
	int fd;
	unsigned char *block_buffer;
	struct block *block;

	fd = open_create_file(receipt->name);

	if (fd == -1)
		die("recover_original_file: cannot create the original file");

	block_buffer = malloc(sizeof(unsigned char) * receipt->block_size);

	for (block = receipt->blocks->head; block != NULL; block = block->next) {
		block->buffer = block_buffer;
		fetch_block_data(block);
		write(fd, block->buffer, block->size);
		block->buffer = NULL;
	}

	close(fd);
	free(block_buffer);
}

void recover_original_file_i(struct receipt *receipt)
{
	int i, fd, integrity_error = 0;
	unsigned char tmp_name[FNAME_LEN];
	unsigned char *block_buffer;
	struct block *block;

	strcpy((char *)tmp_name, "");
	strcat((char *)tmp_name, (char *)receipt->name);
	strcat((char *)tmp_name, ".tmp");

	fd = open_create_file(tmp_name);

	if (fd == -1)
		die("recover_original_file_i: cannot create the original file");

	block_buffer = malloc(sizeof(unsigned char) * receipt->block_size);

	i = 0;
	for (block = receipt->blocks->head; block != NULL; block = block->next) {
		i++;
		block->buffer = block_buffer;
		fetch_block_data(block);
		if (check_block_integrity(block) == 1) {
			write(fd, block->buffer, block->size);
		} else {
			error("integrity failure in Block %i\n [ %s ]",
			      i, block->name);

			block->corrupted = 1;
			struct block *parity = receipt->parities->head;

			printf("recovering Parity [%s]\n", parity->name);
			parity->buffer =
			    malloc(sizeof(unsigned char) * receipt->block_size);
			fetch_block_data(parity);
			recover_block(receipt, parity, fd);

			/*
			   error("integrity failure in Block %i\n [ %s ]",
			   i, block->name);
			   integrity_error = 1;
			   remove_file(tmp_name);
			   break;
			 */
		}
	}

	close(fd);

	if (integrity_error != 1) {
		mv_parent();
		unlink((char *)receipt->name);
		rename((char *)tmp_name, (char *)receipt->name);
		mv_package_root();
	}

	free(block_buffer);
}

void set_receipt_hash(struct receipt *receipt)
{
	int i;
	unsigned char *hash_buffer =
	    malloc(sizeof(unsigned char) * 32 * receipt->size);
	struct block *block;

	i = 0;
	for (block = receipt->blocks->head; block != NULL; block = block->next) {
		memcpy(&hash_buffer[i * 32], block->sha2, 32);
		i++;
	}

	sha256(receipt->sha2, hash_buffer, receipt->size * 32);
	free(hash_buffer);
}

void write_receipt_header(int fd, struct receipt *receipt)
{
	write(fd, receipt->sha2, 32);
	write(fd, receipt->name, FNAME_LEN);
	write(fd, &receipt->size, sizeof(int));
	write(fd, &receipt->parities_num, sizeof(int));
	write(fd, &receipt->block_size, sizeof(int));
}

void write_receipt_blocks(int fd, struct receipt *receipt)
{
	struct block *block;

	for (block = receipt->blocks->head; block != NULL; block = block->next) {
		write(fd, block->sha2, 32);
	}

	for (block = receipt->parities->head; block != NULL;
	     block = block->next) {
		write(fd, block->sha2, 32);
	}
}

void read_receipt_header(int fd, struct receipt *receipt)
{
	read(fd, receipt->sha2, 32);
	read(fd, receipt->name, FNAME_LEN);
	read(fd, &receipt->size, sizeof(int));
	read(fd, &receipt->parities_num, sizeof(int));
	read(fd, &receipt->block_size, sizeof(int));
	receipt->blocks = block_list_alloc();
	receipt->parities = block_list_alloc();
}

void read_receipt_blocks(int fd, struct receipt *receipt)
{
	int i;
	unsigned char block_sha2[32];
	struct block *block;

	for (i = 0; i < receipt->size; i++) {
		read(fd, &block_sha2, 32);
		block = fetch_block(block_sha2, receipt->block_size);
		block_list_add(receipt->blocks, block);
	}
	for (i = 0; i < receipt->parities_num; i++) {
		read(fd, &block_sha2, 32);
		block = fetch_block(block_sha2, receipt->block_size);
		block_list_add(receipt->parities, block);
	}
}

void set_receipt_header(struct receipt *receipt, unsigned char *file_path,
			unsigned int block_size)
{
	int fd, extra;

	fd = open_file(file_path);
	strcpy((char *)receipt->name, (char *)file_path);
	extra = file_size(fd) % block_size;
	if (extra > 0)
		receipt->size = file_size(fd) / block_size + 1;
	else
		receipt->size = file_size(fd) / block_size;
	receipt->parities_num = 0;
	receipt->block_size = block_size;
	receipt->blocks = block_list_alloc();
	receipt->parities = block_list_alloc();
}

void build_receipt(struct receipt *receipt)
{
	int i, fd, readed_bytes;
	unsigned char *buffer;
	struct block *block;

	fd = open_file(receipt->name);
	buffer = malloc(sizeof(unsigned char) * receipt->block_size);
	for (i = 0; i < receipt->size; i++) {
		readed_bytes = fill_buffer(fd, buffer, receipt->block_size);
		block = block_from_buffer(buffer, readed_bytes);
		block_list_add(receipt->blocks, block);
		store_block(block);
	}

	free(buffer);
}

void build_parity(struct receipt *receipt)
{
	// Global parity atm
	int i;
	struct block *parity;
	struct block *block;
	unsigned char *block_buffer;
	unsigned char *parity_buffer;

	block_buffer = malloc(sizeof(unsigned char) * receipt->block_size);
	parity_buffer = malloc(sizeof(unsigned char) * receipt->block_size);

	// Load first block in parity buffer.
	block = receipt->blocks->head;
	block->buffer = block_buffer;
	fetch_block_data(block);
	memcpy(parity_buffer, block->buffer, block->size);

	// Start block XORing for global parity
	for (block = block->next; block != NULL; block = block->next) {
		block->buffer = block_buffer;
		fetch_block_data(block);
		for (i = 0; i < block->size; i++)
			parity_buffer[i] = block->buffer[i] ^ parity_buffer[i];
	}

	receipt->parities_num++;
	parity = block_from_buffer(parity_buffer, receipt->block_size);
	block_list_add(receipt->parities, parity);
	store_block(parity);

	free(block_buffer);
	free(parity_buffer);
}

void
create_receipt(struct receipt *receipt,
	       unsigned char *file_path, unsigned int block_size)
{
	set_receipt_header(receipt, file_path, block_size);
	build_receipt(receipt);
	set_receipt_hash(receipt);
	build_parity(receipt);
}

void unpack_receipt(struct receipt *receipt, int skip_integrity_flag)
{
	if (skip_integrity_flag)
		recover_original_file(receipt);
	else
		recover_original_file_i(receipt);
}

void store_receipt(struct receipt *receipt)
{
	int fd;

	fd = open_create_receipt((unsigned char *)".receipt");
	write_receipt_header(fd, receipt);
	write_receipt_blocks(fd, receipt);
	close(fd);
}

void fetch_receipt(struct receipt *receipt)
{
	int fd;

	fd = open_receipt((unsigned char *)".receipt");
	read_receipt_header(fd, receipt);
	read_receipt_blocks(fd, receipt);
	close(fd);
}
