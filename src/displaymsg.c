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
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "drealm.h"
#include "inputfuncs.h"
#include "readfuncs.h"
#include "mainfuncs.h"
#include "displaymsg.h"
#include "display.h"


static struct areaheader ah;

static int parse_area_hdr_file(char *filename);
static void print_area_header(void);
static void print_area_body(char *filename);

void print_area_message(const char *areasdir, const char *area, int msgno) {
	char *filename;

	filename = (char *)malloc(strlen(areasdir) + strlen(area) + 4 /*msgno*/ + 8 /*fixed bit*/);

	if (!get_LW(1)) {
		/*printf("Could not get terminal characteristics.\n");*/
		printf(Ustring[66],Ustring[397]);
		printf("\n");
		return;
	}
	TOT_LINES = 0;

/*Wed Oct  4 15:28:00 BST 1995*/
	external_term();
/*ends*/

	(void)printf("\nArea: %s\n", area);
	TOT_LINES += 2;

	(void)sprintf(filename,"%s/%s/hdr.%d",areasdir,area,msgno);
	if (parse_area_hdr_file(filename)) {
		print_area_header();
	} else {
		/*(void)printf("Could not read %s\n",filename);*/
		(void)printf(Ustring[66],filename);
		(void)printf("\n");
		TOT_LINES += 1;
	}

	(void)sprintf(filename,"%s/%s/msg.%d",areasdir,area,msgno);
	print_area_body(filename);

	if (ah.flag[0] == 'V') {
		(void)sprintf(filename,"%s/%s/vote.%d",areasdir,area,msgno);
		/*page_file(filename,"Continue?",'y','n');*/
		page_file(filename,Ustring[292],G.bigyes,G.littleno);
	}

	free(filename);
/*Wed Oct  4 15:28:00 BST 1995*/
	internal_term();
/*ends*/
}

static int parse_area_hdr_file(char *filename) {
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

	nr = fread(header,sizeof (char),1023,HDR);
	if (ferror(HDR)) {
#if defined(DEVEL)
		fprintf(stderr,"fread: ");
		perror(filename);
#endif
		fclose(HDR);
		return 0;
	}
	header[nr] = 0;
	fclose(HDR);

	return parse_area_header(header,&ah);
}

static void print_area_header(void) {
/* LENGTHS CHECKED */
	(void)printf("%s %s %s %s %s %s %s %s %s %s %s %s base %s %s\n",
		ah.number, ah.dname, ah.mname, ah.dom, ah.time, ah.year,
		ah.from, ah.author, ah.narrative, ah.parent, ah.by,
		ah.parentby, ah.base, strcmp(ah.next,"-") ? ">" : "");
	if (ah.footer[0]) {
		(void)printf("Replies: %s\n", ah.footer);
	} else {
		(void)printf("No replies\n");
	}
	TOT_LINES += 2;
}

static void print_area_body(char *filename) {
/* LENGTHS CHECKED */
	FILE *BODY;
	char buffer[256];

	if (!(BODY=fopen(filename,"r"))) {
#if defined(DEVEL)
		fprintf(stderr,"fopen: ");
		perror(filename);
#endif
		return;
	}

	/* Subject: line */
	(void)fgets(buffer, sizeof(buffer), BODY);
	(void)printf("%s", buffer);
	TOT_LINES += 1;

	/* Blank line after subject */
	(void)fgets(buffer,sizeof(buffer),BODY);
	(void)printf("%s", buffer);
	TOT_LINES += 1;

	if (LINES && (ah.flag[0] == 'A')) {
		press_enter("");
		TOT_LINES = 0;
	}

	if (LINES && (ah.flag[0] == 'A')) {
		int i=LINES;
		LINES=0;
		(void)pager(BODY,-1,Ustring[292],G.bigyes,G.littleno);	/* no pagination */
		LINES=i;
	} else {
		/*(void)pager(BODY,1,"Continue?",'y','n');*/
		(void)pager(BODY,1,Ustring[292],G.bigyes,G.littleno);	/* always paginate */
	}
	fclose(BODY);

	if (LINES && (ah.flag[0] == 'A')) {
		(void)printf("%c[%d;1H%c[K%c[0;10m%s ",27,LINES,27,27,Ustring[291]);
		(void)fgets(buffer,256,stdin);
		TOT_LINES = 0;
	}
}
