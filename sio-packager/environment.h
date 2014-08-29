#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "cache.h"

class Environment {
    Cache<std::string> block_cache_;

public:
    Environment();
    Cache<std::string> &block_cache();
};

#endif //ENVIRONMENT_H
