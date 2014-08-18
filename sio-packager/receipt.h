#ifndef RECEIPT_H
#define RECEIPT_H

#include <vector>
#include "dir.h"
#include "block.h"

class Receipt
{
    unsigned char sha2[32];
    unsigned char name[FNAME_LEN];
    int size;
    int block_size;
    int parities_num;
    int last_block_size;
    std::vector<Block *> *blocks;
    std::vector<Block *> *parities;

    void set_header (unsigned char *file_path, int block_size);

    void set_hash ();

    void build_blocks ();

    void build_parities ();

    void store_receipt ();

    void fetch_receipt (int fd);

    void fetch_blocks (int fd);

    void recover_original_file ();

    bool check_integrity ();

    bool fix_integrity ();

    Block *build_global_parity ();

    std::vector<Block *> *get_blocks_where_corruption (std::vector<Block *> *blocks, bool condition);

    void prune_blocks_integrity (std::vector<Block *> *store, std::vector<Block *> *blocks, unsigned char *buffer);

    Block *recover_block_from_parity (std::vector<Block *> *blocks, Block *parity, int block_size);

    void fix_one_corrupted_block (std::vector<Block *> *sane_blocks, Block *block, Block *parity, int block_size);

public:
    Receipt ();
    ~Receipt ();


    void open (unsigned char *receipt_name);

    void create (unsigned char *file_path, int block_size);

    void pack ();

    void unpack (bool skip_integrity);
};


#endif //RECEIPT_H
