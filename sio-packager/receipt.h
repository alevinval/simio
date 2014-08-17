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

    void setHeader (unsigned char *file_path, int block_size);

    void setHash ();

    void storeBlocks ();

    void storeParities ();

    void storeReceipt ();

    void fetchReceipt (int fd);

    void fetchBlocks (int fd);

    void recoverOriginalFile ();

    bool checkIntegrity ();

    bool fixIntegrity ();

    Block *buildGlobalParity ();

    void buildParities ();

    std::vector<Block *> *get_blocks_where_corruption (std::vector<Block *> *blocks, bool condition);

    void prune_blocks_integrity (std::vector<Block *> *store, std::vector<Block *> *blocks, unsigned char *buffer);

    Block *recoverBlockFromParity (std::vector<Block *> *blocks, Block *parity, int block_size);

    void fix_one_corrupted_block (std::vector<Block *> *sane_blocks, Block *block, Block *parity, int block_size);

public:
    ~Receipt ();

    Receipt (unsigned char *receipt_name);

    Receipt (unsigned char *file_path, int block_size);

    void pack ();

    void unpack (bool skip_integrity);
};


#endif //RECEIPT_H
