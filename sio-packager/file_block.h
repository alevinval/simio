#ifndef FILEBLOCK_H
#define FILEBLOCK_H

typedef struct {
	unsigned char hash[32];
	int size;
	unsigned char * data;	
} FileBlock;

void dump_block( FileBlock block );

#endif /** FILEBLOCK_H */
