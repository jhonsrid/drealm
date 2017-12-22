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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define WRONGPARAMS 1
#define NOTSUPPORTED 2
#define NOTSET 3

int main (int argc, char *argv[]) {
	char command[1024];

	if (argc != 2) {
		exit(WRONGPARAMS);
	}
	/*
	 * Assume full authority
	 */
	(void)setuid(0);

#if defined(SVR42) || defined(LINUX)
	(void)sprintf(command,"passwd %s",argv[1]);
	if (system(command)) {
		exit(NOTSET);
	} else {
		exit(0);
	}
#else
	exit(NOTSUPPORTED);
#endif
	/* NOTREACHED */
}

