#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "receipt.h"
#include "packager.h"
#include "file_block.h"
#include "sha256.h"

long file_size (int fd)
{
    off_t fsize;
    fsize = lseek (fd, 0, SEEK_END);
    lseek (fd, 0, SEEK_SET);
    return fsize;
}

void init_package ()
{
    mkdir (".package", 0777);
    chdir (".package");
}

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

void blocks_hash (Receipt *receipt)
{
    int i;
    int blocks = receipt->size;
    unsigned char *hash_buffer = malloc (sizeof (unsigned char) * 32 * blocks);

    for (i = 0; i < blocks; i++)
        memcpy (&hash_buffer[i * 32], receipt->blocks[i].hash, 32);

    sha256 (receipt->hash, hash_buffer, blocks * 32);
}

int fill_buffer (unsigned char *buffer, int fd, int blk_size)
{
    // Read from file a maximum of block size bytes
    return read (fd, buffer, blk_size);
}

/**
    Persist the block by it's SHA hash, don't care about the current dir,
    persisting the fileblock whichever our current Dir is.
*/
void persist_block (FileBlock *block, unsigned char *data_buffer)
{
    int fd;
    unsigned char name[SHA256_STRING];

    sha2hexf (name, block->hash);
    fd = open ((char *) name, O_RDWR | O_CREAT, 0777);
    write (fd, data_buffer, block->size);
    close (fd);
}

void fill_block_data (FileBlock *block, unsigned char *buffer, int len)
{
    // Set block data
    block->size = len;
    sha256 (block->hash, buffer, block->size);

    // Persist the block
    persist_block (block, buffer);
    free (block->data);
}

/**
    Create a receipt structure and flush the data Blocks on the .package
*/
void create_receipt (Receipt *receipt, int fd, char *path,
                     unsigned int blk_size)
{
    int i;
    int len;
    unsigned char *data_buffer = malloc (sizeof (unsigned char) * blk_size);
    FileBlock block;

    // Set receipt name
    strcpy ((char *) receipt->name, path);
    // Set receipt size in blocks
    receipt->size = file_size (fd) / blk_size + 1;
    receipt->block_size = blk_size;
    // Allocate space for blocks
    receipt->blocks = malloc (sizeof (FileBlock) * receipt->size );

    // Fill blocks with file data
    for (i = 0; i < receipt->size; i++)
    {
        // Store data in buffer
        len = fill_buffer (data_buffer, fd, blk_size);
        // Put data on block
        fill_block_data (&block, data_buffer, len);
        receipt->blocks[i] = block;
    }

    // Finally set the receipt hash
    blocks_hash (receipt);
}

void persist_receipt (Receipt *receipt, char *path)
{
    int i, fd;
    unsigned char name[SHA256_STRING];

    fd = open (".receipt", O_RDWR | O_CREAT, 0777);

    // Write receipt header
    write (fd, receipt->hash, 32);
    write (fd, receipt->name, 256);
    write (fd, &receipt->size, sizeof (int));
    write (fd, &receipt->block_size, sizeof (int));

    // Write the block chain
    for (i = 0; i < receipt->size; i++)
    {
        //sha2hexf (name, receipt->blocks[i].hash);
        write (fd, receipt->blocks[i].hash, 32);
    }

    close (fd);
}

void pack (char *path, unsigned int blk_size)
{
    int i;
    int fd = open (path, O_RDONLY);
    Receipt receipt;

    init_package ();
    create_receipt (&receipt, fd, path, blk_size);
    persist_receipt (&receipt, path);
    chdir ("..");
}


void recover_receipt (int fd, Receipt *receipt)
{
    int i;
    int block_fd;
    int file_fd;
    long block_size;
    unsigned char *data_buffer;
    unsigned char *block_name[SHA256_STRING];

    read (fd, receipt->hash, 32);
    read (fd, receipt->name, 256);
    read (fd, &receipt->size, sizeof (int));
    read (fd, &receipt->block_size, sizeof (int));

    receipt->blocks = malloc (sizeof (FileBlock) * receipt->size);
    data_buffer = malloc (sizeof (unsigned char) * receipt->block_size);

    for (i = 0; i < receipt->size; i++)
        read (fd, &receipt->blocks[i].hash, 32);

    file_fd = open ( receipt->name, O_RDWR | O_CREAT, 0777);

    // Merge blocks
    for (i = 0; i < receipt->size; i++)
    {
        sha2hexf (block_name, receipt->blocks[i].hash);
        block_fd = open (block_name, O_RDONLY);
        block_size = file_size (block_fd);
        read (block_fd, data_buffer, block_size);
        write (file_fd, data_buffer, block_size);
        close (block_fd);
    }
    close (file_fd);
    close (fd);
}

/**     Receipt path */
void recover (char *path)
{
    chdir (".package");
    int fd = open (".receipt", O_RDONLY);

    Receipt receipt;
    recover_receipt (fd, &receipt);

    close (fd);
}
