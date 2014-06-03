#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "util.h"
#include "sha256.h"
#include "block.h"
#include "dirnav.h"

int check_block_integrity(Block * block)
{
	int i;
	unsigned char match_sha[32];

	sha256(match_sha, block->buffer, block->size);
	for (i = 0; i < 32; i++)
		if (match_sha[i] != block->sha2[i])
			return -1;

	return 1;
}

void set_block_hash(Block * block)
{
	sha256(block->sha2, block->buffer, block->size);
}

Block *fetch_block(unsigned char *block_sha2, int block_size)
{
	Block *block;

	block = block_alloc();
	memcpy(block->sha2, block_sha2, 32);
	sha2hexf(block->name, block->sha2);
	block->size = block_size;
	return block;
}

void fetch_block_data(Block * block)
{
	int fd;
	long block_size;

	if (block->buffer == NULL)
		die("cannot fetch block without allocating a buffer\n");

	fd = open_block(block->name);
	block_size = file_size(fd);

	if (block_size > block->size) {
		die("fetching block bigger than block_size on block [%s]",
		    block->name);
	}

	block->size = block_size;
	read(fd, block->buffer, block_size);
	close(fd);
}

void store_block(Block * block)
{
	int fd;

	fd = open_create_block(block->name);
	write(fd, block->buffer, block->size);
	block->buffer = NULL;
	close(fd);
}

Block *block_alloc()
{
	Block *block = malloc(sizeof(Block));
	block->size = 0;
	block->next = NULL;
	block->buffer = NULL;
	block->corrupted = 0;
	return block;
}

Block *block_from_buffer(unsigned char *buffer, int readed_bytes)
{
	Block *block;

	block = block_alloc();
	block->buffer = buffer;
	block->size = readed_bytes;
	set_block_hash(block);
	sha2hexf(block->name, block->sha2);
	return block;
}

BlockList *block_list_alloc()
{
	BlockList *list;
	list = malloc(sizeof(BlockList));
	list->size = 0;
	return list;
}

void block_list_add(BlockList * list, Block *block)
{
	if (list->size == 0) {
		list->head = block;
		list->tail = block;
		block->next = NULL;
	} else {
		list->tail->next = block;
		list->tail = block;
		block->next = NULL;
	}
	list->size++;
}