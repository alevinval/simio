#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sha256.h"
#include "receipt.h"
#include "block.h"
#include "block-list.h"
#include "block-recover.h"
#include "dirnav.h"
#include "dir.h"
#include "util.h"

struct block_list *retrieve_uncorrupted_blocks(struct receipt *receipt)
{
	struct block_list *list;
	struct block_node *node;
	struct block *block;

	list = block_list_alloc();
	node = receipt->blocks->head;
	for (; node; node = node->next) {
		block = node->block;
		if (!block->corrupted)
			block_list_add(list, block);
	}

	return list;
}

struct block_list *retrieve_corrupted_blocks(struct receipt *receipt)
{
	struct block_list *list;
	struct block_node *node;
	struct block *block;

	list = block_list_alloc();
	node = receipt->blocks->head;
	for (; node; node = node->next) {
		block = node->block;
		if (block->corrupted)
			block_list_add(list, block);
	}

	return list;
}

struct block_list *retrieve_corrupted_parities(struct receipt *receipt)
{
	struct block_list *list;
	struct block_node *node;
	struct block *block;

	list = block_list_alloc();
	node = receipt->parities->head;
	for (; node; node = node->next) {
		block = node->block;
		if (block->corrupted)
			block_list_add(list, block);
	}

	return list;
}

int check_receipt_integrity(struct receipt *receipt)
{
	struct block_list *corrupted_blocks;
	struct block_node *node;
	struct block *block;
	unsigned char *buffer;
	int ret;

	corrupted_blocks = block_list_alloc();
	buffer = calloc(1, sizeof(unsigned char) * receipt->block_size);
	node = receipt->blocks->head;
	for (; node; node = node->next) {
		block = node->block;
		block->buffer = buffer;
		fetch_block_data(block);
		if (!check_block_integrity(block)) {
			block->corrupted = 1;
			block_list_add(corrupted_blocks, block);
		}
	}

	node = receipt->parities->head;
	for (; node; node = node->next) {
		block = node->block;
		block->buffer = buffer;
		fetch_block_data(block);
		if (!check_block_integrity(block)) {
			block->corrupted = 1;
			block_list_add(corrupted_blocks, block);
		}
	}

	if (corrupted_blocks->size > receipt->parities_num)
		die("cannot recover %i corrupted blocks with %i parities\n",
		    corrupted_blocks->size, receipt->parities_num);

	if (corrupted_blocks->size > 0)
		ret = 0;
	else
		ret = 1;

	free_block_list(corrupted_blocks);
	free(buffer);

	return ret;
}

void recover_original_file(struct receipt *receipt)
{
	int fd;
	unsigned char *block_buffer;
	struct block_node *node;
	struct block *block;

	fd = open_create_file(receipt->name);
	block_buffer = malloc(sizeof(unsigned char) * receipt->block_size);

	node = receipt->blocks->head;
	for (; node; node = node->next) {
		block = node->block;
		block->buffer = block_buffer;
		fetch_block_data(block);
		write(fd, block->buffer, block->size);
		block->buffer = NULL;
	}

	close(fd);
	free(block_buffer);
}

void set_receipt_hash(struct receipt *receipt)
{
	int i;
	unsigned char *hash_buffer =
	    malloc(sizeof(unsigned char) * 32 * receipt->size);
	struct block_node *node;

	i = 0;
	node = receipt->blocks->head;
	for (; node; node = node->next) {
		memcpy(&hash_buffer[i * 32], node->block->sha2, 32);
		i++;
	}

	sha256(receipt->sha2, hash_buffer, receipt->size * 32);
	free(hash_buffer);
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

void write_receipt_header(int fd, struct receipt *receipt)
{
	write(fd, receipt->sha2, 32);
	write(fd, receipt->name, FNAME_LEN);
	write(fd, &receipt->size, sizeof(int));
	write(fd, &receipt->parities_num, sizeof(int));
	write(fd, &receipt->block_size, sizeof(int));
	write(fd, &receipt->last_block_size, sizeof(int));
}

void write_receipt_blocks(int fd, struct receipt *receipt)
{
	struct block_node *node;

	node = receipt->blocks->head;
	for (; node; node = node->next) {
		write(fd, node->block->sha2, 32);
	}

	node = receipt->parities->head;
	for (; node; node = node->next) {
		write(fd, node->block->sha2, 32);
	}
}

void read_receipt_header(int fd, struct receipt *receipt)
{
	read(fd, receipt->sha2, 32);
	read(fd, receipt->name, FNAME_LEN);
	read(fd, &receipt->size, sizeof(int));
	read(fd, &receipt->parities_num, sizeof(int));
	read(fd, &receipt->block_size, sizeof(int));
	read(fd, &receipt->last_block_size, sizeof(int));
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
	receipt->blocks->tail->block->last = 1;

	for (i = 0; i < receipt->parities_num; i++) {
		read(fd, &block_sha2, 32);
		block = fetch_block(block_sha2, receipt->block_size);
		block_list_add(receipt->parities, block);
	}
}

void store_receipt_blocks(struct receipt *receipt)
{
	int i, fd, readed_bytes;
	struct block *block;
	unsigned char *buffer;

	fd = open_file(receipt->name);
	buffer = malloc(sizeof(unsigned char) * receipt->block_size);
	for (i = 0; i < receipt->size; i++) {
		readed_bytes = fill_buffer(fd, buffer, receipt->block_size);
		block = block_from_buffer(buffer, readed_bytes);
		block_list_add(receipt->blocks, block);
		store_block(block);
	}
	receipt->last_block_size = block->size;
	free(buffer);
}

void
create_receipt(struct receipt *receipt,
	       unsigned char *file_path, unsigned int block_size)
{
	set_receipt_header(receipt, file_path, block_size);
	store_receipt_blocks(receipt);
	store_receipt_parities(receipt);
	set_receipt_hash(receipt);
}

void unpack_receipt(struct receipt *receipt, int skip_integrity)
{
	if (skip_integrity) {
		recover_original_file(receipt);
	} else {
		if (!check_receipt_integrity(receipt)
		    && fix_corrupted_receipt(receipt)) {
			recover_original_file(receipt);
		} else {
			recover_original_file(receipt);
		}
	}
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

void free_receipt(struct receipt *receipt)
{
	struct block_node *node;

	node = receipt->blocks->head;
	for (; node; node = node->next) {
		free(node->block);
	}

	node = receipt->parities->head;
	for (; node; node = node->next) {
		free(node->block);
	}

	free_block_list(receipt->blocks);
	free_block_list(receipt->parities);
	free(receipt);
}
