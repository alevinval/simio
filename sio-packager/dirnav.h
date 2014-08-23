#ifndef DIRNAV_H
#define DIRNAV_H

#include <iostream>
#include <fstream>

void mv_package_root();

void mv_package_blocks();

void mv_package_receipts();

void mv_parent();

int open_block(const std::string &name);

int open_create_block(const std::string &name);

void delete_block(const std::string &name);

int open_receipt(const std::string &name);

int open_create_receipt(const std::string &name);

int open_file(const std::string &name);

int open_create_file(const std::string &name);

void delete_file(const std::string &name);

void remove_file(const std::string &name);

void rename_file(const std::string &old_name, const std::string &new_name);

#endif // DIRNAV_H
