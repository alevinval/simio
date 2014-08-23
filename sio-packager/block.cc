#include <string.h>

#include "sha256.h"
#include "block.h"
#include "util.h"
#include "dirnav.h"

Block::Block() : sha2_(), size_(0), corrupted_(false), last_(false)
{
    name_ = std::string(64, ' ');
}

Block::~Block() {}

void Block::from_buffer(unsigned char *buffer, unsigned int size)
{
    if (!buffer)
        die("Block::from_buffer: no buffer");

    size_ = size;
    buffer_ = buffer;
    update_hash();
}

/*	
		This method is _lazy_, calling 'from_file' just
        sets the metadata of the block.
        The content of the block must be explicitly retrieved
        with fetch();
*/
void Block::from_file(const unsigned char (&sha2)[32], unsigned int size)
{
    memcpy(sha2_, sha2, 32);
    name_ = sha2hexf(sha2);
    size_ = size;
}

void Block::fetch(unsigned char *buffer)
{
    int fd;
    unsigned int block_size;

    if (!buffer)
        die("Block::fetch: no buffer");

    buffer_ = buffer;

    fd = open_block(name_);
    block_size = file_size(fd);
    if (size_ != block_size) {
        corrupted_ = true;
    }
    read(fd, buffer, block_size);
    close(fd);
}

void Block::store()
{
    int fd;

    if (!buffer_)
        die("Block::store: no buffer\n");

    fd = open_create_block(name_);
    write(fd, buffer_, size_);
    close(fd);
}

void Block::update_hash()
{
    sha256(sha2_, buffer_, size_);
    name_ = sha2hexf(sha2_);
}

unsigned int Block::size() const
{
    return size_;
}

bool Block::corrupted() const
{
    return corrupted_;
}

bool Block::integral() const
{
    unsigned char match_sha[32];

    if (!buffer_)
        die("Block::integral: no buffer\n");

    sha256(match_sha, buffer_, size_);
    return !memcmp(match_sha, sha2_, 32);
}

bool Block::last() const
{
    return last_;
}

const unsigned char *Block::sha2() const
{
    return sha2_;
}

const std::string &Block::name() const
{
    return name_;
}

unsigned char *Block::buffer() const
{
    return buffer_;
}

void Block::set_last(unsigned int size)
{
    size_ = size;
    last_ = true;
}

void Block::set_corrupted()
{
    corrupted_ = true;
}
