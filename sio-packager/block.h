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
public:		
	Block();	
	~Block();
	void from_buffer(unsigned char *buffer, int size);
	void from_file(unsigned char *block_sha2, int block_size);
	void fetch(unsigned char *buffer);

	void set_buffer(unsigned char *buffer);
	unsigned char *get_buffer();

	bool is_corrupted();
	bool is_last();
	void set_last();
	int get_size();
	void set_size(int size);
	void set_corrupted();

	unsigned char *get_sha2();
	unsigned char *get_name();
	void store();
	
	bool check_integrity();
};

#endif //BLOCK_H
