#include <stdio.h>
#include "file_block.h"
#include "sha256.h"

void dump_block( FileBlock block) {	
	printf("Hash: "); sha2hex(block.hash);
	printf("Length: %i", block.size);	
}