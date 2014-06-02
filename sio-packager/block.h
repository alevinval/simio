#ifndef BLOCK_H
#define BLOCK_H

typedef struct {
	unsigned char hash[32];
	unsigned char *buffer;
	int size;
	void *next;
} Block;

typedef struct {
	int size;
	Block *head;
	Block *tail;
} BlockList;

BlockList *block_list_new();

void block_list_add(BlockList * list, Block * block);

void block_store(Block * block);

void set_block_hash(Block * block);

int block_read(unsigned char *block_name, unsigned char *buffer);

int check_block_integrity(Block * block, unsigned char *buffer, int len);

void *block_fill(Block * block, int file, int block_size,
		 unsigned char *buffer);

Block *block_new();

#endif /** BLOCK_H */
