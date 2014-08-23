#include "util.h"
#include "receipt.h"

bool Receipt::check_integrity()
{
	block_vector corrupted_blocks = block_vector();
	block_vector::iterator block;
	unsigned char *buffer;
	bool ret;
	
	buffer = new unsigned char[block_size_];

	prune_blocks_integrity(corrupted_blocks, *blocks_, buffer);
	prune_blocks_integrity(corrupted_blocks, *parities_, buffer);

	if (corrupted_blocks.size() > (size_t)parities_num_)
		die("cannot recover %i corrupted blocks with %i parities\n",
		corrupted_blocks.size(), parities_num_);

	if (corrupted_blocks.size() > 0)
		ret = false;
	else
		ret = true;
	
	delete buffer;

	return ret;
}

void Receipt::prune_blocks_integrity(block_vector &store, block_vector &blocks, unsigned char *buffer)
{
	block_vector::iterator block;

	block = blocks.begin();
	for (; block != blocks.end(); block++) {
		(*block)->fetch(buffer);
		if (!(*block)->integral()) {			
			(*block)->set_corrupted();
			store.push_back((*block));
		}
	}
}


bool Receipt::fix_integrity()
{
	block_vector sane_blocks = get_blocks_where_corruption(blocks_, false);
	block_vector corrupted_blocks = get_blocks_where_corruption(blocks_, true);
	block_vector corrupted_parities = get_blocks_where_corruption(parities_, true);

	if (corrupted_parities.size() == 1) {
		printf("fixing global parity\n");
		Block *parity = build_global_parity();
		parity->store();
	}
	else if (corrupted_blocks.size() == 1) {
		printf("fixing corrupted block\n");
		Block *block = corrupted_blocks.at(0);
		if (block->last()) {
			fix_one_corrupted_block(sane_blocks, *block, *parities_->at(0), last_block_size_);
		}
		else {
			fix_one_corrupted_block(sane_blocks, *block, *parities_->at(0), block_size_);
		}
	}
	else {
		return false;
	}

	return true;
}


Block *
Receipt::recover_block_from_parity(block_vector &blocks, Block &parity, int block_size)
{
	unsigned int i;
	unsigned char *missing_buffer;
	unsigned char *block_buffer;
	unsigned char *parity_buffer;

	block_vector::iterator block;
	Block *missing_block;

	block_buffer = new unsigned char[parity.size()](); //(unsigned char *) calloc (1, sizeof (unsigned char) * parity->size ());
	missing_buffer = new unsigned char[parity.size()](); //(unsigned char *) calloc (1, sizeof (unsigned char) * parity->size ());
	parity_buffer = new unsigned char[parity.size()]; //(unsigned char *) malloc (sizeof (unsigned char) * parity->size ());

	parity.fetch(parity_buffer);
	memcpy(missing_buffer, parity.buffer(), parity.size());

	block = blocks.begin();
	for (; block != blocks.end(); block++) {
		memset(block_buffer, 0, parity.size());
		(*block)->fetch(block_buffer);
		for (i = 0; i < parity.size(); i++) {
			missing_buffer[i] =
				block_buffer[i] ^ missing_buffer[i];
		}
	}

	missing_block = new Block();
	missing_block->from_buffer(missing_buffer, block_size);

	delete block_buffer;
	delete parity_buffer;

	return missing_block;
}

void Receipt::fix_one_corrupted_block(block_vector &blocks, Block &block, Block &parity, int block_size)
{
	Block *recovered_block;

	block.sha2();
	recovered_block = recover_block_from_parity(blocks, parity, block_size);
	delete_block(block.name());
	recovered_block->store();
	//delete_block((unsigned char *)recovered_block->name());	    
}


block_vector
Receipt::get_blocks_where_corruption(block_vector *blocks, bool condition)
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
