#include <vector>
#include <cstdlib>
#include <cstring>

#include "sha256.h"
#include "receipt.h"
#include "dirnav.h"
#include "util.h"

Receipt::Receipt()
    : sha2_(), size_(0), block_size_(0), parities_num_(0), last_block_size_(0)
{
    blocks_ = block_vector();
    parities_ = block_vector();
	sane_blocks_ = block_vector();
	corrupted_blocks_ =  block_vector();
	corrupted_parities_ = block_vector();
}

Receipt::~Receipt()
{
    block_vector::iterator it;
    for (it = blocks_.begin(); it != blocks_.end(); it++)
        delete *it;
    for (it = parities_.begin(); it != parities_.end(); it++)
        delete *it;
}

void Receipt::open(const std::string &receipt_name)
{
    int fd;

    fd = open_receipt(receipt_name);
    fetch_receipt_data(fd);

    blocks_.reserve(size_);
    parities_.reserve(1);

    fetch_blocks_data(fd);

    close(fd);
}

void Receipt::create(const std::string &file_path, int block_size)
{
    set_receipt_data(file_path, block_size);
    blocks_.reserve(size_);
    parities_.reserve(1);
}

void Receipt::pack()
{
    pack_blocks();    
    set_hash();
    store_receipt();
}

void Receipt::unpack(bool skip_integrity)
{
    if (skip_integrity) {
        recover_original_file();
        return;
    }

	try {
		recover_original_file_with_check();
	}
	catch (int offset) {		
		check_integrity(offset);
		if (fix_integrity()) {
			recover_original_file();
		}
	}
}

void Receipt::set_receipt_data(const std::string &file_path, int block_size)
{
    int fd, extra;
    fd = open_file(file_path);

    name_ = file_path;
    extra = file_size(fd) % block_size;
    if (extra > 0)
        size_ = file_size(fd) / block_size + 1;
    else
        size_ = file_size(fd) / block_size;
    parities_num_ = 0;
    this->block_size_ = block_size;
    close(fd);
}

void Receipt::set_hash()
{
    int i;
    unsigned char *hash_buffer;
    block_vector::iterator block;

    hash_buffer = new unsigned char[32 * size_];

    i = 0;
    for (block = blocks_.begin(); block != blocks_.end(); block++) {
        memcpy(&hash_buffer[i * 32], (*block)->sha2(), 32);
        i++;
    }

    sha256(sha2_, hash_buffer, size_ * 32);
    delete[] hash_buffer;
}

void Receipt::pack_blocks()
{
	unsigned int i;	
    int j, fd, readed_bytes;
    unsigned char *buffer;
	unsigned char *parity_buffer;
    Block *block;
	Block *global_parity;		

    fd = open_file(name_);

	// Initialize things
	global_parity = new Block();
    buffer = new unsigned char[block_size_];
	parity_buffer = new unsigned char[block_size_]();
	
	// Process the file and pack the blocks ( build the global parity 
	// at the same time )
    for (i = 0; i < size_; i++) {
        readed_bytes = fill_buffer(fd, buffer, block_size_);
        block = new Block();
        block->from_buffer(buffer, readed_bytes);
        block->store();
        blocks_.push_back(block);		
		for (j = 0; j < readed_bytes; j++)
			parity_buffer[j] ^= buffer[j];
    }
    last_block_size_ = block->size();
    block->set_last(last_block_size_);

	global_parity->from_buffer(parity_buffer, block_size_);
	global_parity->store();
	parities_.push_back(global_parity);

	// Update number of parities.
	parities_num_ = (int)parities_.size();

    delete[] buffer;
	delete[] parity_buffer;
}

void Receipt::store_receipt()
{
    int fd;
    int strlen = name_.length();
    block_vector::iterator block;

    fd = open_create_receipt(".receipt");

    write(fd, &sha2_, 32);
    write(fd, &strlen, sizeof(int));
    write(fd, &name_[0], strlen);
    write(fd, &size_, sizeof(int));
    write(fd, &parities_num_, sizeof(int));
    write(fd, &block_size_, sizeof(int));
    write(fd, &last_block_size_, sizeof(int));

    block = blocks_.begin();
    for (; block != blocks_.end(); block++) {
        write(fd, (*block)->sha2(), 32);
    }

    block = parities_.begin();
    for (; block != parities_.end(); block++) {
        write(fd, (*block)->sha2(), 32);
    }

    close(fd);
}

void Receipt::fetch_receipt_data(int fd)
{
    int strlen;
    read(fd, &sha2_, 32);
    read(fd, &strlen, sizeof(int));
    name_ = std::string(strlen, ' ');
    read(fd, &name_[0], strlen);
    read(fd, &size_, sizeof(int));
    read(fd, &parities_num_, sizeof(int));
    read(fd, &block_size_, sizeof(int));
    read(fd, &last_block_size_, sizeof(int));
}

void Receipt::fetch_blocks_data(int fd)
{
    unsigned int i;
    unsigned char block_sha2[32];
    Block *block;

    for (i = 0; i < size_; i++) {
        read(fd, &block_sha2, 32);
        block = new Block();
        block->from_file(block_sha2, block_size_);
        blocks_.push_back(block);
    }
    block->set_last(last_block_size_);

    for (i = 0; i < parities_num_; i++) {
        read(fd, &block_sha2, 32);
        block = new Block();
        block->from_file(block_sha2, block_size_);
        parities_.push_back(block);
    }
}

void Receipt::recover_original_file()
{
    int fd;

    std::string tmp_name;
    unsigned char *block_buffer;
    block_vector::iterator block;

    tmp_name = name_.c_str();
    tmp_name.append(".tmp");

    fd = open_create_file(tmp_name);
    block_buffer = new unsigned char[block_size_];

    block = blocks_.begin();
    for (; block != blocks_.end(); block++) {
        (*block)->fetch(block_buffer);
        write(fd, block_buffer, (*block)->size());
        // clear buffer?
    }

    close(fd);

    delete_file(name_);
    rename_file(tmp_name, name_);

    delete[] block_buffer;
}

void Receipt::recover_original_file_with_check()
{
	int fd, i;

	std::string tmp_name;
	unsigned char *block_buffer;
	block_vector::iterator block;

	tmp_name = name_.c_str();
	tmp_name.append(".tmp");

	fd = open_create_file(tmp_name);
	block_buffer = new unsigned char[block_size_];

	i = 0;
	block = blocks_.begin();
	for (; block != blocks_.end(); block++) {
		(*block)->fetch(block_buffer);
		if (!(*block)->integral()) {
			(*block)->set_corrupted();
			close(fd);
			delete_file(tmp_name);
			delete[] block_buffer;
			throw i;
		}
		sane_blocks_.push_back(*block);
		write(fd, block_buffer, (*block)->size());
		i++;
	}

	close(fd);

	delete_file(name_);
	rename_file(tmp_name, name_);

	delete[] block_buffer;
}
