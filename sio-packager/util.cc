#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else

#include <unistd.h>

#endif

#include <sys/stat.h>

#include <cstdio>

#include <stdarg.h>
#include <stdlib.h>
#include "util.h"

long file_size(int fd)
{
    struct stat s;
    fstat(fd, &s);
    return s.st_size;
}

/**
Reads a block of data from a file, puts the data in a buffer
*/
int fill_buffer(int fd, unsigned char *buffer, int blk_size)
{
    return read(fd, buffer, blk_size);
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
