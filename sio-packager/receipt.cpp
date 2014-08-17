#include <vector>
#include <stdlib.h>

#include "receipt.h"
#include "dirnav.h"
#include "util.h"


Receipt::Receipt(unsigned char *receipt_name) {
	int fd;

	blocks = new std::vector<Block*>();
	parities = new std::vector<Block*>();

	fd = open_receipt((unsigned char *)receipt_name);
	fetchReceipt(fd);
	fetchBlocks(fd);
	close(fd);
}

Receipt::Receipt(unsigned char *file_path, int block_size)
{
	setHeader(file_path, block_size);
	blocks = new std::vector<Block*>();
	parities = new std::vector<Block*>();
}

Receipt::~Receipt() {
	delete blocks;
	delete parities;
}

void
Receipt::setHeader(unsigned char *file_path, int block_size)
{
	int fd, extra;
	fd = open_file(file_path);

	strcpy((char *)name, (char *)file_path);
	extra = file_size(fd) % block_size;
	if (extra > 0)
		size = file_size(fd) / block_size + 1;
	else
		size = file_size(fd) / block_size;
	parities_num = 0;
	this->block_size = block_size;
}

void
Receipt::setHash()
{
	int i;
	unsigned char *hash_buffer = (unsigned char *)malloc(sizeof(unsigned char)* 32 * size);
	std::vector<Block*>::iterator block;

	i = 0;
	for (block = blocks->begin(); block != blocks->end(); block++) {
		memcpy(&hash_buffer[i * 32], (*block)->getSha2(), 32);
		i++;
	}

	sha256(sha2, hash_buffer, size * 32);
	free(hash_buffer);
}

void
Receipt::storeBlocks()
{
	int i, fd, readed_bytes;
	Block *block;
	unsigned char *buffer;

	fd = open_file(name);
	buffer = (unsigned char *)malloc(sizeof(unsigned char)*block_size);
	for (i = 0; i < size; i++) {
		readed_bytes = fill_buffer(fd, buffer, block_size);
		block = new Block();
		block->from_buffer(buffer, readed_bytes);
		blocks->push_back(block);
		block->store();
	}
	last_block_size = block->getSize();
	free(buffer);
}

void
Receipt::storeReceipt()
{
	int fd;
	std::vector<Block*>::iterator block;

	fd = open_create_receipt((unsigned char *)".receipt");

	write(fd, &sha2, 32);
	write(fd, &name, FNAME_LEN);
	write(fd, &size, sizeof(int));
	write(fd, &parities_num, sizeof(int));
	write(fd, &block_size, sizeof(int));
	write(fd, &last_block_size, sizeof(int));

	block = blocks->begin();
	for (; block != blocks->end(); block++) {
		write(fd, (*block)->getSha2(), 32);
	}

	block = parities->begin();
	for (; block != parities->end(); block++) {
		write(fd, (*block)->getSha2(), 32);
	}

	close(fd);
}

void
Receipt::fetchReceipt(int fd)
{
	read(fd, &sha2, 32);
	read(fd, &name, FNAME_LEN);
	read(fd, &size, sizeof(int));
	read(fd, &parities_num, sizeof(int));
	read(fd, &block_size, sizeof(int));
	read(fd, &last_block_size, sizeof(int));
}

void
Receipt::fetchBlocks(int fd)
{
	int i;
	unsigned char block_sha2[32];
	Block *block;

	for (i = 0; i < size; i++) {
		read(fd, &block_sha2, 32);
		block = new Block();
		block->from_file(block_sha2, block_size);
		blocks->push_back(block);
	}
	blocks->back()->setLast();

	for (i = 0; i < parities_num; i++) {
		read(fd, &block_sha2, 32);
		block = new Block();
		block->from_file(block_sha2, block_size);
		parities->push_back(block);
	}

}

void
Receipt::pack()
{
	storeBlocks();
	buildParities();
	storeReceipt();
	setHash();
}

Block *
Receipt::buildGlobalParity()
{
	int i;
	unsigned char *block_buffer;
	unsigned char *parity_buffer;

	std::vector<Block*>::iterator block;
	Block *parity;

	block_buffer = (unsigned char *)calloc(1, sizeof(unsigned char)* block_size);
	parity_buffer = (unsigned char *)calloc(1, sizeof(unsigned char)* block_size);

	/** Build Global Parity */

	block = blocks->begin();
	(*block)->set_buffer(block_buffer);
	(*block)->fetch();

	memcpy(parity_buffer, (*block)->get_buffer(), (*block)->getSize());
	block++;
	for (; block != blocks->end(); block++) {
		memset(block_buffer, 0, block_size);
		(*block)->set_buffer(block_buffer);
		(*block)->fetch();
		for (i = 0; i < block_size; i++)
			parity_buffer[i] = block_buffer[i] ^ parity_buffer[i];
	}
	parity = new Block();
	parity->from_buffer(parity_buffer, block_size);
	free(block_buffer);
	return parity;
}

void Receipt::buildParities()
{
	Block *block = buildGlobalParity();
	parities->push_back(block);
	parities_num++;
	block->store();
	free(block->get_buffer());
}


Block *Receipt::recoverBlockFromParity(std::vector<Block *> *blocks, Block *parity, int block_size)
{
	int i;
	unsigned char *missing_buffer;
	unsigned char *block_buffer;
	unsigned char *parity_buffer;

	std::vector<Block*>::iterator block;
	Block *missing_block;

	block_buffer = (unsigned char *)calloc(1, sizeof(unsigned char)* parity->getSize());
	missing_buffer = (unsigned char *)calloc(1, sizeof(unsigned char)* parity->getSize());
	parity_buffer = (unsigned char *)malloc(sizeof(unsigned char)* parity->getSize());
	parity->set_buffer(parity_buffer);
	parity->fetch();
	memcpy(missing_buffer, parity->get_buffer(), parity->getSize());

	block = blocks->begin();
	for (; block != blocks->end(); block++) {
		memset(block_buffer, 0, parity->getSize());
		(*block)->set_buffer(block_buffer);
		(*block)->fetch();
		for (i = 0; i < parity->getSize(); i++) {
			missing_buffer[i] =
				block_buffer[i] ^ missing_buffer[i];
		}
	}
	free(block_buffer);
	missing_block = new Block();
	missing_block->from_buffer(missing_buffer, block_size);
	return missing_block;
}

void Receipt::fixOneCorruptedBlock(std::vector<Block*> *blocks, std::vector<Block*> *corrupted_blocks, Block *parity, int block_size)
{
	Block *recovered_block;

	recovered_block = recoverBlockFromParity(blocks, parity, block_size);
	Block *block = corrupted_blocks->at(0);
	block->setSize(recovered_block->getSize());

	delete_block(recovered_block->getName());
	recovered_block->store();

	free(recovered_block->get_buffer());
	free(recovered_block);
	free(parity->get_buffer());
}

bool Receipt::fixIntegrity()
{
	std::vector<Block*> *sane_blocks;
	std::vector<Block*> *corrupted_blocks;
	std::vector<Block*> *corrupted_parities;

	sane_blocks = get_blocks_where_corruption(blocks, false);
	corrupted_blocks = get_blocks_where_corruption(blocks, true);
	corrupted_parities = get_blocks_where_corruption(parities, true);

	if (corrupted_parities->size() == 1) {
		printf("fixing global parity\n");
		Block *parity = buildGlobalParity();
		parity->store();
	}

	if (corrupted_blocks->size() == 1) {
		printf("fixing corrupted block\n");
		Block *block = corrupted_blocks->at(0);
		if (block->isLast())
			fixOneCorruptedBlock(sane_blocks, corrupted_blocks, parities->at(0), last_block_size);
		else
			fixOneCorruptedBlock(sane_blocks, corrupted_blocks, parities->at(0), block_size);
	}

	return true;
}

void
Receipt::unpack(bool skip_integrity)
{
	if (skip_integrity) {
		recoverOriginalFile();
	}
	else {
		if (!checkIntegrity()) {
			if (fixIntegrity()) {
				recoverOriginalFile();
			}
		}
		else {
			recoverOriginalFile();
		}
	}
}

void
Receipt::recoverOriginalFile() {
	int fd;
	unsigned char tmp_name[SHA2_STRING];
	unsigned char *block_buffer;
	std::vector<Block*>::iterator block;

	strcpy((char *)tmp_name, (char *)name);
	strcat((char *)tmp_name, ".tmp");

	fd = open_create_file(tmp_name);
	block_buffer = (unsigned char *)malloc(sizeof(unsigned char)* block_size);

	block = blocks->begin();
	for (; block != blocks->end(); block++) {
		(*block)->set_buffer(block_buffer);
		(*block)->fetch();
		write(fd, block_buffer, (*block)->getSize());
		(*block)->set_buffer(NULL);
	}

	close(fd);

	delete_file(name);
	rename_file(tmp_name, name);

	free(block_buffer);
}

bool Receipt::checkIntegrity()
{
	std::vector<Block*> *corrupted_blocks;
	std::vector<Block*>::iterator block;
	unsigned char *buffer;
	bool ret;

	corrupted_blocks = new std::vector<Block*>();
	buffer = (unsigned char *)calloc(1, sizeof(unsigned char)* block_size);

	block = blocks->begin();
	for (; block != blocks->end(); block++) {
		(*block)->set_buffer(buffer);
		(*block)->fetch();
		if (!(*block)->checkIntegrity()) {
			corrupted_blocks->push_back((*block));
			(*block)->setCorrupted();
		}
	}


	block = parities->begin();
	for (; block != parities->end(); block++) {
		(*block)->set_buffer(buffer);
		(*block)->fetch();
		if (!(*block)->checkIntegrity()) {
			corrupted_blocks->push_back((*block));
			(*block)->setCorrupted();
		}
	}

	if (corrupted_blocks->size() > parities_num)
		die("cannot recover %i corrupted blocks with %i parities\n",
		corrupted_blocks->size(), parities_num);

	if (corrupted_blocks->size() > 0)
		ret = false;
	else
		ret = true;

	delete corrupted_blocks;
	free(buffer);

	return ret;
}

std::vector<Block*> *
Receipt::get_blocks_where_corruption(std::vector<Block*> *blocks, bool condition)
{
	std::vector<Block*>::iterator block;
	std::vector<Block*> *filtered_blocks = new std::vector<Block*>();

	block = blocks->begin();
	for (; block != blocks->end(); block++) {
		if ((*block)->isCorrupted() == condition) {
			filtered_blocks->push_back((*block));
		}			
	}		

	return filtered_blocks;
}