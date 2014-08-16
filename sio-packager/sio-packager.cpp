#include <stdlib.h>
#include <string.h>

#include "package.h"
#include "receipt.h"
#include "util.h"

void pack(unsigned char *file_path, int block_size)
{
	Receipt receipt = Receipt(file_path, block_size);
	receipt.pack();
}

void unpack(bool skip_integrity)
{
	Receipt receipt = Receipt((unsigned char *)".receipt");
	receipt.unpack(skip_integrity);
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		die("Usage: \nsio-packager pack|unpack file_name --skip-integrity");
	}

	Package *package = new Package();
	package->prepare();

	if (strcmp(argv[1], "pack") == 0) {
		pack((unsigned char *)argv[2], 9);
	}
	else if (strcmp(argv[1], "unpack") == 0) {
		bool skip_integrity = false;
		if (argc == 4)
		if (strcmp(argv[3], "--skip-integrity") == 0)
			skip_integrity = true;
		unpack(skip_integrity);
	}

	return 0;
}