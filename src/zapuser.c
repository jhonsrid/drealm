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
#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define WRONGPARAMS 1
#define NOTSUPPORTED 2
#define NOADD 3

/* argv1 is user, argv2 is tmpdir */

int main (int argc, char *argv[]) {
	char lockname[1024];
	char pidstring[21];
	int result = 0;
	int pid = 0;
	FILE *FIL;

	if (argc != 3) {
		exit(WRONGPARAMS);
	}


	(void)sprintf(lockname,"%s/conf.%s",argv[2],argv[1]);
	if (FIL = fopen(lockname,"r")) {
		(void)fgets(pidstring,20,FIL);
		(void)fclose(FIL);
		pidstring[20] = 0;
		pid = atoi(pidstring);
		if (pid) {
	/*
	 * Assume full authority
	 */
			(void)setuid(0);

			if (kill(pid,SIGHUP)) {
				result = 1;
			}
		}
	}
	exit(!result);
	/* NOTREACHED */
}

