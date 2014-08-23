#ifndef RECEIPT_FIXER_H
#define RECEIPT_FIXER_H

#include "block.h"

class ReceiptFixer
{

public:
    bool integral();
    void fix();
    void feed_global_parity(const unsigned char *buffer);

    Block *global_parity();
};

#endif // RECEIPT_FIXER_H
