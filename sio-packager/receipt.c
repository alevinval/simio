#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sha256.h"
#include "receipt.h"
#include "util.h"

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
    int i, fd, readed_bytes;    
    unsigned char block_name[SHA256_STRING];
    unsigned char *blk_buffer;

    chdir ("..");
    fd = open (receipt->name, O_RDWR | O_CREAT, 0777);
    chdir (DIR_PACKAGE);

    if (fd == -1)
        die ("recover_original_file: cannot create the original file");
    blk_buffer = malloc (sizeof (unsigned char) * receipt->block_size);
    for (i = 0; i < receipt->size; i++)
    {
        sha2hexf (block_name, receipt->blocks[i].hash);
        readed_bytes = read_block_to_buffer (block_name, blk_buffer);
        write (fd, blk_buffer, readed_bytes);
    }
    free (blk_buffer);
}

void recover_original_file_i (Receipt *receipt)
{
    int i, fd, readed_bytes, integrity_error = 0;
    unsigned char tmp_name[FNAME_LEN];
    unsigned char block_name[SHA256_STRING];
    unsigned char *blk_buffer;

    strcpy (tmp_name, "");
    strcat (tmp_name, receipt->name);
    strcat (tmp_name, ".tmp");

    chdir ("..");
    fd = open (tmp_name, O_RDWR | O_CREAT, 0777);
    chdir (DIR_PACKAGE);
    
    blk_buffer = malloc (sizeof (unsigned char) * receipt->block_size);
    for (i = 0; i < receipt->size; i++)
    {
        sha2hexf (block_name, receipt->blocks[i].hash);
        readed_bytes = read_block_to_buffer (block_name, blk_buffer);
        if (check_block_integrity(&receipt->blocks[i], blk_buffer, readed_bytes) == 1) 
        {
            write (fd, blk_buffer, readed_bytes);
        } else {
            printf("Integrity failure on block %i\n", i);
            i = receipt->size+1;
            integrity_error = 1;
            unlink (tmp_name);
            close (fd);
        }
    }

    if (integrity_error == 0)
    {
        unlink (receipt->name);
        rename (tmp_name, receipt->name);
    }

    close (fd);
    free (blk_buffer);
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
void create_receipt (Receipt *receipt, unsigned char *path, unsigned int blk_size)
{
    int i;
    int fd;
    int len;
    unsigned char *buffer = malloc (sizeof (unsigned char) * blk_size);
    Block block;

    chdir ("..");
    fd = open_file(path, READ_PERM); 
    chdir (DIR_PACKAGE);

    // Set receipt name
    strcpy ((char *) receipt->name, path);
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

void write_receipt_blocks (int fd, Receipt *receipt)
{
    int i;
    for (i = 0; i < receipt->size; i++)
        write (fd, receipt->blocks[i].hash, 32);
}

void read_receipt_header (int fd, Receipt *receipt) {
    read (fd, receipt->hash, 32);
    read (fd, receipt->name, FNAME_LEN);
    read (fd, &receipt->size, sizeof (int));
    read (fd, &receipt->block_size, sizeof (int));
}

void read_receipt_blocks (int fd, Receipt *receipt) {
    int i;
    receipt->blocks = malloc (sizeof (Block) * receipt->size);
    for (i = 0; i < receipt->size; i++)
        read (fd, &receipt->blocks[i].hash, 32);
}

void recover_receipt (int fd, Receipt *receipt, int skip_integrity_flag)
{
    read_receipt_header (fd, receipt);
    read_receipt_blocks (fd, receipt);
    if (skip_integrity_flag == 1)
        recover_original_file (receipt);
    else
        recover_original_file_i (receipt);
    
}

void store_receipt (Receipt *receipt)
{
    int i, fd;
    unsigned char name[SHA256_STRING];

    chdir (DIR_RECEIPTS);
    fd = open (".receipt", O_RDWR | O_CREAT, 0777);
    chdir ("..");

    write_receipt_header (fd, receipt);
    write_receipt_blocks (fd, receipt);

    close (fd);
}