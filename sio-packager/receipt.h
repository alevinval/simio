#ifndef RECEIPT_H
#define RECEIPT_H

#include <vector>
#include "block.h"
#include "dir.h"
#include "dirnav.h"

typedef std::vector<Block *> block_vector;

class Receipt
{
    std::string name_;
    unsigned char sha2_[32];
    unsigned int size_;
    unsigned int block_size_;
    unsigned int parities_num_;
    unsigned int last_block_size_;
    block_vector blocks_;
    block_vector parities_;
    block_vector sane_blocks_;
    block_vector corrupted_blocks_;
    block_vector corrupted_parities_;

    void set_receipt_data(const std::string &file_path, int block_size);
    void fetch_receipt_data(int fd);
    void set_hash();

    void store_receipt();
    void pack_blocks();
    void fetch_blocks_data(int fd);

    void recover_original_file();
    void recover_original_file_with_check();
    // All this methods should be moved in a separate class, namely, receipt
    // fixer or whatever
    bool check_integrity(int from);
    bool fix_integrity();
    void prune_blocks_integrity(unsigned char *buffer, int from);
    void prune_parities_integrity(unsigned char *buffer, int from);

    Block *build_global_parity();
    block_vector get_blocks_where_corruption(block_vector *blocks,
                                             bool condition);
    Block *recover_block_from_parity(block_vector &blocks, Block &parity,
                                     int block_size);
    void fix_one_corrupted_block(block_vector &sane_blocks, Block &block,
                                 Block &parity, int block_size);
    // End

  public:
    Receipt();
    ~Receipt();

    const block_vector *blocks() const;
    const block_vector *parities() const;

    void open(const std::string &name);
    void create(const std::string &file_path, int block_size);
    void pack();
    void unpack(bool skip_integrity);
};

#endif // RECEIPT_H
