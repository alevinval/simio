#include <string.h>

#include "block.h"
#include "util.h"
#include "dirnav.h"

Block::Block()
{
	corrupted = false;
	last = false;		
	size = 0;
}

Block::Block(unsigned char *buffer, int block_size)
{
	corrupted = false;	
	size = block_size;
	last = false;
	this->buffer = buffer;
	updateHash();
}

Block::~Block()
{
	if (buffer) free(buffer);
}

void 
Block::retrieve(unsigned char *block_sha2, int block_size)
{
	if (!block_sha2)
		die("fetch_block: unallocated sha2\n");
	
	memcpy(sha2, block_sha2, 32);
	sha2hexf(name,sha2);
	size = block_size;
}

void
Block::fetchBlockData()
{
	int fd;
	int block_size;

	if (!buffer)
		die("fetch_block_data: unallocated buffer\n");

	fd = open_block(name);
	block_size = file_size(fd);
	if (block_size > size) {
		setCorrupted();
	} else {
		size = block_size;
	}
	read(fd, buffer, size);	
	close(fd);
}


void
Block::store()
{
	int fd;

	if (!buffer)
		die("store_block: unallocated buffer\n");

	fd = open_create_block(name);
	write(fd, buffer, size);
	close(fd);
}

void
Block::updateHash()
{
	sha256(sha2, buffer, size);
	sha2hexf(name, sha2);
}

void
Block::setBuffer(unsigned char *buffer)
{
	this->buffer = buffer;
}

unsigned char *
Block::getBuffer()
{
	return buffer;
}

unsigned char *
Block::getName()
{
	return name;
}

unsigned char *
Block::getSha2()
{
	return sha2;
}

void
Block::setSize(int size) {
	this->size = size;
}

int
Block::getSize()
{
	return size;
}

void
Block::setCorrupted() {
	corrupted = true;
}

bool
Block::isCorrupted()
{
	return corrupted;
}

void Block::setLast() {
	last = true;
}

bool
Block::isLast()
{
	return last;
}

bool
Block::checkIntegrity()
{
	unsigned char match_sha[32];

	if (!buffer)
		die("check_integrity: unallocated buffer\n");	
	
	sha256(match_sha, buffer, size);		
	return !memcmp(match_sha, sha2, 32);
}