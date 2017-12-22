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
/* BIG NOTE:  Only single dots with no other non-space characater will be
   treated as a separator.
   
   grabfiles and putfiles will have single dots between messages/items
   
   when grabbing, all leading dots will get a protecting dot
   when putting, all leading dots will be stripped
*/  
   
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

#if defined(READ_COMMANDS)
#include "readfuncs.h"
#endif
#if defined(FILE_COMMANDS)
#include "filefuncs.h"
#endif
#if defined(MAIL_COMMANDS)
#include "mailfuncs.h"
#include "sendmail.h"
#endif

#include "olrfuncs.h"


/* ========================================================================= */

/* ARGSUSED0 */
int olr_here (char *dummy) {
/* MENU COMMAND */
#if !defined(FILE_COMMANDS)
	/*printf("Utility not available in this version.\n");*/
	printf(Ustring[473],Ustring[464]);
	printf("\n");
	flushcom("");
	return 0;
#else
	
	struct stat statbuf;
	char filename[MAINLINE + 50];
	char response[2];
	int i;

	shiftword(G.comline,response,2);
	flushcom("");

	while (!menumatch(response,Ustring[477])) {
		/*
		printf("\nOffline Reading and Editing Support Routines\n");
		printf(  "--------------------------------------------\n\n");
		printf("[h] - help and how to use this routine\n");
		printf("[g] - go ahead and do it\n");
		printf("[q] - quit\n");
		printf("\n");
		*/
		printf("\n%s\n",Ustring[474]);
		printf(  "--------------------------------------------\n\n");
		printf("[%s] - %s\n",Ustring[475],Ustring[476]);
		printf("[%s] - %s\n",Ustring[477],Ustring[478]);
		printf("[%s] - %s\n",Ustring[54],Ustring[63]);
		printf("\n");
			
		/*make_prompt("Grab Option: ");*/
		make_prompt(Ustring[479]);
		
	 	get_one_lc_char(response);
		/*if (response[0] == 'h') {*/
		if (menumatch(response,Ustring[475])) {
			display_lang("olr");
		/*} else if (response[0] == 'q') {*/
		} else if (menumatch(response,Ustring[54])) {
			return 0;
		}
	}

	sprintf(filename,"%s/%s/putfile",C.privatefiles,U.id);
	
	strcpy(response,"");
	while (!menumatch(response,Ustring[480]) &&
	!menumatch(response,Ustring[482]) && 
	!menumatch(response,Ustring[484]) &&
	!menumatch(response,Ustring[54])) {
		if (!stat(filename,&statbuf)) {
			/*printf("A putfile already exists in your directory.\n");*/
			printf(Ustring[263],"putfile");
			printf("\n");
			/*
			printf("[e] - use existing putfile\n");
			printf("[Y] - upload new putfile (default)\n");
			printf("[n] - I have no messages to post\n");
			printf("[q] - quit this utility\n");
			*/
			printf("[%s] - %s\n",Ustring[480],Ustring[481]);
			printf("[%s] - %s\n",Ustring[482],Ustring[483]);
			printf("[%s] - %s\n",Ustring[484],Ustring[485]);
			printf("[%s] - %s\n",Ustring[54],Ustring[63]);
			printf("\n");
			/*make_prompt("Option: ");*/
			make_prompt(Ustring[479]);
		} else {
			printf("\n");
			printf("[%s] - %s\n",Ustring[482],Ustring[483]);
			printf("[%s] - %s\n",Ustring[484],Ustring[485]);
			printf("[%s] - %s\n",Ustring[54],Ustring[63]);
			printf("\n");
			make_prompt(Ustring[479]);
		}
		get_one_lc_char(response);
	}
	if (menumatch(response,Ustring[54])) {
		/*printf("Abandoning...\n");*/
		printf("%s\n",Ustring[60]);
		return 0;
	} else if (menumatch(response,Ustring[482])) {
		remove(filename);
		sprintf(filename,"%s/%s",C.privatefiles,U.id);
		uploading(filename,"putfile");
		if (!olrput('v')) {
			return 0;
		}
	} else if (menumatch(response,Ustring[484])) {
		sprintf(filename,"%s/%s/putfile",C.privatefiles,U.id);
		remove(filename);
	} else if (menumatch(response,Ustring[480])) {
		if (!olrput('v')) {
			return 0;
		}
	}


	/*make_prompt("Go on to grab all new messages now? Y/n ");*/
	if (!yes_no(Ustring[486])) {
		return 1;
	}
	
	area_clearup();
	/*make_prompt("Store current pointers first? Y/n ");*/
	if (yes_no(Ustring[487])) {
		store("");
		store_mail("");
	}

	i = grab_all_mail('g');
	if (grab_all_areas('g') || i) {
		printf("\n");
		/*make_prompt("Download now? Y/n ");*/
		if (yes_no(Ustring[488])) {
			sprintf(filename,"%s/%s",C.privatefiles,U.id);
			if (downloading(1,filename,"grabpad")) {
				/*make_prompt("OK to delete your grabpad? Y/n ");*/
				sprintf(filename,Ustring[108],"grabpad");
				if (yes_no(filename)) {
					sprintf(filename,"%s/%s/grabpad",C.privatefiles,U.id);
					remove(filename);
				}
			}
		}
	}
	return 1;
#endif
}

int olrput (char mode) {
	int i;
	struct stat statbuf;
	char filename[MAINLINE + 50];
	char command[MAINLINE + MAINLINE + 50];
	char line[MAINLINE];
	char temp[MAINLINE];
	FILE *PUT;
	FILE *FIL;


	sprintf(filename,"%s/%s/putfile",C.privatefiles,U.id);
	if (!(PUT = fopen(filename,"r"))) {
		if (mode == 'v') {
			/*printf("Error reading putfile.\n");*/
			printf(Ustring[66],"putfile");
			printf("\n");
		} else {
			do_run_error("Error reading putfile.");
		}
		return 0;
	}
	sprintf(filename,"%s/%s/.tfolrm",C.users,U.id);
	if (FIL = fopen(filename,"w")) {
		i = 0;
		while (fgets(line,MAINLINE,PUT)) {
			if (is_msgsep(line) && i) {
				break;
			} else if (!is_msgsep(line)) {
		
				fputs(line,FIL);
				i++;
			}
		}
		fclose(FIL);

	}
	fclose(PUT);

	sprintf(filename,"%s/%s/.lfolrm",C.users,U.id);
	if (!stat(filename,&statbuf)) {
		sprintf(command,"diff %s/%s/.tfolrm %s >/dev/null 2>&1",C.users,U.id,filename);
		if (!dsystem(command)) {
			if (mode == 'v') {
				/*printf("Error: This putfile appears to be a duplicate.\n");*/
				/*printf("Putfile not posted.\n");*/
				printf("%s\n",Ustring[489]);
			} else {
				do_run_error("Error: This putfile appears to be a duplicate.\nPutfile not posted.");
			}
			return 0;
		}
	}
	sprintf(command,"%s/%s/.tfolrm",C.users,U.id);
	rename(command,filename);
	if (mode == 'v') {
		/*printf("Posting your messages now...");*/
		printf(Ustring[271],Ustring[86]);
	}

	sprintf(filename,"%s/%s/putfile",C.privatefiles,U.id);
	if (PUT = fopen(filename,"r")) {
		while(fgets(line,MAINLINE,PUT)) {
			hups_off();
			shiftword(line,temp,MAINLINE);

			if (!strcmp(temp,"mail")) {
#if defined(MAIL_COMMANDS)
				shiftword(line,temp,MAINLINE);
				if (!strcmp(temp,"post")) {
					putmailpost(mode,line,PUT);
				} else if (!strcmp(temp,"reply")) {
					putmailreply(mode,line,PUT);
				} else {
					do_put_error("Bad mail command.",PUT);
				}
#else
				do_put_error("No private mail facilities.",PUT);
#endif
			} else if (!strcmp(temp,"area")) {
#if defined(READ_COMMANDS)
				sscanf(line,"%*s %s",temp); /* skip areaname */

				if (!strcmp(temp,"post")) {
					putareapost(mode,line,PUT);
				} else if (!strcmp(temp,"reply")) {
					putareareply(mode,line,PUT);
				} else if (!strcmp(temp,"comment")) {
					putareacomment(mode,line,PUT);
				} else {
					do_put_error("Bad area command.",PUT);
				}
#else
				do_put_error("No public area facilities.",PUT);
#endif
			}
			hups_on();
		}
		fclose(PUT);
		return 1;
	}
	do_run_error("Could not read putfile.  No messages posted.");
	return 0;
}

#if defined(MAIL_COMMANDS)
int putmailpost(char mode,char *headline,FILE *PUT) {
	int i;
	char filename[MAINLINE + 50];
	char temp[MAINLINE];
	char recip[MAINLINE];
	char *frecip;

	shiftword(headline,temp,MAINLINE); /* recipient list, comma separated */
	tnt(headline);
	/* headline now contains subject */

	if (!temp[0]) {
		do_put_error("No recipient named.",PUT);
		return 0;
	}
	make_message(PUT,"message");
	if (U.level < C.pvtmaillevel) {
		strcpy(temp,"sysop");
	}
	for (i=0;temp[i];i++) {
		if (temp[i] == ',') {
			temp[i] = ' ';
		}
	}
	while (temp[0]) {
		shiftword(temp,recip,MAINLINE);
		frecip = mail_check_recips(mode,recip);
		if (frecip[0]) {
			sendmail(mode,frecip,U.id,"message","base",headline,'f');
		} else {
			sprintf(filename,"Mail to %s not sent. Invalid recipient?",recip);
			do_message_error(filename,"message");
		}
		free(frecip);
	}
	remove("message");
	return 1;
}

int make_message(FILE *PUT, char *filename) {
	FILE *MSG;
	char line[MAINLINE];

	if (MSG = fopen(filename,"w")) {
		while(fgets(line,MAINLINE,PUT) && !is_msgsep(line)) {
			if (line[0] == '.') {
			/* This is just to strip out leading dots, see top of file*/
				fputs(&line[1],MSG);
			} else {
				fputs(line,MSG);
			}
		}
		fclose(MSG);
		return 1;
	} else {
		return 0;
	}
}

int is_msgsep(char *line) {
	char *temp = strdup(line);
	int result = 0;

	tnt(temp);
	if (temp[0] == '.') {
		result = !temp[1];
	}
	free(temp);
	return result;
}

int putmailreply (char mode,char *headline,FILE *PUT) {
	char filename[MAINLINE + 50];
	char temp[MAINLINE];
	char recip[MAINLINE];
	struct mailheader mh;
	char *subject;
	char *header;
	char *frecip;
	
	shiftword(headline,temp,MAINLINE); /* temp contains mail msgno */
	/* headline now contains subject */

	if (!(header = definemail(mode,U.id,atoi(temp)))) {
		sprintf(filename,"'reply' could not find original mail number '%s'.",temp);
		do_put_error(filename,PUT);
		return 0;
	}
	parse_mail_header(header, &mh);
	strcpy(recip,mh.author);

	tnt(headline);
	if (headline[0]) {	
		subject = strdup(headline);
	} else {		
		subject = strdup(mh.subject);
	}

	make_message(PUT,"message");

	frecip = mail_check_recips(mode,recip);

	if (frecip[0]) {
		sendmail(mode,frecip,U.id,"message",temp,subject,'f');
	} else {
		sprintf(filename,"Mail to %s not sent. Invalid recipient?",frecip);
		do_message_error(filename,"message");
	}
	free(frecip);
	free(subject);
	free(header);
	remove("message");
	return 1;
}
#endif

#if defined(READ_COMMANDS)
int putareapost(char mode,char *headline,FILE *PUT) {
	return putareamain('p',mode,headline,PUT);
}

int putareareply(char mode,char *headline,FILE *PUT) {
	return putareamain('r',mode,headline,PUT);
}

int putareacomment(char mode,char *headline,FILE *PUT) {
	return putareamain('c',mode,headline,PUT);
}

int putareamain(char cmd,char mode,char *params,FILE *PUT) {
	char areaname[15];
	char reply_to[15];
	char areaflags[AFLAGMAX + 2];
	struct areaheader phs;
	char filename[MAINLINE + 50];
	char temp[MAINLINE];
	char tempa[21];
	char sender[15];
	char sendhead[40];
	FILE *FIL;
	int parent;
	int moderated = 0;
	int result;
	struct valid_messages *vm;
	char *tempheader;
	char *subject;

	shiftword(params,areaname,15);
	if (!areaname[0]) {
		do_put_error("No message area named.",PUT);
		return 0;
	}
	if (is_area_elig('q',areaname) < 2) {
		sprintf(temp,"Area %s cannot accept your message.",areaname);
		do_put_error(temp,PUT);
		return 0;
	}

	shiftword(params,temp,1); /* this is the command, but we ignore it */
	if (cmd != 'p') {
		shiftword(params,reply_to,15);
		if (!reply_to[0]) {
			do_put_error("No message number given.",PUT);
			return 0;
		}
		vm = get_valid_messages('q',areaname,"",0,reply_to,0);
		if (!vm) {
			sprintf(temp,"'%s' invalid in area %s.",reply_to,areaname);
			do_put_error(temp,PUT);
			return 0;
		}
		if (!strcmp(vm->parse,"thread") || !strcmp(vm->parse,"branch")
			|| (!vm->msglist[0]) || (vm->msglist[1])
			|| (!(tempheader = definemsg('q',areaname,vm->msglist[0])))) {
			sprintf(temp,"'%s' invalid in area %s.",reply_to,areaname);
			do_put_error(temp,PUT);
			free(vm->msglist);
			free(vm->parse);
			free(vm);
			return 0;
		}
		parse_area_header(tempheader,&phs);
		parent = atoi(phs.number);
		free(tempheader);
	} else {
		parent = 0;
	}

	strcpy(sender,U.id);



	tnt(params);
	if (cmd == 'p') {
		subject = strdup(params);
		strcpy(sendhead,"BASE-MESSAGE - - - -");
	} else {
		sprintf(filename,"%s/%s/msg.%d",C.areasdir,areaname,parent);
		if (FIL = fopen(filename,"r")) {
			fgets(temp,MAINLINE,FIL);
			shiftword(temp,tempa,5);
			tnt(temp);
			fclose(FIL);
		}
		tnt(temp);
		if (params[0]) {
			subject = strdup(params);
		} else if (temp[0]) {
			subject = strdup(temp);
		} else {
			subject = (char *)malloc(2 * sizeof (char));
			subject[0] = 0;
		}
	}

	make_message(PUT,"message");

	areaflags_read(areaname,areaflags);
	if (areaflags[SIGS] != '0') {
		sprintf(filename,"%s/%s/.sig",C.privatefiles,U.id);
		if (FIL = fopen(filename,"r")) {
			int numread;
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
	if (areaflags[MODERATED] == '1') {
		moderated = 1;
		sprintf(filename,"%s/%s/chair",C.areasdir,areaname);
		if (is_in_list(filename,U.id)) {
			moderated = 0;
		} else if (U.level >= C.sysoplevel) {
			moderated = 0;
		}
	}

	if (moderated == 1) {
		result = wait_mod('q',sender,areaname,cmd,parent,subject,"message");
	} else {
		result = send_now('q',sender,areaname,cmd,parent,subject,"message");
	}
	remove("message");
	free(subject);	

	totalmessages_write(U.id,totalmessages_read(U.id) + 1);

	return result;
}
#endif

/* ========================================================================= */

int grab_all_mail(char mode) {
#if defined(MAIL_COMMANDS)
	char filename[MAINLINE];
	struct stat statbuf;
	struct valid_mail *vm;

	if (mode != 'q') {
		mode = 'v';
	}
	if (U.level >= C.extmaillevel) {
		sprintf(filename,"%s/%s",C.unixmaildir,U.id);
		if ((!stat(filename,&statbuf)) && (statbuf.st_size)) {
			if (mode == 'q') {
				get_unix_mail(mode,U.id);
			} else {
				any_unix_mail("");
			}
		}
	}
	vm = get_valid_mail('q',U.id,"",0,"new");
	if (vm) {
		if (mode != 'q') {
			/*printf("Grabbing your mail");*/
		}
		grabmail(mode,U.id,vm->msglist);
		if (mode != 'q') {
			/*printf("Done.\n");*/
		}
		free(vm->msglist);
		free(vm->parse);
		free(vm);
		return 1;
	} else {
		return 0;
	}
#else
	return 0;
#endif
}

int grab_all_areas(char mode) {
#if defined(READ_COMMANDS)
	int highmsg;
	int i = 0;
	int j = 0;
	int pointer;
	int dorecent;
	int result = 0;
	FILE *FIL;
	FILE *SCANLIST;
	char areaname[51];
	char filename[MAINLINE + 50];
	char olr_msgindex[4000];

	if (U.recent > 0) {
		/*printf("Your default is to read only the %d latest messages in\n",U.recent);*/
		/*printf("any area.  Should we use that default for the grab?\n");*/
		sprintf(filename,Ustring[491],U.recent);
		if (yes_no(filename)) {
			dorecent = 1;
		} else {
			dorecent = 0;
		}
	} else {
		dorecent = 0;
	}

	sprintf(filename,"%s/%s/.scanlist",C.users,U.id);
	if (SCANLIST = fopen(filename,"r")) {
		if (mode != 'q') {
			/*printf("\nScanning for new area messages.\nPress Ctrl-C to stop scanning.\n");*/
			printf(Ustring[490]);
			G.intflag = 0;
			intr_on();
		}

		filename[0] = 0;

		while((!G.intflag) && fgets(filename,90,SCANLIST) && (j < (SCANMAX))) {
			j++;
			shiftword(filename,areaname,51);
			if (!is_area_elig(mode,areaname)) {
				sprintf(filename,"You may not read %s.",areaname);
				do_run_error(filename);
				continue;
			}
			sprintf(filename,"%s/%s/highest",C.areasdir,areaname);
			if (FIL = fopen(filename,"r")) {
				fscanf(FIL," %d ",&highmsg);
				fclose(FIL);
			} else {
				sprintf(filename,"%s is corrupt (highmsg).",areaname);
				do_run_error(filename);
				continue;
			}
			if (highmsg < 1) {
				sprintf(filename,"%s is corrupt (highmsg).",areaname);
				do_run_error(filename);
				continue;
			}
			
			sprintf(filename,"%s/%s/msgindex",C.areasdir,areaname);
			if (FIL = fopen(filename,"r")) {
				fread(olr_msgindex,(unsigned)highmsg + 1,1,FIL);
				olr_msgindex[highmsg + 1] = 0;
				fclose(FIL);
			} else {
				sprintf(filename,"%s is corrupt (msgindex).",areaname);
				do_run_error(filename);
				continue;
			}
			sprintf(filename,"%s/%s/.areas/%s",C.users,U.id,areaname);
			if (mode != 'q') {
				/*printf("\nScanning %s",areaname);*/
				printf("\n");
				printf(Ustring[183],areaname);
			}
			if (FIL = fopen(filename,"r")) {
				fscanf(FIL," %d ",&pointer);
				fclose(FIL);
			} else {
				pointer = 0;
			}
			
			if (dorecent) {
				if ((highmsg - pointer) > U.recent) {
					pointer = (highmsg - U.recent);
				}
			}

			for (i=pointer + 1;i<=highmsg;i++) {
				if (strchr("BMRC",olr_msgindex[i])) {
					grab_area_message((mode == 'q')?'q':'v',areaname,i);
					pointer = i;
					if (G.intflag) {
						break;
					}
				}
			}
			
			
			/* We seemed to have forgotten to write back the pointer */
			sprintf(filename,"%s/%s/.areas/%s",C.users,U.id,areaname);
			if (FIL = fopen(filename,"w")) {
				fprintf(FIL,"%d",pointer);
				fclose(FIL);
			}
			if (!strcmp(areaname,G.areaname)) {
				G.pointer = pointer;
			}
			result ++;
		}

		if (!result && (mode != 'q')) {
			/*printf("No new area messages.\n");*/
			printf(Ustring[184],0);
			printf("\n");
		}
		result = 1;
		fclose(SCANLIST);
		if (mode != 'q') {

			intr_off();
		}
	} else {
		if (mode != 'q') {
			/*printf("You have no area scanlist.\n");*/
			printf("%s\n",Ustring[180]);
		}
	}
	return result;
#else
	return 0;
#endif
}

/* ========================================================================= */

void do_run_error(char *msg) {
	FILE *PAD = write_subject(msg);

	if (PAD) {
		fputs("There has been an error in placing your messages.\n",PAD);
		fputs(".\n",PAD);
		fclose(PAD);
	}
}

void do_message_error(char *msg, char *filename) {
	FILE *PAD = write_subject(msg);
	FILE *IN;
	char line[MAINLINE];

	if (PAD) {
		fputs("The following message could not be placed:\n\n",PAD);
		if (IN = fopen(filename,"r")) {
			while(fgets(line,MAINLINE,IN)) {
				fputs(line,PAD);
			}
			fclose(IN);
		}
		fputs("\n.\n",PAD);
		fclose(PAD);
	}
}

void do_put_error(char *msg, FILE *PUT) {
	FILE *PAD = write_subject(msg);
	char line[MAINLINE];

	if (PAD) {
		fputs("\nThe following message could not be placed:\n\n",PAD);
		while(fgets(line,MAINLINE,PUT) && !is_msgsep(line)) {
			fputs(line,PAD);
		}
		fputs("\n.\n",PAD);
		fclose(PAD);
	}
}

FILE *write_subject(char *msg) {
	char line[MAINLINE];
	char filename[MAINLINE + 50];
	FILE *FIL;
	char *date;

	sprintf(filename,"%s/%s/grabpad",C.privatefiles,U.id);
	if (FIL = fopen(filename,"a")) {
		date = drealmtime(time(0));
		fputs("\nMail:\n",FIL);
		sprintf(line,"Mail # 0 %s from grab to %s - -\n",date,U.id);
		fputs(line,FIL);
		sprintf(line,"Subject: %s\n",msg);
		fputs(line,FIL);
		free(date);
	}
	return FIL;
}
