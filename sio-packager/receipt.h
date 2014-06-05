#ifndef RECEIPT_H
#define RECEIPT_H

#include "block.h"
#include "dir.h"
#include "sha256.h"

struct receipt {
	unsigned char sha2[32];
	unsigned char name[FNAME_LEN];
	int size;
	int parities_num;
	int block_size;
	int last_block_size;
	struct block_list *blocks;
	struct block_list *parities;
};

void
create_receipt(struct receipt *receipt, unsigned char *path,
	       unsigned int blk_size);

void unpack_receipt(struct receipt *receipt, int skip_integrity);
void store_receipt(struct receipt *receipt);
void fetch_receipt(struct receipt *receipt);
void free_receipt_blocks(struct receipt *receipt);

struct block_list *retrieve_uncorrupted_blocks(struct receipt *receipt);
struct block_list *retrieve_corrupted_blocks(struct receipt *receipt);
struct block_list *retrieve_corrupted_parities(struct receipt *receipt);

#endif /** RECEIPT_H */
