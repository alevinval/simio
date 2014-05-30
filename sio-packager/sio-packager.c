#include <stdio.h>
#include "packager.h"

int main ( int argc, char *argv[] ) 
{
	if ( argc < 3 ) {
		die ("Usage: \nsio-packager pack|unpack file_name --skip-integrity");
	}

	if ( strcmp(argv[1], "pack") == 0 ) {
		pack (argv[2], 16);
	} else if ( strcmp (argv[1], "unpack") == 0 ) {
		int skip_integrity = 0;
		if ( argc == 4 )
			if ( strcmp (argv[3], "--skip-check") == 0 )
				skip_integrity = 1;
		unpack (argv[2], skip_integrity);
	}

	return 0;
}
