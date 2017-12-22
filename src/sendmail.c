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
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#if defined(SVR42)
#  include <wait.h>
#else
#  include <sys/wait.h>
#endif

#include "drealm.h"
#include "genfuncs.h"
#include "drealmgen.h"
#include "sendmail.h"

static int drealm_to_unix(char mode, char *to, char *from, char *msg,
	char *in_reply_to, char *subject, char type, int msgno);
static int drealm_to_drealm(char mode, char *to, char *from, char *msg,
	char *in_reply_to, char *subject, char type, int msgno);
static char *get_reply_to(char *user,char *in_reply_to);

/*
 * sendmail
 *	mode		q	no messages displayed
 *			v	messages displayed
 *	to		Name of intended recipient
 *	from		Name of sender
 *	msg		File name or string to send
 *	in_reply_to	Message number this is a reply to (or zero)
 *	subject		Subject string
 *	type		s	msg is a string to send
 *			f	msg is a filename, send the contents
 */
int sendmail(char mode, char *to, char *from, char *msg,
	char *in_reply_to, char *subject, char type)
{

	int msgno;
	char string[300];
	char filename[300];
	FILE *HANDLE;
	
	/*
	 * Generate a new mail message number
	 */
	/* Lock the mailreached file */
	sprintf(string,"%s/mail.lock",C.datadir);
	if (!place_lock(mode,string,1,0)) {
		return 0;
	}
	sprintf(filename,"%s/mailreached.dd",C.datadir);

	if (HANDLE = fopen(filename,"r")) {
	/* Opened okay - read the number */
		fscanf(HANDLE," %d ",&msgno);
		fclose(HANDLE);
	} else {
	/* Argh - let's start again from zero, then... */
		msgno = 0;
	}
	msgno++;
	if (HANDLE = fopen(filename,"w")) {
	/* Opened okay - write the new value back */
		fprintf(HANDLE,"%d\n",msgno);
		fclose(HANDLE);
	} else {
	/* Better safe than sorry - give up! */
		if (mode != 'q')
#if defined(DEVEL)
			perror(filename);
#endif
		remove(string);
		/* Remove the lock */
		return 0;
	}
	/* Remove the lock */
	remove(string);


	if (!strpbrk(to,"@!")) {
		return drealm_to_drealm(mode, to, from, msg, in_reply_to, subject, type, msgno);
	} else {
		char *rep_field = get_reply_to(from,in_reply_to);

		/* This must run before the fork! */
		if (type == 'f') {
			sprintf(filename,"cp %s %s/d2u_%d 2>/dev/null",msg,C.tmpdir,msgno);
			dsystem(filename);
			sprintf(filename,"%s/d2u_%d",C.tmpdir,msgno);
		} else {
			strcpy(filename,msg);
		}

		/* This must run disconnected from the user's terminal */

		if (fork()) {
		/* This is the parent */
			free(rep_field);
			(void)wait(0); /* Wait for the first child to exit */
			/*printf("External mail being spooled.  No outbasket copy taken.\n");*/
			printf("%s\n",Ustring[326]);
			return msgno;
		} else {
		/* This is the first child */
			if (fork()) {
			/* This is the first child */
				exit(0);
			} else {
			/* This is the second child (grandchild) */
				while(getppid() != (pid_t)1) sleep(1);
				fclose(stdin);
				fclose(stdout);
				fclose(stderr);
				(void)setsid();
				exit(drealm_to_unix(mode, to, from, filename, rep_field, subject, type, msgno));
			}
		}
	}
	/* NOTREACHED */
}

static int drealm_to_unix(char mode, char *to, char *from, char *msg,
	char *in_reply_to, char *subject, char type, int msgno)
{
	char filename[MAINLINE + 50];
	char command[2 * MAINLINE + 100];
	FILE *MSG;

	sprintf(filename,"%s/d2u.%d",C.tmpdir,(int)getpid());
	if (!(MSG = fopen(filename,"w"))) {
		if (mode != 'q') {
#if defined(DEVEL)
			perror(filename);
#endif
		}
		return 0;
	}
	fprintf(MSG,"To: %s\n",to);
	fprintf(MSG,"From: %s\n",from);
	if (!strcmp(in_reply_to,"base")) {
		fprintf(MSG,"Subject: %s\n",subject);
	} else {
		if (strncmp(subject,"Re: ",4)) {
			fprintf(MSG,"Subject: Re: %s\n",subject);
		} else {
			fprintf(MSG,"Subject: %s\n",subject);
		}
		fprintf(MSG,"In-Reply-To: %s\n",in_reply_to);
	}
	fputs("\n",MSG);
	if (type == 'f') {
		fclose(MSG);
		sprintf(command,"cat %s >> %s 2>/dev/null",msg,filename);
		dsystem(command);
		/* msg is what we created before the fork() */
		remove(msg);
	} else {
		fprintf(MSG,"%s\n",msg);
		fclose(MSG);
	}
	sprintf(command,"%s < %s",C.unixmailer,filename);
	if (!dsystem(command)) {
/* decrement allowance */	
		remove(filename);
		return 1;
	} else {
		remove(filename);
		return 0;
	}
}

static char *get_reply_to(char *user,char *in_reply_to) {
	char filename[MAINLINE + 50];
	char *p;

	if (!strcmp(in_reply_to,"base")) {
		return strdup("base");
	}
	sprintf(filename,"%s/%s/.mail/msg.%s",C.maildirs,user,in_reply_to);
	if (p = getfield(filename,"in-reply-to")) {
		return p;
	}
	return strdup("base");
}

static int drealm_to_drealm(char mode, char *to, char *from, char *msg,
	char *in_reply_to, char *subject, char type, int msgno)
{
	char dlmtime[MAINLINE];
	char string[300];
	char filename[300];
	struct stat statbuf;
	FILE *FIL;
	FILE *HANDLE;
	time_t t = time(0);

	(void)dlt(dlmtime,MAINLINE,localtime(&t));

	/* Check the recipient is where it should be */
	sprintf(filename,"%s/%s",C.maildirs,to);
	if (stat(filename,&statbuf) || !S_ISDIR(statbuf.st_mode)) {
		if (mode != 'q') {
			/*printf("'%s' is not a valid user name.\n", to);*/
			printf(Ustring[65],to);
			printf("\n");
		}
		return 0;
	}

	/* Ensure the recipient has a .mail directory */
	strcat(filename,"/.mail");
	if (stat(filename,&statbuf)) {
	/* .mail doesn't exist, so create it */
		if (mkdir(filename,0770)) {
		/* mkdir failed */
			if (mode != 'q') {
#if defined(DEVEL)
				perror(filename);
#endif
			}
			return 0;
		}
	} else if (!S_ISDIR(statbuf.st_mode)) {
	/* .mail exists, but is not a directory - yikes! */
		if (mode != 'q') {
			/*printf("%s's .mail directory is NOT a directory!\n", to);*/
			printf("%s %s\n",Ustring[327],Ustring[61]);
			sprintf(G.errmsg,"%s's .mail directory is NOT a directory!\n",to);
			errorlog(G.errmsg);
		}
		return 0;
	}

	/*
	 * Create the message body
	 */
	if (type == 's') {
		if (HANDLE = fopen("temp","w"))	{
			fprintf(HANDLE,"%s\n",msg);
			fclose(HANDLE);
		} else {
			if (mode != 'q') {
#if defined(DEVEL)
				perror(filename);
#endif
			}
			return 0;
		}
	} else {
		sprintf(string,"cp %s temp 2>/dev/null",msg);
		dsystem(string);
	}

	sprintf(filename,"%s/%s/.sig",C.privatefiles,from);
	if (FIL = fopen(filename,"r")) {
		int numread = 0;
		char *sig = (char *)malloc(C.siglength + 1);
		numread = fread(sig,1,C.siglength,FIL);
		sig[numread] = 0;
		fclose(FIL);

		FIL = fopen("temp","a");
		fputs(sig,FIL);
		fclose(FIL);
		free(sig);
	}


	if (C.filter[0]) {
		sprintf(string,"%s < temp > temp2 2>/dev/null",C.filter);
		dsystem(string);
		rename("temp2","temp");
	}

	/*
	 * Send the message to the recipient
	 */
	/* Write the header */


	sprintf(filename,"%s/%s/.mail/hdr.%d",C.maildirs,to,msgno);
	if (HANDLE = fopen(filename,"w")) {
		if (strchr(from,'@')) { /* The 'T' stands for Toll */
			fprintf(HANDLE,"%s U T %d %s from %s to %s - %s\n",in_reply_to, msgno, dlmtime, from, to, subject);
		} else {
			fprintf(HANDLE,"%s U # %d %s from %s to %s - %s\n",in_reply_to, msgno, dlmtime, from, to, subject);
		}
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
	sprintf(filename,"%s/%s/.mail/msg.%d",C.maildirs,to,msgno);
	sprintf(string,"cp temp %s 2>/dev/null",filename);
	dsystem(string);
	/* Create the mailalert */
	sprintf(filename,"%s/%s/.mailalert",C.users,to);
	if (HANDLE=fopen(filename,"w"))	{
		fclose(HANDLE);
	}
	sprintf(filename,"touch %s/%s",C.users,to);
	dsystem(filename);	
	
	sprintf(filename,"%s/%s",C.maildirs,from);
	if (!stat(filename,&statbuf) && S_ISDIR(statbuf.st_mode)) {
		/*
	 	 * Copy to the sender (if it exists)
	 	 */
		/* Write the header */
		sprintf(filename,"%s/%s/.mail/hdr.%d",C.maildirs,from,msgno);
		if (HANDLE = fopen(filename,"w")) {
			fprintf(HANDLE,"%s U # %d %s from %s to %s - %s\n",
				in_reply_to, msgno, dlmtime, from, to, subject);
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
		sprintf(filename,"%s/%s/.mail/msg.%d",C.maildirs,from,msgno);
		sprintf(string,"cp temp %s 2>/dev/null",filename);
		dsystem(string);

	}
	remove("temp");
	return msgno;
}
