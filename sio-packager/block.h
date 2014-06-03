#ifndef BLOCK_H
#define BLOCK_H

#include "sha256.h"

struct block {
	unsigned char sha2[32];
	unsigned char name[SHA2_STRING];
	unsigned char *buffer;
	int size;
	char corrupted;
	void *next;
};

struct block_list {
	int size;
	struct block *head;
	struct block *tail;
};

int check_block_integrity(struct block *block);
void set_block_hash(struct block *block);

struct block *fetch_block(unsigned char *block_sha2, int block_size);
void fetch_block_data(struct block *block);
void store_block(struct block *block);

struct block *block_alloc();
struct block *block_from_buffer(unsigned char *buffer, int readed_bytes);

struct block_list *block_list_alloc();
void block_list_add(struct block_list *list, struct block *block);

#endif /** BLOCK_H */
