#ifndef RECEIPT_H
#define RECEIPT_H

#include "block.h"

typedef struct {
	unsigned char hash[32];
	unsigned char name[256];
	int size;
	int block_size;
	Block * blocks;
} Receipt;

void dump_receipt (Receipt *receipt);

void set_receipt_hash (Receipt *receipt);

void create_receipt (Receipt *receipt, unsigned char *path, unsigned int blk_size);

void write_receipt_header (int fd, Receipt *receipt);
void write_receipt_blocks (int fd, Receipt *receipt);
void read_receipt_header (int fd, Receipt *receipt);
void read_receipt_blocks (int fd, Receipt *receipt);

void recover_receipt (int fd, Receipt *receipt, int skip_integrity_flag);
void store_receipt (Receipt *receipt);

#endif /** RECEIPT_H */