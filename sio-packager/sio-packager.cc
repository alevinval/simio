#include <string.h>

#include "package.h"
#include "receipt.h"
#include "util.h"

#define BLOCK_4MB 4000000

void pack (std::string file_path, int block_size)
{
    Receipt receipt = Receipt ();
    receipt.create(file_path, block_size);
    receipt.pack ();
}

void unpack (bool skip_integrity)
{
    Receipt receipt = Receipt ();
    receipt.open(".receipt");
    receipt.unpack (skip_integrity);
}

int main (int argc, char *argv[])
{
    if (argc < 3) {
        die ("Usage: \nsio-packager pack|unpack file_name --skip-integrity");
    }

    Package package = Package();
    package.prepare ();

    if (strcmp (argv[1], "pack") == 0) {
        pack (std::string(argv[2]), BLOCK_4MB);
    } else if (strcmp (argv[1], "unpack") == 0) {
        bool skip_integrity = false;
        if (argc == 4) if (strcmp (argv[3], "--skip-integrity") == 0)
                skip_integrity = true;
        unpack (skip_integrity);
    }

    return 0;
}
