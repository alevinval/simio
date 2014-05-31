#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

long file_size (int fd)
{
    off_t fsize;
    fsize = lseek (fd, 0, SEEK_END);
    lseek (fd, 0, SEEK_SET);
    return fsize;
}

/**
    Reads a block of data from a file, puts the data in a buffer
*/
int fill_buffer (int fd, unsigned char *buffer, int blk_size)
{
    return read (fd, buffer, blk_size);
}

static void report(const char *prefix, const char *err, va_list params)
{
	fputs(prefix, stderr);
	vfprintf(stderr, err, params);
	fputs("\n", stderr);
}

void usage(const char *err)
{
	fprintf(stderr, "usage: %s\n", err);
	exit(1);
}

void die(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	report("fatal: ", err, params);
	va_end(params);
	exit(1);
}

int error(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	report("error: ", err, params);
	va_end(params);
	return -1;
}

int open_file (unsigned char *path, int flags)
{
	int fd;

	fd = open ((const char *)path, flags);
	if (fd == -1)
		die ("open_file: cannot open file '%s', reason: %s\n", path, strerror(errno));
	return fd;
}
