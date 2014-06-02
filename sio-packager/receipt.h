#ifndef RECEIPT_H
#define RECEIPT_H

#include "block.h"
#include "dir.h"
#include "sha256.h"

typedef struct {
	unsigned char sha2[32];
	unsigned char name[FNAME_LEN];
	int size;
	int block_size;
	BlockList *blocks;
} Receipt;

void
create_receipt(Receipt * receipt, unsigned char *path, unsigned int blk_size);

void unpack_receipt(Receipt * receipt, int skip_integrity_flag);
void store_receipt(Receipt * receipt);
void fetch_receipt(Receipt * receipt);

#endif /** RECEIPT_H */
