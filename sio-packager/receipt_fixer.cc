#include <cstring>
#include "util.h"
#include "receipt.h"

bool Receipt::check_integrity(int from)
{    
    block_vector::iterator block;
    unsigned char *buffer;
    bool ret;

    buffer = new unsigned char[block_size_];

    prune_blocks_integrity(buffer, from);
    prune_parities_integrity(buffer, 0);

    if (corrupted_blocks_.size() > (size_t)parities_num_)
        die("cannot recover %i corrupted blocks with %i parities\n",
            corrupted_blocks_.size(), parities_num_);

    if (corrupted_blocks_.size() > 0)
        ret = false;
    else
        ret = true;

    delete[] buffer;

    return ret;
}

void Receipt::prune_blocks_integrity(unsigned char *buffer, int from)
{
    block_vector::iterator block;

    block = blocks_->begin() + from;
    for (; block != blocks_->end(); block++) {
        (*block)->fetch(buffer);
        if (!(*block)->integral()) {
            (*block)->set_corrupted();
            corrupted_blocks_.push_back((*block));
		}
		else {
			sane_blocks_.push_back((*block));
		}
    }
}

void Receipt::prune_parities_integrity(unsigned char *buffer, int from)
{
	block_vector::iterator block;

	block = parities_->begin() + from;
	for (; block != parities_->end(); block++) {
		(*block)->fetch(buffer);
		if (!(*block)->integral()) {
			(*block)->set_corrupted();
			corrupted_parities_.push_back((*block));
		}
	}
}

bool Receipt::fix_integrity()
{
    if (corrupted_parities_.size() == 1) {
        printf("fixing global parity\n");
        Block *parity = build_global_parity();
        parity->store();
    } else if (corrupted_blocks_.size() == 1) {
        printf("fixing corrupted block\n");
        Block *block = corrupted_blocks_.at(0);
        if (block->last()) {
            fix_one_corrupted_block(sane_blocks_, *block, *parities_->at(0),
                                    last_block_size_);
        } else {
            fix_one_corrupted_block(sane_blocks_, *block, *parities_->at(0),
                                    block_size_);
        }
    } else {
        return false;
    }

    return true;
}

Block *Receipt::recover_block_from_parity(block_vector &blocks, Block &parity,
        int block_size)
{
    unsigned int i;
    unsigned char *missing_buffer;
    unsigned char *block_buffer;
    unsigned char *parity_buffer;

    block_vector::iterator block;
    Block *missing_block;

    block_buffer = new unsigned char[parity.size()]();
    missing_buffer =
        new unsigned char[parity.size()]();
    parity_buffer = new unsigned char[parity.size()];

    parity.fetch(parity_buffer);
    memcpy(missing_buffer, parity.buffer(), parity.size());

    block = blocks.begin();
    for (; block != blocks.end(); block++) {
        memset(block_buffer, 0, parity.size());
        (*block)->fetch(block_buffer);
        for (i = 0; i < parity.size(); i++) {
            missing_buffer[i] = block_buffer[i] ^ missing_buffer[i];
        }
    }

    missing_block = new Block();
    missing_block->from_buffer(missing_buffer, block_size);

    delete[] block_buffer;
    delete[] parity_buffer;

    return missing_block;
}

void Receipt::fix_one_corrupted_block(block_vector &blocks, Block &block,
                                      Block &parity, int block_size)
{
    Block *recovered_block;

    block.sha2();
    recovered_block = recover_block_from_parity(blocks, parity, block_size);
    delete_block(block.name());
    recovered_block->store();
    // delete_block((unsigned char *)recovered_block->name());
}

block_vector Receipt::get_blocks_where_corruption(block_vector *blocks,
        bool condition)
{
    block_vector::iterator block;
    block_vector filtered_blocks = block_vector();

    block = blocks->begin();
    for (; block != blocks->end(); block++) {
        if ((*block)->corrupted() == condition) {
            filtered_blocks.push_back(*block);
        }
    }

    return filtered_blocks;
}

Block *Receipt::build_global_parity()
{
	unsigned int i;
	unsigned char *block_buffer;
	unsigned char *parity_buffer;

	block_vector::iterator block;
	Block *parity;

	block_buffer = new unsigned char[block_size_]();
	parity_buffer = new unsigned char[block_size_]();

	/** Build Global Parity */

	block = blocks_->begin();
	for (; block != blocks_->end(); block++) {
		memset(block_buffer, 0, block_size_);
		(*block)->fetch(block_buffer);
		for (i = 0; i < block_size_; i++)
			parity_buffer[i] = block_buffer[i] ^ parity_buffer[i];
	}

	parity = new Block();
	parity->from_buffer(parity_buffer, block_size_);

	delete[] block_buffer;

	return parity;
}