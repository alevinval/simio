#ifndef BLOCK_H
#define BLOCK_H

#include "sha256.h"

struct block {
	unsigned char sha2[32];
	unsigned char name[SHA2_STRING];
	unsigned char *buffer;
	char corrupted;
	char last;
	int size;
};

struct block *block_alloc();
struct block *block_from_buffer(unsigned char *buffer, int readed_bytes);
struct block *fetch_block(unsigned char *block_sha2, int block_size);

void fetch_block_data(struct block *block);
void store_block(struct block *block);

int verify_block_integrity(struct block *block);

#endif /** BLOCK_H */
