#include <stdlib.h>
#include <string.h>

#include "receipt.h"
#include "block.h"
#include "block-recover.h"

struct block *recover_block_from_parity(struct block_list *blocks,
					struct block *parity)
{
	int i;
	unsigned char *missing_buffer;
	unsigned char *block_buffer;
	struct block_node *node;
	struct block *block;
	struct block *missing_block;

	block_buffer = calloc(1, sizeof(unsigned char) * parity->size);
	missing_buffer = calloc(1, sizeof(unsigned char) * parity->size);

	memcpy(missing_buffer, parity->buffer, parity->size);
	for (node = blocks->head; node; node = node->next) {
		block = node->block;
		block->buffer = block_buffer;
		fetch_block_data(block);
		for (i = 0; i < block->size; i++) {
			missing_buffer[i] =
			    block->buffer[i] ^ missing_buffer[i];
		}
	}
	missing_block = block_from_buffer(missing_buffer, parity->size);
	free(block_buffer);
	return missing_block;
}

struct block_list *retrieve_uncorrupted_blocks(struct receipt *receipt)
{
	struct block_list *list;
	struct block_node *node;
	struct block *block;

	list = block_list_alloc();
	for (node = receipt->blocks->head; node; node = node->next) {
		block = node->block;
		if (!block->corrupted)
			block_list_add(list, block);
	}

	return list;
}

struct block_list *retrieve_corrupted_blocks(struct receipt *receipt)
{
	struct block_list *list;
	struct block_node *node;
	struct block *block;

	list = block_list_alloc();
	node = receipt->blocks->head;
	for (; node; node = node->next) {
		block = node->block;
		if (block->corrupted)
			block_list_add(list, block);
	}

	return list;
}

struct block_list *retrieve_corrupted_parities(struct receipt *receipt)
{
	struct block_list *list;
	struct block_node *node;
	struct block *block;

	list = block_list_alloc();
	node = receipt->parities->head;
	for (; node; node = node->next) {
		block = node->block;
		if (block->corrupted)
			block_list_add(list, block);
	}

	return list;
}

int fix_one_corrupted_block(struct block_list *blocks, struct block *parity)
{
	struct block *recovered_block;

	parity->buffer = malloc(sizeof(unsigned char) * parity->size);
	fetch_block_data(parity);

	recovered_block = recover_block_from_parity(blocks, parity);
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
	struct block_node *node;

	sane_blocks = retrieve_uncorrupted_blocks(receipt);
	corrupted_blocks = retrieve_corrupted_blocks(receipt);
	corrupted_parities = retrieve_corrupted_parities(receipt);

	if (corrupted_parities->size == 1) {
		build_global_parity(receipt);
	}

	if (corrupted_blocks->size == 1) {
		fix_one_corrupted_block(sane_blocks,
					receipt->parities->head->block);
	}

	free_block_list(sane_blocks);
	free_block_list(corrupted_blocks);
	free_block_list(corrupted_parities);
	return 1;
}
