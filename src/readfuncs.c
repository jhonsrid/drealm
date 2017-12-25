
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#if defined(SVR42)
#  include <libgen.h>
#else
#  if defined(LINUX)
#    include <regex.h>
#  endif
#endif

#include "drealm.h"
#include "drealmgen.h"
#include "mainfuncs.h"
#include "inputfuncs.h"
#include "configfuncs.h"
#include "setupfuncs.h"
#include "genfuncs.h"
#include "display.h"
#include "getvalf.h"
#include "sendmess.h"

#include "displaymsg.h"
#include "readfuncs.h"

static struct msgfunctab md[] = {
	&Ustring[1],	"first",	findfirst,
	&Ustring[3],	"last",		findlast,
	&Ustring[5],	"base",		findbase,
	&Ustring[7],	"upthread",	findbackthread,
	&Ustring[9],	"onthread",	findonthread,
	&Ustring[11],	"current",	findnumber,
	&Ustring[13],	"parent",	findupchain,
	&Ustring[17],	"forward",	findforward,
	&Ustring[19],	"back",		findback,
	&Ustring[21],	"next", 	findnext,
	&Ustring[23],	"thread",	definethread,
	&Ustring[25],	"branch",	definetree,
	(char **)0,	"",		NULL,
};


/* ================================================ */
/* READING MESSAGES */

int readmsg(char *in) {
/* MENU COMMAND */
	int scout;
	int result = 0;
	char temp[21];
	struct valid_messages *vm;
	char *copy;

	if (!check_area('v',G.areaname)) {
		flushcom("");
		return 0;
	}
	scout = G.current ? G.current : G.pointer;

	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		vm = get_valid_messages('v',G.areaname,"",scout,copy,0);
	} else {
		shiftword(G.comline,temp,21);
		vm = get_valid_messages('v',G.areaname,"",scout,temp,0);
	}
	free(copy);


	if (!vm) {
		return 0;
	}
	if (!strcmp(vm->parse,"thread") || !strcmp(vm->parse,"branch")) {
		printf("%s\n",Ustring[72]);/*cannot perform on more than one message*/
	} else {
		if (displaymsg(G.areaname,vm->msglist[0])) {
			G.current = vm->msglist[0];
			G.mymsgindex[G.current] = tolower(G.mymsgindex[G.current]);
			summarise(G.current);
			result = 1;
		}
	}
	free(vm->parse);
	free(vm->msglist);
	free(vm);
	return result;
}

int displaymsg (const char *area, const int msgno) {
#if 0
	char sysline[MAINLINE + 100];

	sprintf(sysline,"displaymsg %s %s %d",C.areasdir,area,msgno);
	external_term();
	dsystem(sysline);
	internal_term();
#else
	print_area_message(C.areasdir,area,msgno);
#endif
	

	return 1;
}

int grab (char *in) {
/* MENU COMMAND */
	int i;
	int scout;
	struct valid_messages *vm;
	char temp[21];
	char *copy;

	if (!check_area('v',G.areaname)) {
		flushcom("");
		return 0;
	}
	scout = G.current ? G.current : G.pointer;

	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		vm = get_valid_messages('v',G.areaname,"",scout,copy,0);
	} else {
		shiftword(G.comline,temp,21);
		vm = get_valid_messages('v',G.areaname,"",scout,temp,0);
	}
	free(copy);
	
	if (!vm) {
		return 0;
	}
	for(i=0;vm->msglist[i];i++) {
		putchar('.');
		grab_area_message('v',G.areaname,vm->msglist[i]);
	}
	putchar('\n');
	free(vm->lock);
	free(vm->parse);
	free(vm->msglist);
	free(vm);
	return 1;
}

void grab_area_message(char mode,char *area, int msgno) {
	char filename[MAINLINE + 100];
	FILE *BODY;
	FILE *GRAB;
	char buffer[1024];
	char *header;
	struct areaheader h;
	int i;

	header = definemsg(mode,area,msgno);
	if (!header) {
		return;
	}
	sprintf(filename,"%s/%s/grabpad",C.privatefiles,U.id);
	if (!(GRAB = fopen(filename,"a"))) {
		free(header);
		return;
	}

	fprintf(GRAB, "\nArea: %s\n", area);
	parse_area_header(header,&h);

	fprintf(GRAB,"%s %s %s %s %s %s %s %s %s %s %s %s base %s %s\n",
	h.number, h.dname, h.mname, h.dom, h.time, h.year,
	h.from, h.author, h.narrative, h.parent, h.by,
	h.parentby, h.base, strcmp(h.next,"-") ? ">" : "");

	if (h.footer[0]) {
		fprintf(GRAB,"Replies: %s\n", h.footer);
	} else {
		fputs("No replies\n",GRAB);
	}
	free(header);

	sprintf(filename,"%s/%s/msg.%d",C.areasdir,area,msgno);
	if ((BODY=fopen(filename,"r"))) {
		while(fgets(buffer,1022,BODY)) {
			if (buffer[0] == '.') {
			/* only while putting strips ALL leading dots */
				char newbuffer[1024];
				i = 0;
				newbuffer[i] = '.';
				while(buffer[i]) {	
					newbuffer[i+1] = buffer[i];
					i++;
				}
				newbuffer[i+1] = 0;
				strcpy(buffer,newbuffer);
			}
			fputs(buffer,GRAB);
		}
		if (mode != 'q') {
			printf(".");
		}
		fclose(BODY);
	} else {
		fputs("\nMessage body unreadable.\n",GRAB);
	}
	fputs("\n.\n",GRAB);
	fclose(GRAB);
}

int quotemsg (char *in) {
/* MENU COMMAND */
	int i;
	int scout;
	struct valid_messages *vm;
	char temp[21];
	char *copy;

	if (!check_area('v',G.areaname)) {
		flushcom("");
		return 0;
	}
	scout = G.current ? G.current : G.pointer;

	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		vm = get_valid_messages('v',G.areaname,"",scout,copy,0);
	} else {
		shiftword(G.comline,temp,21);
		vm = get_valid_messages('v',G.areaname,"",scout,temp,0);
	}
	free(copy);

	if (!vm) {
		return 0;
	}
	for(i=0;vm->msglist[i];i++) {
		putchar('.');
		get_area_message(G.areaname,vm->msglist[i]);
	}
	putchar('\n');
	free(vm->lock);
	free(vm->parse);
	free(vm->msglist);
	free(vm);
	return 1;
}

void get_area_message(char *area, int msgno) {
	char filename[MAINLINE + 50];
	FILE *BODY;
	FILE *GRAB;
	char buffer[1024];
	char *header;
	struct areaheader h;

	header = definemsg('v',area,msgno);
	if (!header) {
		return;
	}
	sprintf(filename,"workpad");
	if (!(GRAB = fopen(filename,"a"))) {
		free(header);
		return;
	}


	sprintf(filename,"%s/%s/msg.%d",C.areasdir,area,msgno);
	if ((BODY=fopen(filename,"r"))) {
		parse_area_header(header,&h);
		fprintf(GRAB,"%s wrote:\n",h.author);
		free(header);
		while (fgets(buffer,1022,BODY)) {
			fprintf(GRAB,">%s",buffer);
		}
		fclose(BODY);
	} else {
		/*printf("Message unreadable.\n");*/
		sprintf(filename,Ustring[193],msgno);
		printf(Ustring[66],filename);
		printf("\n");
	}
	fclose(GRAB);
}


/* ================================================ */
/* WRITING MESSAGES */

int post (char *in) {
/* MENU COMMAND */
	return writemsg('p',in);
}

int reply (char *in) {
/* MENU COMMAND */
	return writemsg('r',in);
}

int comment (char *in) {
/* MENU COMMAND */
	return writemsg('c',in);
}

int writemsg (char cmd, char *in) {
/* CHECKED */
	int left;
	
	if (!check_area('v',G.areaname)) {
		return 0;
	}
	if ((G.areaflags[READONLY] != '0') && (U.flags[CHAIRMAN] == '0')) {
		/*printf("Sorry, %s is readonly.\n",G.areaname);*/
		printf(Ustring[138],G.areaname);
		printf("\n");
		return 0;
	}
	if (U.flags[GAGGED] != '0') {
		/*printf("Sorry, you are not permitted to write in this area.");*/
		printf(Ustring[128],G.areaname);
		printf("\n");
		return 0;
	}
	moretoread(G.highmsg);
	if ((G.highmsg > 3999)) {
		/*printf("No more messages allowed in this area.\n");*/
		printf("%s", Ustring[96]);
		printf("\n");
		return 0;
	}		
	if ((G.highmsg > 3900) && (G.highmsg < 4000)) {
		left = 4000 - G.highmsg;
		/*printf("Only %d more messages allowed in this area.\n",left);*/
		printf(Ustring[95],left);
		printf("\n");
	}		

	return write_main(cmd,in);
}

int write_main (char cmd,char *in) {
/* CHECKED - Not foolproof but more or less safe */
	struct stat statbuf;
	FILE *FIL;
	int parent;
	int moderated = 0;
	int result = 0;
	char filename[MAINLINE + 100];
	char temp[MAINLINE];
	char subject[MAINLINE];
	char alias[15];
	char sender[15];

	struct valid_messages *vm = 0;
	char *params;
	char *copy = strdup(in);

	tnt(copy);
	if (copy[0]) {
		free(copy);
		params = strdup(in);
	} else {
		free(copy);
		params = strdup(G.comline);
		flushcom("");
	}
	tnt(params);
	
	if (cmd != 'p') {
		if (!params[0]) {
			free(params);
			sprintf(temp,"%d",G.current);
			params = strdup(temp);
		}
		vm = get_valid_messages('v',G.areaname,Ustring[89],G.current,params,0);/*"Reply to which message"*/

		if (!vm) {
			free(params);
			return 0;
		}
		if (!strcmp(vm->parse,"thread") || !strcmp(vm->parse,"branch")) {
			/*printf("Cannot comment to more than one message at a time!\n");*/
			printf("%s", Ustring[72]);
			printf("\n");

			free(vm->msglist);
			free(vm->parse);
			free(vm);
			free(params);
			return 0;
		}
		parent = vm->msglist[0];

	} else {
		parent = 0;
	}

	strcpy(sender,U.id);
	if (G.areaflags[ALIASES] != '0') {
		/* CONSTCOND */
		while (1) {
			/*printf("\nPress [Enter] to use your %s ID or type in an alias\n",C.bbsshort);*/
			make_prompt(Ustring[90]);
			alias[0] = 0;
			get_one_file(alias,15,"");
			if (alias[0]) {
				sprintf(filename,"%s/%s",C.users,alias);
				if (!stat(filename,&statbuf)) {
					/*printf("You cannot use someone else's name as an alias.\n");*/
					printf("%s", Ustring[91]);
					printf("\n");
					continue;
				}
				strcpy(sender,alias);
			}
			break;
		}
	}

	if (cmd == 'p') {
		make_prompt(Ustring[92]);/*subject*/
		get_one_line(subject);
		free(params);
	} else {

		sprintf(filename,"%s/%s/msg.%d",C.areasdir,G.areaname,parent);
		if ((FIL = fopen(filename,"r"))) {
			fgets(subject,MAINLINE,FIL);
			shiftword(subject,temp,5);
			tnt(subject);
			fclose(FIL);
		} else {
			subject[0] = 0;
		}

		/*printf("\nSubject: %s\n",subject);*/
		printf("\n%s %s\n",Ustring[92],subject);
		/*make_prompt("Press [Enter] to accept or type in new subject: ");*/
		make_prompt(Ustring[93]);
		get_one_line(temp);
		if (temp[0]) {
			strcpy(subject,temp);
		}
		free(vm->msglist);
		free(vm->parse);
		free(vm);
		free(params);
	}

	if (!edit_special("message")) {
		return 0;
	}

	if (G.areaflags[SIGS] != '0') {
		sprintf(filename,"%s/%s/.sig",C.privatefiles,U.id);
		if ((FIL = fopen(filename,"r"))) {
			int numread = 0;
			char *sig = (char *)malloc(C.siglength + 1);
			numread = fread(sig,1,C.siglength,FIL);
			sig[numread] = 0;
			fclose(FIL);

			FIL = fopen("message","a");
			fputs(sig,FIL);
			fclose(FIL);
			free(sig);
		}
	}

	if (G.areaflags[MODERATED] != '0') {
		moderated = 1;
		sprintf(filename,"%s/%s/chair",C.areasdir,G.areaname);
		if (is_in_list(filename,U.id)) {
			moderated = 0;
		} else if (U.level >= C.sysoplevel) {
			/*if (yes_no("This area is moderated. Override as SysOp?")) {*/
			sprintf(filename,"%s %s %s",G.areaname,Ustring[46],Ustring[129]);
			if (yes_no(filename)) {
				moderated = 0;
			}
		} else {
			/*printf("This area is moderated. Your message will be forwarded to the chairman.\n");*/
			printf("%s %s %s",G.areaname,Ustring[46],Ustring[47]);
			printf("\n");
		}
	}
	sprintf(filename,"message");

	if (moderated == 1) {
		result = wait_mod('v',sender,G.areaname,cmd,parent,subject,filename);
	} else {
		result = send_now('v',sender,G.areaname,cmd,parent,subject,filename);
	}
	if (result) {
		U.totalmessages++;
		totalmessages_write(U.id,U.totalmessages);
	}
	return result;
}


int wait_mod (const char mode, const char *sender, const char *area,
		const char cmd, const int parent, const char *subject,
		const char *messagefile)
{
	char filename[MAINLINE + 100];
	char command[MAINLINE + 100];
	int written;
	int msgno;
	FILE *FIL;
	DIR *DIRT;
	
	sprintf(filename,"%s/%s/pending",C.areasdir,area);
	if (!(DIRT = opendir(filename))) {
		if (mkdir(filename,0770)) {
			return 0;
		}
	} else {
		closedir(DIRT);
	}

	written = 0;
	while (!written) {
		msgno = time(0);

		/* See if anyones already had it */
		sprintf(filename,"%s/%s/pending/hdr.%d",C.areasdir,area,msgno);
		if ((FIL = fopen(filename,"r"))) {
			fclose(FIL);
			sleep(1);
			continue;
		}

		sprintf(filename,"%s/%s/pending/hdr.%d",C.areasdir,area,msgno);
		if ((FIL = fopen(filename,"w"))) {
			fprintf(FIL,"%s %s %c %d %s",sender,area,cmd,parent,subject);
			fclose(FIL);
		} else {
			return 0;
		}
	
		/* Write the body */
		sprintf(filename,"%s/%s/pending/msg.%d",C.areasdir,area,msgno);
		sprintf(command,"cat %s >> %s 2>/dev/null",messagefile,filename);
		dsystem(command);
		remove(messagefile);
		written = 1;
	}
	return 1;
}

int send_now (const char mode, const char *sender, const char *area,
		const char cmd, const int orig_parent, const char *subject,
		const char *messagefile)
{
	int sent;
	int base = 0;
	int previous = 0;
	int parent = orig_parent;
	char sendhead[80];
	char *parent_author;
	char lockname[MAINLINE + 100];
	char filename[MAINLINE + 100];
	FILE *FIL;
	char *tempheader;
	struct areaheader phs;


	/* * * * * * * * * * * * * */
	/* Post a new base message */

	if (cmd == 'p') {
		hups_off();
		sent = sendmess(mode,area,sender,messagefile,subject,"BASE-MESSAGE - - - -",'B','f');
		remove(messagefile);
		hups_on();

		if (!sent) {
			return 0;
		}

		moretoread(G.highmsg);
		if (!U.readown) {
			G.mymsgindex[sent] = tolower(G.mymsgindex[sent]);
		}
		return sent;
	}
	
	
	/* * * * * * * * * * * * * */
	/* EVERYTHING
	 * else in this routine is for reply/comment
	 */
	
	
	if ( (tempheader = definemsg(mode,area,parent)) == NULL) {
		return 0;
	}
	parse_area_header(tempheader,&phs);
	parent = atoi(phs.number);
	base = atoi(phs.base);
	parent_author = strdup(phs.author);
	free(tempheader);

	hups_off();

/* Thu Dec 28 18:47:52 GMT 1995
 * Well, we're gonna try and be a reply/comment at this point...
 * time to grab the thread I think.  If only definemsg took a lock parameter,
 * I wouldn't have to check the message is still there.
 * If something has gone wrong, the message is left in the person's "message" for them
 * to deal with at a later date.
 */
	sprintf(lockname,"%s/%s/%d.lock",C.areasdir,area,base);
	if (!place_lock(mode,lockname,1,0)) {
		hups_on();
		return 0;
	}
	tempheader = definemsg(mode,area,parent);
	if (!tempheader) {
		rem_lock(lockname);
		hups_on();
		return 0;
	} else {
		free(tempheader);
	}
	if (cmd == 'r') {
		sprintf(sendhead,"reply-to # %d by %s",parent,parent_author);
	} else {
		sprintf(sendhead,"comment-at # %d - -",parent);
	}
	free(parent_author);

	sent = sendmess(mode,area,sender,messagefile,subject,sendhead,'M','f');
	remove(messagefile);

	if (!sent) {
		rem_lock(lockname);
		hups_on();
		return 0;
	}


	/* the message is now in the area - not linked yet, though... */

	sprintf(filename,"%s/%s/hdr.%d",C.areasdir,area,parent);
	if ((FIL = fopen(filename,"r+"))) { /* fail if not found */
		fseek(FIL, 0, SEEK_END);
		fprintf(FIL,"%d ",sent);
		fclose(FIL);
	}

	if ( (tempheader = definemsg('q',area,base)) != NULL ) {
		parse_area_header(tempheader,&phs);
		previous = atoi(phs.prev);
		sprintf(filename,"%s/%s/hdr.%d",C.areasdir,area,base);
		if ((FIL = fopen(filename,"w"))) {
			fprintf(FIL,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %d\n%s",
				phs.flag,phs.number,phs.dname,phs.mname,phs.dom,phs.time,
				phs.tzname,phs.year,phs.from,phs.author,phs.narrative,phs.hash,
				phs.parent,phs.by,phs.parentby,phs.base,phs.next,
				sent,
				phs.footer);
			fclose(FIL);
		}
		free(tempheader);
	}

	if ( (tempheader = definemsg('q',area,previous)) != NULL) {
		parse_area_header(tempheader,&phs);
		sprintf(filename,"%s/%s/hdr.%d",C.areasdir,area,previous);
		if ((FIL = fopen(filename,"w"))) {
			fprintf(FIL,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %d %s\n%s",
				phs.flag,phs.number,phs.dname,phs.mname,phs.dom,phs.time,
				phs.tzname,phs.year,phs.from,phs.author,phs.narrative,phs.hash,
				phs.parent,phs.by,phs.parentby,phs.base,sent,phs.prev,phs.footer);
			fclose(FIL);
		}
		free(tempheader);
	}

	if ( (tempheader = definemsg('q',area,sent)) != NULL) {
		parse_area_header(tempheader,&phs);
		sprintf(filename,"%s/%s/hdr.%d",C.areasdir,area,sent);
		if ((FIL = fopen(filename,"w"))) {
			fprintf(FIL,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %d %s %d\n%s",
				phs.flag,phs.number,phs.dname,phs.mname,phs.dom,phs.time,
				phs.tzname,phs.year,phs.from,phs.author,phs.narrative,phs.hash,
				phs.parent,phs.by,phs.parentby,base,phs.next,previous,phs.footer);
			fclose(FIL);
		}
		free(tempheader);
	}

	rem_lock(lockname);
	hups_on();

	moretoread(G.highmsg);
	if (!U.readown) {
		G.mymsgindex[sent] = tolower(G.mymsgindex[sent]);
	}
	return sent;
}

int pendput (char *dummy) {
	char filename[MAINLINE + 100];
	char hdr[MAINLINE + 100];
	char body[MAINLINE + 100];
	FILE *HDR;
	struct valid_files *vf;
	char subject[MAINLINE];
	char areaname[51];
	char sender[15];
	char parentstring[10];
	char cmdstring[2];
	char response[2];
	char cmd;
	int parent;
	int stamp;
	int found_one = 0;
	int done;		

	sprintf(filename,"%s/%s/pending",C.areasdir,G.areaname);

	vf = get_valid_files('q',0,"",filename,"hdr.*",1);
	if (!vf->files[0]) {
		/*printf("No pending messages.\n");*/
		printf("%s", Ustring[48]);
		printf("\n");
		free(vf->files);
		free(vf->input);
		free(vf);
		return 0;
	}

	while (vf->files[0]) {
		shiftword(vf->files,filename,51);
		sprintf(hdr,"%s/%s/pending/%s",C.areasdir,G.areaname,filename);
		sscanf(filename,"hdr.%d",&stamp);
		sprintf(body,"%s/%s/pending/msg.%d",C.areasdir,G.areaname,stamp);
		
		if (!(HDR = fopen(hdr,"r"))) {
			remove(hdr);
			remove(body);
			continue;
		}
		if (!(fgets(filename,MAINLINE + 100,HDR))) {
			fclose(HDR);
			remove(hdr);
			remove(body);
			continue;
		}
		fclose(HDR);
		/* sender, area, cmd, parent, subject */
		found_one = 1;
		tnt(filename);
		shiftword(filename,sender,15);		
		shiftword(filename,areaname,51);		
		shiftword(filename,cmdstring,2);	
		shiftword(filename,parentstring,10);		
		strncpy(subject,filename,MAINLINE);
		cmd = cmdstring[0];
		parent = atoi(parentstring);
		printf("\n\n");
		if (cmd == 'p') {		
			printf("BASE-MESSAGE from %s\nSubject: %s\n\n",sender,subject);
		} else if (cmd == 'r') {
			printf("reply to %d from %s\nSubject: %s\n\n",parent,sender,subject);
		} else if (cmd == 'c') {
			printf("comment at %d from %s\nSubject: %s\n\n",parent,sender,subject);
		}

		press_enter("");
		display(body);

		done = 0;
		while (!done) {
			printf("\n");
			make_prompt(Ustring[55]);
			get_one_char(response);
			if (!response[0]) {
				strcpy(response,Ustring[50]);
				/*send option*/
			}

			if (!Dstrcmp(response,Ustring[50])) {
				/*send*/
				send_now('v',sender,G.areaname,cmd,parent,subject,body);
				remove(hdr);
				remove(body);
				done++;
			} else if (!Dstrcmp(response,Ustring[51])) {
				/*hold*/
				done++;
			} else if (!Dstrcmp(response,Ustring[52])) {
				/*edit*/
				edit_special(body);
			} else if (!Dstrcmp(response,Ustring[53])) {
				/*delete*/
				remove(hdr);
				remove(body);
				done++;
			} else if (!Dstrcmp(response,Ustring[54])) {
				/*quit*/
				done++;
				return 1;
			}
		}		
	}
	free(vf->input);
	free(vf->files);
	free(vf);
	/*printf("No pending messages.\n");*/
	printf("%s\n",Ustring[48]);
	return found_one;	
}


/*===============================================================*/
/* JOINING AND SCANNING */

int area_scan (char *in) {
/* MENU COMMAND */
/* CHECKED */
	int result;
	char temp[51];
	char *copy = strdup(in);

	tnt(copy);
	if (copy[0]) {
		result = scan_area(copy);
	} else {
		shiftword(G.comline,temp,51);
		result = scan_area(temp);
	}
	free(copy);
	return result;
}

int scan_area (char *params) {
/* CHECKED */
	char filename[MAINLINE + 100];
	char areascout[51];
	int temppointer;
	char type = 's';
	char string[21];
	FILE *FIL;
	int new;
	int result = 0;
	int eligibility = 0;
	int pendmsgs;
	char *copy = strdup(params);
	struct valid_files *vf;
	
	area_clearup();
	shiftword(copy,string,21);
	free(copy);
	if (!Dstrcmp(string, "express") || !Dstrcmp(string,"e")) {
		type = 'e';
	}
	if (!Dstrcmp(string, "quiet") || !Dstrcmp(string,"q")) {
		type = 'q';
	}
	if (!G.scan) {
		if (type != 'q') {
			/*printf("You have no areas in your scan list.  Why not add some?\n");*/
			printf("%s\n",Ustring[180]);
		}
		return 0;
	}

	/* Type 'q' only wants to know whether there are any new messages at
	   all in your scan.  Therefore it is quite appropriate for it to
	   return 1 as soon as it hits the very first area with any.
	*/
	if (type == 'q') {
		while(1) {
			pendmsgs = 0;
			if (!get_next_area(areascout)) {
				close_scan();
				open_scan();
				return result;
			}
			if (!(eligibility = is_area_elig('q',areascout))) {			
				continue;					
			}							
			sprintf(filename,"%s/%s/.areas/%s",C.users,U.id,areascout);
			if ((FIL = fopen(filename,"r"))) {
				fscanf(FIL," %d ",&temppointer);
				fclose(FIL);
			} else {
				temppointer = 0;
			}
	
			new = countmsgs(areascout,temppointer+1);
			
			if (eligibility == 3) {
				sprintf(filename,"%s/%s/pending",C.areasdir,areascout);
				vf = get_valid_files('q',0,"",filename,"hdr.*",1);
				if (vf->files[0]) {
					pendmsgs = 1;
				}
				free(vf->files);
				free(vf->input);
				free(vf);
			}
	
			if ((new > 0) || (pendmsgs)) {
				close_scan();
				open_scan();
				return 1;
			}
		}
		
	} else {
	
		printf("Ctrl-C to abandon.\n");
		intr_on();
		/*printf("Scanning for new messages...\n");*/
		printf("%s\n",Ustring[181]);
		
		/* CONSTCOND */
		while(1) {
			pendmsgs = 0;
			if (G.intflag == 1) {
				intr_off();
				return result;
			}

			if (!get_next_area(areascout)) {
				/*printf("One full sweep of your message areas completed.\n");*/
				/*printf("Type 'scan' to scan through again.\n");*/
				printf("%s\n",Ustring[182]);
				close_scan();
				open_scan();
				intr_off();
				return result;
			}
			
			/*printf("Scanning %s... ",areascout);*/
			printf(Ustring[183],areascout);
			printf(" ");
			if (!(eligibility = is_area_elig('v',areascout))) {			
				continue;					
			}							

			sprintf(filename,"%s/%s/.areas/%s",C.users,U.id,areascout);
			if ((FIL = fopen(filename,"r"))) {
				fscanf(FIL," %d ",&temppointer);
				fclose(FIL);

			} else {
				temppointer = 0;
			}

			new = countmsgs(areascout,temppointer+1);
			
			if (eligibility == 3) {
				sprintf(filename,"%s/%s/pending",C.areasdir,areascout);
				vf = get_valid_files('q',0,"",filename,"hdr.*",1);
				if (vf->files[0]) {
					pendmsgs = 1;
				}
				free(vf->files);
				free(vf->input);
				free(vf);
			}

			if ((new > 0) || (pendmsgs)) {
				if (pendmsgs) {
					/*printf("Messages pending moderator. ");*/
					printf("%s", Ustring[49]);
				}
				if (type == 'e') {
					/*printf("%d new message%s. ",new,(new == 1) ? "" : "s");*/
					printf(Ustring[184],new);
					printf("\n");
					result = 1;
				} else {
					intr_off();
					if (change_area('v', areascout)) {
						return 1;
					}
				}
			} else {
				printf(Ustring[184],new);
				printf("\n");
				/*printf("no new messages.\n");*/
			}
		}
	}
	/* NOTREACHED */
}

int area_change (char *in) {
/* MENU COMMAND */
	int result;
	char areascout[51];
	struct valid_files *vf;
	char *copy = strdup(in);

	shiftword(copy,areascout,51);
	free(copy);
	if (!areascout[0]) {
		shiftword(G.comline,areascout,51);
	}		

	/*vf = get_valid_dirs('v',1,"area",C.areasdir,areascout,0);*/
	vf = get_valid_dirs('v',1,Ustring[197],C.areasdir,areascout,0);
	strncpy(areascout,vf->files,50);
	areascout[50] = 0;
	free(vf->input);
	free(vf->files);
	free(vf);
	if (!areascout[0]) {
		return 0;
	}
	
	if (!is_area_elig('v',areascout)) {
		return 0;
	}
	
	result = change_area('v',areascout);
	return result;
}

int change_area (char mode, char *areascout) {
/* CHECKED */
	int i = 0;
	char filename[MAINLINE + 100];

	if (!areascout[0]) {
		return 0;
	}

	area_clearup();

	if (!set_pointers(areascout)) {
		if (mode == 'v') {
			/*printf("%s is out of order.\n",areascout);*/
			printf("%s %s\n",areascout,Ustring[134]);
		}
		return 0;
	}

	/* Now we know we are definitely going in */

	strcpy(G.areaname,areascout); /* G.areaname assumed to be 51 */

	G.arealevel = arealevel_read(G.areaname);

	sprintf(filename,"%s/%s/chair",C.areasdir,areascout);
	if (is_in_list(filename,U.id)) {
		set_userflag(U.id,"CHAIRMAN","1");
	} else {
		set_userflag(U.id,"CHAIRMAN","0");
	}

	sprintf(filename,"%s/%s/gagged",C.areasdir,areascout);
	if (is_in_list(filename,U.id)) {
		set_userflag(U.id,"GAGGED","1");
	} else {
		set_userflag(U.id,"GAGGED","0");
	}

	update("");
	
	if (!G.pointer && (mode == 'v')) {
		printf("\n\n\n");
		sprintf(filename,"%s/%s/info",C.areasdir,areascout);
		display(filename);
	}

	i = countmsgs(areascout,(G.pointer + 1));

	if (mode == 'v') {
		/*
		if (!i) {
			printf("\n%s: No new messages.\n",areascout);
		} else {
			printf("\n%s: %d new message%s.\n",areascout,i,(i == 1 ? "" : "s"));
		}
		*/

		sprintf(filename,Ustring[184],i);
		printf("\n%s: %s\n",areascout,filename);
	}
	if ((U.recent > 0) && (i > U.recent)) {
		sprintf(filename,Ustring[185],U.recent);
		if ((mode != 'v') || ((mode == 'v') && yes_no(filename))) {
			/*printf("Catch up to the %d most recent messages.\n",U.recent);*/
			G.pointer = G.highmsg - U.recent;
		}
	}
	return 1;
}

int is_area_elig (char mode, char *area) {
/* CHECKED */
	int i = 1;
	char aflags[AFLAGMAX + 2];
	char compflags[UFLAGMAX + 2];
	char filename[MAINLINE + 100];
	char tempstring[MAINLINE + 100];

	if (!check_area(mode,area)) {
		return 0;
	}
	
	sprintf(filename,"%s/%s/chair",C.areasdir,area);
	if (is_in_list(filename,U.id)) {
		return 3;
	}

	areaflags_read(area,aflags);
	areamask_read(area,compflags);

	if (!comp_flags(compflags,U.flags)) {
		if (U.level >= C.sysoplevel) {
			if (mode == 'v') {
				/*if (yes_no("Your flags do not match.  Override as SysOp?")) {*/
				sprintf(filename,"%s %s",Ustring[125],Ustring[129]);
				if (yes_no(filename)) {
					return 4;
				} else {
					return 0;
				}
			} else {
				return 4;
			}
		} else {
			if (mode == 'v') {
				/*printf("Access to %s denied.\n",area);*/
				printf(Ustring[130],area);
				printf("\n");
			}
			return 0;
		}
	}

	if ( (i = arealevel_read(area)) < 0) {
		/*printf("Error in reading %s/%s/level\n",C.areasdir,area);*/
		sprintf(filename,"%s/%s/level",C.areasdir,area);
		printf(Ustring[66],filename);
		printf("\n");
		if (U.level < C.sysoplevel) {
			return 0;
		}
	}

	if (U.level < i) {
		if (U.level < C.sysoplevel) {
			/*printf("Access to %s denied.\n",area);*/
			printf(Ustring[130],area);
			printf("\n");
			return 0;
		}
		if (mode == 'v') {
			/*if (yes_no("Your level is not sufficient.  Override as SysOp?")) {*/
			sprintf(filename,"%s %s",Ustring[126],Ustring[129]);
			if (yes_no(filename)) {
				return 4;
			} else {
				return 0;
			}
		} else {
			return 4;
		}
	}


/* endofnewflags */

	if (aflags[PRIVATE] == '1') {
		sprintf(filename,"%s/%s/members",C.areasdir,area);
		if (!is_in_list(filename,U.id)) {
			if (U.level >= C.sysoplevel) {
				if (mode == 'v') {
					/*if (yes_no("Area is private. You are not a member.  Override as SysOp?")) {*/
					sprintf(tempstring,Ustring[136],area);
					sprintf(filename,"%s %s - %s",tempstring,Ustring[127],Ustring[129]);
					if (yes_no(filename)) {
						return 4;
					} else {
						return 0;
					}
				} else {
					return 4;
				}
			} else {
				if (mode == 'v') {
					/*printf("%s is private and you are not a member.\n",area);*/
					printf(Ustring[136],area);
					printf(" %s\n",Ustring[127]);
				}
				return 0;
			}
		}
	}
	if (aflags[READONLY] == '1') {
		if (mode != 'q') {
/*			printf("%s is readonly.\n",area);*/
			/* EMPTY */
		}
		return 1;
	}
	sprintf(filename,"%s/%s/gagged",C.areasdir,area);
	if (is_in_list(filename,U.id)) {
		if (mode != 'q') {
/*			printf("Oh dear it looks like you cannot post in %s.\n",area);*/
			/* EMPTY */
		}
		return 1;
	}
	sprintf(filename,"%s/%s/highest",C.areasdir,area);
	if (get_int_from_file(filename) > 3999) {
		if (mode != 'q') {
/*			printf("Full up!.\n");*/
			/* EMPTY */
		}
		return 1;
	}
	return 2;
}

int read_init (void) {
/* CHECKED */
	int i;
	char filename[MAINLINE + 100];

	G.scan = 0;
	G.highmsg = -1;
	G.pointer = -1;
	G.current = -1;
	G.whichnext = -1;
	G.areapointer = -1;

	open_scan();

	G.areaflags[0] = '#';
	for (i=1;i<=AFLAGMAX;i++) {
		G.areaflags[i] = '0';
	}
	G.areaflags[AFLAGMAX + 1] = 0;

	G.areamask[0] = '#';
	for (i=1;i<=UFLAGMAX;i++) {
		G.areamask[i] = '0';
	}
	G.areamask[UFLAGMAX + 1] = 0;


	G.areaname[0] = 0;
	G.mymsgindex[0] = 0;
	G.chain[0] = 0;

	if (check_area('q',C.startarea) && set_pointers(C.startarea)) {
		strcpy(G.areaname,C.startarea);
		sprintf(filename,"%s/%s/chair",C.areasdir,C.startarea);
		if (is_in_list(filename,U.id)) {
			set_userflag(U.id,"CHAIRMAN","1");
		} else {
			set_userflag(U.id,"CHAIRMAN","0");
		}
		/*
		if ((G.pointer < 1) && (U.recent > 0) && (G.highmsg > U.recent)) {
			G.pointer = (G.highmsg - U.recent);
		}
		*/
		return 1;
	} else {
		return 0;
	}
}

int set_pointers (char *areascout) {
/* CHECKED */
	char filename[MAINLINE + 100];
	FILE *FIL;
	int temp_pointer;
	unsigned temp_highmsg;
	char temp_areaflags[AFLAGMAX + 2];
	char temp_maskflags[UFLAGMAX + 2];


	sprintf(filename,"%s/%s/.areas/%s",C.users,U.id,areascout);
	if ((FIL = fopen(filename,"r"))) {
		fscanf(FIL," %d ",&temp_pointer);
		fclose(FIL);
	} else {
		temp_pointer = 0;
	}

	sprintf(filename,"%s/%s/highest",C.areasdir,areascout);
	if ((FIL = fopen(filename,"r"))) {
		fscanf(FIL," %d ",&temp_highmsg);
		fclose(FIL);
	} else {
		return 0;
	}

	if ((!areaflags_read(areascout,temp_areaflags)) && (U.level < C.sysoplevel)) {
		return 0;
	}
	if ((!areamask_read(areascout,temp_maskflags)) && (U.level < C.sysoplevel)) {
		return 0;
	}

	sprintf(filename,"%s/%s/msgindex",C.areasdir,areascout);
	if ((FIL = fopen(filename,"r"))) {
		/* CHECKED - overprotected! */
		if (temp_highmsg > 4000) {
			temp_highmsg = 4000;
		} 
		fread(G.mymsgindex,temp_highmsg + 1,1,FIL);
		G.mymsgindex[temp_highmsg + 1] = 0;
		fclose(FIL);
	} else {
		return 0;
	}
	
	temp_areaflags[AREATRUE] = '1';

	strcpy(G.areaflags,temp_areaflags);
	strcpy(G.areamask,temp_maskflags);

	G.highmsg = temp_highmsg;
	G.current = G.pointer = temp_pointer;
	G.whichnext = 0;
	G.chain[0] = 0;

	return 1;
}

int area_clearup (void) {
/* MENU COMMAND */
/* CHECKED */
	int scout;
	char filename[MAINLINE + 100];
	FILE *FIL;

	if (!G.areaname[0]) {
		return 1;
	}

	for(scout = G.pointer;!strchr("BMR",G.mymsgindex[scout + 1]);scout++) {
		if (scout >= G.highmsg) {
			scout = G.highmsg;
			break;
		}
	}
	G.pointer = scout;

	sprintf(filename,"%s/%s/.areas/%s",C.users,U.id,G.areaname);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%d\n",G.pointer);
		fclose(FIL);
		return 1;
	} else {
		return 0;
	}
}

int check_area(const char mode, const char *areaname) {
/* CHECKED */
	struct stat statbuf;
	char string[MAINLINE + 100];


	if (!areaname[0]) {
		if (mode != 'q') {
			/*printf("No message area selected.\n");*/
			printf("%s\n",Ustring[124]);
		}
		return 0;
	}

	sprintf(string,"%s/%s",C.areasdir,areaname);
	if (stat(string,&statbuf)) {
		if (mode == 'v') {
			/*printf("There is no such area as %s.\n",areaname);*/
			printf(Ustring[65],areaname);/*"cannot find"*/
			printf("\n");
		}
		return 0;
	}

	sprintf(string,"%s/%s/area_lock",C.areasdir,areaname);
	if (!stat(string,&statbuf)) {
		if (U.level < C.sysoplevel) {
			if (mode != 'q') {
				/*printf("%s is locked for maintenance.  Abandoned.\n",areaname);*/
				printf(Ustring[132],areaname);
				printf("\n");
			}
			sprintf(G.errmsg,"%s has a area_lock",areaname);
			errorlog(G.errmsg);
			return 0;
		}
		if (mode != 'q') {
			printf(Ustring[132],areaname);
			printf("\n");
			/*printf("BEWARE - area_lock found.  Continuing...\n");*/
		}
	}
	return 1;
}

/*===============================================================*/
/* NEWS */

int news (char *dummy) {
/* MENU COMMAND */
	FILE *FIL;
	char filename[MAINLINE + 100];
	int newshigh;
	int marker;
	int last_to_read;
	int highmsg;

	printf("\n%s\n",Ustring[97]);
	if (is_area_elig('q',C.newsarea)) {
		newshigh = get_int_from_userfile(".newshigh",U.id);
		sprintf(filename,"%s/%s/highest",C.areasdir,C.newsarea);
		if ((FIL = fopen(filename,"r"))) {
			fscanf(FIL," %d ",&highmsg);
			fclose(FIL);
			if (newshigh < highmsg) {
				marker = highmsg + 1;
				last_to_read = newshigh + 1;

				/* CONSTCOND */
				while (1) {
					if (marker > last_to_read) {
						marker--;
						if (!definemsg('q',C.newsarea,marker)) {
							continue;
						}
						displaymsg(C.newsarea,marker);
					} else {
						/*
						printf("You have now read all the news.\n");
						printf("All the items are in '%s' if you want to read them again.\n",C.newsarea);
						*/
						printf("%s\n",Ustring[98]);
						break;
					}
					/*make_prompt("[C]ontinue with next item, or [q]uit reading news? ");*/
					if (!yes_no(Ustring[99])) {
						break;
					}
				}
				put_int_in_userfile(".newshigh",U.id,highmsg);
				return 1;
			}
		}
	}
#if 0
	printf("No system news since last time.\n");
#endif
	printf("%s\n",Ustring[100]);
	return 0;
}

/* ==================================================== */
/* MESSAGE NAVIGATION */

/* ARGSUSED0 */
int store (char *dummy) {
/* MENU COMMAND */
	char commandline[2 * MAINLINE + 200];

	sprintf(commandline,"rm -rf %s/%s/.tmpareas",C.users,U.id);
	dsystem(commandline);
	sprintf(commandline,"cp -r %s/%s/.areas %s/%s/.tmpareas",C.users,U.id,C.users,U.id);
	dsystem(commandline);
	/*printf("Pointers stored.\n");*/
	printf("%s\n",Ustring[189]);
	return 1;
}

/* ARGSUSED0 */
int restore (char *dummy) {
/* MENU COMMAND */
	char commandline[2 * MAINLINE + 200];
	struct stat statbuf;
	char filename[MAINLINE + 100];
	char *mod;

	sprintf(filename,"%s/%s/.tmpareas",C.users,U.id);
	if (stat(filename,&statbuf)) {
		/*printf("You have not stored your pointers.\n");*/
		printf("%s\n",Ustring[101]);
		return 0;
	}

	mod = drealmtime(statbuf.st_mtime);
#if 0
	sprintf(commandline,"Pointers last stored %s. Restore these? y/N",mod);
#endif
	sprintf(commandline,Ustring[102],mod);
	free(mod);

	if (no_yes(commandline)) {
		/*printf("Restore abandoned.\n");*/
		printf("%s\n",Ustring[60]);
		return 0;
	}
	sprintf(commandline,"rm -rf %s/%s/.areas",C.users,U.id);
	dsystem(commandline);
	sprintf(commandline,"cp -r %s/%s/.tmpareas %s/%s/.areas",C.users,U.id,C.users,U.id);
	dsystem(commandline);
	/*printf("Pointers restored.\n");*/
	printf("%s\n",Ustring[103]);
	return 1;
}

int point (char *params) {
/* Returns a position vaguely inside the range of messages available */
	struct valid_messages *vm;
	int result;

	if (is_num(params)) {
		result = atoi(params);
		if (result > G.highmsg) {
			moretoread(G.highmsg);
			return G.highmsg;
		} else {
			return result;
		}
	} else if ((vm = get_valid_messages('q',G.areaname,"",G.current,params,0))) {
		result = vm->msglist[0];
		free(vm->parse);
		free(vm->msglist);
		free(vm);
		return(result);
	}
	return 0;
}

int jump (char *in) {
/* MENU COMMAND */
	int p;
	char temp[21];
	char *copy;
	char prompt[MAINLINE];

	if (!check_area('v',G.areaname)) {
		return 0;
	}

	copy = strdup(in);
	shiftword(copy,temp,21);
	if (!temp[0]) {
		shiftword(G.comline,temp,21);
	}
	if (!Dstrcmp(temp,Ustring[501])) {
		shiftword(copy,temp,21);
		if (!temp[0]) {
			shiftword(G.comline,temp,21);
		}
	}
	free(copy);


	if (!temp[0]) {

		/*make_prompt("Jump to where? ");*/
		sprintf(prompt,"%s (%s)",Ustring[41],Ustring[200]);
		make_prompt(prompt);
		get_one_line(temp);
		tnt(temp);

		while(!strcmp(temp,"?")) {

			printf("\n");
			printf("[%s] %s\n",Ustring[11],Ustring[12]);
			printf("[%s] %s\n",Ustring[9],Ustring[10]);
			printf("[%s] %s\n",Ustring[7],Ustring[8]);
			printf("[%s] %s\n",Ustring[13],Ustring[0014]);
			printf("[%s] %s\n",Ustring[5],Ustring[6]);
			printf("[%s] %s\n",Ustring[17],Ustring[18]);
			printf("[%s] %s\n",Ustring[19],Ustring[20]);
			printf("[%s] %s\n",Ustring[1],Ustring[2]);
			printf("[%s] %s\n",Ustring[3],Ustring[4]);
			printf("[%s] %s\n",Ustring[29],Ustring[30]);
			printf("[%s] %s\n",Ustring[27],Ustring[28]);
			printf("    %s\n",Ustring[31]);
			printf("\n");
			/*make_prompt("Jump to where? ");*/
			make_prompt(prompt);
		
			get_one_line(temp);
			tnt(temp);
		}
	}

	if (!temp[0]) {
		return 0;
	}

	if (!Dstrcmp(temp,Ustring[29])) {
		G.pointer = 0;
		G.current = G.pointer;
		moretoread(G.pointer); /* This line is in the right order !*/
		G.chain[0] = 0;
		G.whichnext = 0;
		/*printf("All messages marked unread.\n");*/
		printf("%s\n",Ustring[74]);
		return 1;
	} else if (!Dstrcmp(temp,Ustring[27])) {
		moretoread(G.highmsg); /* This line is in the right order !*/
		G.pointer = G.highmsg;
		G.current = G.pointer;
		G.chain[0] = 0;
		G.whichnext = 0;
		/*printf("All messages marked as read.\n");*/
		printf("%s\n",Ustring[73]);
		return 1;
	} else if ((p = point(temp))) {
		G.pointer = (p - 1);
		moretoread(G.pointer);
		G.chain[0] = 0;
		G.whichnext = 0;
		readmsg("next"); /* readmsg("next") */
		return 1;
	} else {
		/*printf("'%s' invalid.\n",temp);*/
		printf(Ustring[472],temp);
		printf("\n");
		return 0;
	}
}

int skip (char *in) {
/* MENU COMMAND */
	int i;
	int num;
	int *msglist;
	char skipto[21];
	char temp[21];
	char *copy;

	if (!check_area('v',G.areaname)) {
		return 0;
	}

	copy = strdup(in);
	shiftword(copy,temp,21);
	if (!temp[0]) {
		shiftword(G.comline,temp,21);
	}
	free(copy);
	
	if (!temp[0]) {
 		make_prompt(Ustring[36]);
		get_one_char(temp);
	}
	
	if (!temp[0]) {
		return 0;
	}

	if (!Dstrcmp(temp,Ustring[34])) { /*thread*/
		if ((msglist = definethread ('v',G.areaname,G.current))) {
			for(i=0;msglist[i];i++) {
				G.mymsgindex[msglist[i]] = tolower(G.mymsgindex[msglist[i]]);
			}
			G.chain[0] = 0;
			G.whichnext = 0;
			free(msglist);
			/*printf("Current thread marked as read.\n");*/
			printf("%s %s\n",Ustring[78],Ustring[75]);
			return 1;
		} else {
			return 0;
		}

	} else if (!Dstrcmp(temp,Ustring[35])) { /*branch*/
		if ((msglist = definetree('v',G.areaname,G.current))) {
			for(i=0;msglist[i];i++) {
				G.mymsgindex[msglist[i]] = tolower(G.mymsgindex[msglist[i]]);
			}
			grepnums(G.chain,msglist);
			G.whichnext = shiftnum(G.chain);
			free(msglist);
#if 0
			printf("Current branch marked as read.\n");
#endif
			printf("%s %s\n",Ustring[79],Ustring[75]);
			return 1;
		} else {
			return 0;
		}

	} else if (is_num(temp) || !Dstrcmp(temp,Ustring[33])) { /*forward*/
		if (!is_num(temp)) {
			shiftword(in,temp,21);
			if (!temp[0]) {
				shiftword(G.comline,temp,21);
			}
		}
		/*new_get_one_param('v',"Skip forward how many?",temp,skipto,21);*/
		new_get_one_param('v',Ustring[37],temp,skipto,21);
		if (!skipto[0]) {
			return 0;
		}
		if (is_num(skipto)) {
			moretoread(G.highmsg);
			num = atoi(skipto);
			G.chain[0] = 0;
			G.whichnext = 0;
			if ((G.current + num) > G.highmsg) {
				/*printf("Skipping %d would take you past the end - marking all messages read.\n",num);*/
				printf(Ustring[38],num);
				printf(" - %s\n",Ustring[73]);
				G.pointer = G.highmsg;
				G.current = G.pointer;
			} else {
				G.pointer = (G.current + (num - 1));
				readmsg("next");
			}
			return 1;
		} else {
			/*printf("'%s' invalid.\n",skipto);*/
			printf(Ustring[472],skipto);
			printf("\n");
			return 0;
		}

	} else if (!Dstrcmp(temp,Ustring[32])) { /*backward*/
		shiftword(in,temp,21);
		if (!temp[0]) {
			shiftword(G.comline,temp,21);
		}
		/*new_get_one_param('v',"Skip back how many?",temp,skipto,21);*/
		new_get_one_param('v',Ustring[39],temp,skipto,21);
		if (!skipto[0]) {
			return 0;
		}
		if (is_num(skipto)) {
			num = atoi(skipto);
			G.chain[0] = 0;
			G.whichnext = 0;

			if ((G.current - num) < 1) {
				/*printf("Skipping %d would take you back before 1 - marking all messages unread.\n",num);*/
				printf(Ustring[40],num);
				printf(" - %s\n",Ustring[74]);
				G.pointer = 0;
				G.current = G.pointer;
				moretoread(G.pointer); /* This line is in the right order !*/
			} else {
				G.pointer = (G.current - (num + 1));
				moretoread(G.pointer);
				readmsg("next");
			}
			return 1;
		} else {
			/*printf("'%s' invalid.\n",skipto);*/
			printf(Ustring[472],skipto);
			printf("\n");
			return 0;
		}
	} else {
		/*printf("'%s' invalid.\n",temp);*/
		printf(Ustring[472],temp);
		printf("\n");
		return 0;
	}
}

/* ARGSUSED2 */
int *findnumeric(const char mode, const char *area, const int msgno) {
	int scout;
	int *list;
	char *header;

	list = (int *)malloc(2 * sizeof (int));
	list[0] = 0;
	list[1] = 0;

	/* CONSTCOND */
	while (1) {
		scout = G.pointer + 1;
		/* CONSTCOND */

		for(;!strchr("BMR",G.mymsgindex[scout]);scout++) {
			if (scout > G.highmsg) {
				break;
			}
		}
		if (scout > G.highmsg) {
			if (moretoread(G.highmsg)) {
				continue;
			}
			if (mode == 'v') {
				/*printf("There are no more unread messages in %s.\n",area);*/
				printf(Ustring[186],area);
				printf("\n");
			}
			G.pointer = G.highmsg;
			free(list);
			return NULL;
		}
		G.pointer = scout;
		G.whichnext = 0;
		header = definemsg(mode,area,scout);
		if (header) {
			G.mymsgindex[scout] = tolower(G.mymsgindex[scout]);
			list[0] = scout;
			list[1] = 0;
			free(header);
		} else {
			G.mymsgindex[scout] = 'd';
			G.whichnext = 0;
			continue;
		}
		break;
	}
	return list;
}

/* ARGSUSED2 */
int *findtreewise(const char mode, const char *area, const int msgno) {
	int scout;
	int i;
	char string[5];
	int *temp;
	int *list;
	int *stol;
	char *header;

	list = (int *)malloc(2 * sizeof (int));
	list[0] = 0;
	list[1] = 0;

	/* CONSTCOND */
	while (1) {
		if (G.whichnext) {
			scout = G.whichnext;
		} else {
			scout = G.pointer + 1;
			/* CONSTCOND */
			while (1) {
				if (mode == 'v') {
					/*printf("Finding next active chain.\n");*/
				}

				for(;!strchr("BMR",G.mymsgindex[scout]);scout++) {
					if (scout > G.highmsg) {
						break;
					}
				}
				if (scout > G.highmsg) {
					if (moretoread(G.highmsg)) {
						continue;
					}
					if (mode == 'v') {
						/*printf("There are no more unread messages in %s.\n",area);*/
						printf(Ustring[186],area);
						printf("\n");
					}
					G.pointer = G.highmsg;
					free(list);
					return NULL;
				}
				G.pointer = scout;
				break;
			}
		}
		/*
		 * This happens whether whichnext was set or not
		 */
		header = definemsg(mode,area,scout);
		if (header) {
			for (i=0;i<=17;i++) {
				shiftword(header,string,5);
			}

			stol = string_to_list(header);
			temp = combinenums(stol,G.chain);
			i = 0;
			for(i=0;(i < 255) && temp[i];i++) {
				G.chain[i] = temp[i];
			}
			G.chain[i] = 0;
			free(temp);
			free(stol);			

			free(header);

			G.whichnext = shiftnum(G.chain);
			if (!strchr("BMR",G.mymsgindex[scout])) {
				putchar('.');
				continue;
			}
			G.mymsgindex[scout] = tolower(G.mymsgindex[scout]);
			list[0] = scout;
			list[1] = 0;
		} else {
			G.mymsgindex[scout] = 'd';
			G.whichnext = shiftnum(G.chain);
			continue;
		}
		break;
	}
	return list;
}

/* ARGSUSED2 */
int *findthreadwise(const char mode, const char *area, const int msgno) {
	int scout;
	int i;
	char string[5];
	char *header;
	int *list;

	list = (int *)malloc(2 * sizeof (int));
	list[0] = 0;
	list[1] = 0;


	/* CONSTCOND */
	while (1) {
		if (G.whichnext) {
			scout = G.whichnext;
		} else {
			scout = G.pointer + 1;
			/* CONSTCOND */
			while (1) {
				if (mode == 'v') {
					/*printf("Finding next active thread.\n");*/
				}

				for(;!strchr("BMR",G.mymsgindex[scout]);scout++) {
					if (scout > G.highmsg) {
						break;
					}
				}
				if (scout > G.highmsg) {
					if (moretoread(G.highmsg)) {
						continue;
					}

					if (mode == 'v') {
						/*printf("There are no more unread messages in %s.\n",area);*/
						printf(Ustring[186],area);
					}
					G.pointer = G.highmsg;
					free(list);
					return NULL;
				}
				G.pointer = scout;
				break;
			}
		}
		header = definemsg(mode,area,scout);
		if (header) {
			for (i=0;i<=16;i++) {
				shiftword(header,string,5);
			}
			free(header);
			G.whichnext = atoi(string);
			if (!strchr("BMR",G.mymsgindex[scout])) {
				putchar('.');
				continue;
			}
			G.mymsgindex[scout] = tolower(G.mymsgindex[scout]);
			if (!G.whichnext) {
				if (mode == 'v') {
#if 0
					printf("\nThis was the last message in the current thread.\n");
					printf(Ustring[88],Ustring[78]);
					printf("\n");
#endif
				}
			}

			list[0] = scout;
			list[1] = 0;
		} else {
			G.mymsgindex[scout] = 'd';
			G.whichnext = 0;
			continue;
		}
		break;
	}
	return list;
}

int *findnext(const char mode, const char *area, const int msgno) {
	if (!strcmp(U.readmode,"REFERENCE")) {
		return findtreewise(mode,area,msgno);
	} else if (!strcmp(U.readmode,"THREADWISE")) {
		return findthreadwise(mode,area,msgno);
	} else if (!strcmp(U.readmode,"NUMERIC")) {
		return findnumeric(mode,area,msgno);
	} else {
		if (mode != 'q') {
			printf("%s %s %s\n",Ustring[190],U.readmode,Ustring[62]);
		}
		return NULL;
	}
	/* NOTREACHED */
}

/* ARGSUSED2 */
int *findlast(const char mode, const char *area, const int msgno) {
	int scout;
	int *list;
	char *header;

	moretoread(G.highmsg);
	scout = G.highmsg;

	for(;!strchr("BbMmRr",G.mymsgindex[scout]);scout--) {
		if (scout < 1) {
			break;
		}
	}
	if (scout >= 1) {
		header = definemsg(mode,area,scout);
		if (header) {
			free(header);
			list = (int *)malloc(2 * sizeof (int));
			list[0] = scout;
			list[1] = 0;
			return list;
		} else {
			return NULL;
		}
	} else {
		if (mode == 'v') {
			/*printf("There are no messages in %s.  Why not post one?\n",area);*/
			printf(Ustring[188],area);
			printf("\n");
		}
		return NULL;
	}
}

/* ARGSUSED2 */
int *findfirst(const char mode, const char *area, const int msgno) {
	int scout = 1;
	int *list;
	char *header;

	/* CONSTCOND */
	while (1) {
		for(;!strchr("BbMmRr",G.mymsgindex[scout]);scout++) {
			if (scout > G.highmsg) {
				break;
			}
		}
		if (scout <= G.highmsg) {
			header = definemsg(mode,area,scout);
			if (header) {
				free(header);
				list = (int *)malloc(2 * sizeof (int));
				list[0] = scout;
				list[1] = 0;
				return list;
			} else {
				return NULL;
			}
		} else {
			if (moretoread(G.highmsg)) {
				continue;
			}
			if (mode == 'v') {
				/*printf("There are no messages in %s.  Why not post one?\n",area);*/
				printf(Ustring[188],area);
				printf("\n");
			}
			return NULL;
		}
	}
	/* NOTREACHED */
}

int *findnumber(const char mode, const char *area, const int msgno) {
	char *header;
	int *list;

	if (!msgno) {
		if (mode == 'v') {
			/*printf("That value has not yet been defined.\n");*/
			printf("%s\n",Ustring[81]);
		}
		return NULL;
	}

	header = definemsg(mode,area,msgno);
	if (!header) {
		return NULL;
	}

	free(header);

	list = (int *)malloc(2 * sizeof (int));
	list[0] = msgno;
	list[1] = 0;
	return list;
}

int *findbase(const char mode, const char *area, const int msgno) {
	int i;
	int scout;
	char string[5];
	char *header;
	int *list;

	if (!msgno) {
		if (mode == 'v') {
			/*printf("You have not just read a message of which to find the base.\n");*/
			printf("%s\n",Ustring[82]);
		}
		return NULL;
	}

	header = definemsg(mode,area,msgno);
	if (!header) {
		return NULL;
	}
	for (i=0;i<=15;i++) {
		shiftword(header,string,5);
	}

	free(header);
	scout = atoi(string);
	header = definemsg(mode,area,scout);

	if (!header) {
		return NULL;
	}
	free(header);
	list = (int *)malloc(2 * sizeof (int));
	list[0] = scout;
	list[1] = 0;
	return list;
}

int *findupchain(const char mode, const char *area, const int msgno) {
	int i;
	int scout;
	char string[5];
	int *list;
	char *header;


	if (!msgno) {
		if (mode == 'v') {
			/*printf("You have not just read a message to find the parent of.\n");*/
			printf("%s\n",Ustring[82]);
		}
		return NULL;
	}

	header = definemsg(mode,area,msgno);
	if (!header) {
		return NULL;
	}
	for (i=0;i<=12;i++) {
		shiftword(header,string,5);
	}
	free(header);
	scout = atoi(string);
	if (!scout) {
		if (mode == 'v') {
			/*printf("The current message does not have a parent.\n");*/
			printf("%s\n",Ustring[83]);
		}
		return NULL;
	}
	header = definemsg(mode,area,scout);

	if (!header) {
		return NULL;
	}
	free(header);
	list = (int *)malloc(2 * sizeof (int));
	list[0] = scout;
	list[1] = 0;
	return list;

}

int *findbackthread(const char mode, const char *area, const int msgno){
	int scout;
	int i;
	char string[5];
	char *header;
	int *list;

	if (!msgno) {
		if (mode == 'v') {
			/*printf("You have not just read a message to travel upthread from.\n");*/
			printf("%s\n",Ustring[82]);
		}
		return NULL;
	}

	header = definemsg(mode,area,msgno);
	if (!header) {
		return NULL;
	}
	for (i=0;i<=15;i++) {
		shiftword(header,string,5);
	}

	if (atoi(string) == msgno) {
		if (mode == 'v') {
			/*printf("There are no lower messages in the thread.\n");*/
			printf(Ustring[76],Ustring[78]);
			printf("\n");
		}
		free(header);
		return NULL;
	}
	for (;i<=17;i++) {
		shiftword(header,string,5);
	}
	free(header);
	scout = atoi(string);
	header = definemsg(mode,area,scout);

	if (!header) {
		return NULL;
	}
	free(header);
	list = (int *)malloc(2 * sizeof (int));
	list[0] = scout;
	list[1] = 0;
	return list;
}

int *findonthread(const char mode, const char *area, const int msgno) {
	int scout;
	int i;
	char string[5];
	char *header;
	int *list;

	if (!msgno) {
		if (mode == 'v') {
			/*printf("You have not just read a message to travel onthread from.\n");*/
			printf("%s\n",Ustring[82]);
		}
		return NULL;
	}

	header = definemsg(mode,area,msgno);
	if (!header) {
		return NULL;
	}
	for (i=0;i<=16;i++) {
		shiftword(header,string,5);
	}
	free(header);
	if (!strcmp(string, "-")) {
		if (mode == 'v') {
			/*printf("There are no further messages in the thread.\n");*/
			printf(Ustring[77],Ustring[78]);
			printf("\n");
		}
		return NULL;
	}
	scout = atoi(string);
	header = definemsg(mode,area,scout);
	if (!header) {
		return NULL;
	}
	free(header);
	list = (int *)malloc(2 * sizeof (int));
	list[0] = scout;
	list[1] = 0;
	return list;
}

int *findback(const char mode, const char *area, const int msgno) {
	int scout;
	int *list;
	char *header;

	scout = msgno ? (msgno - 1) : (G.pointer ? (G.pointer - 1) : G.highmsg);

	for(;!strchr("BbMmRr",G.mymsgindex[scout]);scout--) {
		if (scout < 1) {
			break;
		}
	}
	if (scout >= 1) {
		header = definemsg(mode,area,scout);
		if (header) {
			free(header);
			list = (int *)malloc(2 * sizeof (int));
			list[0] = scout;
			list[1] = 0;
			return list;
		} else {
			return NULL;
		}
	} else {
		if (mode == 'v') {
			/*printf("There are no lower messages in %s.\n",area);*/
			printf(Ustring[76],area);
			printf("\n");
		}
		return NULL;
	}
}

int *findforward(const char mode, const char *area, const int msgno) {
	int scout;
	int *list;
	char *header;

	scout = msgno ? (msgno + 1) : (G.pointer + 1);

	/* CONSTCOND */
	while (1) {
		for(;!strchr("BbMmRr",G.mymsgindex[scout]);scout++) {
			if (scout > G.highmsg) {
				break;
			}
		}
		if (scout <= G.highmsg) {
			header = definemsg(mode,area,scout);
			if (header) {
				free(header);
				list = (int *)malloc(2 * sizeof (int));
				list[0] = scout;
				list[1] = 0;
				return list;
			} else {
				return NULL;
			}
		} else {
			if (moretoread(G.highmsg)) {
				continue;
			}
			if (mode == 'v') {
				/*printf("There are no higher messages in %s.\n",area);*/
				printf(Ustring[77],area);
				printf("\n");
			}
			return NULL;
		}
	}
	/* NOTREACHED */
}



/* ============ */


int is_msg_elig (int msgno) {
	if (U.level >= C.sysoplevel) {
		return 3;
	} else if (U.flags[CHAIRMAN] != '0') {
		return 2;
	} else {
		char temp[MAINLINE + 50];
		struct stat statbuf;
		int i;
		char *header = definemsg('q',G.areaname,msgno);

		if (!header) {
			return 0;
		}
		sprintf(temp,"%s/%s/msg.%d",C.areasdir,G.areaname,msgno);
		if (stat(temp,&statbuf)) {
			free(header);
			return 0;
		}
		if (statbuf.st_uid == G.uid) {
			free(header);
			return 1;
		}
		for(i=0; i <= 9; i++) {
			shiftword(header,temp,9);
		}
		free(header);
		return !strcmp(temp,U.id);
	}
}


/* ======================================= */
/* THREADING, LINKING, COPYING, DELETING AND FLAGGING */

int *definetree (const char mode, const char *area, const int msgno) {
	struct areaheader ah;
	int test_parent;
	int i;
	int j;
	int k;
	int temptree[1024];

	char *header;
	int *thread;
	int *tree;

	thread = definethread('q',area,msgno);
	if (!thread) {
		if (mode == 'v') {
			/*printf("Could not define a branch from your current position.\n");*/
			printf("%s\n",Ustring[84]);
		}
		return NULL;
	}

	k = 0;
	temptree[k] = msgno;
	temptree[k + (unsigned)1] = 0;
	k++;
	for (i=1;thread[i];i++) {
		if ((header = definemsg('q',area,thread[i]))) {
			parse_area_header(header,&ah);
			test_parent = atoi(ah.parent);	/*parent of one under scrutiny */

			/* then look to see if the parent is already in the tree list */
			for (j=0;temptree[j];j++) {
				if (temptree[j] == test_parent) {
					/* the message should be in the tree */
					if (k < 1023) {
						temptree[k] = thread[i];
						temptree[k+ (unsigned)1] = 0;
						k++;
					} else {
						free(header);
						free(thread);
						sprintf(G.errmsg,"Tree too big from msg %d area %s.\n",msgno,area);
						errorlog(G.errmsg);
						return NULL;
					}
					break;
				}
			}
			free(header);
		}
	}
	free(thread);

	tree = (int *)malloc((k+1) * sizeof (int));
	i = 0;
	for (i=0;temptree[i];i++) {
		
		tree[i] = temptree[i];
	}
	tree[i] = 0;		
	return tree;
}

int *definethread (const char mode, const char *area, const int msgno) {
	char string[5];
	struct areaheader hs;
	int tempthread[1024];
	char next[5];
	int i;
	char *header;
	int *thread;

	header = definemsg('q',area,msgno);
	if (!header) {
		if (mode == 'v') {
			/*printf("Could not define a thread from your current position.\n");*/
			printf("%s\n",Ustring[85]);
		}
		return NULL;
	}
	parse_area_header(header,&hs);

	strcpy(string,hs.base);
	free(header);

	header = definemsg('q',area,atoi(string));
	if (!header) {
		if (mode == 'v') {
			/*printf("Could not define a thread from your current position.\n");*/
			printf("%s\n",Ustring[85]);
		}
		return NULL;
	}

	parse_area_header(header,&hs);
	strcpy(next,hs.next);
	i = 0;
	tempthread[i] = atoi(hs.number);
	i++;
	free(header);

	while (is_num(next)) {
		if (i > 1022) {
			sprintf(G.errmsg,"Thread too big from msg %d area %s.\n",msgno,area);
			errorlog(G.errmsg);
			return NULL;
		}
		header = definemsg('q', area, atoi(next));
		if (!header) {
			break;
		}
		tempthread[i] = atoi(hs.number);
		i++;

		parse_area_header(header,&hs);
		strcpy(next,hs.next);
		free(header);
	}
	tempthread[i] = 0;	
	
	thread = (int *)malloc((i + 1) * sizeof (int));
	for (i=0;tempthread[i];i++) {
		thread[i] = tempthread[i];
	}
	thread[i] = 0;
	
	return thread;
}


int attach_messages (char *area, int link_to, int *sublist) {
	char newheader[1024];
	char string[50];
	unsigned int i = 0; /* it only holds +ve numbers */
	int j = 0;
	char temp[MAINLINE + 100];
	FILE *HANDLE;
	char *header;
	int *thread;
	int *newlist;

	if (!(header = definemsg('q',area,link_to))) {
		return 0;
	}
	while (i <= 15) {
		shiftword(header,temp,5);
		i++;
	}
	free(header);

	thread = definethread('q',area,atoi(temp));
	if (!thread) {
		return 0;
	}

	header = definemsg('q',area,sublist[0]);
	if (!header) {
		free(thread);
		return 0;
	}
	i = 0;

	newheader[0] = 0;
	while (i <= 9) {
		shiftword(header,string,50);
		strcat(newheader,string);
		strcat(newheader," ");
		i++;
	}

	sprintf(string,"linked-to # %d ",link_to);
	strcat(newheader,string);


	while (i <= 12) {
		shiftword(header,string,50);
		i++;
	}

	if (!strncmp(header,"[Parent deleted] ",17)) {
		strcat(newheader,"[Parent deleted] ");
	} else {
		strcat(newheader,"- - ");
	}
	while (i <= 14) {
		shiftword(header,string,50);
		i++;
	}

	while (i <= 16) {
		shiftword(header,string,50);
		strcat(newheader,string);
		strcat(newheader," ");
		i++;
	}
	shiftword(header,string,50);
	strcat(newheader,string);
	i++;
	strcat(newheader,"\n");

	while (header[0]) {
		shiftword(header,string,50);
		strcat(newheader,string);
		strcat(newheader," ");
	}

	sprintf(temp,"%s/%s/hdr.%d",C.areasdir,area,sublist[0]);
	HANDLE = fopen(temp,"w");
	fputs(newheader,HANDLE);
	fclose(HANDLE);

	newheader[0] = 0;
	free(header);

	header = definemsg('q',area,link_to);
	if (!header) {
		free(thread);
		return 0;
	}
	i = 0;
	while (i <= 16) {
		shiftword(header,string,50);
		strcat(newheader,string);
		strcat(newheader," ");
		i++;
	}
	shiftword(header,string,50);
	strcat(newheader,string);
	i++;
	strcat(newheader,"\n");

	i=j=0;

	while (header[i]) {
		if (header[i] == ' ') {
			j++;
		}
		i++;
	}

	newlist = (int *)malloc((j+2) * sizeof (int));

	tnt(header);
	for (i=0;header[0];i++) {
		shiftword(header,string,5);
		newlist[i] = atoi(string);
	}
	newlist[i] = sublist[0];
	newlist[i+(unsigned)1] = 0;
	qsort((void *)newlist,i+1,sizeof (int),intsort);

	for(i=0;newlist[i];i++) {
		sprintf(string,"%d ",newlist[i]);
		strcat(newheader,string);
	}
	free(newlist);

	sprintf(temp,"%s/%s/hdr.%d",C.areasdir,area,link_to);
	HANDLE = fopen(temp,"w");
	fputs(newheader,HANDLE);
	fclose(HANDLE);

	newlist = combinenums(thread,sublist);
	relink_thread(area,newlist);
	newlist[0] = sublist[0];
	newlist[1] = 0;
	msgindex(area,'M',newlist);
	free(newlist);
	free(header);
	free(thread);
	return 1;
}

int detach_messages (char *area, int msgno, int *sublist) {
	char string[MAINLINE + 100];
	FILE *HANDLE;
	int sub_base;
	int from_base;
	int from_msg;
	int i = 0;

	char *oldheader;
	char *newheader;
	int *thread;

	oldheader = definemsg('q',area,msgno);
	if (!oldheader) {
		return 0;
	}

	sub_base = sublist[0];

	while (i <= 12) {
		shiftword(oldheader,string,50);
		i++;
	}
	from_msg = atoi(string);
	while (i <= 15) {
		shiftword(oldheader,string,50);
		i++;
	}
	from_base = atoi(string);
	free(oldheader);

	thread = definethread('q',area,from_base);
	if (!thread) {
		return 0;
	}

	if ((oldheader = definemsg('q',area,from_msg))) {
		newheader = strdup(oldheader);
		newheader[0] = 0;

		i = 0;
		while (i <= 16) {
			shiftword(oldheader,string,50);
			strcat(newheader,string);
			strcat(newheader," ");
			i++;
		}
		shiftword(oldheader,string,50);
		strcat(newheader,string);
		i++;

		sprintf(string,"%s/%s/hdr.%d",C.areasdir,area,from_msg);
		HANDLE = fopen(string,"w");
		fprintf(HANDLE,"%s\n",newheader);
		while (oldheader[0]) {
			shiftword(oldheader,string,50);
			if (sub_base != atoi(string)) {
				fprintf(HANDLE,"%s ",string);
			}
		}
		fclose(HANDLE);
		free(newheader);
		free(oldheader);
	}

	grepnums(thread,sublist);
	if (thread[0]) {
		relink_thread(area,thread);
	}

	relink_thread(area,sublist);
	free(thread);
	return 1;

}

int relink_thread (const char *area, int *msglist) {
	char finalheader[MAINLINE];
	char finalfooter[1024];
	char shiftlook[80];
	unsigned int listlen = 0;
	int listindex = 0;
	int base[2];
	int numlook = 0;
	int previous = 0;
	int i = 0;
	FILE *HEADER;
	char *startheader;


	base[0] = msglist[0];
	base[1] = 0;
	while (msglist[listlen]) {
		listlen++;
	}
	qsort((void *)msglist,listlen,sizeof (int),intsort);

/* This is the base message header being written */
	startheader = definemsg('q',area,msglist[0]);
	if (!startheader) {
		return 0;
	}

	finalheader[0] = 0;
	for (i = 0;i <= 9;i++) {
		shiftword (startheader,shiftlook,80);
		strcat(finalheader, shiftlook);
		strcat(finalheader, " ");

	}

	strcat(finalheader, "BASE-MESSAGE - - ");
	for(;i <= 12; i++) {
		shiftword (startheader,shiftlook,80);
	}

	if (strncmp(startheader,"[Parent deleted] ",17)) {
		strcat(finalheader, "- - ");
	} else {
		strcat(finalheader, "[Parent deleted] ");
	}
	for(;i <= 14; i++) {
		shiftword (startheader,shiftlook,80);
	}

	sprintf(shiftlook,"%d ",msglist[0]);
	strcat(finalheader,shiftlook);

	if (listlen > 1) {
		sprintf(shiftlook,"%d ",msglist[1]);
		strcat(finalheader,shiftlook);
	} else {
		strcat(finalheader,"- ");
	}

	sprintf(shiftlook, "%d", msglist[listlen - (unsigned)1]);
	strcat(finalheader,shiftlook);

	for(;i <= 17; i++) {
		shiftword (startheader,shiftlook,80);
	}

	finalfooter[0] = 0;
	while (startheader[0]) {
		shiftword (startheader,shiftlook,80);
		numlook	= atoi(shiftlook);

		i = 0;
		while (msglist[i]) {
			if (numlook == msglist[i]) {
				sprintf(shiftlook, "%d ",msglist[i]);
				strcat(finalfooter,shiftlook);
				break;
			}
			i++;
		}
	}

	sprintf(shiftlook,"%s/%s/hdr.%d", C.areasdir, area, msglist[0]);
	HEADER = fopen(shiftlook, "w");
	fprintf(HEADER,"%s\n%s",finalheader,finalfooter);
	fclose(HEADER);
	free(startheader);


	(void)msgindex(area,'B',base);
/* The end of the base header being written */

	previous = msglist[0];
	listindex = 1;
	while (listindex < listlen) {
		startheader = definemsg('q',area,msglist[listindex]);
		if (!startheader) {
			/*printf("what didn't like %d\n",msglist[listindex]);*/
			return 0;
		}

		finalheader[0] = 0;
		for (i = 0;i <= 14;i++) {
			shiftword (startheader,shiftlook,80);
			strcat(finalheader, shiftlook);
			strcat(finalheader, " ");
		}

		sprintf(shiftlook,"%d ",msglist[0]);
		strcat(finalheader,shiftlook);

		if (listindex+1 < listlen) {
			sprintf(shiftlook,"%d ", msglist[listindex + (unsigned)1]);
			strcat(finalheader,shiftlook);
		} else {
			strcat(finalheader,"- ");
		}
		sprintf(shiftlook,"%d",previous);

		strcat(finalheader,shiftlook);
		for (;i <= 17;i++) {
			shiftword (startheader,shiftlook,80);
		}

		finalfooter[0] = 0;
		while (startheader[0]) {
			shiftword (startheader,shiftlook,80);
			numlook	= atoi(shiftlook);

			i = 0;
			while (msglist[i]) {
				if (numlook == msglist[i]) {
					sprintf(shiftlook, "%d ",msglist[i]);
					strcat(finalfooter,shiftlook);
					break;
				}
				i++;
			}
		}

		sprintf(shiftlook,"%s/%s/hdr.%d", C.areasdir, area, msglist[listindex]);
		HEADER = fopen(shiftlook, "w");
		fprintf(HEADER,"%s\n%s",finalheader,finalfooter);
		fclose(HEADER);
		free(startheader);
		previous = msglist[listindex];
		listindex++;
	}

	(void)msgindex(area,'M',&msglist[1]);
	return 1;
}


int linkmsg (char *in) {
/* MENU COMMAND */
	char string[21];
	char fromlock[MAINLINE + 100];
	char tolock[MAINLINE + 100];
	char objectlock[MAINLINE + 100];
	int object;
	int target;
	int *list;
	struct valid_messages *vm;
	int result = 0;
	char *copy;

	if (!check_area('v',G.areaname)) {
		flushcom("");
		
		return 0;
	}

	copy = strdup(in);
	shiftword(copy,string,21);
	if (!string[0]) {
		shiftword(G.comline,string,21);
	}

	if (!Dstrcmp(string,Ustring[501])) {
		strcpy(string,"current");
	}

	/*vm = get_valid_messages('v',G.areaname,"Link which message?",G.current,string,1);*/
	vm = get_valid_messages('v',G.areaname,Ustring[113],G.current,string,1);
	if (!vm) {
		free(copy);
		flushcom("");
		return 0;
	}
	object = vm->msglist[0];
	strcpy(fromlock,vm->lock);
	free(vm->lock);
	free(vm->parse);
	free(vm->msglist);
	free(vm);

	if (!is_msg_elig(object)) {
		/*printf("You do not have any rights over %d.\n",object);*/
		printf(Ustring[80],object);
		printf("\n");
		rem_lock(fromlock);
		flushcom("");
		return 0;
	}
	list = definetree('v',G.areaname,object);
	if (!list) {
		rem_lock(fromlock);
		flushcom("");
		return 0;
	}

	shiftword(copy,string,21);
	if (!string[0]) {
		shiftword(G.comline,string,21);
	}

	if (!Dstrcmp(string,Ustring[501])) {
		shiftword(copy,string,21);
		if (!string[0]) {
			shiftword(G.comline,string,21);
		}
	}

	free(copy);
	/*vm = get_valid_messages('v',G.areaname,"To which message?",G.current,string,1);*/
	vm = get_valid_messages('v',G.areaname,Ustring[114],G.current,string,1);
	if (!vm) {
		rem_lock(fromlock);
		flushcom("");
		free(list);
		return 0;
	}

	if (!strcmp(vm->parse,"thread") || !strcmp(vm->parse,"branch")) {
		/*printf("You must provide a specific message to accept the link.\n");*/
		printf("%s\n",Ustring[115]);
		rem_lock(vm->lock);
		free(vm->lock);
		free(vm->parse);
		free(vm->msglist);
		free(vm);
		rem_lock(fromlock);
		flushcom("");
		free(list);
		return 0;
	}

	target = vm->msglist[0];
	strcpy(tolock,vm->lock);
	free(vm->lock);
	free(vm->parse);
	free(vm->msglist);
	free(vm);

	if (target >= object) {
		/*printf("You may only link a newer message to an older one.\n");*/
		printf("%s\n",Ustring[116]);
		rem_lock(fromlock);
		rem_lock(tolock);
		flushcom("");
		free(list);
		return 0;
	}

	hups_off();
	sprintf(objectlock,"%s/%s/%d.lock",C.areasdir,G.areaname,object);
	if (strcmp(fromlock,objectlock)) {
		if (!place_lock('v',objectlock,1,1)) {
			rem_lock(fromlock);
			rem_lock(tolock);
			flushcom("");
			free(list);
			return 0;
		}
	}

	if (detach_messages(G.areaname,object,list)) {
		result = attach_messages(G.areaname,target,list);
	}

	rem_lock(objectlock);
	rem_lock(fromlock);
	rem_lock(tolock);
	hups_on();
	if (result) {
		/*printf("Linked.\n");*/
		printf("%s\n",Ustring[117]);
	}
	free(list);
	return result;
}


int unlinkmsg (char *in) {
/* MENU COMMAND */
	char fromlock[MAINLINE + 100];
	char objectlock[MAINLINE + 100];
	int object;
	int *list;
	int result = 0;
	struct valid_messages *vm;
	char temp[21];
	char *copy;

	if (!check_area('v',G.areaname)) {
		return 0;
	}

	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		vm = get_valid_messages('v',G.areaname,"",G.current,copy,1);
	} else {
		shiftword(G.comline,temp,21);
		vm = get_valid_messages('v',G.areaname,"",G.current,temp,1);
	}
	free(copy);

	if (!vm) {
		return 0;
	}

	strcpy(fromlock,vm->lock);
	object = vm->msglist[0];
	sprintf(objectlock,"%s/%s/%d.lock",C.areasdir,G.areaname,object);

	free(vm->lock);
	free(vm->parse);

	if (!strcmp(fromlock,objectlock)) {
		printf("That would be a pointless move.\n");
		printf("%s\n",Ustring[118]);
		rem_lock(fromlock);
		free(vm->msglist);
		free(vm);
		return 0;
	}
	if (!is_msg_elig(object)) {
		/*printf("You do not have any rights over %d.\n",object);*/
		printf(Ustring[80],object);
		printf("\n");
		rem_lock(fromlock);
		free(vm->msglist);
		free(vm);
		return 0;
	}

	if (!vm->msglist[1]) {
		list = definetree('v',G.areaname,object);
		free(vm->msglist);
	} else {
		list = vm->msglist;
	}

	place_lock('q',objectlock,1,1);


	hups_off();

	result = detach_messages(G.areaname,object,list);

	rem_lock(objectlock);
	rem_lock(fromlock);

	hups_on();

	free(vm);
	free(list);
	if (result) {
		/*printf("Unlinked.\n");*/
		printf("%s\n",Ustring[191]);
	}
	return result;
}


int delete (char *in) {
/* MENU COMMAND */
	FILE *FIL;
	unsigned int i; /* only ever holds +ve numbers */
	int base;
	char tempheader[MAINLINE];
	int grandad;
	char filename[MAINLINE + 100];
	char string[20];
	char stringa[20];
	struct areaheader ah;
	int delno;

	char *copy;

	struct valid_messages *vm;
	int *thread;
	int *grandkids;
	int *siblings;
	int *adopted;
	char *grandchild;
	char *grandparent;
	char *header;

	if (!check_area('v',G.areaname)) {
		flushcom("");
		return 0;
	}
	
	copy = strdup(in);
	shiftword(copy,string,20);
	if (!string[0]) {
		shiftword(G.comline,string,20);
	}
	free(copy);

	vm = get_valid_messages('v',G.areaname,"",G.current,string,1);
	if (!vm) {
		return 0;
	}

	if (!vm->msglist[1]) {
	/* All this block is for a single message deletion */
	
		delno = vm->msglist[0];
		header = definemsg('q',G.areaname,delno);
		if (!header) {
			/*puts("Sorry, the header has disappeared.");*/
			sprintf(filename,Ustring[193],delno);
			printf(Ustring[65],filename);
			printf("\n");


			rem_lock(vm->lock);
			free(vm->msglist);
			free(vm->parse);
			free(vm->lock);
			free(vm);	
			return 0;
		}

		parse_area_header(header,&ah);
		
		if (!is_msg_elig(delno)) {
			/*printf("Sorry, you do not have any rights over %s.\n",vm->parse);*/
			printf(Ustring[80],vm->msglist[0]);
			printf("\n");
			rem_lock(vm->lock);
			free(header);
			free(vm->msglist);
			free(vm->parse);
			free(vm->lock);
			free(vm);	
			return 0;
		}
		
		sprintf(stringa,"%d",delno);
		sprintf(string,Ustring[108],stringa);
		if (!no_yes(string)) {
			rem_lock(vm->lock);
			free(header);
			free(vm->msglist);
			free(vm->parse);
			free(vm->lock);
			free(vm);	
			return 0;
		}

		base = atoi(ah.base);

		if ((delno == base) && (strcmp(ah.next,"-"))) {
		/* If this is a base message we don't erase it */
			sprintf(filename,"%s/%s/msg.%d",C.areasdir,G.areaname,base);
			FIL = fopen(filename,"w");
			fputs("Subject:\n\nMessage deleted.\n",FIL);
			fclose(FIL);
			rem_lock(vm->lock);
			free(header);
			free(vm->msglist);
			free(vm->parse);
			free(vm->lock);
			free(vm);	
			return 1;
		} 
/*===============================================*/
		/* This section for single non-base message */
		
		thread = definethread('q',G.areaname,delno);
		if (!thread) {
			rem_lock(vm->lock);
			free(header);
			free(vm->msglist);
			free(vm->parse);
			free(vm->lock);
			free(vm);	
			return 0;
		}
		hups_off();

		grandkids = string_to_list(ah.footer);
		grandad = atoi(ah.parent);
		grandparent = definemsg('q',G.areaname,grandad);

		free(header);

		if (!grandparent) {
			/* Find the eldest child to look after the family*/
			/* We could have searched back for an ancestor first but see how this works*/
			while (grandkids[0]) {
				grandad = shiftnum(grandkids);
				grandparent = definemsg('q',G.areaname,grandad);
				if (grandparent) {
					break;
				}
			}
		}
		if (!grandparent) {
			/* EMPTY */
			/* No family left to care so SIMPLY REMOVE THE MESSAGE!*/					
			/* by slipping through to the bottom */
		} else {
			
			/* Now make the grandparent's new header */
			parse_area_header(grandparent,&ah);				
		
			/* Remember who we finally decided was the grandad */
			/* We'll need to tell the grandkids who's adopting them */
			grandad = atoi(ah.number);

			/* get the kids adopted by the grandparent, with aunts etc*/
			siblings = string_to_list(ah.footer);
			adopted = combinenums(grandkids,siblings);
			for(i = 0;adopted[i];i++); /* Just counting adopted */
			qsort((void *)adopted,i,sizeof (int),intsort);

sprintf(tempheader,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",
ah.flag,ah.number,ah.dname,ah.mname,ah.dom,ah.time,ah.tzname,ah.year,
ah.from,ah.author,ah.narrative,ah.hash,ah.parent,ah.by,ah.parentby,
ah.base,ah.next,ah.prev);
/*** NB 'prev' might need to be changed if this had been a child! */			
/*** Don't worry - we are going to do a relink-thread.*/

			for (i = 0;adopted[i];i++) {
			/* Adding footer back */
				sprintf(string,"%d ",adopted[i]);
				strcat(tempheader,string);
			}
			free(adopted);
			free(siblings);
			free(grandparent);
	
			/* Lets write it out now! */
			sprintf(filename,"%s/%s/hdr.%d",C.areasdir,G.areaname,grandad);
			FIL = fopen(filename,"w");
			fputs(tempheader,FIL);
			fclose(FIL);
	
			/* OK now the remaining grandkids need to be told their father is dead */
			for (i = 0;grandkids[i];i++) {
				grandchild = definemsg('q',G.areaname,grandkids[i]);
				if (grandchild) {
					parse_area_header(grandchild,&ah);
	
sprintf(tempheader,"%s %s %s %s %s %s %s %s %s %s linked-to %s %d [Parent deleted] %s %s %s\n%s",
ah.flag,ah.number,ah.dname,ah.mname,ah.dom,ah.time,ah.tzname,ah.year,
ah.from,ah.author,ah.hash,grandad,ah.base,ah.next,ah.prev,ah.footer);

					free(grandchild);
					sprintf(filename,"%s/%s/hdr.%d",C.areasdir,G.areaname,grandkids[i]);
					FIL = fopen(filename,"w");
					fprintf(FIL,"%s",tempheader);
					fclose(FIL);
				}
			}
			free(grandkids);
		}			
		grepnums(thread,vm->msglist);
		relink_thread(G.areaname,thread);
		free(thread);
/* All above freed in correct order - no small feet(feat)! */
/*==============================================*/
/* BELOW HERE FOR A WHOLE THREAD/BRANCH */
	
	} else {
		if (U.flags[CHAIRMAN] == '0') {
			/*printf("Only chairmen can delete a %s.",vm->parse);*/
			printf("%s\n",Ustring[109]);
			rem_lock(vm->lock);
			free(vm->msglist);
			free(vm->parse);
			free(vm->lock);
			free(vm);	
			return 0;
		}
		if (!strcmp(vm->parse,"branch")) {
			/*
			puts("This choice deletes all messages from the current to the end of the branch.");
			puts("The 'branch' includes this message and all its dependants.");
			puts("If you want the thread deleted right from the beginning, you need to be");
			puts("at the BASE-MESSAGE or choose 'thread'.");
			*/
			printf("%s\n",Ustring[110]);
		} else {
			/*puts("This choice deletes all messages in this thread from the base to the end.");*/
			printf("%s\n",Ustring[111]);
		}
		
		/*make_prompt("[d]elete now, or [A]bort? ");*/

		sprintf(filename,Ustring[108],vm->parse);
		if (!no_yes(filename)) {
			/*printf("Deletion abandoned.\n");*/
			printf("%s\n",Ustring[60]);
			rem_lock(vm->lock);
			free(vm->msglist);
			free(vm->parse);
			free(vm->lock);
			free(vm);	
			return 0;
		}
		hups_off();
		header = definemsg('q',G.areaname,vm->msglist[0]);
		if (!header) {
			/*printf("Deletion abandoned.\n");*/
			printf("%s\n",Ustring[60]);
			rem_lock(vm->lock);
			free(vm->msglist);
			free(vm->parse);
			free(vm->lock);
			free(vm);	
			hups_on();
			return 0;
		}

		for (i=0; i<=15;i++) {
			shiftword(header,string,5);
		}
		base = atoi(string);
		free(header);
		detach_messages(G.areaname,vm->msglist[0],vm->msglist);

	}
	delete_list('v',G.areaname,vm->msglist);

	rem_lock(vm->lock);
	free(vm->msglist);
	free(vm->parse);
	free(vm->lock);
	free(vm);	
	hups_on();
	return 1;
}


int delete_list (char mode,char *area,int *msglist) {
	char filename[MAINLINE + 100];
	int i;

/*	detach_messages(area,msglist[0],msglist);*/
	if ((mode == 'v') && !G.hupflag) {
		/*printf("\nDeleting message/s ");*/
		printf("\n%s ",Ustring[112]);
		fflush(stdout);
	}
	msgindex(area,'D',msglist);
	for (i=0;msglist[i];i++) {
		if ((mode == 'v') && !G.hupflag) {
			printf("%d ",msglist[i]);
		}
		sprintf(filename,"%s/%s/hdr.%d",C.areasdir,area,msglist[i]);
		remove(filename);
		sprintf(filename,"%s/%s/msg.%d",C.areasdir,area,msglist[i]);
		remove(filename);
		sprintf(filename,"%s/%s/vote.%d",C.areasdir,area,msglist[i]);
		remove(filename);

	}
	if ((mode == 'v') && !G.hupflag) {
		printf("\n");
	}
	return 1;
}



int copymsg (char *in) {
/* MENU COMMAND */
	char from[11];
	char to[51];
	struct valid_messages *vm;
	struct valid_files *vf;
	char *copy;

	if (!check_area('v',G.areaname)) {
		return 0;
	}

	copy = strdup(in);
	shiftword(copy,from,11);
	if (!from[0]) {
		shiftword(G.comline,from,11);
	}

	if (!Dstrcmp(from,Ustring[501])) {
		sprintf(from,"%d",G.current);
	}

	vm = get_valid_messages('v',G.areaname,"",G.current,from,0);
	if (!vm) {
		free(copy);
		return 0;
	}
	strcpy(from,vm->parse);
	free(vm->parse);
	free(vm->msglist);
	free(vm);

	shiftword(copy,to,51);
	if (!to[0]) {
		shiftword(G.comline,to,51);
	}

	if (!Dstrcmp(to,Ustring[501])) {
		shiftword(copy,to,51);
		if (!to[0]) {
			shiftword(G.comline,to,51);
		}
	}
	free(copy);

	/*vf = get_valid_dirs('v',1,"area",C.areasdir,to,0);*/
	vf = get_valid_dirs('v',1,Ustring[197],C.areasdir,to,0);
	if (!vf->files[0]) {
		free(vf->input);
		free(vf->files);
		free(vf);
		return 0;
	}
	strcpy(to,vf->files);
	free(vf->input);
	free(vf->files);
	free(vf);

	if (is_area_elig('v',to) < 1) {
		return 0;
	}
	if (is_area_elig('v',to) < 2) {
		/*printf("Area %s cannot accept the message\n");*/
		printf(Ustring[128],to);
		printf("\n");
		return 0;
	}

	vm = get_valid_messages('v',G.areaname,"",G.current,from,1);
	if (!vm) {
		return 0;
	}
	hups_off();
	copy_list('v',G.areaname,to,vm->msglist);
	rem_lock(vm->lock);
	hups_on();
	free(vm->lock);
	free(vm->parse);
	free(vm->msglist);
	free(vm);
	return 1;
}

int copy_list (char mode, char *from, char *to, int *fromlist) {
	int *tolist;
	int i;
	int wasreplyto;
	int sending;
	int tobase;
	int place;
	int reply_no;
	FILE *FIL;
	struct areaheader fhs;
	char lockname[MAINLINE + 100];
	char fromperson[80];
	char filename[MAINLINE + 100];
	char resthead[MAINLINE];
	char *subject;
	char *fromheader = definemsg('q',from,fromlist[0]);

	for(i=0;fromlist[i];i++);
	tolist = (int *)malloc((i + 1)*sizeof (int));
	place = 0;


	if (!fromheader) {
		if (mode != 'q') {
			/*printf("The message has vanished right under your nose!\n");*/
			sprintf(filename,Ustring[193],fromlist[0]);
			printf(Ustring[65],filename);
			printf("\n");
		}
		return 0;
	}
	parse_area_header(fromheader,&fhs);

	strcpy(fromperson,fhs.author);
	free(fromheader);

	sprintf(filename,"%s/%s.temp",C.tmpdir,U.id);
	subject = compose_msgbody(from,fromlist[0],filename);

	sending = sendmess(mode,to,fromperson,filename,subject,"BASE-MESSAGE - - - -",'B','f');
	free(subject);
	tobase = sending;
	sprintf(lockname,"%s/%s/%d.lock",C.areasdir,to,tobase);
	if (!place_lock(mode,lockname,1,0)) {
		free(subject);
		return 0;
	}

	tolist[place] = sending;
	place++;

	while (fromlist[place]) {
		fromheader = definemsg('q',G.areaname,fromlist[place]) ;
		/* maybe test at some point */
		parse_area_header(fromheader,&fhs);
		strcpy(fromperson,fhs.author);

		sprintf(filename,"%s/%s.temp",C.tmpdir,U.id);
		subject = compose_msgbody(from,fromlist[place],filename);

		wasreplyto = atoi(fhs.parent);
		for(i=0;(i < place) && (fromlist[i] != wasreplyto);i++);
		if (fromlist[i] != wasreplyto) {
			reply_no = sending; /* the previous message */
		} else {
			reply_no = tolist[i];
		}
		sprintf(resthead,"%s %s %d %s %s",fhs.narrative,fhs.hash,reply_no,fhs.by,fhs.parentby);
		sending = sendmess(mode,to,fromperson,filename,subject,resthead,'B','f');

		free(fromheader);
		free(subject);
		tolist[place] = sending;
		place++;

		sprintf(filename,"%s/%s/hdr.%d",C.areasdir,to,reply_no);
		if ((FIL = fopen(filename,"a"))) {
			fprintf(FIL,"%d ",sending);
			fclose(FIL);
		}
	}
	tolist[place] = 0;
	relink_thread(to,tolist);
	rem_lock(lockname);
	free(tolist);
	return 1;

}

char *compose_msgbody (char *area, int msgno, char *msgfile) {
	FILE *OUT;
	FILE *IN;
	char subject[MAINLINE];
	char temp[500];
	char inbody[MAINLINE + 100];


	sprintf(inbody,"%s/%s/msg.%d",C.areasdir,area,msgno);
	if ((IN = fopen(inbody,"r"))) {
		fgets(subject,MAINLINE,IN);
		shiftword(subject,temp,5);
		tnt(subject);
		if ((OUT = fopen(msgfile,"w"))) {
			fprintf(OUT,"Copied from %s by %s\n",area,U.id);
			fputs("------\n\n",OUT);
			fclose(OUT);
		}
		fclose(IN);
	}
	sprintf(temp,"tail +3 %s >> %s",inbody,msgfile);
	dsystem(temp);

	return strdup(subject);
}

int flagmsg (char *in) {
/* MENU COMMAND */
	int result = 0;
	struct valid_messages *vm;
	char filename[MAINLINE + 100];
	FILE *FIL;
	char *header;
	char temp[21];
	char *copy;
	char *params;

	if (!check_area('v',G.areaname)) {
		return 0;
	}
	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		free(copy);
		params = strdup(copy);
	} else {
		free(copy);
		params = strdup(G.comline);
		flushcom("");
	}
	
	if (!params[0]) {
		free(params);
		sprintf(temp,"%d",G.current);
		params = strdup(temp);
	}
	vm = get_valid_messages('v',G.areaname,Ustring[70],G.current,params,0);/*"Reply to which message"*/

	free(params);

	if (!vm) {
		return 0;
	}
	if (!strcmp(vm->parse,"thread") || !strcmp(vm->parse,"branch")) {
		/*printf("Cannot do that to more than one message at a time!\n");*/
		printf("%s", Ustring[72]);
		printf("\n");
		free(vm->msglist);
		free(vm->parse);
		free(vm);
		return 0;
	}

	if (!is_msg_elig(vm->msglist[0])) {
		/*printf("You do not have any rights over %d.\n",vm->msglist[0]);*/
		printf(Ustring[80],vm->msglist[0]);
		printf("\n");
		rem_lock(vm->lock);
		free(vm->lock);
		free(vm->parse);
		free(vm->msglist);
		free(vm);
		return 0;
	}

	header = definemsg('q',G.areaname,vm->msglist[0]);
	if (!header) {
		rem_lock(vm->lock);
		free(vm->lock);
		free(vm->parse);
		free(vm->msglist);
		free(vm);
		return 0;
	}
	if (header[0] == '#') {
		header[0] = 'A';
	} else {
		header[0] = '#';
	}
	sprintf(filename,"%s/%s/hdr.%d",C.areasdir,G.areaname,vm->msglist[0]);
	if ((FIL = fopen(filename,"w"))) {
		fputs(header,FIL);
		fclose(FIL);
		result = 1;
		if (header[0] == '#') {
			/*printf("Message %d unflagged.\n",vm->msglist[0]);*/
			printf(Ustring[107],vm->msglist[0],Ustring[495]);
			printf("\n");
		} else {
			/*printf("Message %d flagged.\n",vm->msglist[0]);*/
			printf(Ustring[106],vm->msglist[0],Ustring[495]);
			printf("\n");
		}				
	}
	free(header);
	rem_lock(vm->lock);
	free(vm->lock);
	free(vm->parse);
	free(vm->msglist);
	free(vm);
	return result;
}

/* ============================================== */
/* LISTING MESSAGES */


int textsearch (char *in) {
/* MENU COMMAND */
	char string[MAINLINE];
	char pattern[MAINLINE];
	FILE *FIL;
	char filename[MAINLINE + 100];
	char line[MAINLINE + 1];
	char caseline[MAINLINE + 1];
	char startnumstring[11];
	char stopnumstring[11];
	int linecount = 0;
	int startnum;
	int stopnum;
	int nr_found = 0;
	int i = 0;
	char *copy;
#if defined(LINUX) || defined(SVR42)
	char *shellpat;
	int j = 0;
#  if defined(LINUX)
	int re;
	regex_t preg;
#  else
	char *re;
#  endif
#endif


	if (!check_area('v',G.areaname)) {
		return 0;
	}

	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		if (copy[0] == '"') {
			strshift(copy,pattern,MAINLINE,"\"");
			strshift(copy,pattern,MAINLINE,"\"");
		} else {
			strshift(copy,pattern,MAINLINE,"from");
			tnt(pattern);
		}
	} else {
		if (G.comline[0] == '"') {
			strshift(G.comline,pattern,MAINLINE,"\"");
			strshift(G.comline,pattern,MAINLINE,"\"");
		} else {
			strshift(G.comline,pattern,MAINLINE,"from");
			tnt(pattern);
		}
	}
	
	shiftword(copy,startnumstring,21);
	if (!Dstrcmp(startnumstring,"from")) {
		shiftword(copy,startnumstring,21);
	}
	if (!startnumstring[0]) {
		shiftword(G.comline,startnumstring,21);
		if (!Dstrcmp(startnumstring,"from")) {
			shiftword(G.comline,startnumstring,21);
		}
	}

	shiftword(copy,stopnumstring,21);
	if (!Dstrcmp(stopnumstring,Ustring[501])) {
		shiftword(copy,stopnumstring,21);
	}
	if (!stopnumstring[0]) {
		shiftword(G.comline,stopnumstring,21);
		if (!Dstrcmp(stopnumstring,Ustring[501])) {
			shiftword(G.comline,stopnumstring,21);
		}
	}

	flushcom("");
	free(copy);

	if (!pattern[0]) {
		/* CONSTCOND */
		while (1) {
			/*printf("Please type search pattern between double quotes (\"\") or ? for help.\n");*/
			printf("%s (%s)\n",Ustring[194],Ustring[200]);
			make_prompt(Ustring[119]);
			get_one_line(string);
			tnt(string);

			if (!string[0]) {
				return 0;
			}
			if (!strcmp(string,"?")) {
				do_ts_help();
				continue;
			}

			if ((string[0] == '"') && (string[strlen(string) - 1] == '"')) {
				strshift(string,pattern,21,"\"");
				strshift(string,pattern,MAINLINE,"\"");
			} else {
				strshift(string,pattern,MAINLINE,"from");
				tnt(pattern);
			}
			if (!pattern[0]) {
				return 0;
			}
			break;
		}
	}

	new_get_one_param('v',Ustring[120],startnumstring,string,21);/*start where?*/
	if (!string[0]) {
		strcpy(string,"first");
	}
	if (!(startnum = point(string))) {
		printf(Ustring[472],string);
		printf("\n");
		return 0;
	}
	new_get_one_param('v',Ustring[121],stopnumstring,string,21);/*End where?*/
	if (!string[0]) {
		strcpy(string,"last");
	}
	if (!(stopnum = point(string))) {
		printf(Ustring[472],string);
		printf("\n");
		return 0;
	}
	if (stopnum < startnum) {
		/*printf("I am sorry but we cannot search backwards.\n");*/
		printf("%s\n",Ustring[122]);
		return 0;
	}
	lower_string(pattern);
	
#if defined(LINUX) || defined(SVR42)
	shellpat = (char *)malloc(strlen(pattern) * 2 + 1);

	for(i = 0;pattern[i];i++) {
		switch(pattern[i]) {
			case '*':
				shellpat[j++] = '.';
				shellpat[j++] = '*';
				break;
			case '?':
				shellpat[j++] = '.';
				break;
			default:
				if (ispunct(pattern[i])) {
					shellpat[j++] = '\\';
				}
				shellpat[j++] = pattern[i];
		}
	}
	shellpat[j] = 0;

#  if defined(SVR42)
	re = regcmp(shellpat,NULL);
#  else /* LINUX */
	re = !regcomp(&preg,shellpat,0);
#  endif

	if (re) {
		for(i=startnum;i<=stopnum;i++) {
			sprintf(filename,"%s/%s/msg.%d",C.areasdir,G.areaname,i);
			if ((FIL = fopen(filename,"r"))) {
				while (fgets(line,MAINLINE,FIL)) {
					j=strlen(line)-1;
					while(j && isspace(line[j])) {
						line[j] = 0;
						j--;
					}
					strcpy(caseline,line);
					lower_string(line);
#  if defined(LINUX)
					if (!regexec(&preg,line,0,0,0)) /* brace below */
#  else
					if (regex(re,line) != NULL) /* brace below */
#  endif
					{

						printf("%4d:%-70.70s\n",i,caseline);
						nr_found++;

						linecount++;
						if (linecount > LINES-5) {
							if (!do_continue("")) {
								i = stopnum;
								break;
							}
							linecount = 0;
						}

					}
				}
				fclose(FIL);
			}
		}
#  if defined(SVR42)
		free(re);
#  else
		regfree(&preg);
#  endif
		if (!nr_found) {
			/*printf("No matches.\n");*/
			printf("%s\n",Ustring[123]);
		}
		return 1;
	} else {
		/*printf("Regex error.\n");*/
		printf(Ustring[472],shellpat);
		printf("\n");
		return 0;
	}
#else
	if (!C.awkname[0]) {
		for(i=startnum;i<=stopnum;i++) {
			sprintf(filename,"%s/%s/msg.%d",C.areasdir,G.areaname,i);
			if (FIL = fopen(filename,"r")) {
				while (!feof(FIL)) {
					fgets(line,MAINLINE,FIL);
					tnt(line);
					if (strstr(line,pattern)) {
						printf("%4.4d:%-70.70s\n",i,line);
					}
				}
				fclose(FIL);
			}
		}
	} else {
		/*puts("Sorry, no awk script written yet! (-:");*/
		return 0;
	}
	return 1;
#endif
}

void do_ts_help (void) {
	display_lang("ts");
}

int listheaders (char *in) {
/* MENU COMMAND */

	char *params;
	char *copy;
	char string[21];
	int startnum;
	int linecount = -1;
	int i;
	int *msglist;

	if (!check_area('v',G.areaname)) {
		flushcom("");
		return 0;
	}

	copy = strdup(in);
	shiftword(copy,string,21);
	if (!string[0]) {
		shiftword(G.comline,string,21);
	}

	if (!Dstrcmp(string,"from")) {
		shiftword(copy,string,21);
		if (!string[0]) {
			shiftword(G.comline,string,21);
		}
	}
	free(copy);

	if (!string[0]) {
		params = (char *)malloc(MAINLINE);
		/*make_prompt("List [n]ew, threa[d], [s]ubjects, or (from)<msgno>? ");*/
		make_prompt(Ustring[45]);
		get_one_line(params);
		shiftword(params,string,21);
		free(params);
	}
	if (!string[0]) {
		return 0;
	}

	moretoread(G.highmsg);
	/*if (!strcmp(string,"n") || !strcmp(string,"new")) {*/
	if (!Dstrcmp(string,Ustring[42]) || !Dstrcmp(string,"new")) {
		startnum = G.pointer + 1;
		for (i=startnum;i<=G.highmsg;i++) {
			if (strchr("BbMmRr",G.mymsgindex[i])) {
				linecount += list_one(G.areaname,i);
				if (linecount > LINES-5) {
					if (!do_continue("")) {
						return 1;
					}
					linecount = 0;
				}
			}
		}
	/*} else if (!strcmp(string,"s") || !strcmp(string,"subjects")) {*/
	} else if (!Dstrcmp(string,Ustring[44]) || !Dstrcmp(string,"subjects")) {
		startnum = 1;
		for (i=startnum;i<=G.highmsg;i++) {
			if (strchr("BbRr",G.mymsgindex[i])) {
				linecount += list_one(G.areaname,i);
				if (linecount > LINES-5) {
					if (!do_continue("")) {
						return 1;
					}
					linecount = 0;
				}
			}
		}
	/*} else if (!strcmp(string,"d") || !strcmp(string,"thread")) {*/
	} else if (!Dstrcmp(string,Ustring[43]) || !Dstrcmp(string,"thread")) {
		if ((msglist = definethread('v',G.areaname,G.current))) {
			startnum = msglist[0];
			for (i=0;msglist[i];i++) {
				linecount += list_one(G.areaname,msglist[i]);
				if (linecount > LINES-5) {
					if (!do_continue("")) {
						return 1;
					}
					linecount = 0;
				}
			}
			free(msglist);
		} else {
			return 0;
		}
	/*} else if (!strcmp(string,"h") || !strcmp(string,"branch")) {*/
	} else if (!Dstrcmp(string,Ustring[195]) || !Dstrcmp(string,"branch")) {
		if ((msglist = definetree('v',G.areaname,G.current))) {
			startnum = msglist[0];
			for (i=0;msglist[i];i++) {
				linecount += list_one(G.areaname,msglist[i]);
				if (linecount > LINES-5) {
					if (!do_continue("")) {
						return 1;
					}
					linecount = 0;
				}
			}
			free(msglist);
		} else {
			return 0;
		}
	} else if ((startnum = point(string))) {
		for (i=startnum;i<=G.highmsg;i++) {
			if (strchr("BbMmRr",G.mymsgindex[i])) {
				linecount += list_one(G.areaname,i);
				if (linecount > LINES-5) {
					if (!do_continue("")) {
						return 1;
					}
					linecount = 0;
				}
			}
		}
	} else {
		/*printf("'%s' invalid.\n",string);*/
		printf(Ustring[472],string);
		printf("\n");
		return 0;
	}
	if (linecount < 0) {
		/*printf("No messages found in specified range");*/
		printf("%s\n",Ustring[123]);
		return 0;
	}		
	return 1;
}


int list_one (char *area,int msgno) {
	struct areaheader hs;
	char subject[MAINLINE];
	char *header;
	FILE *FIL;
	char filename[MAINLINE + 100];
	char response[2];

	header = definemsg('q',area,msgno);
	if (!header) {
		return 0;
	}

	sprintf(filename,"%s/%s/msg.%d",C.areasdir,area,msgno);
	if ((FIL = fopen(filename,"r"))) {
		fgets(subject,MAINLINE,FIL);
		shiftword(subject,response,2);
		tnt(subject);
		parse_area_header(header,&hs);
		sprintf(filename,"%s %s %s %s %s %s %s",hs.number,hs.dname,hs.dom,hs.mname,hs.year,hs.from,hs.author);
		printf("%-36.36s - %-40.40s\n",filename,subject);
		free(header);
		fclose(FIL);
		return 1;
	}
	return 0;
}


/* ========================================================= */
/* AREA INFORMATION */

int participants (char *dummy) {
/* MENU COMMAND */
	char temp[MAINLINE + 100];
	char *list;
	char party[9];
	int  reached;
	struct valid_files *vf;
	FILE *FIL;
	int linecount = -1;

	vf = get_valid_dirs('q',0,Ustring[287],C.users,"*",0);
	list = strdup(vf->files);
	free(vf->input);
	free(vf->files);
	free(vf);

	printf("\n");
	while (list[0]) {
		shiftword(list,party,9);		
		sprintf(temp,"%s/%s/.areas/%s",C.users,party,G.areaname);
		if ((FIL = fopen(temp,"r"))) {
			reached = 0;
			fscanf(FIL," %d ",&reached);
			printf("%-10s %4d\n",party,reached);
			fclose(FIL);
			linecount++;
			if (linecount > (LINES - 5)) {
				if (!do_continue("")) {
					return 1;
				}
				linecount = 0;
			}
		}
	}
	free(list);
	if (linecount < 0) {
		/*printf("No one has read any messages in %s yet.\n",G.areaname);*/
		printf(Ustring[135],G.areaname);
		printf("\n");
	}
	return 1;
}


/* ARGSUSED0 */
int area_status (char *dummy) {
/* MENU COMMAND */
/* CHECKED */

	if (!check_area('v',G.areaname)) {
		return 0;
	}
	printf("%s :-\n",G.areaname);
	/*
	printf((G.areaflags[PRIVATE] != '0') ? "is private.\n" : "is open to all.\n");
	printf((G.areaflags[READONLY] != '0') ? "is readonly.\n" : "is read/write.\n");
	printf((G.areaflags[ALIASES] != '0') ? "allows use of aliases.\n" : "allows real IDs only.\n");
	printf((G.areaflags[SIGS] != '0') ? "uses .sig files.\n" : "does not use .sig files.\n");
	printf("The total number of messages in %s is %d.\n",G.areaname,countmsgs(G.areaname,0));
	*/

	printf((G.areaflags[PRIVATE] != '0') ? Ustring[136],G.areaname : Ustring[137],G.areaname);
	printf("\n");
	printf((G.areaflags[READONLY] != '0') ? Ustring[138],G.areaname : Ustring[139],G.areaname);
	printf("\n");
	printf((G.areaflags[ALIASES] != '0') ? Ustring[140],G.areaname : Ustring[141],G.areaname);
	printf("\n");
	printf((G.areaflags[SIGS] != '0') ? Ustring[142],G.areaname : Ustring[143],G.areaname);
	printf("\n");
	printf(Ustring[144],G.areaname,countmsgs(G.areaname,0));
	printf("\n");
	return 1;
}

int countmsgs (char *area, int msgno) {
	int i = 0;
	char c;
	FILE *FIL;
	char filename[MAINLINE + 100];

	sprintf(filename,"%s/%s/msgindex",C.areasdir,area);
	if ((FIL = fopen(filename,"r"))) {
		fseek(FIL,msgno,SEEK_SET);
		while (fread(&c,1,1,FIL)) {
			if (strchr("BM",c)) {
				i++;
			}
		}
		fclose(FIL);
	}
	return i;
}




/* ================================================= */
/* SCANLIST STUFF AND AREA LISTING */


/* ARGSUSED0 */
int scanlist_edit (char *dummy) {
/* MENU COMMAND */
/* CHECKED - Nothing to go wrong */
	flushcom("");
	edit_scanlist(U.id);
	return 1;
}

int edit_scanlist (char *user) {
/* CHECKED */
	char filename[MAINLINE + 100];
	int result;

	close_scan();
	sprintf(filename,"%s/%s/.scanlist",C.users,user);
	result = edit_special(filename);
	open_scan();
	return result;
}

void open_scan (void) {
	char filename[MAINLINE + 100];
	char areascout[51];
		
	G.scan = 0;

	sprintf(filename,"%s/%s/.scanlist",C.users,U.id);
	while (!(SCAN = fopen(filename,"r"))) {
		SCAN = fopen(filename,"w");
		fclose(SCAN);
	}
	if (get_next_area(areascout)) {
		G.scan = 1;
		rewind(SCAN);
	}
}

int get_next_area (char *areascout) {
	char line[91];
	while (fgets(line,90,SCAN)) {
		shiftword(line,areascout,51);
		tnt(areascout);
		if (areascout[0]) {
			return 1;
		}
	}
	return 0;
}

void close_scan (void) {
	fclose(SCAN);	
}
	

int area_add (char *in) {
/* MENU COMMAND */
/* for use while in area */
	char filename[MAINLINE + 100];
	int result = 0;
	char *copy = strdup(in);

	sprintf(filename,"%s/%s/.scanlist",C.users,U.id);

	tnt(copy);
	close_scan();
	if (copy[0]) {
		result = add_user_to_list('v', "area", filename, copy);
	} else {	
		result = add_user_to_list('v', "area", filename, G.comline);
		flushcom("");
	}
	open_scan();
	free(copy);
	return result;
}

int area_drop (char *in) {
/* MENU COMMAND */
/* for use while in area */
	char filename[MAINLINE + 100];
	int result;
	char *copy = strdup(in);

	sprintf(filename,"%s/%s/.scanlist",C.users,U.id);

	tnt(copy);

	close_scan();
	if (copy[0]) {
		result = rem_from_list('v', filename, copy);
	} else {	
		result = rem_from_list('v', filename, G.comline);
		flushcom("");
	}
	open_scan();
	free(copy);
	return result;
}


	
int area_list (char *params) {
/* MENU COMMAND */
	char temp[AFLAGMAX + 2];
	char tempa[UFLAGMAX + 2];
	char levelstring[5];
	int level;
	char maskstring[UFLAGMAX + 2];
	char aflagstring[AFLAGMAX + 2];
	char *copy = strdup(params);
		
	tnt(copy);
	shiftword(copy,temp,AFLAGMAX + 2);
	shiftword(copy,levelstring,5);
	shiftword(copy,tempa,UFLAGMAX + 2);
	free(copy);
		
	if (!temp[0]) {
		strcpy(aflagstring,"#?");
	} else {
		sprintf(aflagstring,"#%s",temp);
	}
	if ((levelstring[0]) && is_num(levelstring)) {
		level = atoi(levelstring);
	} else {
		level = C.sysoplevel;
	}
	if (!tempa[0]) {
		strcpy(maskstring,"#?");
	} else {
		sprintf(maskstring,"#%s",tempa);
	}
	return list_areas(aflagstring,level,maskstring);
} 	
	
	
int list_areas(char *wantedflags, int level, char *usermask) {
	char *list;
	char onearea[51];
	struct valid_files *vf;	
	char filename[MAINLINE + 100];
	FILE *FIL;	
	char areaflags[AFLAGMAX + 2];
	char maskflags[UFLAGMAX + 2];
	char mtoken;
	char stoken;
	char desc[52];	
	int linecount = -1;	
	
	vf = get_valid_dirs('v',0,Ustring[171],C.areasdir,"*",0);
	list = strdup(vf->files);
	free(vf->files);
	free(vf->input);
	free(vf);

	while (list[0]) {
		shiftword(list,onearea,51);
		areaflags_read(onearea,areaflags);
		areamask_read(onearea,maskflags);

		if (comp_flags(wantedflags,areaflags) && comp_flags(usermask,maskflags) && (arealevel_read(onearea) <= level)) {
			
			/* Is it in your scanlist? */
			sprintf(filename,"%s/%s/.scanlist",C.users,U.id);
			if (is_in_list(filename,onearea)) {
				stoken='s';
			} else {
				stoken=' ';
			}

			/* What access do you have to it? */
			sprintf(filename,"%s/%s/chair",C.areasdir,onearea);
			if (is_in_list(filename,U.id)) {
				mtoken='c';
			} else {
				if (areaflags[PRIVATE] != '0') {
					sprintf(filename,"%s/%s/chair",C.areasdir,onearea);
					if (is_in_list(filename,U.id)) {
						mtoken='m';
					} else {
						mtoken='p';
					}
				} else {
					mtoken = ' ';
				}
			}
			
			desc[0] = 0;
			sprintf(filename,"%s/%s/description",C.areasdir,onearea);
			if ((FIL = fopen(filename,"r"))) {
				fgets(desc,52,FIL);
				tnt(desc);
				fclose(FIL);
			}
			printf("%c %c %-14s %-50s\n",mtoken,stoken,onearea,desc);
			linecount++;
			if (linecount > (LINES - 5)) {
				if (!do_continue("")) {
					return 1;
				}
				linecount = 0;
			}
		}
	}
	if (linecount < 0) {
		/*puts("There are no message areas matching the type specified.");*/
		printf("%s\n",Ustring[104]);
		return 0;
	} else {
		/*puts("c = chaired by you, m = you are member, p = private, s = in scanlist.");*/
		printf("%s\n",Ustring[105]);
		return 1;
	}
}

/* ============================================== */
/* AREA MAINTENANCE */

int area_create (char *in) {
/* MENU COMMAND */
/* CHECKED - More or less safe */
	struct stat statbuf;
	FILE *LOG;
	char desc[51];
	char filename[MAINLINE + 100];
	char areascout[51];
	char *date;
	char *copy = strdup(in);

	
	shiftword(copy,filename,51);
	free(copy);
	if (!filename[0]) {
		shiftword(G.comline,filename,51);
	}
	
	if (!filename[0]) {
		/*make_prompt("Create area by what name? [Enter to abandon] ");*/
		make_prompt(Ustring[146]);
	}
	get_one_area(areascout,C.maxfilename + 1,filename);

	if (!areascout[0]) {
		return 0;
	}

	/*sprintf(filename,"Create area named %s?",areascout);*/
	sprintf(filename,Ustring[147],areascout);
	if (!yes_no(filename)) {
		return 0;
	}

	sprintf(filename,"%s/%s",C.areasdir,areascout);
	if (!stat(filename,&statbuf)) {
		/*printf("There is already an area called %s.\n",areascout);*/
		printf(Ustring[148],areascout);
		printf("\n");
		return 0;
	}

	/*printf("Please enter up to 50 chars description of %s.\n",areascout);*/
	printf(Ustring[149],areascout);
	printf("\n");
	/*make_prompt("Description: ");*/
	make_prompt(Ustring[150]);
	get_raw(1,51,desc,1);

	/*printf("Please wait a few moments... ");*/
	printf("%s", Ustring[151]);
	if (create_area(areascout,desc)) {
		date = shorttime(time(0));
		sprintf(filename,"%s/newarealog",C.datadir);	
		if ( (LOG=fopen(filename,"a")) ) {
			fprintf(LOG,"%s %s created %s for %s\n",date,U.id,areascout,desc);
			fclose(LOG);
		}
		free(date);
		printf("\n");
	
		return 1;
	}
	return 0;
}


int create_area (char *areascout,char *desc) {
/* CHECKED */
	FILE *FIL;
	char filename[MAINLINE + 100];
	int result = 1;
	
	sprintf(filename,"%s/%s",C.areasdir,areascout);
	if (mkdir(filename,0770)) {
		/*printf("Could not create area - abandoned.\n");*/
		sprintf(filename,Ustring[68],areascout);
		printf("%s - %s\n",filename,Ustring[60]);
		return 0;
	}

	sprintf(filename,"%s/%s/highest",C.areasdir,areascout);
	if ((FIL = fopen(filename,"w"))) {
		fputs("0\n",FIL);
		fclose(FIL);
	} else {
		/*printf("Could not write to %s.\n",filename);*/
		printf(Ustring[67],filename);
		printf("\n");
		result = 0;
	}

	if (!arealevel_write(areascout,C.newarealevel)) {
		/*printf("Could not create %s/%s/level.\n",C.areasdir,areascout);*/
		sprintf(filename,"%s/%s/level.\n",C.areasdir,areascout);
		printf(Ustring[68],filename);
		printf("\n");
		result = 0;
	}

	sprintf(filename,"%s/%s/msgindex",C.areasdir,areascout);
	if ((FIL = fopen(filename,"w"))) {
		fputs("0",FIL);
		fclose(FIL);
	} else {
		/*printf("Could not write %s.\n",filename);*/
		printf(Ustring[67],filename);
		printf("\n");
		result = 0;
	}

	sprintf(filename,"%s/%s/members",C.areasdir,areascout);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%s\n",U.id);
		fclose(FIL);
	} else {
		/*printf("Could not write %s.\n",filename);*/
		printf(Ustring[67],filename);
		result = 0;

	}

	sprintf(filename,"%s/%s/chair",C.areasdir,areascout);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%s\n",U.id);
		fclose(FIL);
	} else {
		/*printf("Could not write %s.\n",filename);*/
		printf(Ustring[67],filename);
		result = 0;

	}

	sprintf(filename,"%s/%s/gagged",C.areasdir,areascout);
	if ((FIL = fopen(filename,"w"))) {
		fclose(FIL);
	} else {
		/*printf("Could not write %s.\n",filename);*/
		printf(Ustring[67],filename);
		result = 0;

	}

	sprintf(filename,"%s/%s/flags",C.areasdir,areascout);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%s\n",&C.newareaflags[1]);
		fclose(FIL);
	} else {
		/*printf("Could not write %s.\n",filename);*/
		printf(Ustring[67],filename);
		result = 0;

	}

	sprintf(filename,"%s/%s/areamask",C.areasdir,areascout);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%s\n",&C.newareamask[1]);
		fclose(FIL);
	} else {
		/*printf("Could not write %s.\n",filename);*/
		printf(Ustring[67],filename);
		result = 0;

	}

	sprintf(filename,"%s/%s/description",C.areasdir,areascout);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%s\n",desc);
		fclose(FIL);
	} else {
		/*printf("Could not write %s.\n",filename);*/
		printf(Ustring[67],filename);
		result = 0;

	}
	if (!result) {
		sprintf(filename,"rm -rf %s/%s",C.areasdir,areascout);
		dsystem(filename);
	}
	return result;
}

int area_destroy (char *in) {
/* MENU COMMAND */
	struct valid_files *vf;
	FILE *LOG;
	char filename[MAINLINE + 100];
	char areascout[51];
	char *copy = strdup(in);

	tnt(copy);
	if (copy[0]) {
		/*vf = get_valid_dirs('v',1,"area",C.areasdir,copy,0);*/
		vf = get_valid_dirs('v',1,Ustring[197],C.areasdir,copy,0);
	} else {
		shiftword(G.comline,filename,51);
		/*vf = get_valid_dirs('v',1,"area",C.areasdir,filename,0);*/
		vf = get_valid_dirs('v',1,Ustring[197],C.areasdir,filename,0);
	} 
	free(copy);		

	strncpy(areascout,vf->files,51);
	areascout[50] = 0;
	free(vf->files);
	free(vf->input);
	free(vf);

	if (!areascout[0]) {
		return 0;
	}

	if (is_area_elig('v',areascout) < 3) {
		if (U.level < C.sysoplevel) {
			/*printf("You do not have authority to destroy %s.\n",areascout);*/
			printf(Ustring[152],areascout);
			printf("\n");
			return 0;
		}  
		/*sprintf(filename,"You are not chairman of %s.  Override as SysOp?",areascout);*/
		sprintf(filename,Ustring[153],areascout);
		strcat(filename," ");
		strcat(filename,Ustring[129]);
	} else {
		/*sprintf(filename,"Delete %s? ",areascout);*/
		sprintf(filename,Ustring[108],areascout);
	}
	if (!no_yes(filename)) {
		return 0;
	}

	/*printf("Please wait a few moments... ");*/
	printf("%s", Ustring[151]);
	
	if (destroy_area(areascout)) {
		char *date = shorttime(time(0));

		sprintf(filename,"%s/newarealog",C.datadir);	
		if ( (LOG=fopen(filename,"a")) ) {
			fprintf(LOG,"%s %s destroyed %s\n",date,U.id,areascout);
			fclose(LOG);
		}

		/*printf("%s destroyed.\n",areascout);*/
		printf(Ustring[257],areascout);
		printf("\n");

		free(date);
		return 1;
	} else {
		printf("%s\n",Ustring[268]);
		/*printf("cannot delete %s.\n");*/
		sprintf(filename,"%s was unable to destroy %s.",U.id,areascout);
		errorlog(filename);
		return 0;
	}
}

int destroy_area (char *areascout) {
	char command[MAINLINE + 100];
	int result;
	
	hups_off();
	sprintf(command,"rm -r %s/%s 2>/dev/null",C.areasdir,areascout);
	if (!dsystem(command)) {
		sprintf(command,"rm %s/*/.*areas/%s 2>/dev/null",C.users,areascout);
		dsystem(command);
		result = 1;
	} else {
		result = 0;
	}
	hups_on();
	return result;
}

/* ARGSUSED0 */
int describe (char *dummy) {
/* MENU COMMAND */
/* G.areaname must be set */
	char filename[MAINLINE + 100];
	FILE *FIL;
	char desc[51];
	char temp[60];

	desc[0] = 0;

	flushcom("");

	sprintf(filename,"%s/%s/description",C.areasdir,G.areaname);
	if ((FIL = fopen(filename,"r"))) {
		fgets(desc,51,FIL);
		tnt(desc);
		fclose(FIL);
	}
	if (desc[0]) {
		/*printf("Current description: %s\n",desc);*/
		printf("%s", Ustring[157]);
		printf(" %s\n",desc);
	}
	/*puts("Type up to 50 chars description, or press [Enter] to leave unchanged.");*/
	printf(Ustring[149],G.areaname);
	printf("\n");
	make_prompt(Ustring[150]);
	get_raw(1,51,temp,1);
	if (temp[0]) {
		strcpy(desc,temp);
	} else {
		return 0;
	}
	if ((FIL = fopen(filename,"w"))) {
		sprintf(temp,"%s\n",desc);
		fputs(temp,FIL);
		fclose(FIL);
	} else {
		/*printf("Unable to write description.\n");*/
		printf(Ustring[67],Ustring[199]);
		printf(" - %s\n",Ustring[60]);
		return 0;
	}
	return 1;
}

/* ARGSUSED0 */
int edit_info (char *dummy) {
/* MENU COMMAND */
/* CHECKED */
	int result = 0;
	char filename[MAINLINE + 100];

	if (G.areaname[0] == 0) {
		/*printf("Area name undefined.  Command not actioned.\n");*/
		printf("%s - %s\n",Ustring[124],Ustring[60]);
		return 0;
	}

	sprintf(filename,"%s/%s/info",C.areasdir,G.areaname);
	result = edit_special(filename);
	return result;
}


int add_members (char *in) {
/* MENU COMMAND */
	return add_a("members",in);
}

int add_chairmen (char *in) {
/* MENU COMMAND */
	return add_a("chair",in);
}

int add_gagged (char *in) {
/* MENU COMMAND */
	return add_a("gagged",in);
}

int rem_members (char *in) {
/* MENU COMMAND */
	return rem_a("members",in);
}

int rem_chairmen (char *in) {
/* MENU COMMAND */
	return rem_a("chair",in);
}

int rem_gagged (char *in) {
/* MENU COMMAND */
	return rem_a("gagged",in);
}



int show_members (char *dummy) {
/* MENU COMMAND */
	return show_a("members");
}

int show_chairmen (char *dummy) {
/* MENU COMMAND */
	return show_a("chair");
}

int show_gagged (char *dummy) {
/* MENU COMMAND */
	return show_a("gagged");
}

int show_info (char *dummy) {
/* MENU COMMAND */
	return show_a("info");
}

int show_a (char *whichfile) {
	int result = 0;
	char filename[MAINLINE + 100];

	if (G.areaname[0] == 0) {
		/*printf("Area name undefined - abandoned.\n");*/
		printf("%s - %s\n",Ustring[124],Ustring[60]);
		return 0;
	}
	if (!strcmp(whichfile,"members") && (!G.areaflags[PRIVATE])) {
		/*printf("Area is not private - members list irrelevant.\n");*/
		printf(Ustring[145],G.areaname);
		printf("\n");
		return 0;
	}

	sprintf(filename,"%s/%s/%s",C.areasdir,G.areaname,whichfile);
	result = display(filename);
	return result;
}



int add_a (char *whichlist,char *in) {
/* for use while in area */
	int result = 0;
	char filename[MAINLINE + 100];
	char *copy;

	if (G.areaname[0] == 0) {
		/*printf("Area name undefined - abandoned.\n");*/
		printf("%s - %s\n",Ustring[124],Ustring[60]);
		return 0;
	}
	if (!strcmp(whichlist,"members") && (!G.areaflags[PRIVATE])) {
		/*printf("Area is not private - members list irrelevant.\n");*/
		printf(Ustring[145],G.areaname);
		printf("\n");
		return 0;
	}

	sprintf(filename,"%s/%s/%s",C.areasdir,G.areaname,whichlist);
	copy = strdup(in);

	tnt(copy);
	if (copy[0]) {
		/*result = add_user_to_list('v', "user", filename, copy);*/
		result = add_user_to_list('v',Ustring[196], filename, copy);
	} else {
		/*result = add_user_to_list('v', "user", filename, G.comline);*/
		result = add_user_to_list('v',Ustring[196], filename, G.comline);
		flushcom("");
	}
	free(copy);
	return result;
}



int rem_a (char *whichlist,char *in) {
/* MENU COMMAND */
/* for use while in area */
	char filename[MAINLINE + 100];
	int result;
	char *copy;

	if (G.areaname[0] == 0) {
		/*printf("Area name undefined - abandoned.\n");*/
		printf("%s - %s\n",Ustring[124],Ustring[60]);
		return 0;
	}
	if (!strcmp(whichlist,"members") && (!G.areaflags[PRIVATE])) {
		/*printf("Area is not private - members list irrelevant.\n");*/
		printf(Ustring[145],G.areaname);
		printf("\n");
		return 0;
	}
	sprintf(filename,"%s/%s/%s",C.areasdir,G.areaname,whichlist);

	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {	
		result = rem_from_list('v', filename, copy);
	} else {
		result = rem_from_list('v', filename, G.comline);
		flushcom("");
	}
	free(copy);
	return result;
}

int setarea_level(char *in) {
/* MENU COMMAND */
	char *copy = strdup(in);
	char temp[MAINLINE];
	char tempa[MAINLINE];
	int result = 0;
	char levelstring[5];

	tnt(copy);
	shiftword(copy, temp, MAINLINE);
	free(copy);
	if (!temp[0]) {
		shiftword(G.comline, temp, MAINLINE);
	}

	/*printf("Current access level is %d.\n",G.arealevel);*/
	sprintf(tempa,"%d",G.arealevel);
	printf(Ustring[391],Ustring[166],tempa);
	printf("\n");
	/*new_get_one_param('v',"New access level: ",temp,levelstring,5);*/
	sprintf(tempa,Ustring[400],Ustring[166]);
	new_get_one_param('v',tempa,temp,levelstring,5);

	if (!levelstring[0]) {
		/*printf("No level specified\n");*/
		printf(Ustring[294],Ustring[167]);
		printf("\n");
		return 0;
	}

	if ((result = set_arealevel(G.areaname,levelstring))) {
		G.arealevel = arealevel_read(G.areaname);
		/*printf("Level set to %s.\n");*/
		printf(Ustring[387],Ustring[166],levelstring);
		printf("\n");
	} else {
		/*printf("Unable to write level.\n");*/
		printf(Ustring[67],Ustring[166]);
		printf("\n");
	}
	return result;
}

int set_arealevel(const char *area, const char *levelstring) {
	int level;

	if (!area || !area[0]) {
		strcpy(G.errmsg,"No area specified.\n");
		errorlog(G.errmsg);
		return 0;
	}
	
	if (!levelstring || !levelstring[0]) {
		strcpy(G.errmsg,"No level specified.\n");
		errorlog(G.errmsg);
		return 0;
	}
	
	if (!is_num(levelstring)) {
		strcpy(G.errmsg,"Invalid parameter for setting level.\n");
		errorlog(G.errmsg);
		return 0;
	}	

	level = atoi(levelstring);

	if (!arealevel_write(area,level)) {
		sprintf(G.errmsg,"%s failed to set level on %s to %d\n", U.id, area, level);
		errorlog(G.errmsg);
		return 0;
	}
	return 1;
}

int arealevel_write(const char *area, const int level) {
	FILE *F;
	char file[MAINLINE + 100];

	sprintf(file,"%s/%s/level",C.areasdir,area);
	if ( !(F = fopen(file, "w"))) {
		return 0;
	}

	if (!fprintf(F, "%d\n", level)) {
		fclose(F);
		return 0;
	}
	
	fclose(F);
	
	return 1;
}

int arealevel_read(const char *area) {
	FILE *F;
	int level;
	char file[MAINLINE + 100];

	sprintf(file,"%s/%s/level",C.areasdir,area);
	if ( !(F = fopen(file, "r"))) {
		return -1;
	}

	if (!fscanf(F, " %d ", &level)) {
		fclose(F);
		return -1;
	}
	
	fclose(F);
	
	return level;
}



int areaflags_read (char *area,char *flags) {
	char filename[MAINLINE + 100];
	FILE *CFG;
	char c;
	int i = 1;

	flags[0] = '#';
	sprintf(filename,"%s/%s/flags",C.areasdir,area);
	if ((CFG = fopen(filename,"r"))) {
		while ((i <= AFLAGMAX) && !feof(CFG)) {
			fread(&c,1,1,CFG);
			if (!isgraph(c)) {
				continue;
			}
			flags[i] = c;
			i++;
		}
		flags[i] = 0;
		fclose(CFG);
		return 1;
	} else {
		while (i <= AFLAGMAX) {
			flags[i] = '0';
			i++;
		}
		flags[i] = 0;
		return 0;
	}
}


int area_flagset (char *params) {
/* MENU COMMAND */
/* should pass flag offset and value.*/
/* meant for use while 'in' area */

	char string[140];
	int flag = 0;
	char value;
	int i;
	char *copy = strdup(params);
	
	if (!G.areaname[0]) {
		/*printf("Area name not specified.\n");*/
		printf("%s\n",Ustring[124]);
		free(copy);
		return 0;
	}

	shiftword(copy,string,21);

	if (!string[0]) {
		/*printf("No flag specified.\n");*/
		printf(Ustring[294],Ustring[71]);
		printf("\n");
		free(copy);
		return 0;
	}

	if (is_num(string)) {
		flag = atoi(string);
	} else {
		for (i=1;i < (AFLAGMAX + 1);i++) {
			if (!strcmp(C.aflagnames[i],string)) {
				flag = i;
			}
		}
	}

 	shiftword(copy,string,21);
	free(copy);
	
	if (!string[0]) {
		/*printf("No value specified.\n");*/
		printf(Ustring[294],Ustring[161]);
		printf("\n");
		return 0;
	}

	tnt(string);
	value = string[0]; /* just in case case was significant! */
	lower_string(string);

	if (!Dstrcmp(string,"off")) {
		value = '0';
	} else if (!Dstrcmp(string,"on")) {
		value = '1';
	} else if (!Dstrcmp(string,"toggle")) {
		if (G.areaflags[flag] == '0') {
			value = '1'; 
		} else {
			value = '0';
		}
	}

	if (set_areaflag(G.areaname, flag, value)) {
		G.areaflags[flag] = value;
		/*printf("Flag set to %s.\n");*/
		sprintf(string,"%c",value);
		printf(Ustring[387],Ustring[71],string);
		printf("\n");
	} else {
		/*printf("Cannot write flag.\n");*/
		printf(Ustring[67],Ustring[71]);
		printf("\n");
		return 0;
	}
	return 1;
}

int set_areaflag (char *area, int flag, char value) {
/* Writes it to file. Any area */

	char aflags[AFLAGMAX + 2];
	char filename[MAINLINE + 100];
	FILE *FIL;

	if ((!area[0]) || (!value)) {
		return 0;
	}
	if ((flag < 1) || (flag > AFLAGMAX)) {
		return 0;
	}

	if (!areaflags_read(area,aflags)) {
		return 0;
	}

	aflags[flag] = value;
	sprintf(filename,"%s/%s/flags",C.areasdir,G.areaname);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%s\n",&aflags[1]);
		fclose(FIL);
	} else {
		return 0;
	}
	return 1;
}

int areamask_read (char *area,char *flags) {
	char filename[MAINLINE + 100];
	FILE *CFG;
	char c;
	int i = 1;

	flags[0] = '#';
	sprintf(filename,"%s/%s/areamask",C.areasdir,area);
	if ((CFG = fopen(filename,"r"))) {
		while ((i <= UFLAGMAX) && !feof(CFG)) {
			fread(&c,1,1,CFG);
			if (!isgraph(c)) {
				continue;
			}
			flags[i] = c;
			i++;
		}
		flags[i] = 0;
		fclose(CFG);
		return 1;
	} else {
		while (i <= UFLAGMAX) {
			flags[i] = '0';
			i++;
		}
		flags[i] = 0;
		return 0;
	}
}


int mask_flagset (char *params) {
/* MENU COMMAND */
/* should pass flag offset and value.*/
/* meant for use while 'in' area */

	char string[140];
	int flag;
	char value;
	int i;
	char *copy = strdup(params);

	if (!G.areaname[0]) {
		/*printf("Area name not specified.\n");*/
		printf("%s\n",Ustring[124]);
		free(copy);
		return 0;
	}

	shiftword(copy,string,21);
	if (!string[0]) {
		/*printf("No flag specified.\n");*/
		printf(Ustring[294],Ustring[71]);
		printf("\n");
		free(copy);
		return 0;
	}

	if (is_num(string)) {
		flag = atoi(string);
	} else {
		for (i=1;i < (UFLAGMAX + 1);i++) {
			if (!strcmp(C.uflagnames[i],string)) {
				flag = i;
				break;
			}
		}
		flag = 0;
	}

 	shiftword(copy,string,21);
	free(copy);
	if (!string[0]) {
		/*printf("No value specified.\n");*/
		printf(Ustring[294],Ustring[161]);
		printf("\n");
		return 0;
	}

	tnt(string);
	value = string[0]; /* just in case case was significant! */
	lower_string(string);

	if (!Dstrcmp(string,"off")) {
		value = '0';
	} else if (!Dstrcmp(string,"on")) {
		value = '1';
	} else if (!Dstrcmp(string,"toggle")) {
		if (G.areamask[flag] == '0') {
			value = '1'; 
		} else {
			value = '0';
		}
	}

	if (set_maskflag(G.areaname, flag, value)) {
		G.areamask[flag] = value;
		/*printf("Flag set to %s.\n");*/
		sprintf(string,"%c",value);
		printf(Ustring[367],Ustring[71],string);
		printf("\n");
	} else {
		/*printf("Unable to write flag.\n");*/
		printf(Ustring[67],Ustring[71]);
		printf("\n");
		return 0;
	}
	return 1;
}

int set_maskflag (char *area, int flag, char value) {
/* Writes it to file. Any area */

	char mflags[UFLAGMAX + 2];
	char filename[MAINLINE + 100];
	FILE *FIL;

	if ((!area[0]) || (!value)) {
		return 0;
	}
	if ((flag < 1) || (flag > UFLAGMAX)) {
		return 0;
	}

	if (!areamask_read(area,mflags)) {
		return 0;
	}

	mflags[flag] = value;
	sprintf(filename,"%s/%s/areamask",C.areasdir,G.areaname);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%s\n",&mflags[1]);
		fclose(FIL);
	} else {
		return 0;
	}
	return 1;
}


int mask_menu (char *dummy) {
/* MENU COMMAND */
	return maskmenu(G.areaname);
}


int maskmenu (char *area) {
	char uflag[UFLAGMAX + 2];
	char filename[MAINLINE + 100];
	char tempstring[MAINLINE + 100];
	char newvalue[2];
	FILE *FIL;
	char flagstring[3];
	int choice;

	sprintf(filename,"%s/%s/areamask",C.areasdir,area);

	/* CONSTCOND */
	while (1) {
		if (!areamask_read(area,uflag)) {
			/*make_prompt("Can't read flag file. Create from scratch? y/N ");*/
			sprintf(tempstring,Ustring[66],Ustring[168]);
			sprintf(filename,"%s %s",tempstring,Ustring[165]);
			if (!no_yes(filename)) { 
				return 0;
			}

			sprintf(filename,"%s/%s/areamask",C.areasdir,area);
			if ((FIL = fopen(filename,"w"))) {
				fprintf(FIL,"%s\n",&C.newareamask[1]);
				fclose(FIL);
			} else {
				/*printf("There is an error in writing the areamask file.\n");*/
				printf(Ustring[67],Ustring[168]);
				printf("\n");
				return 0;
			}
			continue;			
		}

printf("%s\n",Ustring[168]);
/*printf("No.     Name        No.     Name        No.     Name\n");*/
printf("%s\n",Ustring[169]);
printf("[1]  %c  %-10s   [11]  %c  %-10s   [21]  %c  %-10s\n",uflag[1],C.uflagnames[1],uflag[11],C.uflagnames[11],uflag[21],C.uflagnames[21]);
printf("[2]  %c  %-10s   [12]  %c  %-10s   [22]  %c  %-10s\n",uflag[2],C.uflagnames[2],uflag[12],C.uflagnames[12],uflag[22],C.uflagnames[22]);
printf("[3]  %c  %-10s   [13]  %c  %-10s   [23]  %c  %-10s\n",uflag[3],C.uflagnames[3],uflag[13],C.uflagnames[13],uflag[23],C.uflagnames[23]);
printf("[4]  %c  %-10s   [14]  %c  %-10s   [24]  %c  %-10s\n",uflag[4],C.uflagnames[4],uflag[14],C.uflagnames[14],uflag[24],C.uflagnames[24]);
printf("[5]  %c  %-10s   [15]  %c  %-10s   [25]  %c  %-10s\n",uflag[5],C.uflagnames[5],uflag[15],C.uflagnames[15],uflag[25],C.uflagnames[25]);
printf("[6]  %c  %-10s   [16]  %c  %-10s   [26]  %c  %-10s\n",uflag[6],C.uflagnames[6],uflag[16],C.uflagnames[16],uflag[26],C.uflagnames[26]);
printf("[7]  %c  %-10s   [17]  %c  %-10s   [27]  %c  %-10s\n",uflag[7],C.uflagnames[7],uflag[17],C.uflagnames[17],uflag[27],C.uflagnames[27]);
printf("[8]  %c  %-10s   [18]  %c  %-10s   [28]  %c  %-10s\n",uflag[8],C.uflagnames[8],uflag[18],C.uflagnames[18],uflag[28],C.uflagnames[28]);
printf("[9]  %c  %-10s   [19]  %c  %-10s   [29]  %c  %-10s\n",uflag[9],C.uflagnames[9],uflag[19],C.uflagnames[19],uflag[29],C.uflagnames[29]);
printf("[10] %c  %-10s   [20]  %c  %-10s   [30]  %c  %-10s\n",uflag[10],C.uflagnames[10],uflag[20],C.uflagnames[20],uflag[30],C.uflagnames[30]);
/*printf("[0]  quit\n");*/
printf("[0]  %s\n",Ustring[63]);

		/*make_prompt("\nWhich flag? ");*/
		sprintf(tempstring,Ustring[393],Ustring[71]);
		make_prompt(tempstring);
		choice = get_one_num(2,0);

		if (!choice) {
			break;
		}

		if ((choice > 0) || (choice <= UFLAGMAX)) {
			sprintf(flagstring,"%d",choice);
			/*make_prompt("New value: ");*/
			sprintf(tempstring,Ustring[400],Ustring[161]);
			make_prompt(tempstring);
			get_one_char(newvalue);
			if (newvalue[0]) {
				set_maskflag(area,choice,newvalue[0]);
			} else {
				/*printf("Flag unchanged as %s.\n");*/
				printf(Ustring[394],Ustring[71],uflag[choice]);
				printf("\n");
			}
		} else if (choice > UFLAGMAX) {
			/*printf("Invalid input %s.\n",choice);*/
			printf(Ustring[472],choice);
			printf("\n");
		}
	}
	return 1;
}

int areaflag_menu (char *dummy) {
/* MENU COMMAND */
	return areaflagmenu(G.areaname);
}


int areaflagmenu (char *area) {
	char uflag[AFLAGMAX + 2];
	char filename[MAINLINE + 100];
	char tempstring[MAINLINE + 100];
	char newvalue[2];
	FILE *FIL;
	char flagstring[3];
	int choice;

	sprintf(filename,"%s/%s/flags",C.areasdir,area);

	/* CONSTCOND */
	while (1) {
		if (!areaflags_read(area,uflag)) {
			/*make_prompt("Can't read flag file. Create from scratch? y/N ");*/
			sprintf(tempstring,Ustring[66],Ustring[168]);
			sprintf(filename,"%s %s",tempstring,Ustring[165]);
			if (!no_yes(filename)) { 
				return 0;
			}

			sprintf(filename,"%s/%s/flags",C.areasdir,G.areaname);
			if ((FIL = fopen(filename,"w"))) {
				fprintf(FIL,"%s\n",&C.newareaflags[1]);
				fclose(FIL);
			} else {
				/*printf("There is an error in writing the flags file.\n");*/
				printf(Ustring[67],Ustring[168]);
				printf("\n");
				return 0;
			}
			continue;			
		}

/*printf("Flags\n");*/
printf("%s\n",Ustring[168]);
/*printf("No.     Name        No.     Name        No.     Name\n");*/
printf("%s\n",Ustring[169]);
printf("[1]  %c  %-10s   [11]  %c  %-10s   [21]  %c  %-10s\n",uflag[1],C.aflagnames[1],uflag[11],C.aflagnames[11],uflag[21],C.aflagnames[21]);
printf("[2]  %c  %-10s   [12]  %c  %-10s   [22]  %c  %-10s\n",uflag[2],C.aflagnames[2],uflag[12],C.aflagnames[12],uflag[22],C.aflagnames[22]);
printf("[3]  %c  %-10s   [13]  %c  %-10s   [23]  %c  %-10s\n",uflag[3],C.aflagnames[3],uflag[13],C.aflagnames[13],uflag[23],C.aflagnames[23]);
printf("[4]  %c  %-10s   [14]  %c  %-10s   [24]  %c  %-10s\n",uflag[4],C.aflagnames[4],uflag[14],C.aflagnames[14],uflag[24],C.aflagnames[24]);
printf("[5]  %c  %-10s   [15]  %c  %-10s   [25]  %c  %-10s\n",uflag[5],C.aflagnames[5],uflag[15],C.aflagnames[15],uflag[25],C.aflagnames[25]);
printf("[6]  %c  %-10s   [16]  %c  %-10s   [26]  %c  %-10s\n",uflag[6],C.aflagnames[6],uflag[16],C.aflagnames[16],uflag[26],C.aflagnames[26]);
printf("[7]  %c  %-10s   [17]  %c  %-10s   [27]  %c  %-10s\n",uflag[7],C.aflagnames[7],uflag[17],C.aflagnames[17],uflag[27],C.aflagnames[27]);
printf("[8]  %c  %-10s   [18]  %c  %-10s   [28]  %c  %-10s\n",uflag[8],C.aflagnames[8],uflag[18],C.aflagnames[18],uflag[28],C.aflagnames[28]);
printf("[9]  %c  %-10s   [19]  %c  %-10s   [29]  %c  %-10s\n",uflag[9],C.aflagnames[9],uflag[19],C.aflagnames[19],uflag[29],C.aflagnames[29]);
printf("[10] %c  %-10s   [20]  %c  %-10s   [30]  %c  %-10s\n",uflag[10],C.aflagnames[10],uflag[20],C.aflagnames[20],uflag[30],C.aflagnames[30]);
/*printf("[0]  quit\n");*/
printf("[0]  %s\n",Ustring[63]);

		/*make_prompt("\nWhich flag? ");*/
		sprintf(tempstring,Ustring[393],Ustring[71]);
		make_prompt(tempstring);
		choice = get_one_num(2,0);

		if (!choice) {
			break;
		}

		if ((choice > 0) || (choice <=AFLAGMAX)) {
			sprintf(flagstring,"%d",choice);
			/*make_prompt("New value: ");*/
			sprintf(tempstring,Ustring[400],Ustring[161]);
			make_prompt(tempstring);
			get_one_char(newvalue);
			if (newvalue[0]) {
				set_areaflag(area,choice,newvalue[0]);
			} else {
				/*printf("Flag unchanged as %s.\n");*/
				printf(Ustring[394],Ustring[71],uflag[choice]);
				printf("\n");
			}
		} else if (choice > AFLAGMAX) {
			/*printf("No such flag.\n");*/
			printf(Ustring[472],choice);
			printf("\n");
		}
	}
	return 1;
}



/* ========================================================= */
/* UTILITY BITS */

int parse_area_header(char *header, struct areaheader *h) {
	char *p, *q;

	p = header;

	for(q = p; !isspace(*p); p++)
		; 
	h->flag = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->number = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->dname = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->mname = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->dom = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->time = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->tzname = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->year = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->from = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->author = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->narrative = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->hash = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->parent = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->by = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->parentby = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->base = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->next = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	h->prev = q;
	for(;isspace(*p); *p++='\0');

	h->footer = p;

	return 1;
}

int moretoread (int highest) {
	char string[MAINLINE + 100];
	FILE *HANDLE;
	int i = 0;

	sprintf(string,"%s/%s/highest",C.areasdir,G.areaname);
	if (!(HANDLE = fopen(string,"r"))) {
		return 0;
	}
	fgets(string,5,HANDLE);
	string[5] = 0;
	fclose(HANDLE);
	G.highmsg = atoi(string);
	if (highest >= G.highmsg) {
		return 0;
	}

	sprintf(string,"%s/%s/msgindex",C.areasdir,G.areaname);
	if (!(HANDLE = fopen(string,"r"))) {
		return 0;
	}
	fseek(HANDLE,highest + 1,SEEK_SET);

	for (i = highest + 1;i <= G.highmsg;i++) {
		fread(&G.mymsgindex[i],1,1,HANDLE);
	}
	fclose(HANDLE);
	return 1;
}

void msgdesclist (void) {
	printf("\n");
	printf("[%s] %s\n",Ustring[21],Ustring[22]);/*next*/
	printf("[%s] %s\n",Ustring[11],Ustring[12]);/*current*/
	printf("[%s] %s\n",Ustring[9],Ustring[10]);/*next in thread*/
	printf("[%s] %s\n",Ustring[7],Ustring[8]);/*previous in thread*/
	printf("[%s] %s\n",Ustring[13],Ustring[14]);/*parent*/
	printf("[%s] %s\n",Ustring[5],Ustring[6]);/*base*/
	printf("[%s] %s\n",Ustring[1],Ustring[2]);/*first*/
	printf("[%s] %s\n",Ustring[3],Ustring[4]);/*last*/
	printf("[%s] %s\n",Ustring[17],Ustring[18]);/*forward numeric*/
	printf("[%s] %s\n",Ustring[19],Ustring[20]);/*backward numeric*/
	printf("[%s] %s\n",Ustring[23],Ustring[24]);/*thread*/
	printf("[%s] %s\n",Ustring[25],Ustring[26]);/*branch*/
	/*printf("    or any message number\n");*/
	printf("    %s\n",Ustring[31]);
	printf("\n");
}

char *definemsg(const char mode, const char *area, const int msgno) {
	FILE *HEADER;
	char filename[MAINLINE + 100];
	struct stat statbuf;
	struct areaheader h;
	char *header;
	
	sprintf(filename, "%s/%s/hdr.%d",C.areasdir,area,msgno);
	if (!(HEADER = fopen(filename,"r"))) {
		if (errno == EPERM) {
			char errorstring[MAINLINE];
			if (mode == 'v') {
				/*printf("Unable to read message no. %d.  Fault reported.\n",msgno);*/
				sprintf(filename,Ustring[193],msgno);
				printf(Ustring[66],filename);
				printf("\n");
			}
			sprintf(errorstring,"%s hdr.%d permission denied",area,msgno);
			errorlog(errorstring);
		} else if (errno == ENOENT) {
			if (mode == 'v') {
				/*puts("Sorry, the message is not available.");*/
				sprintf(filename,Ustring[193],msgno);
				printf(Ustring[65],filename);
				printf("\n");
			}
		}
		return NULL;
	}

	stat(filename,&statbuf);

	header = (char *)malloc((unsigned)statbuf.st_size + 1);
	fread(header,(unsigned)statbuf.st_size,1,HEADER);
	header[statbuf.st_size] = 0;
	
	parse_area_header(header,&h);
	if (!is_num(h.prev)) {
		sprintf(G.errmsg,"Header %d corrupt in %s",msgno,area);
		errorlog(G.errmsg);
		if (mode == 'v') {
			/*printf("Sorry, header %d is corrupt.\n",msgno);*/
			sprintf(filename,Ustring[193],msgno);
			printf(Ustring[66],filename);
			printf("\n");
		}
		free(header);
		fclose(HEADER);
		return NULL;
	}		

	rewind(HEADER);
	fread(header,(unsigned)statbuf.st_size,1,HEADER);

	fclose(HEADER);

	header[statbuf.st_size] = 0;
	return header;
}

struct valid_messages *get_valid_messages (const char mode, const char *area,
	const char *inprompt, const int current, const char *inparams,
	const int threadlock) {

	int i = 0;
	char msgchoice[12];
	struct valid_messages *result;
	char *prompt = NULL;
	char *params = NULL;
	char temp[MAINLINE];

	if (inprompt[0] == 0) {
		/*prompt = strdup("Which message? (? for suggestions): ");*/
		sprintf(temp,"%s (%s)",Ustring[70],Ustring[200]);
		prompt = strdup(temp);
	} else {
		prompt = strdup(inprompt);
	}

	if (inparams[0] == 0) {
		params = (char *)malloc(MAINLINE);
		params[0]=0;
	} else {
		params = strdup(inparams);
	}
	shiftword(params,msgchoice,12);
	lower_string(msgchoice);

	if (msgchoice[0] == 0) {

		if (mode == 'v') {
			/* CONSTCOND */
			while (1) {
				new_get_one_param('v',prompt,params,msgchoice,12);

				if (msgchoice[0] == 0) {
					free(params);
					free(prompt);
					return 0;
				}
				if (!strcmp(msgchoice,"?")) {
					msgdesclist();
					continue;
				}
				break;
			}
		} else {
			free(params);
			free(prompt);
			return 0;
		}
	}
	free(params);
	free(prompt);

	result = (struct valid_messages *)malloc(sizeof (struct valid_messages));
	result->lock = NULL;
	result->parse = NULL;
	result->msglist = NULL;

	while (md[i].keyword) {
	/* When a descriptor word was input */
		if (!Dstrcmp(*(md[i].keyword),msgchoice) || !Dstrcmp(md[i].parse,msgchoice)) {
			result->parse = strdup(md[i].parse);
			result->msglist = md[i].func(mode,area,current);
			break;
		}
		i++;
	}
	if (!result->parse) {
	/* When a number was input */
		if (is_num(msgchoice)) {
			result->parse = strdup(msgchoice);
			result->msglist = findnumber(mode,area,atoi(msgchoice));
		}
	}
	if (!result->parse) {
		if (mode == 'v') {
			/*printf("%s is not a valid message descriptor.\n",msgchoice);*/
			printf(Ustring[472],msgchoice);
			printf("\n");
		}
		free(result);
		return NULL;
	}
	if (!result->msglist) {
		free(result->parse);
		free(result);
		return NULL;
	}
	if (threadlock) {
		char lockname[MAINLINE];
		char base[5];
		char *header = definemsg('q',area,result->msglist[0]);

		if (!header) {
			free(result->msglist);
			free(result->parse);
			free(result);
			return NULL;
		}

		i = 0;
		while (i <= 15) {
			shiftword(header,base,5);
			i++;
		}
		sprintf(lockname,"%s/%s/%s.lock",C.areasdir,area,base);
		if (!place_lock('q',lockname,1,1)) {
			free(header);
			free(result->msglist);
			free(result->parse);
			free(result);
			return NULL;
		} else {
			result->lock = strdup(lockname);
		}
	}
	return result;
}

int msgindex (const char *area, const char value, const int *offsets) {
	FILE *INDEX;
	int i = 0;
	char temp[MAINLINE + 100];
	char tempa[MAINLINE + 100];

	sprintf(temp,"%s/%s/indexlock",C.areasdir,area);
	if (!place_lock('q',temp,1,0)) {
		return 0;
	}
	sprintf(tempa,"%s/%s/msgindex",C.areasdir,area);
	if (!(INDEX = fopen(tempa,"r+"))) {
		sprintf(G.errmsg,"msgindex: fopen %s: %s",tempa,strerror(errno));
		errorlog(G.errmsg);
		rem_lock(temp);
		return 0;
	}
	while (offsets[i]) {
		fseek(INDEX,offsets[i],SEEK_SET);
		if (offsets[i] > 4000) {
			/*printf("Overenthusiastic index!\n");*/
			printf("%s\n",Ustring[87]);
			break;
		}
		fwrite(&value,sizeof (char),1,INDEX);
		i++;
	}
	fclose(INDEX);
	rem_lock(temp);
	if (!strcmp(area,G.areaname)) {
		mlo(value,offsets);
	}
	return 1;
}

void mco (const char updown, const int *offsets) {
	int pointer = 0;

	if (updown == 'u') {

		while (offsets[pointer]) {
			G.mymsgindex[offsets[pointer]] =
				toupper(G.mymsgindex[offsets[pointer]]);
			pointer++;
		}
	} else {
		while (offsets[pointer]) {
			G.mymsgindex[offsets[pointer]] =
				tolower(G.mymsgindex[offsets[pointer]]);
			pointer++;
		}
	}
}

void mlo (char value, const int *offsets) {
	int pointer = 0;

	while (offsets[pointer]) {
		if (isupper(G.mymsgindex[offsets[pointer]])) {
			value = toupper(value);
		} else {
			value = tolower(value);
		}
		G.mymsgindex[offsets[pointer]] = value;
		pointer++;
	}
}

int createvotemsg (char *in) {
/* MENU COMMAND */
	int message;
	int result = 0;
	struct valid_messages *vm;
	char filename[MAINLINE + 100];
	FILE *FIL;
	char temp[21];
	char *params;
	char *copy;

	if (!check_area('v',G.areaname)) {
		return 0;
	}
	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		free(copy);
		params = strdup(copy);
	} else {
		free(copy);
		params = strdup(G.comline);
		flushcom("");
	}
	
	if (!params[0]) {
		free(params);
		sprintf(temp,"%d",G.current);
		params = strdup(temp);
	}
	vm = get_valid_messages('v',G.areaname,Ustring[70],G.current,params,0);/*"Reply to which message"*/

	free(params);

	if (!vm) {
		return 0;
	}
	if (!strcmp(vm->parse,"thread") || !strcmp(vm->parse,"branch")) {
		/*printf("Cannot do that to more than one message at a time!\n");*/
		printf("%s", Ustring[72]);
		printf("\n");
		free(vm->msglist);
		free(vm->parse);
		free(vm);
		return 0;
	}

	message = vm->msglist[0];
	free(vm->parse);
	free(vm->msglist);
	free(vm);

	if (!is_msg_elig(message)) {
		/*printf("You do not have any rights over %d.\n",message);*/
		printf(Ustring[80],message);
		printf("\n");
		return 0;
	}

	sprintf(filename,"%s/%s/vote.%d",C.areasdir,G.areaname,message);
	if ((FIL = fopen(filename,"w"))) {
		fclose(FIL);
		/*printf("Message %d flagged as vote.\n",message);*/
		printf(Ustring[106],message,Ustring[496]);
		printf("\n");
		result = 1;
	} else {
		/*printf("Cannot create vote.\n");*/		
		printf(Ustring[68],Ustring[496]);
		printf("\n");
		result = 0;
	}

	return result;
}

int vote (char *in) {
/*MENU ACTION*/
	FILE *FIL;
	int message;
	char filename[MAINLINE + 100];
	char temp[MAINLINE];
	char tempid[9];
	struct valid_messages *vm = 0;
	char *params;
	char *copy;
	char vote_opt[2];

	if (!check_area('v',G.areaname)) {
		return 0;
	}

	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		free(copy);
		params = strdup(copy);
	} else {
		free(copy);
		params = strdup(G.comline);
		flushcom("");
	}
	
	if (!params[0]) {
		free(params);
		sprintf(temp,"%d",G.current);
		params = strdup(temp);
	}
	vm = get_valid_messages('v',G.areaname,Ustring[70],G.current,params,0);/*"Reply to which message"*/

	free(params);

	if (!vm) {
		return 0;
	}
	if (!strcmp(vm->parse,"thread") || !strcmp(vm->parse,"branch")) {
		/*printf("Cannot do that to more than one message at a time!\n");*/
		printf("%s", Ustring[72]);
		printf("\n");
		free(vm->msglist);
		free(vm->parse);
		free(vm);
		return 0;
	}

	message = vm->msglist[0];
	free(vm->parse);
	free(vm->msglist);
	free(vm);

	sprintf(filename,"%s/%s/vote.%d",C.areasdir,G.areaname,message);
	if ((FIL = fopen(filename,"r"))) {
		while (fgets(temp,11,FIL)) {
			shiftword(temp,vote_opt,2);
			shiftword(temp,tempid,9);
			if (!strcmp(tempid,U.id)) {
				/*You have already voted*/
				printf("%s\n",Ustring[497]);
				fclose(FIL);
			 	return 0;
			}
		}
		fclose(FIL);
	} else {
		/*print this isn't a voting message;*/
		printf("%s\n",Ustring[498]);
		return 0;
	}
	
	sprintf(filename,"%s/%s/msg.%d",C.areasdir,G.areaname,message);
	display(filename);
	printf("\n");
	make_prompt(Ustring[479]);
	get_one_lc_char(vote_opt);
	if (!vote_opt[0]) {
		return 1;
	}

	sprintf(filename,"%s/%s/vote.%d",C.areasdir,G.areaname,message);
	if ((FIL = fopen(filename,"a"))) {
		fprintf(FIL,"%c %s\n",vote_opt[0],U.id);
		fclose(FIL);
		return 1;
	} else {
		/*print cannot write file;*/
		printf(Ustring[67],Ustring[496]);
		printf("\n");
		return 0;
	}
}

void summarise (int msgno) {
	char filename[MAINLINE + 1];
	char temp[MAINLINE];
	char vote_opt;
	char next_opt;
	FILE *PIPE;
	FILE *FIL;
	int vote_opt_score = 0;

	sprintf(filename,"%s/%s/vote.%d",C.areasdir,G.areaname,msgno);
	if (!(FIL = fopen(filename,"r"))) {
		/*Don't bother to put out error message as this will be run even for messages which were not meant to have votes*/
		return;
	} else {
		fclose(FIL);
		sprintf(temp,"sort %s",filename);
		
		if (!(PIPE = popen(temp,"r"))) {
			return;
		}

		printf("\n");
		printf("%s\n",Ustring[500]);/*This is a voting message - the results are:*/		
		if (fgets(temp,MAINLINE,PIPE)) {
			vote_opt = temp[0];
			vote_opt_score++;
			while (fgets(temp,MAINLINE,PIPE)) {
				next_opt = temp[0];
				if (next_opt == vote_opt) {
					vote_opt_score++;
				} else {
					printf("%c) %d\n",vote_opt,vote_opt_score);	
					vote_opt = next_opt;
					vote_opt_score = 1;
				}
			}
			printf("%c) %d\n",vote_opt,vote_opt_score);	
		} else {
			/*print No votes have been caste*/
			printf("%s\n",Ustring[499]);
		}
		pclose(PIPE);
	}
}
	
