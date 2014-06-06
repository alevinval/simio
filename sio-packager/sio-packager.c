#include <stdlib.h>
#include <string.h>

#include "package.h"
#include "receipt.h"
#include "util.h"

void pack(unsigned char *file_path, unsigned int block_size)
{
	struct receipt *receipt = calloc(1, sizeof(struct receipt));

	package_prepare();
	create_receipt(receipt, file_path, block_size);
	store_receipt(receipt);
	free_receipt(receipt);
}

void unpack(int flag_skip_integrity)
{
	struct receipt *receipt = calloc(1, sizeof(struct receipt));

	package_prepare();
	fetch_receipt(receipt);
	unpack_receipt(receipt, flag_skip_integrity);
	free_receipt(receipt);
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		die("Usage: \nsio-packager pack|unpack file_name --skip-integrity");
	}

	if (strcmp(argv[1], "pack") == 0) {
		pack((unsigned char *)argv[2], 16);
	} else if (strcmp(argv[1], "unpack") == 0) {
		int skip_integrity = 0;
		if (argc == 4)
			if (strcmp(argv[3], "--skip-integrity") == 0)
				skip_integrity = 1;
		unpack(skip_integrity);
	}

	return 0;
}
