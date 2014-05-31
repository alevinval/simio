#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "util.h"
#include "sha256.h"
#include "block.h"
#include "dir.h"

void dump_block( Block block) {	
	printf("Hash: "); sha2hex(block.hash);
	printf("Length: %i", block.size);	
}

int check_block_integrity (Block *block, unsigned char *buffer, int len)
{
    int i;
    unsigned char match_sha[32];

    sha256(match_sha, buffer, len);
    for (i = 0; i < 32; i++)
        if (match_sha[i] != block->hash[i])
            return -1;

    return 1;
}

/**
    Store the block in a file with the name being the SHA of the block.
    Block for the metadata, buffer for the real data. Block->data field
    is empty, so we keep RAM usage low for big files.
*/
void write_block (Block *block, unsigned char *data)
{
    int fd;
    unsigned char name[SHA256_STRING];

    sha2hexf (name, block->hash);

    chdir (DIR_BLOCKS);
    fd = open ((char *) name, O_RDWR | O_CREAT, 0777);
    chdir ("..");

    if (fd < 0)
        die ("write_block: cannot open file");
    write (fd, data, block->size);
    close (fd);
}

/**
    Given a block in a buffer, obtain the size, the sha identifier
    and store the block as well.
*/
void buffer2block (Block *block, unsigned char *buffer, int len)
{
    // Set block data
    block->size = len;
    sha256 (block->hash, buffer, block->size);

    // Persist the block
    write_block (block, buffer);
}

int block2buffer (unsigned char *block_name, unsigned char *buffer)
{
    int fd;
    long block_size;

    chdir (DIR_BLOCKS);
    fd = open ((char *) block_name, O_RDONLY);
    chdir ("..");
    
    if (fd == -1)
        die ("block2buffer: cannot open block file");

    block_size = file_size (fd);
    read (fd, buffer, block_size);
    close (fd);
    
    return block_size;
}
