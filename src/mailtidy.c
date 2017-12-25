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

pass the config directory
there should be a config.mtidy if you want exclusions

*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include "drealm.h"
#include "configfuncs.h"
#include "genfuncs.h"
#include "drealmgen.h"

struct globalvars G;
struct configvars C;
char *Ustring[USTRINGCOUNT + 1];


int main(int argc, char *argv[]) {
/* LENGTHS CHECKED  - nothing adds up to > 1024 */
	DIR *DIRT;
	DIR *MDIRT;
	FILE *TMP;
	time_t nowtime;
	time_t comparetime;
	int age = 0;
	char agestring[MAINLINE];
	char temp[1024];
	char item[12];
	char body[12];
	char smalltemp[80];
	struct stat statbuf;
	struct dirent *d;
	struct dirent *e;
	char exclusions[MAINLINE];
	char dirname[MAINLINE + 100];
	char removal[MAINLINE + 150];
	char user[9];
	int totalshift = 0;
	int matched;

	exclusions[0] = 0;

	if (argc > 2) {
		fprintf(stderr,"Usage: %s [config file]\n",argv[0]);
		exit(1);
	} else if (argc == 2) {
		C.configfile = strdup(argv[1]);
	} else {
		C.configfile = strdup("config.drealm");
	}


	if ( (TMP = fopen(C.configfile,"r")) ) {
		fclose(TMP);
	} else {
		fprintf(stderr,"mailtidy: Config file not read.\n");
		exit(2);
	}

	if (!cfgdrealm_read()) {
		fprintf(stderr,"mailtidy: Config file unusable.\n");
		exit(3);
	}

	sprintf(temp,"%s/config.mtidy",C.configdir);
	/* This is optional */
	if ( (TMP = fopen(temp,"r")) ) {
		get_next_cfgfield (TMP,&totalshift,agestring,MAINLINE);
		age = atoi(agestring); /*age in days - 0 in this case means no mail is cleared!*/
		get_next_cfgfield (TMP,&totalshift,exclusions,MAINLINE);
		tnt(exclusions);
		fclose(TMP);
	}
	if (age < 1) {
		fprintf(stderr,"mailtidy: Asked to delete private mail 0 days old. Command not executed.\n");
		exit(4);
	}	
	nowtime = time(0);
	comparetime = (nowtime - (age * 24 * 60 * 60));

	if ((DIRT = opendir(C.maildirs))) {
		while ((d = readdir(DIRT))) {
			strncpy(user,d->d_name,8);

			user[8] = 0;

			if (user[0] == '.') {
				continue;
			}

			/* Check not on exclusions */
			strcpy(temp,exclusions);
			matched = 0;
			while (temp[0]) {
				shiftword(temp,smalltemp,9);
				if (!strcmp(smalltemp,user)) { /*if this is an exclusion */
					matched = 1;
				}					
			}
			if (matched == 1) {
				continue;
			}
			sprintf(dirname,"%s/%s/.mail",C.maildirs,user);
			if ((MDIRT = opendir(dirname))) {
				while ((e = readdir(MDIRT))) {
					strncpy(item,e->d_name,12);
					item[11] = 0;

					if (strncmp(item,"hdr.",4)) {
						continue;
					}

					sprintf(removal,"%s/%s/.mail/%s",C.maildirs,user,item);
					stat(removal,&statbuf);

					if (statbuf.st_mtime < comparetime) {
						remove(removal);	

						body[0] = 'm';
						body[1] = 's';
						body[2] = 'g';
						body[3] = '.';
						body[4] = item[4];
						body[5] = item[5];
						body[6] = item[6];
						body[7] = item[7];
						body[8] = item[8];
						body[9] = item[9];
						body[10] = item[10];
						body[11] = 0;
						sprintf(removal,"%s/%s/.mail/%s",C.maildirs,user,body);
						remove(removal);	


					}		
				}
				closedir(MDIRT);
			}

		}
		closedir(DIRT);

	}
	return 0;

}	
