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
/*
 * Syntax: username user real name
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	char buffer[256];
	int i;

	if (argc<3) {
#if defined(DEVEL)
		(void)fprintf(stderr,"Usage: %s username real name\n",argv[0]);
#endif
		exit(1);
	}

	(void)strcpy(buffer,argv[2]);
	for (i=3;(i<argc) && (strlen(buffer)+strlen(argv[i]) < (size_t) 256);i++) {
		(void)strcat(buffer," ");
		(void)strcat(buffer,argv[i]);
	}

	/*
	 * Assume full authority
	 */
	(void)setuid(0);

	(void)execlp("usermod","usermod","-c",buffer,argv[1],(char *)0);
#if defined(DEVEL)
	perror(argv[0]);
#endif
	exit(1);
	/* NOTREACHED */
}
