#ifndef DIRNAV_H
#define DIRNAV_H

void mv_package_root ();
void mv_package_blocks ();
void mv_package_receipts ();
void mv_parent ();

int open_block ( unsigned char *block_name );
int open_create_block ( unsigned char *block_name );

int open_receipt ( unsigned char *receipt_name );
int open_create_receipt ( unsigned char *receipt_name );

int open_file ( unsigned char *file_name );
int open_create_file ( unsigned char *file_name );

#endif /**  DIRNAV_H */