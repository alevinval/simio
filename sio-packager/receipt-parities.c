#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "block-list.h"
#include "receipt.h"
#include "receipt-parities.h"

struct block *build_global_parity(struct block_list *blocks, int block_size)
{
	// Global parity atm
	int i;
	struct block_node *node;
	struct block *parity;
	struct block *block;
	unsigned char *block_buffer;
	unsigned char *parity_buffer;

	block_buffer = calloc(1, sizeof(unsigned char) * block_size);
	parity_buffer = calloc(1, sizeof(unsigned char) * block_size);

	// Load first block in parity buffer
	block = blocks->head->block;
	block->buffer = block_buffer;
	fetch_block_data(block);
	memcpy(parity_buffer, block->buffer, block->size);

	node = blocks->head->next;
	for (; node; node = node->next) {
		memset(block_buffer, 0, block_size);
		block = node->block;
		block->buffer = block_buffer;
		fetch_block_data(block);
		for (i = 0; i < block_size; i++)
			parity_buffer[i] = block->buffer[i] ^ parity_buffer[i];
	}
	parity = block_from_buffer(parity_buffer, block_size);

	free(block_buffer);
	return parity;
}

void store_receipt_parities(struct receipt *receipt)
{
	struct block *block;

	/* Global XOR parity */
	block = build_global_parity(receipt->blocks, receipt->block_size);
	block_list_add(receipt->parities, block);
	receipt->parities_num++;
	store_block(block);
	free(block->buffer);
}
