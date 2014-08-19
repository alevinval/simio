#include <vector>
#include <stdlib.h>

#include "sha256.h"
#include "receipt.h"
#include "dirnav.h"
#include "util.h"

Receipt::Receipt() : sha2_(), name_(), size_(0), block_size_(0), parities_num_(0), last_block_size_(0)
{
    blocks_ = new block_vector();
    parities_ = new block_vector();
}

Receipt::~Receipt ()
{
    block_vector::iterator it;
    for (it = blocks_->begin(); it!= blocks_->end(); it++ )
        delete *it;
    for (it = parities_->begin(); it!= parities_->end(); it++ )
        delete *it;
    delete blocks_;
    delete parities_;
}

void
Receipt::open (const unsigned char *receipt_name)
{
    int fd;

    fd = open_receipt (receipt_name);
    fetch_receipt_data (fd);
    fetch_blocks_data (fd);

    blocks_->reserve(size_);
    parities_->reserve(1);

    close (fd);
}

void
Receipt::create (const unsigned char *file_path, int block_size)
{
    set_receipt_data (file_path, block_size);
    blocks_->reserve (size_);
    parities_->reserve (1);
}

void
Receipt::pack ()
{
    printf("Blocks packed\n");
    pack_blocks ();
    printf("Parities packed\n");
    pack_parities ();
    printf("OK\n");
    set_hash ();
    store_receipt ();
}

void
Receipt::unpack (bool skip_integrity)
{
    if (skip_integrity) {
        recover_original_file ();
        return;
    }

    if (!check_integrity ()) {
        if (fix_integrity ()) {
            recover_original_file ();
        }
    } else {
        recover_original_file ();
    }
}

void
Receipt::set_receipt_data (const unsigned char *file_path, int block_size)
{
    int fd, extra;
    fd = open_file (file_path);

    strcpy ((char *) name_, (char *) file_path);
    extra = file_size (fd) % block_size;
    if (extra > 0)
        size_ = file_size (fd) / block_size + 1;
    else
        size_ = file_size (fd) / block_size;
    parities_num_ = 0;
    this->block_size_ = block_size;
    close(fd);
}

void
Receipt::set_hash ()
{
    int i;
    unsigned char *hash_buffer;
    block_vector::iterator block;

    hash_buffer = new unsigned char[32*size_];

    i = 0;
    for (block = blocks_->begin (); block != blocks_->end (); block++) {
        memcpy (&hash_buffer[i * 32], (*block)->sha2 (), 32);
        i++;
    }

    sha256 (sha2_, hash_buffer, size_ * 32);
    free (hash_buffer);
}

void
Receipt::pack_blocks ()
{
    unsigned int i;
    int fd, readed_bytes;
    unsigned char *buffer;
    Block *block;

    fd = open_file (name_);
    buffer = new unsigned char[block_size_];

    for (i = 0; i < size_; i++) {
        readed_bytes = fill_buffer (fd, buffer, block_size_);
        block = new Block();
        block->from_buffer (buffer, readed_bytes);
        block->store ();
        blocks_->push_back (block);
    }
    last_block_size_ = block->size ();
    delete buffer;
}

void
Receipt::store_receipt ()
{
    int fd;
    block_vector::iterator block;

    fd = open_create_receipt ((unsigned char *)".receipt");

    write (fd, &sha2_, 32);
    write (fd, &name_, FNAME_LEN);
    write (fd, &size_, sizeof (int));
    write (fd, &parities_num_, sizeof (int));
    write (fd, &block_size_, sizeof (int));
    write (fd, &last_block_size_, sizeof (int));

    block = blocks_->begin ();
    for (; block != blocks_->end (); block++) {
        write (fd, (*block)->sha2 (), 32);
    }

    block = parities_->begin ();
    for (; block != parities_->end (); block++) {
        write (fd, (*block)->sha2 (), 32);
    }

    close (fd);
}

void
Receipt::fetch_receipt_data (int fd)
{
    read (fd, &sha2_, 32);
    read (fd, &name_, FNAME_LEN);
    read (fd, &size_, sizeof (int));
    read (fd, &parities_num_, sizeof (int));
    read (fd, &block_size_, sizeof (int));
    read (fd, &last_block_size_, sizeof (int));
}

void
Receipt::fetch_blocks_data (int fd)
{
    unsigned int i;
    unsigned char block_sha2[32];
    Block *block;

    for (i = 0; i < size_; i++) {
        read (fd, &block_sha2, 32);
        block = new Block();
        block->from_file (block_sha2, block_size_);
        blocks_->push_back (block);
    }
    blocks_->back ()->set_last (last_block_size_);

    for (i = 0; i < parities_num_; i++) {
        read (fd, &block_sha2, 32);
        block = new Block();
        block->from_file (block_sha2, block_size_);
        parities_->push_back (block);
    }

}

Block *
Receipt::build_global_parity ()
{
    unsigned int i;
    unsigned char *block_buffer;
    unsigned char *parity_buffer;

    block_vector::iterator block;
    Block *parity;

    block_buffer = new unsigned char[block_size_];// (unsigned char *) calloc (1, sizeof (unsigned char) * block_size_);
    parity_buffer = new unsigned char[block_size_]();// (unsigned char *) calloc (1, sizeof (unsigned char) * block_size_);

    /** Build Global Parity */

    block = blocks_->begin ();
    (*block)->fetch (block_buffer);
    memcpy (parity_buffer, block_buffer, (*block)->size ());
    block++;
    for (; block != blocks_->end (); block++) {
        memset (block_buffer, 0, block_size_);
        (*block)->fetch (block_buffer);
        for (i = 0; i < block_size_; i++)
            parity_buffer[i] = block_buffer[i] ^ parity_buffer[i];
    }

    parity = new Block();
    parity->from_buffer (parity_buffer, block_size_);

    delete block_buffer;

    return parity;
}

void Receipt::pack_parities ()
{
    Block *block;

    block = build_global_parity ();
    block->store ();

    parities_->push_back (block);
    parities_num_ = (int) parities_->size ();

    delete block->buffer ();
}


Block *
Receipt::recover_block_from_parity (block_vector *blocks, Block *parity, int block_size)
{
    unsigned int i;
    unsigned char *missing_buffer;
    unsigned char *block_buffer;
    unsigned char *parity_buffer;

    block_vector::iterator block;
    Block *missing_block;

    block_buffer = new unsigned char[parity->size()](); //(unsigned char *) calloc (1, sizeof (unsigned char) * parity->size ());
    missing_buffer = new unsigned char[parity->size()](); //(unsigned char *) calloc (1, sizeof (unsigned char) * parity->size ());
    parity_buffer = new unsigned char[parity->size()]; //(unsigned char *) malloc (sizeof (unsigned char) * parity->size ());

    parity->fetch (parity_buffer);
    memcpy (missing_buffer, parity->buffer (), parity->size ());

    block = blocks->begin ();
    for (; block != blocks->end (); block++) {
        memset (block_buffer, 0, parity->size ());
        (*block)->fetch (block_buffer);
        for (i = 0; i < parity->size (); i++) {
            missing_buffer[i] =
                block_buffer[i] ^ missing_buffer[i];
        }
    }

    missing_block = new Block();
    missing_block->from_buffer (missing_buffer, block_size);

    delete block_buffer;
    delete parity_buffer;

    return missing_block;
}

void Receipt::fix_one_corrupted_block (block_vector *blocks, Block *block, Block *parity, int block_size)
{
    Block *recovered_block;

    block->sha2();
    recovered_block = recover_block_from_parity (blocks, parity, block_size);
    delete_block(block->name());
    //delete_block((unsigned char *)recovered_block->name());
    recovered_block->store();
}

bool Receipt::fix_integrity ()
{
    block_vector *sane_blocks;
    block_vector *corrupted_blocks;
    block_vector *corrupted_parities;

    sane_blocks = get_blocks_where_corruption (blocks_, false);
    corrupted_blocks = get_blocks_where_corruption (blocks_, true);
    corrupted_parities = get_blocks_where_corruption (parities_, true);

    if (corrupted_parities->size () == 1) {
        printf ("fixing global parity\n");
        Block *parity = build_global_parity ();
        parity->store ();
    } else if (corrupted_blocks->size () == 1) {
        printf ("fixing corrupted block\n");
        Block *block = corrupted_blocks->at (0);
        if (block->last ())
            fix_one_corrupted_block (sane_blocks, block, parities_->at (0), last_block_size_);
        else
            fix_one_corrupted_block (sane_blocks, block, parities_->at (0), block_size_);
    } else {
        return false;
    }

    delete sane_blocks;
    delete corrupted_blocks;
    delete corrupted_parities;

    return true;
}

void
Receipt::recover_original_file ()
{
    int fd;
    unsigned char tmp_name[SHA2_STRING];
    unsigned char *block_buffer;
    block_vector::iterator block;

    strcpy ((char *) tmp_name, (char *) name_);
    strcat ((char *) tmp_name, ".tmp");

    fd = open_create_file (tmp_name);
    block_buffer = new unsigned char[block_size_];//(unsigned char *) malloc (sizeof (unsigned char) * block_size_);

    block = blocks_->begin ();
    for (; block != blocks_->end (); block++) {
        (*block)->fetch (block_buffer);
        write (fd, block_buffer, (*block)->size ());
        // clear buffer?
    }

    close (fd);

    delete_file (name_);
    rename_file (tmp_name, name_);

    delete block_buffer;
}

void Receipt::prune_blocks_integrity (block_vector *store, block_vector *blocks, unsigned char *buffer)
{
    block_vector::iterator block;

    block = blocks->begin ();
    for (; block != blocks->end (); block++) {
        (*block)->fetch (buffer);
        if (!(*block)->integral ()) {
            store->push_back ((*block));
            (*block)->set_corrupted ();
        }
    }
}

bool Receipt::check_integrity ()
{
    block_vector *corrupted_blocks;
    block_vector::iterator block;
    unsigned char *buffer;
    bool ret;

    corrupted_blocks = new block_vector();
    buffer = new unsigned char[block_size_];

    prune_blocks_integrity (corrupted_blocks, this->blocks_, buffer);
    prune_blocks_integrity (corrupted_blocks, this->parities_, buffer);

    if (corrupted_blocks->size () > (size_t) parities_num_)
        die ("cannot recover %i corrupted blocks with %i parities\n",
             corrupted_blocks->size (), parities_num_);

    if (corrupted_blocks->size () > 0)
        ret = false;
    else
        ret = true;

    delete corrupted_blocks;
    delete buffer;

    return ret;
}

block_vector *
Receipt::get_blocks_where_corruption (block_vector *blocks, bool condition)
{
    block_vector::iterator block;
    block_vector *filtered_blocks = new block_vector();

    block = blocks->begin ();
    for (; block != blocks->end (); block++) {
        if ((*block)->corrupted () == condition) {
            filtered_blocks->push_back (*block);
        }
    }

    return filtered_blocks;
}
