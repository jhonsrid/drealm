#if 0
drealmBBS - Bulletin Board System for Linux
Copyright (C) 1994, 1995, 1996  Inge Cubitt and Peter Jones

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

The GNU General Public License should be in a file called COPYING.
#endif
#if !defined(_POSIX_SOURCE)
#  define _POSIX_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "display.h"

int main(int argc,char *argv[]) {
    FILE *file;

    switch(argc) {
    case 1:
	file = fdopen(dup(0),"r");
	if (!file) {
	    perror("dup");
	    exit(1);
	}
	(void)close(0);
	if (open("/dev/tty",O_RDONLY,0) < 0) {
	    perror("/dev/tty");
	    exit(1);
	}
        if (!get_LW(1)) {
            fprintf(stderr,"Could not get terminal characteristics.\n");
            return 1;
        }
	TOT_LINES = 0;
        if (!pager(file,0,"Continue?",'y','n')) {
            perror("stdin");
            return 1;
        }
        fclose(file);
        break;
    case 2:
	TOT_LINES = 0;
	page_file(argv[1],"Continue?",'y','n');
        break;
    default:
        (void)fprintf(stderr,"Usage: %s <file>\n   or: ... | %s\n",
            argv[0],argv[0]);
        return 1;
    }
    return 0;
}
