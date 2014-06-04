#include <stdlib.h>
#include <string.h>

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
