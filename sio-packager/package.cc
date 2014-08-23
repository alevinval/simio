#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else

#include <unistd.h>

#endif

#include <sys/stat.h>
#include <cstdio>

#include "package.h"
#include "dir.h"
#include "dirnav.h"

Package::Package() {}

Package::~Package() {}

int Package::create()
{
#ifdef _WIN32
    return mkdir(DIR_PACKAGE);
#else
    return mkdir(DIR_PACKAGE, DIR_PERM);
#endif
}

void Package::initialize()
{
    printf("Initialising empty .package/\n");
    chdir(DIR_PACKAGE);
#ifdef _WIN32
    mkdir(DIR_BLOCKS);
    mkdir(DIR_RECEIPTS);
    mkdir(DIR_SYNC);
#else
    mkdir(DIR_BLOCKS, DIR_PERM);
    mkdir(DIR_RECEIPTS, DIR_PERM);
    mkdir(DIR_SYNC, DIR_PERM);
#endif
}

void Package::prepare()
{
    if (!create()) {
        initialize();
    }
    mv_package_root();
}
