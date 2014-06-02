#ifndef BLOCK_H
#define BLOCK_H

typedef struct {
	unsigned char hash[32];
	unsigned char *data;
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

void block_store(Block * block, unsigned char *data);

void buffer_to_block(Block * block, unsigned char *buffer);

int block_read(unsigned char *block_name, unsigned char *buffer);

int check_block_integrity(Block * block, unsigned char *buffer, int len);

Block *block_create(int fd, int block_size, unsigned char *buffer);

#endif /** BLOCK_H */
