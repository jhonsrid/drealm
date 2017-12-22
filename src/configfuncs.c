
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
/* */

/* ANSI headers */
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* Non-ANSI headers */
/* This is only needed because of the stuff in drealm.h */
#include <unistd.h>

/* Local headers */
#include "drealm.h"
#include "drealmgen.h"
#include "mainfuncs.h"
#include "inputfuncs.h"
#include "configfuncs.h"
#include "setupfuncs.h"
#include "genfuncs.h"

char *Ustring[USTRINGCOUNT + 1];
char *Mstring[MSTRINGCOUNT + 1];
char Newareaflags[AFLAGMAX + 1];
char Newareamask[UFLAGMAX + 1];
char Newuserflags[UFLAGMAX + 1];
char Pvtmailmask[UFLAGMAX + 1];
char Pvtfilesmask[UFLAGMAX + 1];
char Extmailmask[UFLAGMAX + 1];


int uflagnames (void) {
/* CHECKED */
	int totalshift = 0;
	char result[MAINLINE];
	char temp[MAINLINE];
	FILE *FIL;
	char filename[MAINLINE + 100];
	int foundname = 0;
	int flagno = 11;

	strcpy(C.uflagnames[SILENT],"SILENT");
	strcpy(C.uflagnames[CHAIRMAN],"CHAIRMAN");
	strcpy(C.uflagnames[GAGGED],"GAGGED");
	strcpy(C.uflagnames[LIVE],"LIVE");
	strcpy(C.uflagnames[ABEND],"ABEND");
	strcpy(C.uflagnames[NODEGROUP],"NODEGROUP");
	strcpy(C.uflagnames[7],"RESERVED");
	strcpy(C.uflagnames[8],"RESERVED");
	strcpy(C.uflagnames[9],"RESERVED");
	strcpy(C.uflagnames[10],"RESERVED");

	sprintf(filename,"%s/config.uflags",C.configdir);
	if (FIL = fopen(filename,"r")) {
		for (flagno=11;flagno < (UFLAGMAX + 1);flagno++) {
			temp[0] = 0;
			foundname = 0;
			if (get_next_cfgfield (FIL,&totalshift,result, MAINLINE)) {
				sscanf(result, " %s ", temp);
				if (temp[0]) {
					foundname = 1;
				}
			}
			if (foundname) {
				strncpy(C.uflagnames[flagno],temp,10);
				C.uflagnames[flagno][10] = 0;
			} else {
				strcpy(C.uflagnames[flagno],"UNNAMED");
			}
		}
		fclose(FIL);
	} else {
		for (flagno=11;flagno < (UFLAGMAX + 1);flagno++) {
			strcpy(C.uflagnames[flagno],"UNNAMED");
		}
	}
	C.uflagnames[flagno][0] = 0;
	return 1;
}

int aflagnames (void) {
/* CHECKED */
	int totalshift = 0;
	char result[MAINLINE];
	char temp[MAINLINE];
	FILE *FIL;
	char filename[MAINLINE + 100];
	int flagno = 11;
	int foundname = 0;

	strcpy(C.aflagnames[AREATRUE],"AREATRUE");
	strcpy(C.aflagnames[SIGS],"SIGS");
	strcpy(C.aflagnames[ALIASES],"ALIASES");
	strcpy(C.aflagnames[READONLY],"READONLY");
	strcpy(C.aflagnames[PRIVATE],"PRIVATE");
	strcpy(C.aflagnames[MODERATED],"MODERATED");
	strcpy(C.aflagnames[7],"RESERVED");
	strcpy(C.aflagnames[8],"RESERVED");
	strcpy(C.aflagnames[9],"RESERVED");
	strcpy(C.aflagnames[10],"RESERVED");

	sprintf(filename,"%s/config.aflags",C.configdir);
	if (FIL = fopen(filename,"r")) {

		for (flagno=11;flagno < (AFLAGMAX + 1);flagno++) {
			temp[0] = 0;
			foundname = 0;
			if (get_next_cfgfield (FIL,&totalshift,result, MAINLINE)) {
				sscanf(result, " %s ", temp);
				if (temp[0]) {
					foundname = 1;
				}
			}
			if (foundname) {
				strncpy(C.aflagnames[flagno],temp,10);
				C.aflagnames[flagno][10] = 0;
			} else {
				strcpy(C.aflagnames[flagno],"UNNAMED");
			}
		}
		fclose(FIL);
	} else {
		for (flagno=11;flagno < (AFLAGMAX + 1);flagno++) {
			strcpy(C.aflagnames[flagno],"UNNAMED");
		}
	}
	C.aflagnames[flagno][0] = 0;
	return 1;
}

int cfgdrealm_read (void) {
/* CHECKED */
	FILE *CFG;
	int i;

	if (CFG = fopen(C.configfile,"r")) {
		i = cfgdrealm_parse(CFG);
		fclose(CFG);
		if (!i) {
			printf("Config file format error.\n");
			/*
			printf(Ustring[133],Ustring[231]);
			printf("\n");
			*/
		}
	} else {
		i = 0;
		printf("Cannot read config file\n");
		/*
		printf(Ustring[66],Ustring[231]);
		printf("\n");
		*/
	}
	return i;
}

int cfgdrealm_parse (FILE *CFG) {
/* LENGTHS CHECKED */
	char result[MAINLINE];
	int totalshift = 0;

/* bbs config */
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.bbsname=strdup(result);
	} else {
		run_err("Invalid field $bbsname$ in config.drealm - probably too long");
		return 0;
	}
	
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.bbsshort=strdup(result);
	} else {
		run_err("Invalid field $bbsshort$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.sysopname=strdup(result);
	} else {
		run_err("Invalid field $sysopname$ in config.drealm - probably too long");
		return 0;
	}

/* new user config */
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %d ", &C.admin) != 1) {
			run_err("Invalid value for C.admin");
			return 0;
		}
	} else {
		run_err("Invalid field C.admin in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %d ", &C.group) != 1) {
			run_err("Invalid value for C.group");
			return 0;
		}
	} else {
		run_err("Invalid field $groupnum$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %d ", &C.public) != 1) {
			run_err("Invalid value for C.public");
			return 0;
		}
	} else {
		run_err("Invalid field C.public in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.bbsshell=strdup(result);
	} else {
		run_err("Invalid field $bbsshell$ in config.drealm - probably too long");
		return 0;
	}


	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.homedirs=strdup(result);
	} else {
		run_err("Invalid field $homedirs$ in config.drealm - probably too long");
		return 0;
	}

/* bbs directories */
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.users=strdup(result);
	} else {
		run_err("Invalid field $users$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.privatefiles=strdup(result);
	} else {
		run_err("Invalid field $privatefiles$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.maildirs=strdup(result);
	} else {
		run_err("Invalid field $maildirs$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.library=strdup(result);
	} else {
		run_err("Invalid field $library$ in config.drealm - probably too long");
		return 0;
	}


	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.bin=strdup(result);
	} else {
		run_err("Invalid field $bin$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.configdir=strdup(result);
	} else {
		run_err("Invalid field $configdir$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %zd ", &C.maxfilename) != 1) {
			run_err("Invalid value for C.maxfilename");
			return 0;
		}
		if (C.maxfilename > 50) {
			C.maxfilename = 50;
		}
	} else {
		run_err("Invalid field C.maxfilename in config.drealm - probably too long");
		return 0;
	}
	
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.datadir=strdup(result);
	} else {
		run_err("Invalid field $datadir$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.menus=strdup(result);
	} else {
		run_err("Invalid field $menudir$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.areasdir=strdup(result);
	} else {
		run_err("Invalid field $areasdir$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.tmpdir=strdup(result);
	} else {
		run_err("Invalid field $tmpdir$ in config.drealm - probably too long");
		return 0;
	}

/* path */
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.path=strdup(result);
	} else {
		run_err("Invalid field PATH in config.drealm - probably too long");
		return 0;
	}

/* area names */
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.startarea=strdup(result);
	} else {
		run_err("Invalid field $startarea$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.newsarea=strdup(result);
	} else {
		run_err("Invalid field $newsarea$ in config.drealm - probably too long");
		return 0;
	}

/* new area flags */
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		strcpy(Newareaflags,result);
		C.newareaflags=(char *)malloc(AFLAGMAX + 2);
		C.newareaflags[0] = '#';
		strcpy(&C.newareaflags[1],result);
		C.newareaflags[AFLAGMAX + 1] = 0;
	} else {
		run_err("Invalid field $newareaflags$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %d ", &C.newarealevel) != 1) {
			run_err("Invalid value for C.newarealevel");
			return 0;
		}
	} else {
		run_err("Invalid field C.newarealevel in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		strcpy(Newareamask,result);
		C.newareamask=(char *)malloc(UFLAGMAX + 2);
		C.newareamask[0] = '#';
		strcpy(&C.newareamask[1],result);
		C.newareamask[UFLAGMAX + 1] = 0;
	} else {
		run_err("Invalid field $newareamask$ in config.drealm - probably too long");
		return 0;
	}

/* new user flags */
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		strcpy(Newuserflags,result);
		C.newuserflags=(char *)malloc(UFLAGMAX + 2);
		C.newuserflags[0] = '#';
		strcpy(&C.newuserflags[1],result);
		C.newuserflags[UFLAGMAX + 1] = 0;
	} else {
		run_err("Invalid field $newuserflags$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %d ", &C.newuserlevel) != 1) {
			run_err("Invalid value for C.newuserlevel");
			return 0;
		}
	} else {
		run_err("Invalid field C.newuserlevel in config.drealm - probably too long");
		return 0;
	}

/* external mail */
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if ((sscanf(result, " %d ", &C.mailmonitor) != 1) || ((C.mailmonitor != 0) && (C.mailmonitor != 1))) {
			run_err("Invalid value for C.mailmonitor");
			return 0;
		}
	} else {
		run_err("Invalid field $mailmonitor$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.unixmailer=strdup(result);
	} else {
		run_err("Invalid field C.unixmailer in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.unixmaildir=strdup(result);
	} else {
		run_err("Invalid field C.unixmaildir in config.drealm - probably too long");
		return 0;
	}

/* flags */
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if ((sscanf(result, " %d ", &C.autocreate) != 1) || ((C.autocreate != 0) && (C.autocreate != 1))) {
			run_err("Invalid value for $autocreate$");
			return 0;
		}
	} else {
		run_err("Invalid field $autocreate$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if ((sscanf(result, " %d ", &C.sensitive) != 1) || ((C.sensitive != 0) && (C.sensitive != 1))) {
			run_err("Invalid value for $casesensitive$");
			return 0;
		}
	} else {
		run_err("Invalid field $casesensitive$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if ((sscanf(result, " %d ", &C.commandstacking) != 1) || ((C.commandstacking != 0) && (C.commandstacking != 1))) {
			run_err("Invalid value for $commandstacking$");
			return 0;
		}
	} else {
		run_err("Invalid field $commandstacking$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if ((sscanf(result, " %d ", &C.quickreturn) != 1) || ((C.quickreturn != 0) && (C.quickreturn != 1))) {
			run_err("Invalid value for $quickreturn$");
			return 0;
		}
	} else {
		run_err("Invalid field $quickreturn$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if ((sscanf(result, " %d ", &C.uploadreport) != 1) || ((C.uploadreport != 0) && (C.uploadreport != 1))) {
			run_err("Invalid value for C.uploadreport");
			return 0;
		}
	} else {
		run_err("Invalid field C.uploadreport in config.drealm - probably too long");
		return 0;
	}

/* what to do with messages */
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %zd ", &C.siglength) != 1) {
			run_err("Invalid value for C.siglength");
			return 0;
		}
	} else {
		run_err("Invalid field C.siglength in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if ((sscanf(result, " %d ", &C.filesensitive) != 1) || ((C.filesensitive != 0) && (C.filesensitive != 1))) {
			run_err("Invalid value for $filesensitive$");
			return 0;
		}
	} else {
		run_err("Invalid field $filesensitive$ in config.drealm - probably too long");
		return 0;
	}


	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %d ", &C.pvtmaillevel) != 1) {
			run_err("Invalid value for $pvtmaillevel$");
			return 0;
		}
	} else {
		run_err("Invalid field $pvtmaillevel$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		strcpy(Pvtmailmask,result);
		C.pvtmailmask=(char *)malloc(UFLAGMAX + 2);
		C.pvtmailmask[0] = '#';
		strcpy(&C.pvtmailmask[1],result);
		C.pvtmailmask[UFLAGMAX + 1] = 0;
	} else {
		run_err("Invalid field $pvtmailmask$ in config.drealm - probably too long");
		return 0;
	}


	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %d ", &C.pvtfileslevel) != 1) {
			run_err("Invalid value for $pvtfileslevel$");
			return 0;
		}
	} else {
		run_err("Invalid field $pvtfileslevel$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		strcpy(Pvtfilesmask,result);
		C.pvtfilesmask=(char *)malloc(UFLAGMAX + 2);
		C.pvtfilesmask[0] = '#';
		strcpy(&C.pvtfilesmask[1],result);
		C.pvtfilesmask[UFLAGMAX + 1] = 0;
	} else {
		run_err("Invalid field $pvtfilesmask$ in config.drealm - probably too long");
		return 0;
	}


	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %d ", &C.extmaillevel) != 1) {
			run_err("Invalid value for $extmaillevel$");
			return 0;
		}
	} else {
		run_err("Invalid field $extmaillevel$ in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		strcpy(Extmailmask,result);
		C.extmailmask=(char *)malloc(UFLAGMAX + 2);
		C.extmailmask[0] = '#';
		strcpy(&C.extmailmask[1],result);
		C.extmailmask[UFLAGMAX + 1] = 0;
	} else {
		run_err("Invalid field $extmailmask$ in config.drealm - probably too long");
		return 0;
	}


	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %d ", &C.sysoplevel) != 1) {
			run_err("Invalid value for $sysoplevel$");
			return 0;
		}
	} else {
		run_err("Invalid field $sysoplevel$ in config.drealm - probably too long");
		return 0;
	}


	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.displayname1=strdup(result);
	} else {
		run_err("Invalid field C.displayname1 in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.display1=strdup(result);
	} else {
		run_err("Invalid field C.display1 in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.displayname2=strdup(result);
	} else {
		run_err("Invalid field C.displayname2 in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.display2=strdup(result);
	} else {
		run_err("Invalid field C.display2 in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.displayname3=strdup(result);
	} else {
		run_err("Invalid field C.displayname3 in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.display3=strdup(result);
	} else {
		run_err("Invalid field C.display3 in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.editorname1=strdup(result);
	} else {
		run_err("Invalid field C.editorname1 in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.editor1=strdup(result);
	} else {
		run_err("Invalid field C.editor1 in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.editorname2=strdup(result);
	} else {
		run_err("Invalid field C.editorname2 in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.editor2=strdup(result);
	} else {
		run_err("Invalid field C.editor2 in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.editorname3=strdup(result);
	} else {
		run_err("Invalid field C.editorname3 in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.editor3=strdup(result);
	} else {
		run_err("Invalid field C.editor3 in config.drealm - probably too long");
		return 0;
	}


	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.filter=strdup(result);
	} else {
		run_err("Invalid field C.filter in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		numtostr(result);
		C.extras=strdup(result);
	} else {
		run_err("Invalid field C.extras in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		tnt(result);
		C.awkname=strdup(result);
	} else {
		run_err("Invalid field C.awkname in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %d ", &C.chatstyle) != 1) {
			run_err("Invalid value for C.chatstyle");
			return 0;
		}
	} else {
		run_err("Invalid field C.chatstyle in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if ((sscanf(result, " %d ", &C.tp) != 1) || ((C.tp != 0) && (C.tp != 1))) {
			run_err("Invalid value for C.tp");
			return 0;
		}
	} else {
		run_err("Invalid field C.tp in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if ((sscanf(result, " %d ", &C.canchown) != 1) || ((C.canchown != 0) && (C.canchown != 1))) {
			run_err("Invalid value for C.canchown");
			return 0;
		}
	} else {
		run_err("Invalid field C.canchown in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if ((sscanf(result, " %d ", &C.canlogin) != 1) || ((C.canlogin != 0) && (C.canlogin != 1))) {
			run_err("Invalid value for C.canlogin");
			return 0;
		}
	} else {
		run_err("Invalid field C.canlogin in config.drealm - probably too long");
		return 0;
	}

	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if (sscanf(result, " %zd ", &C.menucache) != 1) {
			run_err("Invalid value for C.menucache");
			return 0;
		}
	} else {
		run_err("Invalid field C.menucache in config.drealm - probably too long");
		return 0;
	}
	
	if (get_next_cfgfield (CFG,&totalshift,result,MAINLINE)) {
		if ((sscanf(result, " %o ", &C.filemode) != 1) || (C.filemode < 0) || (C.filemode > 4095)) {
			run_err("Invalid value for C.filemode");
			return 0;
		}
	} else {
		run_err("Invalid field C.filemode in config.drealm - probably too long");
		return 0;
	}
	
	return 1;
}

int get_next_cfgfield (FILE *CFG, int *totalshift, char *result, size_t reslen) {
	unsigned int numread = 0;
	char temp[1024];
	size_t this_shift = 0;

	/* get rid of the comments field */
	if (!(fseek(CFG,*totalshift,SEEK_SET))) {
		numread = fread(temp,1,1023,CFG);
		if (!numread) {
			errorlog("Unexpected end of CFG file");
			result[0] = 0;
			return 0;
		}
		temp[numread] = 0;
		*totalshift += strshift(temp,result,reslen,"::");
	} else {
		errorlog("Rare error reading CFG file (fseek failed)");
		result[0] = 0;
		return 0;
	}

	/* Find the real data */
	if (!(fseek(CFG,*totalshift,SEEK_SET))) {
		numread = fread(temp,1,1023,CFG);
		if (!numread) {
			errorlog("Unexpected end of CFG file");
			result[0] = 0;
			return 0;
		}
		temp[numread] = 0;
		if (menushift(temp,result,reslen,"::",&this_shift)) {
			*totalshift += this_shift;
		} else {
			errorlog("Field too long in CFG file");
			result[0] = 0;
			return 0;
		}
	} else {
		errorlog("Rare error reading CFG file (fseek failed)");
		result[0] = 0;
		return 0;
	}
	return 1;
}

