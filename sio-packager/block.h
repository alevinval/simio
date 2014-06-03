#ifndef BLOCK_H
#define BLOCK_H

#include "sha256.h"

typedef struct {
	unsigned char sha2[32];
	unsigned char name[SHA256_STRING];
	unsigned char *buffer;
	int size;
	char corrupted;
	void *next;
} Block;

typedef struct {
	int size;
	Block *head;
	Block *tail;
} BlockList;

int check_block_integrity(Block * block);
void set_block_hash(Block *block);

Block *fetch_block(unsigned char *block_sha2, int block_size);
void fetch_block_data(Block * block);
void store_block(Block *block);

Block *block_alloc();
Block *block_from_buffer(unsigned char *buffer, int readed_bytes);

BlockList *block_list_alloc();
void block_list_add(BlockList *list, Block *block);

#endif /** BLOCK_H */
