#ifndef BLOCK_LIST_H
#define BLOCK_LIST_H

#include "block.h"

struct block_node {
	struct block *block;
	struct block_node *next;
};

struct block_list {
	int size;
	struct block_node *head;
	struct block_node *tail;
};

struct block_list *block_list_alloc();

void free_block_list(struct block_list *list);
void block_list_add(struct block_list *list, struct block *block);

#endif /** BLOCK_LIST_H */
