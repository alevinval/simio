#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sha256.h"
#include "receipt.h"
#include "dirnav.h"
#include "dir.h"
#include "util.h"

void recover_original_file(Receipt * receipt)
{
	int fd, block_size;
	unsigned char block_name[SHA256_STRING];
	unsigned char *block_buffer;
	Block *block;

	fd = open_create_file(receipt->name);

	if (fd == -1)
		die("recover_original_file: cannot create the original file");

	block_buffer = malloc(sizeof(unsigned char) * receipt->block_size);

	for (block = receipt->blocks->head; block != NULL; block = block->next) {
		block->buffer = block_buffer;
		fetch_block(block);
		write(fd, block->buffer, block->size);
		block->buffer = NULL;
	}

	close(fd);
	free(block_buffer);
}

void recover_original_file_i(Receipt * receipt)
{
	int i, fd, integrity_error = 0;
	unsigned char tmp_name[FNAME_LEN];
	unsigned char *block_buffer;
	Block *block;

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
		fetch_block(block);
		if (check_block_integrity(block) == 1) {
			write(fd, block->buffer, block->size);
		} else {
			error("integrity failure in Block %i\n [ %s ]",
			      i, block->name);
			integrity_error = 1;
			remove_file(tmp_name);
			break;
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

void set_receipt_hash(Receipt * receipt)
{
	int i;
	int n_blocks = receipt->size;
	unsigned char *hash_buffer =
	    malloc(sizeof(unsigned char) * 32 * n_blocks);
	Block *block;

	i = 0;
	for (block = receipt->blocks->head; block != NULL; block = block->next) {
		memcpy(&hash_buffer[i * 32], block->sha2, 32);
		i++;
	}

	sha256(receipt->sha2, hash_buffer, n_blocks * 32);
	free(hash_buffer);
}

void write_receipt_header(int fd, Receipt * receipt)
{
	write(fd, receipt->sha2, 32);
	write(fd, receipt->name, FNAME_LEN);
	write(fd, &receipt->size, sizeof(int));
	write(fd, &receipt->block_size, sizeof(int));
}

void write_receipt_blocks(int fd, Receipt * receipt)
{
	Block *block;

	for (block = receipt->blocks->head; block != NULL; block = block->next) {
		write(fd, block->sha2, 32);
	}
}

void read_receipt_header(int fd, Receipt * receipt)
{
	read(fd, receipt->sha2, 32);
	read(fd, receipt->name, FNAME_LEN);
	read(fd, &receipt->size, sizeof(int));
	read(fd, &receipt->block_size, sizeof(int));
	receipt->blocks = block_list_alloc();
}

void read_receipt_blocks(int fd, Receipt * receipt)
{
	int i;
	Block *block;

	for (i = 0; i < receipt->size; i++) {
		block = block_alloc();
		read(fd, block->sha2, 32);
		block->size = receipt->block_size;
		sha2hexf(block->name, block->sha2);
		block_list_add(receipt->blocks, block);
	}
}

void set_receipt_header(Receipt * receipt, unsigned char *file_path,
			unsigned int block_size)
{
	int fd;

	fd = open_file(file_path);
	strcpy((char *)receipt->name, (char *)file_path);
	receipt->size = file_size(fd) / block_size;
	receipt->block_size = block_size;
	receipt->blocks = block_list_alloc();

	close(fd);
}

void build_receipt(Receipt * receipt)
{
	int i, fd;
	unsigned char *buffer;
	Block *block;

	fd = open_file(receipt->name);
	buffer = malloc(sizeof(unsigned char) * receipt->block_size);
	for (i = 0; i < receipt->size; i++) {
		block = block_alloc();
		fill_block(block, fd, receipt->block_size, buffer);
		block_list_add(receipt->blocks, block);
		store_block(block);
	}

	free(buffer);
}

void
create_receipt(Receipt * receipt,
	       unsigned char *file_path, unsigned int block_size)
{
	set_receipt_header(receipt, file_path, block_size);
	build_receipt(receipt);
	set_receipt_hash(receipt);
}

void unpack_receipt(Receipt * receipt, int skip_integrity_flag)
{
	if (skip_integrity_flag)
		recover_original_file(receipt);
	else
		recover_original_file_i(receipt);
}

void store_receipt(Receipt * receipt)
{
	int fd;

	fd = open_create_receipt((unsigned char *)".receipt");
	write_receipt_header(fd, receipt);
	write_receipt_blocks(fd, receipt);
	close(fd);
}

void fetch_receipt(Receipt * receipt)
{
	int fd;

	fd = open_receipt((unsigned char *)".receipt");
	read_receipt_header(fd, receipt);
	read_receipt_blocks(fd, receipt);
	close(fd);
}
