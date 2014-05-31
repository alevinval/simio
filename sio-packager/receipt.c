#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sha256.h"
#include "receipt.h"
#include "dirnav.h"
#include "dir.h"
#include "util.h"
#include "block.h"

void dump_receipt (Receipt *receipt)
{
    int i;
    int blocks = receipt->size;

    printf ("\n== RECEIPT DUMP ==\n");
    printf ("Name: %s\n", receipt->name);
    printf ("Hash: "); sha2hex(receipt->hash);
    printf ("Size: %i\n\n", receipt->size);
    printf ("== BLOCK DUMP == \n");
    for (i = 0; i < blocks; i++)
    {
        printf ("[ Block %i ]\n", i);
        dump_block (receipt->blocks[i]); printf ("\n\n");
    }
}

void recover_original_file (Receipt *receipt)
{
    int i, fd, block_size;
    unsigned char block_name[SHA256_STRING];
    unsigned char *block_buffer;

    mv_parent ();
    fd = open ((char *) receipt->name, O_RDWR | O_CREAT, 0777);
    mv_package_root ();

    if (fd == -1)
        die ("recover_original_file: cannot create the original file");

    block_buffer = malloc (sizeof (unsigned char) * receipt->block_size);
    for (i = 0; i < receipt->size; i++)
    {
        sha2hexf (block_name, receipt->blocks[i].hash);
        block_size = block2buffer (block_name, block_buffer);
        write (fd, block_buffer, block_size);
    }
    free (block_buffer);
}

void recover_original_file_i (Receipt *receipt)
{
    int i, fd, block_size, integrity_error = 0;
    unsigned char tmp_name[FNAME_LEN];
    unsigned char block_name[SHA256_STRING];
    unsigned char *block_buffer;

    strcpy ((char *) tmp_name, "");
    strcat ((char *) tmp_name, (char *) receipt->name);
    strcat ((char *) tmp_name, ".tmp");

    mv_parent ();
    fd = open ((char *) tmp_name, O_RDWR | O_CREAT, 0777);
    mv_package_root ();

    if (fd == -1)
        die ("recover_original_file_i: cannot create the original file");

    block_buffer = malloc (sizeof (unsigned char) * receipt->block_size);
    for (i = 0; i < receipt->size; i++)
    {
        sha2hexf (block_name, receipt->blocks[i].hash);
        block_size = block2buffer (block_name, block_buffer);
        if (check_block_integrity(&receipt->blocks[i], block_buffer, block_size) == 1)
        {
            write (fd, block_buffer, block_size);
        } else {
            unsigned char * block_hash_hex;
            sha2hexf(block_hash_hex, receipt->blocks[i].hash);
            error ("integrity failure in Block %i [ %s ] \n", i, block_hash_hex);
            integrity_error = 1;
            unlink ((char *) tmp_name);
            close (fd);
            break;
        }
    }

    if (!integrity_error)
    {
        unlink ((char *) receipt->name);
        rename ((char *) tmp_name, (char *) receipt->name);
    }

    close (fd);
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

    for (i = 0; i < n_blocks; i++)
        memcpy (&hash_buffer[i * 32], receipt->blocks[i].hash, 32);
    sha256 (receipt->hash, hash_buffer, n_blocks * 32);

    free (hash_buffer);
}

/**
    Create a receipt structure and flush the blocks on the .package
    dir.
*/
void receipt_create (   Receipt *receipt,
                        unsigned char *file_path,
                        unsigned int blk_size )
{
    int i;
    int fd;
    int len;
    unsigned char *buffer = malloc (sizeof (unsigned char) * blk_size);
    Block block;

    chdir ("..");
    fd = open_file(file_path, READ_PERM);
    chdir (DIR_PACKAGE);

    // Set receipt name
    strcpy ((char *) receipt->name, (char *) file_path);
    // Set receipt size in blocks
    receipt->size = file_size (fd) / blk_size + 1;
    // Set the used block size
    receipt->block_size = blk_size;
    // Allocate space for blocks
    receipt->blocks = malloc (sizeof (Block) * receipt->size);
    // Fill blocks with file data
    for (i = 0; i < receipt->size; i++)
    {
        // Store data in buffer
        len = fill_buffer (fd, buffer, blk_size);
        // Put data on block
        buffer2block (&block, buffer, len);
        // Store block on receipt for further processing
        receipt->blocks[i] = block;
    }
    // Set the receipt hash
    set_receipt_hash (receipt);

    free(buffer);
}

void write_receipt_header (int fd, Receipt *receipt)
{
    write (fd, receipt->hash, 32);
    write (fd, receipt->name, FNAME_LEN);
    write (fd, &receipt->size, sizeof (int));
    write (fd, &receipt->block_size, sizeof (int));
}

void write_receipt_blocks ( int fd, Receipt *receipt )
{
    int i;
    for (i = 0; i < receipt->size; i++)
        write (fd, receipt->blocks[i].hash, 32);
}

void read_receipt_header ( int fd, Receipt *receipt )
{
    read (fd, receipt->hash, 32);
    read (fd, receipt->name, FNAME_LEN);
    read (fd, &receipt->size, sizeof (int));
    read (fd, &receipt->block_size, sizeof (int));
}

void read_receipt_blocks (int fd, Receipt *receipt)
{
    int i;
    receipt->blocks = malloc (sizeof (Block) * receipt->size);
    for (i = 0; i < receipt->size; i++)
        read (fd, &receipt->blocks[i].hash, 32);
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
    fd = open (".receipt", O_RDWR | O_CREAT, 0777);
    mv_parent ();

    write_receipt_header (fd, receipt);
    write_receipt_blocks (fd, receipt);
    close (fd);
}

void receipt_fetch ( Receipt *receipt )
{
    mv_package_receipts ();
    int fd = open (".receipt", O_RDONLY);
    mv_parent ();

    read_receipt_header (fd, receipt);
    read_receipt_blocks (fd, receipt);
    close (fd);
}