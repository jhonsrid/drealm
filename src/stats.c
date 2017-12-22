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
	FILE *TMP;
	FILE *PT;
	FILE *RT;
	FILE *CT;
	DIR *DIRT;
	struct stat statbuf;
	time_t nowdate;
	time_t thendate;
	time_t comparetime;
	int messages;
	int calls;
	float ratio;
	char temp[1024];
	char smalltemp[80];
	char *dlmdate;
	char agestring[MAINLINE];
	int age;
	struct dirent *d;
	char exclusions[MAINLINE];
	char filename[MAINLINE + 100];
	char user[9];
	int totalshift = 0;
	int matched;

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
		fprintf(stderr,"stats: Config file not read.\n");
		exit(2);
	}

	if (!cfgdrealm_read()) {
		fprintf(stderr,"stats: Config file unusable.\n");
		exit(3);
	}
	age = 0; /* this is the default */
	sprintf(temp,"%s/config.stats",C.configdir);
	/* This is optional */
	if ( (TMP = fopen(temp,"r")) ) {
		get_next_cfgfield (TMP,&totalshift,agestring,MAINLINE);
		if (is_num(agestring)) {
			age = atoi(agestring); /*age in days - 0 in this case means age doesn't matter!*/
		}
		get_next_cfgfield (TMP,&totalshift,exclusions,MAINLINE);
		tnt(exclusions);
		fclose(TMP);
	}
	
	nowdate = time(0);
	if (age) {
		comparetime = (nowdate - (age * 24 * 60 * 60));
	} else {
		comparetime = 0;
	}

	sprintf(filename,"%s/callerstemp",C.tmpdir);	
	CT = fopen(filename,"w");
	sprintf(filename,"%s/posterstemp",C.tmpdir);	
	PT = fopen(filename,"w");
	sprintf(filename,"%s/ratiostemp",C.tmpdir);	
	RT = fopen(filename,"w");


	if (DIRT = opendir(C.users)) {
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

			sprintf(filename,"%s/%s/.lastcall",C.users,user);
			if (!stat(filename,&statbuf)) {
				thendate = statbuf.st_mtime;
			} else {
				thendate = 0;
			}
	
			if (thendate < comparetime) {
				continue;
			}
	
			sprintf(filename,"%s/%s/.totalmessages",C.users,user);	
			if (TMP = fopen(filename,"r")) {
				fscanf(TMP," %d ",&messages);
				fclose(TMP);
			} else {
				messages = 0;
			}
			fprintf(PT,"%-16s%5d\n",user,messages);

			sprintf(filename,"%s/%s/.totalcalls",C.users,user);	
			if (TMP = fopen(filename,"r")) {
				fscanf(TMP," %d ",&calls);
				fclose(TMP);
			} else {
				calls = 0;
			}
			fprintf(CT,"%-16s%5d\n",user,calls);
		

	
			if (calls > 0) {
				ratio = ((float)messages/(float)calls);
			} else {
				ratio = 0;
			}
			fprintf(RT,"%-16s%5.2f\n",user,ratio);

		}
		closedir(DIRT);
	}
	fclose(RT);
	fclose(PT);
	fclose(CT);

	dlmdate = drealmtime(nowdate);

	sprintf(filename,"%s/callerstats",C.datadir);
	if (TMP = fopen(filename,"w")) {
		fprintf(TMP,"Call Stats - compiled %s\n",dlmdate);
		fclose(TMP);
	}
	sprintf(filename,"%s/posterstats",C.datadir);
	if (TMP = fopen(filename,"w")) {
		fprintf(TMP,"Message Posting Stats - compiled %s\n",dlmdate);
		fclose(TMP);
	}
	sprintf(filename,"%s/ratiostats",C.datadir);
	if (TMP = fopen(filename,"w")) {
		fprintf(TMP,"Message:Calls Ratio - compiled %s\n",dlmdate);
		fclose(TMP);
	}
	
	free(dlmdate);
	
	sprintf(temp,"sort -rn +1 %s/callerstemp >> %s/callerstats",C.tmpdir,C.datadir);
	system(temp);
	sprintf(temp,"sort -rn +1 %s/posterstemp >> %s/posterstats",C.tmpdir,C.datadir);
	system(temp);
	sprintf(temp,"sort -rn +1 %s/ratiostemp >> %s/ratiostats",C.tmpdir,C.datadir);
	system(temp);
	return 0;
}	

