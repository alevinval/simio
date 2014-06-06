#include <stdlib.h>

#include "util.h"
#include "block-list.h"

struct block_node *block_node_from_block(struct block *block)
{
	struct block_node *node;

	if (!block)
		die("block_node_from_block: unallocated block\n");

	node = calloc(1, sizeof(struct block_node));
	node->block = block;
	return node;
}

struct block_list *block_list_alloc()
{
	return calloc(1, sizeof(struct block_list));
}

void free_block_list(struct block_list *list)
{
	struct block_node *node;
	struct block_node *next;

	if (!list)
		die("free_block_list: unallocated list\n");

	node = list->head;
	while (node) {
		next = node->next;
		free(node);
		node = next;
	}
	free(list);
}

void block_list_add(struct block_list *list, struct block *block)
{
	struct block_node *node;

	if (!block)
		die("block_list_add: unallocated list\n");

	if (!block)
		die("block_list_add: unallocated block\n");

	node = block_node_from_block(block);
	if (!list->size) {
		list->head = node;
		list->tail = node;
	} else {
		list->tail->next = node;
		list->tail = node;
	}
	list->size++;
}
