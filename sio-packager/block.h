#ifndef BLOCK_H
#define BLOCK_H

#include "sha256.h"

class Block {
private:
	unsigned char sha2[32];
	unsigned char name[SHA2_STRING];	
	bool corrupted;
	bool last;
	int size;	

	void init();
	void updateHash();	
	void setName(unsigned char *name);
public:	
	unsigned char *buffer;

	Block();	
	~Block();
	void from_buffer(unsigned char *buffer, int size);
	void from_file(unsigned char *block_sha2, int block_size);
	bool isCorrupted();
	bool isLast();
	void setLast();
	int getSize();
	void setSize(int size);
	void setBuffer(unsigned char *buffer);
	void setCorrupted();
	unsigned char *getBuffer();
	unsigned char *getSha2();
	unsigned char *getName();	
	void store();
	void fetchBlockData();
	bool checkIntegrity();
};

#endif //BLOCK_H
