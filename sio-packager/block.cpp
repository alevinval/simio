#include <string.h>

#include "block.h"
#include "util.h"
#include "dirnav.h"

Block::Block()
{
	init();
}

void Block::from_buffer(unsigned char *buffer, int block_size)
{	
	size = block_size;
	this->buffer = buffer;
	updateHash();
}

/*	This method is _lazy_, calling 'from_file' just
	sets the metadata of the block.
	The content of the block must be explicitly retrieved
	with fetch();
*/
void Block::from_file(unsigned char *block_sha2, int block_size)
{
	if (!block_sha2)
		die("fetch_block: unallocated sha2\n");

	memcpy(sha2, block_sha2, 32);
	sha2hexf(name, sha2);
	size = block_size;
}

void Block::init()
{
	memset(&sha2, 0, 32);
	memset(&name, 0, SHA2_STRING);
	corrupted = false;
	last = false;
	size = 0;
}

Block::~Block()
{
	if (buffer) free(buffer);
}

void
Block::fetch(unsigned char *buffer)
{
	int fd;
	int block_size;

	if (!buffer)
		die("fetch_block_data: unallocated buffer\n");

	set_buffer(buffer);

	fd = open_block(name);
	block_size = file_size(fd);
	if (block_size != size) {
		set_corrupted();
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
Block::set_buffer(unsigned char *buffer)
{
	this->buffer = buffer;
}

unsigned char *
Block::get_buffer()
{
	return buffer;
}

unsigned char *
Block::get_name()
{
	return name;
}

unsigned char *
Block::get_sha2()
{
	return sha2;
}

void
Block::set_size(int size) {
	this->size = size;
}

int
Block::get_size()
{
	return size;
}

void
Block::set_corrupted() {
	corrupted = true;
}

bool
Block::is_corrupted()
{
	return corrupted;
}

void Block::set_last() {
	last = true;
}

bool
Block::is_last()
{
	return last;
}

bool
Block::check_integrity()
{
	unsigned char match_sha[32];

	if (!buffer)
		die("check_integrity: unallocated buffer\n");	
	
	sha256(match_sha, buffer, size);		
	return !memcmp(match_sha, sha2, 32);
}