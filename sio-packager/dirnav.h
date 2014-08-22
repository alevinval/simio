#ifndef DIRNAV_H
#define DIRNAV_H

#include <iostream>
#include <fstream>
#include <string>

void mv_package_root ();

void mv_package_blocks ();

void mv_package_receipts ();

void mv_parent ();

int open_block (const std::string &name);

int open_create_block (const std::string &name);

void delete_block (const std::string &name);

int open_receipt (const unsigned char *receipt_name);

int open_create_receipt (const unsigned char *receipt_name);

int open_file (const unsigned char *file_name);

int open_create_file (const unsigned char *file_name);

void delete_file (const unsigned char *file_name);

void remove_file (const unsigned char *file_name);

void rename_file (const unsigned char *old_name, const unsigned char *new_name);

#endif //DIRNAV_H

