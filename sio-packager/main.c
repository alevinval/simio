#include "object.h"
#include "receipt.h"
#include "file_block.h"
#include "sha256.h"
#include "packager.h"
#include <stdio.h>

int main() 
{
	pack ("main.c", 16);
	recover ("asd");
	return 0;
}