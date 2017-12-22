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

/* ANSI headers */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "drealm.h"
#include "mailfuncs.h"
#include "inputfuncs.h"
#include "displaymail.h"
#include "display.h"

static struct mailheader mh;

static int parse_mail_hdr_file(char *filename);
static void print_mail_header(void);
static void print_mail_body(char *filename);

void print_mail_message(char *usersdir, char *user, int msgno) {
	char *filename;

	filename = (char *)malloc(strlen(usersdir) + strlen(user) + 10 /*msgno*/ + 13 /*fixed bit*/);

	if (!get_LW(1)) {
		/*printf("Could not get terminal characteristics.\n");*/
		printf(Ustring[66],Ustring[397]);
		printf("\n");
		return;
	}

/*Wed Oct  4 15:28:35 BST 1995*/
	external_term();
/*ends*/

	TOT_LINES = 0;

	(void)printf("\nMail:\n");
	TOT_LINES += 2;

	(void)sprintf(filename,"%s/%s/.mail/hdr.%d",usersdir,user,msgno);
	if (parse_mail_hdr_file(filename)) {
		print_mail_header();
	} else {
		/*(void)printf("Could not read %s\n",filename);*/
		(void)printf(Ustring[66],filename);
		(void)printf("\n");
		TOT_LINES += 1;
	}

	(void)sprintf(filename,"%s/%s/.mail/msg.%d",usersdir,user,msgno);
	print_mail_body(filename);

	free(filename);

/*Wed Oct  4 15:28:35 BST 1995*/
	internal_term();
/*ends*/
}

static int parse_mail_hdr_file(char *filename) {
/* LENGTHS CHECKED */
	static char header[1024];
	int nr;
	FILE *HDR = fopen(filename,"r");

	if (!HDR) {
#if defined(DEVEL)
		fprintf(stderr,"fopen: ");
		perror(filename);
#endif
		return 0;
	}

	nr = fread(header,sizeof (char),1024,HDR);
	if (ferror(HDR)) {
#if defined(DEVEL)
		fprintf(stderr,"fread: ");
		perror(filename);
#endif
		fclose(HDR);
		return 0;
	}
	header[nr] = 0;
	parse_mail_header(header,&mh);
	fclose(HDR);
	return 1;
}

static void print_mail_header(void) {
/* LENGTHS CHECKED */
	(void)printf("%s %s %s %s %s %s %s %s %s %s %s %s\n",
		mh.number, mh.dname, mh.mname, mh.dnum, mh.time, mh.year,
		mh.from, mh.author,
		strcmp(mh.replyto,"base") ? "reply-to" : "-",
		strcmp(mh.replyto,"base") ? mh.replyto : "-",
		strcmp(mh.replyto,"base") ? "by" : "to",
		mh.recip);
	(void)printf("Subject: %s\n",mh.subject);
	(void)printf("\n");
	TOT_LINES += 3;
}

static void print_mail_body(char *filename) {
/* LENGTHS CHECKED */
	FILE *BODY;

	if (!(BODY=fopen(filename,"r"))) {
#if defined(DEVEL)
		fprintf(stderr,"fopen: ");
		perror(filename);
#endif
		return;
	}
	/*(void)pager(BODY,1,"Continue?",'y','n');*/
	(void)pager(BODY,1,Ustring[292],G.bigyes,G.littleno);	/* always paginate */
	fclose(BODY);
}
