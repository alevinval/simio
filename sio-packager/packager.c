#include <stdio.h>
#include <unistd.h>

#include "packager.h"
#include "receipt.h"
#include "dirnav.h"
#include "dir.h"

int package_create ()
{
    return mkdir (DIR_PACKAGE, DIR_PERM);
}

void package_initialize () 
{
    printf ("Initialising empty .package/\n");
    chdir (DIR_PACKAGE);
    mkdir (DIR_BLOCKS, DIR_PERM);
    mkdir (DIR_RECEIPTS, DIR_PERM);
    mkdir (DIR_SYNC, DIR_PERM);
}

void package_prepare ()
{
    if (!package_create ()) {
        package_initialize ();
    }    
    mv_package_root ();
}

void pack ( char * file_path, 
            unsigned int block_size )
{
    Receipt receipt;

    package_prepare ();
    receipt_create (&receipt, (unsigned char *) file_path, block_size);
    receipt_store (&receipt);
}

void unpack ( char *file_path,
              int flag_skip_integrity )
{
    Receipt receipt;
    
    package_prepare();
    receipt_fetch (&receipt);
    receipt_unpack (&receipt, flag_skip_integrity);
}