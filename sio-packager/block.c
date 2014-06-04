#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "util.h"
#include "sha256.h"
#include "block.h"
#include "dirnav.h"

int check_block_integrity(struct block *block)
{
	unsigned char match_sha[32];
	sha256(match_sha, block->buffer, block->size);
	return !memcmp(match_sha, block->sha2, 32);
}

void set_block_hash(struct block *block)
{
	if (!block->buffer)
		die("cannot set block hash if there is no data-buffer\n");

	sha256(block->sha2, block->buffer, block->size);
	sha2hexf(block->name, block->sha2);
}

struct block *block_alloc()
{
	return calloc(1, sizeof(struct block));
}

struct block *fetch_block(unsigned char *block_sha2, int block_size)
{
	struct block *block;

	block = block_alloc();
	memcpy(block->sha2, block_sha2, 32);
	sha2hexf(block->name, block->sha2);
	block->size = block_size;
	return block;
}

void fetch_block_data(struct block *block)
{
	int fd;
	long block_size;

	if (!block->buffer)
		die("cannot fetch block without allocating a buffer\n");

	fd = open_block(block->name);
	block_size = file_size(fd);

	if (block_size > block->size) {
		die("fetching block bigger than block_size on block [%s]\n",
		    block->name);
	}

	block->size = block_size;
	read(fd, block->buffer, block_size);
	close(fd);
}

void store_block(struct block *block)
{
	int fd;

	if (!block->buffer)
		die("cannot store a block without data-buffer\n");

	fd = open_create_block(block->name);
	write(fd, block->buffer, block->size);
	close(fd);
}

struct block *block_from_buffer(unsigned char *buffer, int readed_bytes)
{
	struct block *block;

	block = block_alloc();
	block->buffer = buffer;
	block->size = readed_bytes;
	set_block_hash(block);
	return block;
}

void free_block(struct block *block)
{
	free(block);
}

struct block_list *block_list_alloc()
{
	return calloc(1, sizeof(struct block_list));
}

void free_block_list(struct block_list *list)
{
	struct block_node *node;
	struct block_node *tmp;

	node = list->head;
	while (node) {
		tmp = node->next;
		free(node);
		node = tmp;
	}
}

struct block_node *block_node(struct block *block)
{
	struct block_node *node;

	node = calloc(1, sizeof(struct block_node));
	node->block = block;
	return node;
}

void block_list_add(struct block_list *list, struct block *block)
{
	if (!list)
		die("cannot add block to unallocated block_list\n");

	if (!block)
		die("cannot add non-existant block to block_list\n");

	struct block_node *node = block_node(block);

	if (!list->size) {
		list->head = node;
		list->tail = node;
	} else {
		list->tail->next = node;
		list->tail = node;
	}
	list->size++;
}

struct block_list *copy_block_list(struct block_list *list)
{
	struct block_list *copy;
	struct block_node *node;

	copy = block_list_alloc();

	node = list->head;
	while (node) {
		block_list_add(copy, node->block);
		node = node->next;
	}
	return copy;
}
