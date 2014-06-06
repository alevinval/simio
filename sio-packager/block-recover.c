#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "receipt.h"
#include "block-list.h"
#include "block-recover.h"
#include "receipt-parities.h"

struct block *recover_block_from_parity(struct block_list *blocks,
					struct block *parity, int block_size)
{
	int i;
	unsigned char *missing_buffer;
	unsigned char *block_buffer;
	struct block_node *node;
	struct block *block;
	struct block *missing_block;

	block_buffer = calloc(1, sizeof(unsigned char) * parity->size);
	missing_buffer = calloc(1, sizeof(unsigned char) * parity->size);

	parity->buffer = malloc(sizeof(unsigned char) * parity->size);
	fetch_block_data(parity);
	memcpy(missing_buffer, parity->buffer, parity->size);

	node = blocks->head;
	for (; node; node = node->next) {
		memset(block_buffer, 0, parity->size);
		block = node->block;
		block->buffer = block_buffer;
		fetch_block_data(block);
		for (i = 0; i < parity->size; i++) {
			missing_buffer[i] =
			    block->buffer[i] ^ missing_buffer[i];
		}
	}
	free(block_buffer);
	missing_block = block_from_buffer(missing_buffer, block_size);
	return missing_block;
}

void fix_one_corrupted_block(struct block_list *blocks, struct block *parity,
			    int block_size)
{
	struct block *recovered_block;

	recovered_block = recover_block_from_parity(blocks, parity, block_size);
	store_block(recovered_block);

	free(recovered_block->buffer);
	free(recovered_block);
	free(parity->buffer);
}

int fix_corrupted_receipt(struct receipt *receipt)
{
	struct block_list *sane_blocks;
	struct block_list *corrupted_blocks;
	struct block_list *corrupted_parities;

	sane_blocks = retrieve_uncorrupted_blocks(receipt);
	corrupted_blocks = retrieve_corrupted_blocks(receipt);
	corrupted_parities = retrieve_corrupted_parities(receipt);

	if (corrupted_parities->size == 1) {
		printf("fixing global parity\n");
		build_global_parity(sane_blocks, receipt->block_size);
	}

	if (corrupted_blocks->size == 1) {
		printf("fixing corrupted block\n");
		if (corrupted_blocks->tail->block->last)
			fix_one_corrupted_block(sane_blocks,
						receipt->parities->head->block,
						receipt->last_block_size);
		else
			fix_one_corrupted_block(sane_blocks,
						receipt->parities->head->block,
						receipt->block_size);
	}

	free_block_list(sane_blocks);
	free_block_list(corrupted_blocks);
	free_block_list(corrupted_parities);
	return 1;
}
