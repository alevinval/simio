#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "util.h"
#include "dirnav.h"
#include "dir.h"

void mv_package_root()
{
	chdir(DIR_PACKAGE);
}

void mv_package_blocks()
{
	chdir(DIR_BLOCKS);
}

void mv_package_receipts()
{
	chdir(DIR_RECEIPTS);
}

void mv_parent()
{
	chdir("..");
}

int open_block(unsigned char *name)
{
	int fd;

	mv_package_blocks();
	fd = open((char *)name, O_RDONLY);
	mv_parent();

	if (fd < 0)
		die("open_block: cannot open requested block\n[%s]\n", name);

	return fd;
}

void delete_block(unsigned char *name)
{
	mv_package_blocks();
	unlink((char *)name);
	mv_parent();
}

int open_create_block(unsigned char *name)
{
	int fd;

	mv_package_blocks();
	fd = open((char *)name, O_RDWR | O_CREAT, 0666);
	mv_parent();

	if (fd < 0)
		die("open_create_block: cannot open requested block\n[%s]\n",
		    name);
	return fd;
}

int open_receipt(unsigned char *name)
{
	int fd;

	mv_package_receipts();
	fd = open((char *)name, O_RDONLY);
	mv_parent();

	if (fd < 0)
		die("open_receipt: cannot open requested receipt \"%s\"\n",
		    name);

	return fd;
}

int open_create_receipt(unsigned char *name)
{
	int fd;

	mv_package_receipts();
	fd = open((char *)name, O_RDWR | O_CREAT, 0666);
	mv_parent();

	if (fd < 0)
		die("open_create_receipt: cannot create requested receipt \"%s\"\n", name);

	return fd;
}

int open_file(unsigned char *name)
{
	int fd;

	mv_parent();
	fd = open((char *)name, READ_PERM);
	mv_package_root();

	if (fd < 0)
		die("open_file: cannot open requested file \"%s\"\n", name);

	return fd;
}

void delete_file(unsigned char *file_name)
{
	mv_parent();
	unlink((char *)file_name);
	mv_package_root();
}

void rename_file(unsigned char *old_name, unsigned char *new_name)
{
	mv_parent();
	rename((char *)old_name, (char *)new_name);
	mv_package_root();
}

int open_create_file(unsigned char *name)
{
	int fd;

	mv_parent();
	fd = open((char *)name, O_RDWR | O_CREAT, 0666);
	mv_package_root();

	if (fd < 0)
		die("open_create_file: cannot create requested file \"%s\"\n",
		    name);

	return fd;
}

void remove_file(unsigned char *name)
{
	mv_parent();
	unlink((char *)name);
	mv_package_root();
}
