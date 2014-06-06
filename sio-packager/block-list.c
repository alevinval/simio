#include <stdlib.h>

#include "util.h"
#include "block-list.h"

struct block_list *block_list_alloc()
{
	return calloc(1, sizeof(struct block_list));
}

void free_block_list(struct block_list *list)
{
	struct block_node *node;
	struct block_node *tmp;

	node = list->head;
	while (node) {
		tmp = node->next;
		free(node);
		node = tmp;
	}

	free(list);
}

struct block_node *block_node(struct block *block)
{
	struct block_node *node;

	node = calloc(1, sizeof(struct block_node));
	node->block = block;
	return node;
}

void block_list_add(struct block_list *list, struct block *block)
{
	if (!list)
		die("cannot add block to unallocated block_list\n");

	if (!block)
		die("cannot add non-existant block to block_list\n");

	struct block_node *node = block_node(block);

	if (!list->size) {
		list->head = node;
		list->tail = node;
	} else {
		list->tail->next = node;
		list->tail = node;
	}
	list->size++;
}

struct block_list *copy_block_list(struct block_list *list)
{
	struct block_list *copy;
	struct block_node *node;

	copy = block_list_alloc();

	node = list->head;
	while (node) {
		block_list_add(copy, node->block);
		node = node->next;
	}
	return copy;
}
