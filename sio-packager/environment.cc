#include <dirent.h>
#include "environment.h"
#include "dirnav.h"

Environment::Environment ()
{
    std::string name;
    std::vector<std::string>::iterator it;

    mv_package_blocks ();

    struct dirent *current;
    DIR *root = opendir(".");
    while ( (current = readdir(root)) != NULL ) {
        if ( current->d_type != DT_REG)
            continue;

        name = current->d_name;
        block_cache_.add(name);
    }

    block_cache_.sort();

    mv_parent ();

    closedir (root);
}

Cache<std::string> & Environment::block_cache() { return block_cache_; }