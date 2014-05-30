#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "util.h"
#include "packager.h"
#include "receipt.h"
#include "block.h"
#include "sha256.h"

void mv_package_root () {
    chdir (DIR_PACKAGE);
}

void mv_package_receipts () {
    chdir (DIR_RECEIPTS);
}

void mv_package_previous () {
    chdir ("..");
}

int package_create () {
    return mkdir (DIR_PACKAGE, DIR_PERM);
}

void package_initialize () {
    printf("Initialising empty .package/\n");
    chdir (DIR_PACKAGE);    
    mkdir (DIR_BLOCKS, DIR_PERM);
    mkdir (DIR_RECEIPTS, DIR_PERM);
    mkdir (DIR_SYNC, DIR_PERM);    
}

void package_prepare ()
{
    if ( package_create () == 0 ) {        
        package_initialize ();
    }    
    mv_package_root ();
}

void pack ( char * file_path, 
            unsigned int block_size )
{
    int fd;
    Receipt receipt;

    package_prepare ();
    create_receipt (&receipt, file_path, block_size);
    store_receipt (&receipt);
}

void unpack (   char * file_path,
                int skip_integrity_flag )
{
    mv_package_root();
    mv_package_receipts();
    int fd = open (".receipt", O_RDONLY);
    mv_package_previous();

    Receipt receipt;
    recover_receipt (fd, &receipt, skip_integrity_flag);

    close (fd);
}