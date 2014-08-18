#include <vector>
#include <stdlib.h>

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

    blocks = new std::vector<Block *>();
    parities = new std::vector<Block *>();

    fd = open_receipt (receipt_name);
    fetch_receipt (fd);
    fetch_blocks (fd);
    close (fd);
}

void
Receipt::create (unsigned char *file_path, int block_size)
{
    set_header (file_path, block_size);
    blocks = new std::vector<Block *>();
    parities = new std::vector<Block *>();
}

Receipt::~Receipt ()
{
    delete blocks;
    delete parities;
}

void
Receipt::pack ()
{
    build_blocks ();
    build_parities ();
    store_receipt ();
    set_hash ();
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
Receipt::set_header (unsigned char *file_path, int block_size)
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
}

void
Receipt::set_hash ()
{
    int i;
    unsigned char *hash_buffer = (unsigned char *) malloc (sizeof (unsigned char) * 32 * size);
    std::vector<Block *>::iterator block;

    i = 0;
    for (block = blocks->begin (); block != blocks->end (); block++) {
        memcpy (&hash_buffer[i * 32], (*block)->get_sha2 (), 32);
        i++;
    }

    sha256 (sha2, hash_buffer, size * 32);
    free (hash_buffer);
}

void
Receipt::build_blocks ()
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
    last_block_size = block->get_size ();
    delete buffer;
}

void
Receipt::store_receipt ()
{
    int fd;
    std::vector<Block *>::iterator block;

    fd = open_create_receipt ((unsigned char *) ".receipt");

    write (fd, &sha2, 32);
    write (fd, &name, FNAME_LEN);
    write (fd, &size, sizeof (int));
    write (fd, &parities_num, sizeof (int));
    write (fd, &block_size, sizeof (int));
    write (fd, &last_block_size, sizeof (int));

    block = blocks->begin ();
    for (; block != blocks->end (); block++) {
        write (fd, (*block)->get_sha2 (), 32);
    }

    block = parities->begin ();
    for (; block != parities->end (); block++) {
        write (fd, (*block)->get_sha2 (), 32);
    }

    close (fd);
}

void
Receipt::fetch_receipt (int fd)
{
    read (fd, &sha2, 32);
    read (fd, &name, FNAME_LEN);
    read (fd, &size, sizeof (int));
    read (fd, &parities_num, sizeof (int));
    read (fd, &block_size, sizeof (int));
    read (fd, &last_block_size, sizeof (int));
}

void
Receipt::fetch_blocks (int fd)
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
    blocks->back ()->set_last ();
    blocks->back ()->set_size (last_block_size);

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

    std::vector<Block *>::iterator block;
    Block *parity;

    block_buffer = (unsigned char *) calloc (1, sizeof (unsigned char) * block_size);
    parity_buffer = (unsigned char *) calloc (1, sizeof (unsigned char) * block_size);

    /** Build Global Parity */

    block = blocks->begin ();
    (*block)->fetch (block_buffer);

    memcpy (parity_buffer, block_buffer, (*block)->get_size ());
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

void Receipt::build_parities ()
{
    Block *block;

    block = build_global_parity ();
    block->store ();

    parities->push_back (block);
    parities_num = (int) parities->size ();

    delete block->get_buffer ();
}


Block *Receipt::recover_block_from_parity (std::vector<Block *> *blocks, Block *parity, int block_size)
{
    int i;
    unsigned char *missing_buffer;
    unsigned char *block_buffer;
    unsigned char *parity_buffer;

    std::vector<Block *>::iterator block;
    Block *missing_block;

    block_buffer = (unsigned char *) calloc (1, sizeof (unsigned char) * parity->get_size ());
    missing_buffer = (unsigned char *) calloc (1, sizeof (unsigned char) * parity->get_size ());
    parity_buffer = (unsigned char *) malloc (sizeof (unsigned char) * parity->get_size ());

    parity->fetch (parity_buffer);
    memcpy (missing_buffer, parity->get_buffer (), parity->get_size ());

    block = blocks->begin ();
    for (; block != blocks->end (); block++) {
        memset (block_buffer, 0, parity->get_size ());
        (*block)->fetch (block_buffer);
        for (i = 0; i < parity->get_size (); i++) {
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

void Receipt::fix_one_corrupted_block (std::vector<Block *> *blocks, Block *block, Block *parity, int block_size)
{
    Block *recovered_block;

    recovered_block = recover_block_from_parity (blocks, parity, block_size);
    block->set_size (recovered_block->get_size ());
    delete_block (recovered_block->get_name ());
    recovered_block->store ();

    delete recovered_block;
}

bool Receipt::fix_integrity ()
{
    std::vector<Block *> *sane_blocks;
    std::vector<Block *> *corrupted_blocks;
    std::vector<Block *> *corrupted_parities;

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
        if (block->is_last ())
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
    std::vector<Block *>::iterator block;

    strcpy ((char *) tmp_name, (char *) name);
    strcat ((char *) tmp_name, ".tmp");

    fd = open_create_file (tmp_name);
    block_buffer = (unsigned char *) malloc (sizeof (unsigned char) * block_size);

    block = blocks->begin ();
    for (; block != blocks->end (); block++) {
        (*block)->fetch (block_buffer);
        write (fd, block_buffer, (*block)->get_size ());
        (*block)->set_buffer (NULL);
    }

    close (fd);

    delete_file (name);
    rename_file (tmp_name, name);

    delete block_buffer;
}

void Receipt::prune_blocks_integrity (std::vector<Block *> *store, std::vector<Block *> *blocks, unsigned char *buffer)
{
    std::vector<Block *>::iterator block;

    block = blocks->begin ();
    for (; block != blocks->end (); block++) {
        (*block)->fetch (buffer);
        if (!(*block)->check_integrity ()) {
            store->push_back ((*block));
            (*block)->set_corrupted ();
        }
    }
}

bool Receipt::check_integrity ()
{
    std::vector<Block *> *corrupted_blocks;
    std::vector<Block *>::iterator block;
    unsigned char *buffer;
    bool ret;

    corrupted_blocks = new std::vector<Block *>();
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

std::vector<Block *> *
Receipt::get_blocks_where_corruption (std::vector<Block *> *blocks, bool condition)
{
    std::vector<Block *>::iterator block;
    std::vector<Block *> *filtered_blocks = new std::vector<Block *>();

    block = blocks->begin ();
    for (; block != blocks->end (); block++) {
        if ((*block)->is_corrupted () == condition) {
            filtered_blocks->push_back ((*block));
        }
    }

    return filtered_blocks;
}
