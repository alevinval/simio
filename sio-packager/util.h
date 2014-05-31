#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>

static 
void report(const char *prefix, const char *err, va_list params);

int error(const char *err, ...);
long file_size (int fd);
void die(const char *err, ...);
void usage(const char *err);

int fill_buffer (int fd, unsigned char *buffer, int blk_size);

#endif /**  UTIL_H */