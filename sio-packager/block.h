#ifndef BLOCK_H
#define BLOCK_H

typedef struct {
	unsigned char hash[32];
	int size;
	unsigned char * data;	
} Block;

void dump_block (Block block);

void write_block (Block *block, unsigned char *data);
void buffer2block (Block *block, unsigned char *buffer, int len);

int check_block_integrity (Block *block, unsigned char *buffer, int len);

int read_block_to_buffer (unsigned char *block_name, unsigned char *buffer);

#endif /** BLOCK_H */
