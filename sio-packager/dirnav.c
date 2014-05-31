#include <unistd.h>

#include "dirnav.h"
#include "dir.h"

void mv_package_root () { chdir (DIR_PACKAGE); }
void mv_package_blocks () { chdir (DIR_BLOCKS); }
void mv_package_receipts () { chdir (DIR_RECEIPTS); }
void mv_parent () { chdir (".."); }