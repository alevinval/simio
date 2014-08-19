#include <vector>
#include <stdlib.h>

#include "sha256.h"
#include "receipt.h"
#include "dirnav.h"
#include "util.h"

Receipt::Receipt()
{
}

void
Receipt::open (unsigned char *receipt_name)
{
    int fd;

    blocks = new block_vector();
    parities = new block_vector();

    fd = open_receipt (receipt_name);
    fetch_receipt_data (fd);
    fetch_blocks_data (fd);

    blocks->reserve(size);
    parities->reserve(1);

    close (fd);
}

void
Receipt::create (unsigned char *file_path, int block_size)
{
    set_receipt_data (file_path, block_size);
    blocks = new block_vector();
    parities = new block_vector();
    blocks->reserve (size);
    parities->reserve (1);
}

Receipt::~Receipt ()
{
    block_vector::iterator it;
    for (it = blocks->begin(); it!=blocks->end(); it++ )
        delete *it;
    for (it = parities->begin(); it!=parities->end(); it++ )
        delete *it;
    delete blocks;
    delete parities;
}

void
Receipt::pack ()
{
    pack_blocks ();
    pack_parities ();
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
Receipt::set_receipt_data (unsigned char *file_path, int block_size)
{
    int fd, extra;
    fd = open_file (file_path);

    strcpy ((char *) name, (char *) file_path);
    extra = file_size (fd) % block_size;
    if (extra > 0)
        size = file_size (fd) / block_size + 1;
    else
        size = file_size (fd) / block_size;
    parities_num = 0;
    this->block_size = block_size;
    close(fd);
}

void
Receipt::set_hash ()
{
    int i;
    unsigned char *hash_buffer = (unsigned char *) malloc (sizeof (unsigned char) * 32 * size);
    block_vector::iterator block;

    i = 0;
    for (block = blocks->begin (); block != blocks->end (); block++) {
        memcpy (&hash_buffer[i * 32], (*block)->sha2 (), 32);
        i++;
    }

    sha256 (sha2, hash_buffer, size * 32);
    free (hash_buffer);
}

void
Receipt::pack_blocks ()
{
    int i, fd, readed_bytes;
    unsigned char *buffer;
    Block *block;

    fd = open_file (name);
    buffer = (unsigned char *) malloc (sizeof (unsigned char) * block_size);
    for (i = 0; i < size; i++) {
        readed_bytes = fill_buffer (fd, buffer, block_size);
        block = new Block();
        block->from_buffer (buffer, readed_bytes);
        block->store ();
        blocks->push_back (block);
    }
    last_block_size = block->size ();
    delete buffer;
}

void
Receipt::store_receipt ()
{
    int fd;
    block_vector::iterator block;

    fd = open_create_receipt ((unsigned char *) ".receipt");

    write (fd, &sha2, 32);
    write (fd, &name, FNAME_LEN);
    write (fd, &size, sizeof (int));
    write (fd, &parities_num, sizeof (int));
    write (fd, &block_size, sizeof (int));
    write (fd, &last_block_size, sizeof (int));

    block = blocks->begin ();
    for (; block != blocks->end (); block++) {
        write (fd, (*block)->sha2 (), 32);
    }

    block = parities->begin ();
    for (; block != parities->end (); block++) {
        write (fd, (*block)->sha2 (), 32);
    }

    close (fd);
}

void
Receipt::fetch_receipt_data (int fd)
{
    read (fd, &sha2, 32);
    read (fd, &name, FNAME_LEN);
    read (fd, &size, sizeof (int));
    read (fd, &parities_num, sizeof (int));
    read (fd, &block_size, sizeof (int));
    read (fd, &last_block_size, sizeof (int));
}

void
Receipt::fetch_blocks_data (int fd)
{
    int i;
    unsigned char block_sha2[32];
    Block *block;

    for (i = 0; i < size; i++) {
        read (fd, &block_sha2, 32);
        block = new Block();
        block->from_file (block_sha2, block_size);
        blocks->push_back (block);
    }
    blocks->back ()->set_last (last_block_size);

    for (i = 0; i < parities_num; i++) {
        read (fd, &block_sha2, 32);
        block = new Block();
        block->from_file (block_sha2, block_size);
        parities->push_back (block);
    }

}

Block *
Receipt::build_global_parity ()
{
    int i;
    unsigned char *block_buffer;
    unsigned char *parity_buffer;

    block_vector::iterator block;
    Block *parity;

    block_buffer = (unsigned char *) calloc (1, sizeof (unsigned char) * block_size);
    parity_buffer = (unsigned char *) calloc (1, sizeof (unsigned char) * block_size);

    /** Build Global Parity */

    block = blocks->begin ();
    (*block)->fetch (block_buffer);

    memcpy (parity_buffer, block_buffer, (*block)->size ());
    block++;
    for (; block != blocks->end (); block++) {
        memset (block_buffer, 0, block_size);
        (*block)->fetch (block_buffer);
        for (i = 0; i < block_size; i++)
            parity_buffer[i] = block_buffer[i] ^ parity_buffer[i];
    }
    parity = new Block();
    parity->from_buffer (parity_buffer, block_size);

    delete block_buffer;

    return parity;
}

void Receipt::pack_parities ()
{
    Block *block;

    block = build_global_parity ();
    block->store ();

    parities->push_back (block);
    parities_num = (int) parities->size ();

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

    block_buffer = (unsigned char *) calloc (1, sizeof (unsigned char) * parity->size ());
    missing_buffer = (unsigned char *) calloc (1, sizeof (unsigned char) * parity->size ());
    parity_buffer = (unsigned char *) malloc (sizeof (unsigned char) * parity->size ());

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

    recovered_block = recover_block_from_parity (blocks, parity, block_size);

    delete_block((unsigned char *)block->name());
    //delete_block((unsigned char *)recovered_block->name());
    recovered_block->store();
}

bool Receipt::fix_integrity ()
{
    block_vector *sane_blocks;
    block_vector *corrupted_blocks;
    block_vector *corrupted_parities;

    sane_blocks = get_blocks_where_corruption (blocks, false);
    corrupted_blocks = get_blocks_where_corruption (blocks, true);
    corrupted_parities = get_blocks_where_corruption (parities, true);

    if (corrupted_parities->size () == 1) {
        printf ("fixing global parity\n");
        Block *parity = build_global_parity ();
        parity->store ();
    } else if (corrupted_blocks->size () == 1) {
        printf ("fixing corrupted block\n");
        Block *block = corrupted_blocks->at (0);
        if (block->last ())
            fix_one_corrupted_block (sane_blocks, block, parities->at (0), last_block_size);
        else
            fix_one_corrupted_block (sane_blocks, block, parities->at (0), block_size);
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

    strcpy ((char *) tmp_name, (char *) name);
    strcat ((char *) tmp_name, ".tmp");

    fd = open_create_file (tmp_name);
    block_buffer = (unsigned char *) malloc (sizeof (unsigned char) * block_size);

    block = blocks->begin ();
    for (; block != blocks->end (); block++) {
        (*block)->fetch (block_buffer);
        write (fd, block_buffer, (*block)->size ());
        // clear buffer?
    }

    close (fd);

    delete_file (name);
    rename_file (tmp_name, name);

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
    buffer = (unsigned char *) calloc (1, sizeof (unsigned char) * block_size);

    prune_blocks_integrity (corrupted_blocks, this->blocks, buffer);
    prune_blocks_integrity (corrupted_blocks, this->parities, buffer);

    if (corrupted_blocks->size () > (size_t) parities_num)
        die ("cannot recover %i corrupted blocks with %i parities\n",
             corrupted_blocks->size (), parities_num);

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
