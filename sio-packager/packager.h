#ifndef PACKAGER_H
#define PACKAGER_H

void pack (char * file_path, unsigned int block_size);
void unpack (char * path, int skip_integrity_flag);

#endif /**  PACKAGER_H */