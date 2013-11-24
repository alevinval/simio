#ifndef RECEIPT_H
#define RECEIPT_H

#include "file_block.h"

typedef struct {
	unsigned char hash[32];
	unsigned char name[256];
	int size;
	int block_size;
	FileBlock * blocks;
} Receipt;

#endif /** RECEIPT_H */