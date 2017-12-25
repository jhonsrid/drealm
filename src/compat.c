/*
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
*/

/* ANSI headers */
#include <stdio.h>
#include <string.h>

/* Non-ANSI headers */
#include <unistd.h>
#include <pwd.h>

/* cuserid emulation for platforms that don't have it. (But with one concession to string length safety) */
char * cuserid_s(char *s, int sz)
{
	struct passwd *pwd;
	if (!s) return (char *)NULL;

	if ((pwd = getpwuid(geteuid())) == NULL) {
		*s = '\0';
	}
	else {
		(void)strncpy(s, pwd->pw_name, sz);
	}
	return (s);
}

