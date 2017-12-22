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
#include <unistd.h>
#include <sys/stat.h>

/* Local headers */
#include "drealm.h"
#include "drealmgen.h"
#include "mainfuncs.h"
#include "inputfuncs.h"
#include "configfuncs.h"
#include "setupfuncs.h"
#include "genfuncs.h"

#include "sendmess.h"

/*
 * sendmess
 *	mode		q	no messages displayed
 *			v	messages displayed
 *	areaname	Name of message area
 *	from		Name of sender
 *	msg		File name or string to send
 *	subject		Subject string
 *	resthead	Header fields nn..mm
 *	msgtype		B	base message
 *			M	reply or comment
 *	type		s	msg is a string to send
 *			f	msg is a filename, send the contents
 *				(ignored - always a filename)
 */

/* ARGSUSED7 */
int sendmess(const char mode, const char *areaname, const char *from, 
	const char *msg, const char *subject, const char *resthead, 
	const char msgtype, const char type)
{
	int msgno;
	char dlmtime[MAINLINE];
	char string[MAINLINE + 50];
	char numlock[MAINLINE + 50];
	char indexlock[MAINLINE + 50];
	char filename[MAINLINE + 50];
	struct stat statbuf;
	FILE *HANDLE;
	time_t t = time(0);

	(void)dlt(dlmtime,MAINLINE,localtime(&t));

	/* Check the recipient is where it should be */
	sprintf(filename,"%s/%s",C.areasdir,areaname);
	if (stat(filename,&statbuf) || !S_ISDIR(statbuf.st_mode)) {
		if (mode != 'q') {
			/*printf("'%s' is not a valid message area.\n", areaname);*/
			printf(Ustring[65],areaname);/*"cannot find"*/
			printf("\n");
		}
		return 0;
	}
	if (mode != 'q') {
		/*printf("Sending message... ");*/
		printf("%s",Ustring[94]);
		fflush(stdout);
	}

	/*
	 * Generate a new message number
	 */
	/* Place the numlock */
	sprintf(numlock,"%s/%s/numlock",C.areasdir,areaname);
	if (!place_lock(mode,numlock,1,0)) {
		return 0;
	}
	sprintf(filename,"%s/%s/highest",C.areasdir,areaname);
	if (HANDLE = fopen(filename,"r")) {
	/* Opened okay - read the number */
		fscanf(HANDLE," %d ",&msgno);
		fclose(HANDLE);
	} else {
	/* highest is missing! */
		if (mode != 'q') {
#if defined(DEVEL)
			perror(filename);
#endif
		}
		sprintf(string,"%s not readable!",filename);
		errorlog(string);
		remove(numlock);
		return 0;
	}
	msgno++;

	if (msgno > 4000) {
		/*printf("Message number exceeds 4000 - abandoned.\n");*/
		printf("%s - %s\n",Ustring[328],Ustring[60]);
		sprintf(G.errmsg,"Area %s full.",areaname);
		errorlog(G.errmsg);
		remove(numlock);
		return 0;
	}
	/*
	 * Lock msgindex
	 */
	sprintf(indexlock,"%s/%s/indexlock",C.areasdir,areaname);
	if (!place_lock(mode,indexlock,1,0)) {
		remove(numlock);
		return 0;
	}

	/* Write the header */
	sprintf(filename,"%s/%s/hdr.%d",C.areasdir,areaname,msgno);
	if (HANDLE = fopen(filename,"w")) {
		fprintf(HANDLE,"# %d %s from %s %s %d - %d\n",
			msgno, dlmtime, from, resthead, msgno, msgno);
		fclose(HANDLE);
	} else {
		if (mode != 'q') {
#if defined(DEVEL)
			perror(filename);
#endif
		}
		return 0;
	}

	/* Write the body */
	sprintf(filename,"%s/%s/msg.%d",C.areasdir,areaname,msgno);
	if (HANDLE = fopen(filename,"w")) {
		fprintf(HANDLE,"Subject: %s\n\n",subject);
		fclose(HANDLE);
	}
	sprintf(string,"cat %s >> %s 2>/dev/null",msg,filename);
	dsystem(string);

	sprintf(filename,"%s/%s/highest",C.areasdir,areaname);
	if (HANDLE = fopen(filename,"w")) {
	/* Opened okay - write the new value back */
		fprintf(HANDLE,"%d\n",msgno);
		fclose(HANDLE);
	} else {
	/* Couldn't update highest, so back everything out */
		if (mode != 'q') {
#if defined(DEVEL)
			perror(filename);
#endif
		}
		sprintf(filename,"%s/%s/hdr.%d",C.areasdir,areaname,msgno);
		remove(filename);
		sprintf(filename,"%s/%s/msg.%d",C.areasdir,areaname,msgno);
		remove(filename);
		remove(indexlock);
		remove(numlock);
		return 0;
	}

	/* Update msgindex (locked way up there ^^^) */
	sprintf(filename,"%s/%s/msgindex",C.areasdir,areaname);
	if (HANDLE=fopen(filename,"a")) {
		fputc(msgtype,HANDLE);
		fclose(HANDLE);
	} else {
	/* We've created the message and updated highest.  It's pretty
	   horrendous if we can't open the msgindex at this point.  We
	   have to soldier on regardless, I'm afraid... */

		if (mode != 'q') {
#if defined(DEVEL)
			perror(filename);
#endif
		}
	}

	/* Remove the locks */
	remove(indexlock);
	remove(numlock);
	if (mode != 'q') {
		printf("%d\n",msgno);
	}
	return msgno;
}

