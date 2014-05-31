#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "sha256.h"
#include "receipt.h"
#include "dirnav.h"
#include "dir.h"
#include "util.h"
#include "block.h"

void recover_original_file (Receipt *receipt)
{
    int i, fd, block_size;
    unsigned char block_name[SHA256_STRING];
    unsigned char *block_buffer;
    Block *block;

    mv_parent ();
    fd = open ((char *) receipt->name, O_RDWR | O_CREAT, 0777);
    mv_package_root ();

    if (fd == -1)
        die ("recover_original_file: cannot create the original file");

    block_buffer = malloc (sizeof (unsigned char) * receipt->block_size);

    for ( block = receipt->blocks->head;
          block != NULL;
          block = block->next )
    {
        sha2hexf (block_name, block->hash);
        block_size = block2buffer (block_name, block_buffer);
        write (fd, block_buffer, block_size);
    }

    close (fd);
    free (block_buffer);
}

void recover_original_file_i (Receipt *receipt)
{
    int i, fd, block_size, integrity_error = 0;
    unsigned char tmp_name[FNAME_LEN];
    unsigned char block_name[SHA256_STRING];
    unsigned char *block_buffer;
    Block *block;

    strcpy ((char *) tmp_name, "");
    strcat ((char *) tmp_name, (char *) receipt->name);
    strcat ((char *) tmp_name, ".tmp");
    
    mv_parent ();
    fd = open ((char *) tmp_name, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    mv_package_root ();

    if (fd == -1)
        die ("recover_original_file_i: cannot create the original file");

    block_buffer = malloc (sizeof (unsigned char) * receipt->block_size);
    
    for ( block = receipt->blocks->head;
          block != NULL;
          block = block->next )
    {
        i++;
        sha2hexf (block_name, block->hash);
        block_size = block2buffer (block_name, block_buffer);
        if (check_block_integrity(block, block_buffer, block_size) == 1)
        {
            write (fd, block_buffer, block_size);
        } else {
            error ("integrity failure in Block %i\n [ %s ]", i, block_name);
            integrity_error = 1;
            mv_parent ();
            unlink ((char *) tmp_name);
            mv_package_root ();
            break;
        }
    }

    close (fd);

    if (integrity_error != 1)
    {
        mv_parent ();
        unlink ((char *) receipt->name);
        rename ((char *) tmp_name, (char *) receipt->name);
        mv_package_root ();
    }


    free (block_buffer);
}

/**
    Concatenate the hashes of each `file block` calculate
    their hash.
*/
void set_receipt_hash (Receipt *receipt)
{
    int i;
    int n_blocks = receipt->size;
    unsigned char *hash_buffer = malloc (sizeof (unsigned char) * 32 * n_blocks);
    Block *block;

    i = 0;
    for ( block = receipt->blocks->head;
          block != NULL; 
          block = block->next )
    {
        memcpy (&hash_buffer[i * 32], block->hash, 32);
        i++;
    }

    sha256 (receipt->hash, hash_buffer, n_blocks * 32);
    free (hash_buffer);
}

/**
    Create a receipt structure and flush the blocks on the .package
    dir.
*/
void receipt_create ( Receipt *receipt,
                      unsigned char *file_path,
                      unsigned int block_size )
{
    int i, fd, len;
    unsigned char *buffer = malloc (sizeof (unsigned char) * block_size);
    Block *block;

    mv_parent ();
    fd = open_file(file_path, READ_PERM);
    mv_package_root ();

    strcpy ((char *) receipt->name, (char *) file_path);
    receipt->size = file_size (fd) / block_size;
    receipt->block_size = block_size;
    receipt->blocks = block_list_new ();

    for (i = 0; i < receipt->size; i++)
    {
        block = malloc(sizeof(Block));   
        len = fill_buffer (fd, buffer, block_size);
        buffer2block (block, buffer, len);
        block_list_add (receipt->blocks, block);
    }
    set_receipt_hash (receipt);
    close (fd);
    free (buffer);
}

void write_receipt_header (int fd, Receipt *receipt)
{
    write (fd, receipt->hash, 32);
    write (fd, receipt->name, FNAME_LEN);
    write (fd, &receipt->size, sizeof (int));
    write (fd, &receipt->block_size, sizeof (int));
}

void write_receipt_blocks ( int fd, 
                            Receipt *receipt )
{
    Block *block;

    for ( block = receipt->blocks->head; 
          block != NULL; 
          block = block->next ) 
    {
        write (fd, block->hash, 32);
    }
}

void read_receipt_header ( int fd, 
                           Receipt *receipt )
{
    read (fd, receipt->hash, 32);
    read (fd, receipt->name, FNAME_LEN);
    read (fd, &receipt->size, sizeof (int));
    read (fd, &receipt->block_size, sizeof (int));
    receipt->blocks = malloc (sizeof (BlockList));
}

void read_receipt_blocks ( int fd, 
                           Receipt *receipt)
{
    int i;
    Block *block;

    for (i = 0; i < receipt->size; i++) {
        block = malloc (sizeof (Block));
        read (fd, block->hash, 32);
        block->size = receipt->block_size;
        block_list_add (receipt->blocks, block);
    }
}

void receipt_unpack ( Receipt *receipt,
                      int skip_integrity_flag )
{
    if (skip_integrity_flag)
        recover_original_file (receipt);
    else
        recover_original_file_i (receipt);
}

void receipt_store ( Receipt *receipt )
{
    int i, fd;
    unsigned char name[SHA256_STRING];

    mv_package_receipts ();
    fd = open (".receipt", O_RDWR | O_CREAT, 0666);
    mv_parent ();

    write_receipt_header (fd, receipt);
    write_receipt_blocks (fd, receipt);
    close (fd);
}

void receipt_fetch ( Receipt *receipt )
{
    int fd;

    mv_package_receipts ();
    fd = open (".receipt", O_RDONLY);
    mv_parent ();

    read_receipt_header (fd, receipt);
    read_receipt_blocks (fd, receipt);
    close (fd);
}