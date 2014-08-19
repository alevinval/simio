#ifndef RECEIPT_H
#define RECEIPT_H

#include <vector>

#include "dir.h"
#include "block.hpp"


typedef std::vector<Block *> block_vector;

class Receipt
{

    unsigned char sha2[32];
    unsigned char name[FNAME_LEN];
    int size;
    int block_size;
    int parities_num;
    int last_block_size;
    block_vector *blocks;
    block_vector *parities;

    void set_receipt_data (unsigned char *file_path, int block_size);
    void fetch_receipt_data (int fd);

    void set_hash ();

    void pack_blocks ();

    void pack_parities ();

    void store_receipt ();

    void fetch_blocks_data (int fd);

    void recover_original_file ();

    bool check_integrity ();

    bool fix_integrity ();

    Block *build_global_parity ();

    block_vector *get_blocks_where_corruption (block_vector *blocks, bool condition);

    void prune_blocks_integrity (block_vector *store, block_vector *blocks, unsigned char *buffer);

    Block *recover_block_from_parity (block_vector *blocks, Block *parity, int block_size);

    void fix_one_corrupted_block (block_vector *sane_blocks, Block *block, Block *parity, int block_size);

public:
    Receipt ();
    ~Receipt ();


    void open (unsigned char *receipt_name);

    void create (unsigned char *file_path, int block_size);

    void pack ();

    void unpack (bool skip_integrity);
};


#endif //RECEIPT_H
