#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "util.h"
#include "sha256.h"
#include "dirnav.h"
#include "block.h"

void set_block_hash(struct block *block)
{
	if (!block)
		die("set_block_hash: unallocated block\n");
	if (!block->buffer)
		die("set_block_hash: unallocated buffer\n");

	sha256(block->sha2, block->buffer, block->size);
	sha2hexf(block->name, block->sha2);
}

struct block *block_alloc()
{
	return calloc(1, sizeof(struct block));
}

struct block *block_from_buffer(unsigned char *buffer, int readed_bytes)
{
	struct block *block;

	if (!buffer)
		die("block_from_buffer: unallocated buffer\n");

	block = block_alloc();
	block->buffer = buffer;
	block->size = readed_bytes;
	set_block_hash(block);
	return block;
}

struct block *fetch_block(unsigned char *block_sha2, int block_size)
{
	struct block *block;

	if (!block_sha2)
		die("fetch_block: unallocated sha2\n");

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

	if (!block)
		die("fetch_block_data: unallocated block\n");
	if (!block->buffer)
		die("fetch_block_data: unallocated buffer\n");

	fd = open_block(block->name);
	block_size = file_size(fd);
	if (block_size > block->size) {
		block->corrupted = 1;
	} else {
		block->size = block_size;
	}
	read(fd, block->buffer, block_size);
	close(fd);
}

void store_block(struct block *block)
{
	int fd;

	if (!block)
		die("store_block: unallocated block\n");
	if (!block->buffer)
		die("store_block: unallocated buffer\n");

	fd = open_create_block(block->name);
	write(fd, block->buffer, block->size);
	close(fd);
}

int verify_block_integrity(struct block *block)
{
	unsigned char match_sha[32];

	if (!block)
		die("verify_block_integrity: unallocated block\n");
	if (!block->buffer)
		die("verify_block_integrity: unallocated buffer\n");

	sha256(match_sha, block->buffer, block->size);
	return !memcmp(match_sha, block->sha2, 32);
}
