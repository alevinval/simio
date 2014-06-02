#include <string.h>

#include "package.h"
#include "receipt.h"
#include "util.h"

void pack(char *file_path, unsigned int block_size)
{
	Receipt receipt;

	package_prepare();
	create_receipt(&receipt, (unsigned char *)file_path, block_size);
	store_receipt(&receipt);
}

void unpack(char *file_path, int flag_skip_integrity)
{
	Receipt receipt;

	package_prepare();
	fetch_receipt(&receipt);
	unpack_receipt(&receipt, flag_skip_integrity);
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		die("Usage: \nsio-packager pack|unpack file_name --skip-integrity");
	}

	if (strcmp(argv[1], "pack") == 0) {
		pack(argv[2], 16);
	} else if (strcmp(argv[1], "unpack") == 0) {
		int skip_integrity = 0;
		if (argc == 4)
			if (strcmp(argv[3], "--skip-integrity") == 0)
				skip_integrity = 1;
		unpack(argv[2], skip_integrity);
	}

	return 0;
}
