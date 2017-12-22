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
there should be a config.stats if you want exclusions

*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
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
	DIR *FDIRT;
	FILE *TMP;
	time_t nowtime;
	time_t comparetime;
	int age = 0;
	char agestring[MAINLINE];
	char temp[1024];
	char item[MAINLINE];
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
		fprintf(stderr,"filetidy: Config file not read.\n");
		exit(2);
	}

	if (!cfgdrealm_read()) {
		fprintf(stderr,"filetidy: Config file unusable.\n");
		exit(3);
	}

	sprintf(temp,"%s/config.ftidy",C.configdir);
	/* This is optional */
	if ( (TMP = fopen(temp,"r")) ) {
		get_next_cfgfield (TMP,&totalshift,agestring,MAINLINE);
		age = atoi(agestring); /*age in days - 0 in this case means no mail is cleared!*/
		get_next_cfgfield (TMP,&totalshift,exclusions,MAINLINE);
		tnt(exclusions);
		fclose(TMP);
	}
	if (age < 1) {
		fprintf(stderr,"filetidy: Asked to delete private files 0 days old. Command not executed.\n");
		exit(4);
	}	
	nowtime = time(0);
	comparetime = (nowtime - (age * 24 * 60 * 60));

	if (DIRT = opendir(C.privatefiles)) {
		while (d = readdir(DIRT)) {
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

			sprintf(dirname,"%s/%s",C.privatefiles,user);
			if (FDIRT = opendir(dirname)) {
				while (e = readdir(FDIRT)) {
					strncpy(item,e->d_name,MAINLINE);
					item[MAINLINE - 1] = 0;
					if (item[0] == '.') {
						continue;
					}
					sprintf(removal,"%s/%s/%s",C.privatefiles,user,item);

					stat(removal,&statbuf);
					if (statbuf.st_mtime < comparetime) {
						remove(removal);	
					}		
				}
				closedir(FDIRT);
			}

		}
		closedir(DIRT);

	}
	return 0;
}	

