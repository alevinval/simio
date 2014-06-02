#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"
#include "sha256.h"
#include "block.h"
#include "dirnav.h"

int check_block_integrity(Block * block, unsigned char *buffer, int len)
{
	int i;
	unsigned char match_sha[32];

	sha256(match_sha, buffer, len);
	for (i = 0; i < 32; i++)
		if (match_sha[i] != block->hash[i])
			return -1;

	return 1;
}

void set_block_hash(Block * block)
{
	sha256(block->hash, block->buffer, block->size);
}

int block_read(unsigned char *block_name, unsigned char *buffer)
{
	int fd;
	long block_size;

	fd = open_block(block_name);
	block_size = file_size(fd);
	read(fd, buffer, block_size);
	close(fd);

	return block_size;
}

void block_store(Block * block)
{
	int fd;
	unsigned char block_name[SHA256_STRING];

	sha2hexf(block_name, block->hash);
	fd = open_create_block(block_name);
	write(fd, block->buffer, block->size);
	block->buffer = NULL;
	close(fd);
}

Block *block_new()
{
	Block *block = malloc(sizeof(Block));
	block->size = 0;
	block->next = NULL;
	block->buffer = NULL;
	return block;
}

void *block_fill(Block * block, int file, int block_size, unsigned char *buffer)
{
	block->buffer = buffer;
	block->size = block_size;
	fill_buffer(file, buffer, block_size);
	set_block_hash(block);
}

BlockList *block_list_new()
{
	BlockList *list;
	list = malloc(sizeof(BlockList));
	list->size = 0;
	return list;
}

void block_list_add(BlockList * list, Block * block)
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
