#ifndef SIO_PACKAGER_H
#define SIO_PACKAGER_H

void pack(char *file_path, unsigned int block_size);
void unpack(char *file_path, int flag_skip_integrity);

#endif /**  SIO_PACKAGER_H */
