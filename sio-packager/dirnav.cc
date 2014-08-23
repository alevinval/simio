#include <fcntl.h>
#include <cstdio>

#include "util.h"
#include "dirnav.h"
#include "dir.h"

void mv_package_root ()
{
    chdir (DIR_PACKAGE);
}

void mv_package_blocks ()
{
    chdir (DIR_BLOCKS);
}

void mv_package_receipts ()
{
    chdir (DIR_RECEIPTS);
}

void mv_parent ()
{
    chdir ("..");
}

int open_block (const std::string &name)
{
    int fd;

    mv_package_blocks ();
    fd = open (name.c_str(), O_RDONLY | O_BINARY);
    mv_parent ();

    if (fd < 0)
        die ("open_block: cannot open requested block\n[%s]\n", name.c_str());

    return fd;
}

void delete_block (const std::string &name)
{
    mv_package_blocks ();
    unlink (name.c_str());
    mv_parent ();
}

int open_create_block (const std::string &name)
{	
    int fd;

    mv_package_blocks ();
    fd = open (name.c_str(), O_RDWR | O_CREAT | O_BINARY, 0666);
    mv_parent ();

    if (fd < 0)
        die ("open_create_block: cannot open requested block\n[%s]\n",
			 name.c_str());
    return fd;
}

int
open_receipt(const std::string &name)
{
    int fd;

    mv_package_receipts ();

    fd = open (name.c_str(), O_RDONLY | O_BINARY);
    mv_parent ();

    if (fd < 0)
        die ("open_receipt: cannot open requested receipt \"%s\"\n",
			 name.c_str());

    return fd;
}

int open_create_receipt(const std::string &name)
{
    int fd;

    mv_package_receipts ();
    fd = open (name.c_str(), O_RDWR | O_CREAT | O_BINARY, 0666);
    mv_parent ();

    if (fd < 0)
		die("open_create_receipt: cannot create requested receipt \"%s\"\n", name.c_str());

    return fd;
}

int open_file(const std::string &name)
{
    int fd;

    mv_parent ();
    fd = open (name.c_str(), READ_PERM | O_BINARY);
    mv_package_root ();

    if (fd < 0)
		die("open_file: cannot open requested file \"%s\"\n", name.c_str());

    return fd;
}

void delete_file(const std::string &name)
{
    mv_parent ();
    unlink (name.c_str());
    mv_package_root ();
}

void rename_file (const std::string &old_name, const std::string &new_name)
{
    mv_parent ();
    rename (old_name.c_str(), new_name.c_str());
    mv_package_root ();
}

int open_create_file(const std::string &name)
{
    int fd;

    mv_parent ();
    fd = open (name.c_str(), O_RDWR | O_CREAT | O_BINARY, 0666);
    mv_package_root ();

    if (fd < 0)
        die ("open_create_file: cannot create requested file \"%s\"\n",
			 name.c_str());

    return fd;
}

void remove_file(const std::string &name)
{
    mv_parent ();
    unlink (name.c_str());
    mv_package_root ();
}
