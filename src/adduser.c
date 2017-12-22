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
 * adduser 'users-dir' 'shell' 'real-name' 'group' 'username' 'prompt'
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include <unistd.h>
#include <sys/wait.h>

#define WRONGPARAMS	1
#define NOTSUPPORTED	2
#define NOADD		3
#define WEIRDERROR	4

int main (int argc, char *argv[]) {
/* LENGTHS CHECKED */
	struct passwd *pw;
	char *command;
	unsigned int i;
	int result;
	char temp[365];

	if (argc != 7) {
		exit(WRONGPARAMS);
	}
	/*
	 * Assume full authority
	 */
	(void)setuid(0);

	/*
	 * This next command will add a user but it locks the account until
	 * they set up a unix password. <SIGH>  Okay, let 'em have one, then.
	 */
	i = strlen(argv[1]) + strlen(argv[5]) + strlen(argv[2]) +
		strlen(argv[3]) + strlen(argv[4]) + strlen(argv[5]) +
		43 /* length of fixed part */;
	command = (char *)malloc(i);
	(void)sprintf(command,"useradd -m -d %s/%s -s %s -c '%s' -g %s %s 2>/dev/null",argv[1],argv[5],argv[2],argv[3],argv[4],argv[5]);
	result = system(command);
	free(command);
	if (result != 0) {
		exit(NOADD);
	}

	(void)sprintf(temp,"%s/%s",argv[1],argv[5]);
	if ( (pw = getpwnam(argv[5])) ) {
		(void)chown(temp,pw->pw_uid,-1);
	} else {
		exit(NOADD);
	}

	(void)printf("\n");
	(void)printf("%s", argv[6]);
	(void)printf("\n");
	i = strlen(argv[5]) + 10 /* length of fixed part */;
	command = (char *)malloc(i);
	(void)sprintf(command,"passwd %s",argv[5]);
#  if !defined(LINUX)
	while (system(command)) {
		(void)printf("\n\n");
		(void)printf(argv[6]);
		(void)printf("\n");
	}
#  else
	system(command);
	pw = getpwnam(argv[5]);
	while(pw && !strcmp("!",pw->pw_passwd)) {
		(void)printf("\n\n");
		(void)printf("%s", argv[6]);
		(void)printf("\n");
		system(command);
		free(pw);
		pw = getpwnam(argv[5]);
	}
	if (!pw) exit(WEIRDERROR);
#  endif
	free(command);

	exit(0);
	/* NOTREACHED */
}
