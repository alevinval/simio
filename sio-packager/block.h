#ifndef BLOCK_H
#define BLOCK_H

#include "sha256.h"

typedef struct {
	unsigned char sha2[32];
	unsigned char name[SHA256_STRING];
	unsigned char *buffer;
	int size;
	void *next;
} Block;

typedef struct {
	int size;
	Block *head;
	Block *tail;
} BlockList;

void set_block_hash(Block * block);
int check_block_integrity(Block * block);

Block *block_alloc();

void fetch_block(Block * block);	//unsigned char *block_name, unsigned char *buffer);
void store_block(Block * block);
void fill_block(Block * block, int file, int block_size, unsigned char *buffer);

BlockList *block_list_alloc();
void block_list_add(BlockList * list, Block * block);

#endif /** BLOCK_H */
