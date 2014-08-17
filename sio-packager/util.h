#ifndef UTIL_H
#define UTIL_H

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#define O_BINARY 0

#include <unistd.h>

#endif

#include <stdarg.h>

int error (const char *err, ...);

long file_size (int fd);

void die (const char *err, ...);

void usage (const char *err);

int fill_buffer (int fd, unsigned char *buffer, int blk_size);

#endif /**  UTIL_H */
