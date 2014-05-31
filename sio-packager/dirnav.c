#include <unistd.h>

#include "util.h"
#include "dirnav.h"
#include "dir.h"

void
mv_package_root ()
{
    chdir (DIR_PACKAGE);
}

void
mv_package_blocks ()
{
    chdir (DIR_BLOCKS);
}

void
mv_package_receipts ()
{
    chdir (DIR_RECEIPTS);
}

void
mv_parent ()
{
    chdir ("..");
}

int
open_block (unsigned char *block_name)
{
    int fd;

    mv_package_blocks ();
    fd = open ((char *) block_name, O_RDONLY);
    mv_parent ();

    if (fd == -1)
        die ("open_block: cannot open requested file");

    return fd;
}

int
open_create_block (unsigned char *block_name)
{
    int fd;

    mv_package_blocks ();
    fd = open ((char *) block_name, O_RDWR | O_CREAT, 0666);
    mv_parent ();

    if (fd == -1)
        die ("open_create_block: cannot open or create requested file");

    return fd;
}

int
open_receipt (unsigned char *receipt_name)
{
    int fd;

    mv_package_receipts ();
    fd = open ((char *) receipt_name, O_RDONLY);
    mv_parent ();

    if (fd == -1)
        die ("open_receipt: cannot open requested file");

    return fd;
}

int
open_create_receipt (unsigned char *receipt_name)
{
    int fd;

    mv_package_receipts ();
    fd = open ((char *) receipt_name, O_RDWR | O_CREAT, 0666);
    mv_parent ();

    if (fd == -1)
        die ("open_create_receipt: cannot open or create requested file");

    return fd;
}

int
open_file (unsigned char *file_name)
{
    int fd;

    mv_parent ();
    fd = open ((char *) file_name, READ_PERM);
    mv_package_root ();

    if (fd == -1)
        die ("open_file: cannot open requested file");

    return fd;
}

int
open_create_file (unsigned char *file_name)
{
    int fd;

    mv_parent ();
    fd = open ((char *) file_name, O_RDWR | O_CREAT, 0666);
    mv_package_root ();

    if (fd == -1)
        die ("open_create_file: cannot open or create the requested file");

    return fd;
}

void
remove_file (unsigned char *file_name)
{
    mv_parent ();
    unlink ((char *) file_name);
    mv_package_root ();
}