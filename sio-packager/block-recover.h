#ifndef BLOCK_RECOVER_H
#define BLOCK_RECOVER_H

#include "block-list.h"

struct block *recover_block_from_parity(struct block_list *blocks,
					struct block *parity, int block_size);

#endif /** BLOCK_RECOVER_H */
