#include <stdio.h>
#include <unistd.h>

#include "package.h"
#include "dirnav.h"
#include "dir.h"

int package_create()
{
	return mkdir(DIR_PACKAGE, DIR_PERM);
}

void package_initialize()
{
	printf("Initialising empty .package/\n");
	chdir(DIR_PACKAGE);
	mkdir(DIR_BLOCKS, DIR_PERM);
	mkdir(DIR_RECEIPTS, DIR_PERM);
	mkdir(DIR_SYNC, DIR_PERM);
}

void package_prepare()
{
	if (!package_create()) {
		package_initialize();
	}
	mv_package_root();
}
