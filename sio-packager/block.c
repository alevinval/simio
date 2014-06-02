#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

void fetch_block(Block * block)
{
	int fd;
	long block_size;

	fd = open_block(block->name);
	block_size = file_size(fd);

	if (block_size != block->size) {
		die("file is larger than block size in block [%s]",
		    block->name);
	}
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
	return block;
}

void fill_block(Block * block, int file, int block_size, unsigned char *buffer)
{
	int readed_bytes;

	block->buffer = buffer;
	readed_bytes = fill_buffer(file, buffer, block_size);
	block->size = readed_bytes;
	set_block_hash(block);
	sha2hexf(block->name, block->sha2);
}

BlockList *block_list_alloc()
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
