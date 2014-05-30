#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <fcntl.h>

#define READ_PERM O_RDONLY

#define FNAME_LEN 255
#define DIR_LEN 255

#define DIR_PACKAGE ".package"
#define DIR_BLOCKS "blocks"
#define DIR_RECEIPTS "receipts"
#define DIR_SYNC "sync"
#define DIR_PERM 0777

static 
void report(const char *prefix, const char *err, va_list params);

int error(const char *err, ...);
int open_file (unsigned char *path, int flags);
long file_size (int fd);
void die(const char *err, ...);
void usage(const char *err);

int fill_buffer (int fd, unsigned char *buffer, int blk_size);

#endif /**  UTIL_H */