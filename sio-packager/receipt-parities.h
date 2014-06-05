#ifndef RECEIPT_PARITIES_H
#define RECEIPT_PARITIES_H

struct block *build_global_parity(struct block_list *blocks, int block_size);

void store_receipt_parities(struct receipt *receipt);

#endif /** RECEIPT_PARITIES_H */
