#ifndef SIO_PACKAGER_H
#define SIO_PACKAGER_H

void pack(unsigned char *file_path, unsigned int block_size);
void unpack(int flag_skip_integrity);

#endif /**  SIO_PACKAGER_H */
