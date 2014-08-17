#ifndef BLOCK_H
#define BLOCK_H

#include "sha256.h"

class Block {
private:
	unsigned char sha2[32];
	unsigned char name[SHA2_STRING];
	unsigned char *buffer;
	bool corrupted;
	bool last;
	int size;	

	void init();
	void updateHash();	
	void setName(unsigned char *name);
public:		
	Block();	
	~Block();
	void from_buffer(unsigned char *buffer, int size);
	void from_file(unsigned char *block_sha2, int block_size);
	void fetch(unsigned char *buffer);

	void set_buffer(unsigned char *buffer);
	unsigned char *get_buffer();

	bool isCorrupted();
	bool isLast();
	void setLast();
	int getSize();
	void setSize(int size);	
	void setCorrupted();

	unsigned char *getSha2();
	unsigned char *getName();	
	void store();
	
	bool checkIntegrity();
};

#endif //BLOCK_H
