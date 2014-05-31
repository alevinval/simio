#ifndef DIR_H
#define DIR_H

#include <sys/stat.h> 
#include <fcntl.h>

#define READ_PERM O_RDONLY
#define FNAME_LEN 255
#define DIR_LEN 255

#define DIR_PACKAGE ".package"
#define DIR_BLOCKS "blocks"
#define DIR_RECEIPTS "receipts"
#define DIR_SYNC "sync"
#define DIR_PERM 0777

#endif