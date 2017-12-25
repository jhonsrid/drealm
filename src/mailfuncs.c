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
#include <errno.h>
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
#include "display.h"
#include "getvalf.h"

#include "sendmail.h"
#include "mailfuncs.h"
#include "displaymail.h"

#if defined(READ_COMMANDS)
#  include "sendmess.h"
#  include "readfuncs.h"
#endif

struct valid_mail *get_valid_mail (char mode,char *user,char *inprompt,int current,char *inparams) {
	char string[MAINLINE];
	char params[MAINLINE];
	int  i;
	int  listahold[256];
	int  listbhold[2];
	char *prompt;
	int  *lista;
	int  *listb;
	struct valid_mail *vm;

	if (mode != 'q') {
		if (!inparams[0]) {
			if (inprompt[0]) {
				prompt = strdup(inprompt);
			} else {
				/*prompt = strdup("Which message/s? (? for suggestions): ");*/
				sprintf(string,"%s (%s)",Ustring[70],Ustring[200]);   
				prompt = strdup(string);
			}
			/* CONSTCOND */
			while (1) {
				make_prompt(prompt);
				get_one_line(string);
				tnt(string);
				if (!strcmp(string,"?")) {
					maildesclist();
				} else {
					break;
				}
			}
			free(prompt);
			strcpy(params,string);
		} else {
			strcpy(params,inparams);
		}
	} else {
		strcpy(params,inparams);
	}
	if (!params[0]) {
		return NULL;
	}
	lower_string(params);

	vm = (struct valid_mail *)malloc(sizeof (struct valid_mail));

	if (menumatch(params,Ustring[436]) || !strcmp(params,"all")) {
		vm->parse = strdup("all");
		vm->msglist = get_mail("all",user);
	} else if (menumatch(params,Ustring[442]) || !strcmp(params,"new")) {
		vm->parse = strdup("new");
		vm->msglist = get_mail("new",user);
	} else if (menumatch(params,Ustring[440]) || !strcmp(params,"out")) {
		vm->parse = strdup("out");
		vm->msglist = get_mail("out",user);
	} else if (menumatch(params,Ustring[438]) || !strcmp(params,"in")) {
		vm->parse = strdup("in");
		vm->msglist = get_mail("in",user);
	} else if (menumatch(params,Ustring[434]) || !strcmp(params,"next")) {
		vm->parse = strdup("next");
		vm->msglist = findnextmail(mode,user,current);
	} else if (menumatch(params,Ustring[446]) || !strcmp(params,"parent")) {
		vm->parse = strdup("parent");
		vm->msglist = findorigmail(mode,user,current);
	} else if (menumatch(params,Ustring[448]) || !strcmp(params,"place")) {
		vm->parse = strdup("place");
		vm->msglist = findspecmail(mode,user,G.mailplace);
	} else if (menumatch(params,Ustring[444]) || !strcmp(params,"current")) {
		vm->parse = strdup("current");
		vm->msglist = findspecmail(mode,user,current);
	} else {
		vm->parse = strdup("numberlist");
		vm->msglist = (int *)malloc(sizeof (int));
		vm->msglist[0] = 0;

		while (params[0]) {
			shiftword(params,string,21);
			if (is_num(string)) {
				listb = findspecmail(mode,user,atoi(string));
				listbhold[0] = listb[0];
				listbhold[1] = 0;
				free(listb);
				
				lista = combinenums(vm->msglist,listbhold);
				i = 0;
				for (i=0;lista[i] && (i < (MAINLINE - 1));i++) {
					listahold[i] = lista[i];
				}
				listahold[i] = 0;				
				free(lista);
				free(vm->msglist);
				
				for (i=0;listahold[i];i++);
				vm->msglist = (int *)malloc((i+1) * sizeof (int));				
				for (i=0;listahold[i] && (i < 255);i++) {
					vm->msglist[i] = listahold[i];
				}
				vm->msglist[i] = 0;
			} else {
				if (mode != 'v') {
					/*printf("%s is not a valid message descriptor.\n",string);*/
					printf(Ustring[472],string);
					printf("\n");
				}
				break;
			}
		}

	}
	if (!vm->msglist[0]) {
		free(vm->msglist);
		free(vm->parse);
		free(vm);

		return NULL;
	}
	return vm;
}


char *definemail (char mode,char *user,int msgno) {
	char header[MAINLINE];
	FILE *FIL;
	char filename[MAINLINE + 100];

	if (msgno < 1) {
		if (mode != 'q') {
			/*printf("That value is undefined.\n");*/
			printf("%s\n",Ustring[81]);
		}
		return NULL;
	}

	sprintf(filename,"%s/%s/.mail/hdr.%d",C.maildirs,user,msgno);
	if ((FIL = fopen(filename,"r"))) {
		fgets(header,MAINLINE,FIL);
		fclose(FIL);
	} else {
		if (mode != 'q') {
			if (errno == ENOENT) {
				/*printf("You do not have a message %d.\n",msgno);*/
				sprintf(filename,Ustring[193],msgno);
				printf(Ustring[65],filename);
				printf("\n");
			} else {
				/*printf("Header of message %d is unreadable.\n",msgno);*/
				sprintf(filename,Ustring[193],msgno);
				printf(Ustring[66],filename);
				printf("\n");
			}
		}
		return NULL;
	}
	
	return strdup(header);
}

int displaymail (char *user, int msgno) {
#if 0
	char string[MAINLINE + 100];
	sprintf(string,"displaymail %s %s %d 2>/dev/null",C.maildirs,user,msgno);
	external_term();
	dsystem(string);
	internal_term();
#else
	print_mail_message(C.maildirs,user,msgno);
#endif
	return 1;
}

void maildesclist (void) {
	/*
	printf("\n");
	printf("[n] next\n");
	printf("[a] all\n");
	printf("[i] in\n");
	printf("[o] out\n");
	printf("[w] new\n");
	printf("[.] current (again)\n");
	printf("[u] parent\n");

	printf("[~] place\n");
	printf("    or any message number\n");
	printf("\n");
	*/
	printf("\n");
	printf("[%s] %s\n",Ustring[434],Ustring[435]);
	printf("[%s] %s\n",Ustring[436],Ustring[437]);
	printf("[%s] %s\n",Ustring[438],Ustring[439]);
	printf("[%s] %s\n",Ustring[440],Ustring[441]);
	printf("[%s] %s\n",Ustring[442],Ustring[443]);
	printf("[%s] %s\n",Ustring[444],Ustring[445]);
	printf("[%s] %s\n",Ustring[446],Ustring[447]);
	printf("[%s] %s\n",Ustring[448],Ustring[449]);
	printf("    %s\n",Ustring[31]);
	printf("\n");
}

int *findspecmail (char mode, char *user, int msgno) {
	struct stat statbuf;
	char filename[MAINLINE + 50];
	int *msglist = (int *)malloc(2 * sizeof (int));

	msglist[0] = 0;
	msglist[1] = 0;

	if (msgno < 1) {
		if (mode != 'q') {
			/*printf("That value is undefined.\n");*/
			printf("%s\n",Ustring[81]);
		}
		return msglist;
	}

	sprintf(filename,"%s/%s/.mail/hdr.%d",C.maildirs,user,msgno);
	if (!stat(filename,&statbuf)) {
		msglist[0] = msgno;
	} else {
		if (mode != 'q') {
			/*printf("You do not have a message %d.\n",msgno);*/
			sprintf(filename,Ustring[193],msgno);
			printf(Ustring[65],filename);
			printf("\n");
		}
	}
	return msglist;
}

/* ARGSUSED2 */
int *findnextmail (char mode, char *user, int msgno) {
	int i;
	int *msglist;
	int *temp;

	msglist = (int *)malloc(2 * sizeof (int));
	msglist[0] = 0;
	msglist[1] = 0;

	if (G.newmail[0]) {
		msglist[0] = shiftnum(G.newmail);
	} else {
		temp = get_mail("new",user);
		i = 0;
		for(i=0;temp[i] && (i < (MAINLINE -1));i++) {
			G.newmail[i] = temp[i];
		}
		G.newmail[i] = 0;
		free(temp);

		if (G.newmail[0]) {
			msglist[0] = shiftnum(G.newmail);
		} else {
			if (mode != 'q') {
				/*printf("No unread private mail.\n");*/
				printf("%s\n",Ustring[450]);
			}
		}
	}
	return msglist;
}

int *findorigmail (char mode, char *user, int msgno) {
	struct stat statbuf;
	char filename[MAINLINE + 100];
	struct mailheader mh;
	int *msglist = (int *)malloc(2 * sizeof (int));
	char *header;

	msglist[0] = 0;
	msglist[1] = 0;

	if (msgno < 1) {
		if (mode != 'q') {
			/*printf("That value is undefined.\n");*/
			printf("%s\n",Ustring[81]);
		}
		return msglist;
	}

	header = definemail(mode,user,msgno);
	if (! header) {
		if (mode != 'q') {
			/*printf("The message you have just read has dematerialised.\n");*/
			sprintf(filename,Ustring[193],msgno);
			printf(Ustring[65],filename);
			printf("\n");
		}
		return msglist;
	}
	parse_mail_header(header,&mh);

	if (!strcmp(mh.replyto,"base")) {	
		if (mode != 'q') {
			/*printf("This was not a reply.\n");*/
			printf("%s\n",Ustring[83]);
		}
		free(header);
		return msglist;
	}

	sprintf(filename,"%s/%s/.mail/hdr.%s",C.maildirs,user,mh.replyto);
	if (!stat(filename,&statbuf)) {
		msglist[0] = atoi(mh.replyto);
	} else {
		if (mode != 'q') {
			printf("You no longer have a copy of the original.\n");
			printf("%s\n",Ustring[451]);
		}
	}
	free(header);
	return msglist;
}

void markasread (int msgno) {
	int little_list[2];
	FILE *FIL;
	char filename[MAINLINE + 100];
	struct mailheader mh;
	char *header = definemail('q',U.id,msgno);

	if (!header) {
		return;
	}

	little_list[0] = msgno;
	little_list[1] = 0;
	grepnums(G.newmail,little_list);

	parse_mail_header(header,&mh);

	mh.read[0] = 'R';

	sprintf(filename,"%s/%s/.mail/hdr.%d",C.maildirs,U.id,msgno);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",mh.replyto,mh.read,mh.hash,mh.number,mh.dname,mh.mname,mh.dnum,mh.time,mh.dtz,mh.year,mh.from,mh.author,mh.to,mh.recip,mh.dash,mh.subject);
		fclose(FIL);
	}
	sprintf(filename,"%s/%s/.mail/hdr.%d",C.maildirs,mh.author,msgno);
	if ((FIL = fopen(filename,"r"))) {
		fclose(FIL);
		if ((FIL = fopen(filename,"w"))) {
			fprintf(FIL,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",mh.replyto,mh.read,mh.hash,mh.number,mh.dname,mh.mname,mh.dnum,mh.time,mh.dtz,mh.year,mh.from,mh.author,mh.to,mh.recip,mh.dash,mh.subject);
			fclose(FIL);
		}
	}
	free(header);
}

int unread (char *in) {
/* MENU COMMAND */
	int i;
	struct valid_mail *vm;
	char *copy = strdup(in);

	tnt(copy);
	if (copy[0]) {
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,copy);
	} else {
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,G.comline);
		flushcom("");
	}
	
	if (!vm) {
		free(copy);
		return 0;
	}

	if (!strcmp(vm->parse,"new") || !strcmp(vm->parse,"next") || !strcmp(vm->parse,"out")) {
		/*printf("%s invalid.\n",vm->parse);*/
		printf("%s\n",Ustring[457]);
		free(vm->msglist);
		free(vm->parse);
		free(vm);
		free(copy);
		return 0;
	}
	if (!strcmp(vm->parse,"all")) {
		free(vm->msglist);
		free(vm->parse);
		free(vm);
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,"in");
		if (!vm) {
			free(copy);
			return 0;
		}
	}				
	
	for (i=0;vm->msglist[i];i++) {
		markunread(vm->msglist[i]);
	}

	free(vm->msglist);
	free(vm->parse);
	free(vm);
	free(copy);
	/*printf("Marked as unread\n");*/
	printf("%s\n",Ustring[452]);
	return 1;
}


int markunread (int msgno) {
	FILE *FIL;
	char filename[MAINLINE + 100];
	struct mailheader mh;

	char *header = definemail('q',U.id,msgno);

	if (!header) {
		return 0;
	}
	parse_mail_header(header,&mh);
	
	if (strcmp(mh.author,U.id)) {
	/* Don't do it to outbasket copies */

		mh.read[0] = 'U';

		sprintf(filename,"%s/%s/.mail/hdr.%d",C.maildirs,U.id,msgno);
		if ((FIL = fopen(filename,"w"))) {
			fprintf(FIL,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",mh.replyto,mh.read,mh.hash,mh.number,mh.dname,mh.mname,mh.dnum,mh.time,mh.dtz,mh.year,mh.from,mh.author,mh.to,mh.recip,mh.dash,mh.subject);
			fclose(FIL);
			return 1;
		} 
	}
	return 0;
}




int restore_mail (char *dummy) {
/* MENU COMMAND */
	int i;
	int mailstored = 0;
	char filename[MAINLINE + 100];
	FILE *FIL;
	struct valid_mail *vm;

	sprintf(filename,"%s/%s/.mail/mailstore",C.maildirs,U.id);
	if ((FIL = fopen(filename,"r"))) {
		fscanf(FIL," %d ",&mailstored);
		fclose(FIL);
	}
	if (!mailstored) {
		/*printf("Private mail pointers have not been stored.\n");*/
		printf("%s\n",Ustring[101]);
		return 0;
	}

	vm = get_valid_mail('q',U.id,"",G.mailcurrent,"in");
	if (!vm) {
		/*printf("No private mail needs restoring.\n");*/
		printf("%s\n",Ustring[453]);
		return 0;
	}

	i = 0;
	while (vm->msglist[i]) {
		if (vm->msglist[i] >= mailstored) {
			markunread(vm->msglist[i]);
		}
		i++;
	}

	free(vm->msglist);
	free(vm->parse);
	free(vm);
	/*printf("Private mail pointers restored.\n");*/
	printf("%s\n",Ustring[103]);
	return 1;
}

int store_mail (char *dummy) {
/* MENU COMMAND */
	int mailstored = 0;
	int i;
	char filename[MAINLINE + 100];
	FILE *FIL;
	struct valid_mail *vm;

	vm = get_valid_mail('q',U.id,"",G.mailcurrent,"new");
	if (vm) {
		mailstored = vm->msglist[0];
		free(vm->msglist);
		free(vm->parse);
		free(vm);
	} else {
		vm = get_valid_mail('q',U.id,"",G.mailcurrent,"all");
		if (vm) {
			i = 0;
			while (vm->msglist[i]) {
				i++;
			}		
			mailstored = (vm->msglist[i] + 1);
		} else {
			mailstored = 0;
		}
	}

	sprintf(filename,"%s/%s/.mail/mailstore",C.maildirs,U.id);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%d\n",mailstored);
		fclose(FIL);
		/*printf("Mail pointer stored as %d.\n",mailstored);*/
		printf("%s\n",Ustring[189]);
	} 
	return 1;
}



void parse_mail_header (char *header,struct mailheader *mh) {
	char *p, *q;

	p = header;

	for(q = p; !isspace(*p); p++)
		; 
	mh->replyto = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->read = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->hash = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->number = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->dname = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->mname = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->dnum = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->time = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->dtz = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->year = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->from = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->author = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->to = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->recip = q;
	for(;isspace(*p); *p++='\0');
	for(q = p; !isspace(*p); p++)
		; 
	mh->dash = q;
	for(;isspace(*p); *p++='\0');

	mh->subject = p;
	
	/* trial bit to get rid of newline on subject */
	tnt(mh->subject);
}

int *get_mail(const char *opt, const char *recip) {
	int i;
	int onenum[2];
	int result[MAINLINE];
	struct mailheader mh;
	char dirname[MAINLINE + 100];
	char string[21];
	struct valid_files *vf;
	char *header;
	int *temp;

	onenum[0] = 0;
	onenum[1] = 0;
	result[0] = 0;
	
		
	sprintf(dirname,"%s/%s/.mail",C.maildirs,U.id);
	vf = get_valid_files('q',0,"mail",dirname,"hdr.*",0);

	while (vf->files[0]) {
		shiftword(vf->files,string,21);

		tnt(string);
		if (!string[0]) {
			continue;
		}

		sscanf(string,"hdr.%d",&onenum[0]);
		if (!(header = definemail('q',U.id,onenum[0]))) {
			continue;
		}
		parse_mail_header(header,&mh);
		if (!strcmp(opt,"all") ||
		   (!strcmp(opt,"in") && !strcmp(mh.recip,recip)) ||
		   (!strcmp(opt,"in") && strcmp(mh.author,recip)) ||
		   (!strcmp(opt,"out") && !strcmp(mh.author,recip)) ||
		   (!strcmp(opt,"new") && !strcmp(mh.recip,recip) && strcmp(mh.read,"R"))) {

			temp = combinenums(result,onenum);
			i = 0;
			for (i=0;temp[i] && (i < (MAINLINE - 1));i++) {
				result[i] = temp[i];
			}
			result[i] = 0;			
			free(temp);
		}
		free(header);
	}
	free(vf->input);
	free(vf->files);
	free(vf);

	for (i =0; result[i]; i++);

	temp = (int *)malloc((i + 1) * sizeof (int));
	i = 0;
	for (i =0; result[i]; i++) {
		temp[i] = result[i];
	}
	temp[i] = 0;
	return temp;
}

int mailread (char *in) {
/* MENU COMMAND */
	int msgno[2];
	struct mailheader mh;
	struct stat statbuf;
	char filename[MAINLINE + 100];
	FILE *FIL;
	char *copy = strdup(in);
	char *header;
	struct valid_mail *vm;

	tnt(copy);
	if (copy[0]) {
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,copy);
	} else {
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,G.comline);
		flushcom("");
	}

	if (!vm) {
		free(copy);
		return 0;
	}

	if (vm->msglist[1]) {
		/*printf("'%s' invalid.\n",vm->parse);*/
		printf(Ustring[472],vm->parse);
		printf("\n");
		free(vm->msglist);
		free(vm->parse);
		free(vm);
		free(copy);
		return 0;
	}

	msgno[0] = vm->msglist[0];
	msgno[1] = 0;
	free(vm->msglist);
	free(vm->parse);
	free(vm);
	free(copy);
	
	header = definemail('v',U.id,msgno[0]);
	if (!header) {
		return 0;
	}

	parse_mail_header(header,&mh);

	if ((C.mailmonitor == 1) && !strcmp(mh.hash,"T")) {
		if (U.mailreserves <= 0) {
			/*printf("You do not have any external mail allowance left.\n");*/
			sprintf(filename,Ustring[160],0);
			printf(Ustring[454],filename,Ustring[159]);
			printf("\n");
			return 0;
		} else {

			sprintf(filename,"%s/%s/.mail/msg.%d",C.maildirs,U.id,msgno[0]);
			if (!stat(filename,&statbuf)) {
				/*printf("This mail from %s is %ld bytes in size.\n",mh.author,statbuf.st_size);*/
				printf(Ustring[455],mh.author,statbuf.st_size);
				printf("\n");
				/*if (!yes_no("Accept to read?")) */
				if (!yes_no(Ustring[456])) {
					return 0;
				}
			} else {
				/*printf("Message %d unreadable.\n",msgno[0]);*/
				sprintf(filename,Ustring[193],msgno[0]);
				printf(Ustring[66],filename);
				printf("\n");
				return 0;
			}
			U.mailreserves -= statbuf.st_size;
			mailreserves_write(U.id,U.mailreserves);
			strcpy(mh.hash,"#");
			sprintf(filename,"%s/%s/.mail/hdr.%d",C.maildirs,U.id,msgno[0]);
			if ((FIL = fopen(filename,"w"))) {
				fprintf(FIL,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",mh.replyto,mh.read,mh.hash,mh.number,mh.dname,mh.mname,mh.dnum,mh.time,mh.dtz,mh.year,mh.from,mh.author,mh.to,mh.recip,mh.dash,mh.subject);
				fclose(FIL);
			}
		}
	}
	if (strcmp(mh.read,"R") && !strcmp(mh.recip,U.id)) {
		grepnums(G.newmail,msgno);
		G.mailplace = msgno[0];
		markasread(msgno[0]);
	}
	free(header);
	G.mailcurrent = msgno[0];
	if (!displaymail(U.id,msgno[0])) {
		return 0;
	}
	return 1;
}

int mailgrab (char *in) {
/* MENU COMMAND */
	struct valid_mail *vm;
	char *copy = strdup(in);

	tnt(copy);
	if (copy[0]) {
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,copy);
	} else {
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,G.comline);
		flushcom("");
	}

	if (!vm) {
		free(copy);
		return 0;
	}
	grabmail('v',U.id,vm->msglist);
	free(vm->msglist);
	free(vm->parse);
	free(vm);
	free(copy);
	return 1;
}

void grabmail (char mode,char *user,int *msglist) {
	char filename[MAINLINE + 100];
	FILE *BODY;
	FILE *GRAB;
	FILE *FIL;
	char buffer[1024];
	struct stat statbuf;
	struct mailheader mh;
	int msgno[2];
	int i;
	int j;
	char *header;

	msgno[0] = 0;
	msgno[1] = 0;

	sprintf(filename,"%s/%s/grabpad",C.privatefiles,U.id);
	if (!(GRAB = fopen(filename,"a"))) {
		if (mode != 'q') {
			/*printf("Cannot write to grabpad.\n");*/
			printf(Ustring[67],"grabpad");
			printf("\n");
		}
		return;
	}

	for (i = 0;msglist[i];i++) {
		msgno[0] = msglist[i];

		header = definemail('v',U.id,msgno[0]);
		if (!header) {
			continue;
		}

		parse_mail_header(header,&mh);

		if ((C.mailmonitor == 1) && !strcmp(mh.hash,"T")) {
			if (U.mailreserves <= 0) {
				if (mode != 'q') {
					/*printf("You do not have any external mail allowance left.\n");*/
					sprintf(filename,Ustring[160],0);
					printf(Ustring[454],filename,Ustring[159]);
					printf("\n");
				}
				free(header);
				continue;
			} else {

				sprintf(filename,"%s/%s/.mail/msg.%d",C.maildirs,U.id,msgno[0]);
				if (stat(filename,&statbuf)) {
					if (mode != 'q') {
						/*printf("Message %d unreadable.\n",msgno[0]);*/
						sprintf(filename,Ustring[193],msgno[0]);
						printf(Ustring[66],filename);
						printf("\n");
					}
					free(header);
					continue;
				}
				U.mailreserves -= statbuf.st_size;
				mailreserves_write(U.id,U.mailreserves);
				strcpy(mh.hash,"#");
				sprintf(filename,"%s/%s/.mail/hdr.%d",C.maildirs,U.id,msgno[0]);
				if ((FIL = fopen(filename,"w"))) {
					fprintf(FIL,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",mh.replyto,mh.read,mh.hash,mh.number,mh.dname,mh.mname,mh.dnum,mh.time,mh.dtz,mh.year,mh.from,mh.author,mh.to,mh.recip,mh.dash,mh.subject);
					fclose(FIL);
				}
			}
		}

		if (strcmp(mh.read,"R") && !strcmp(mh.recip,U.id)) {
			markasread(msgno[0]);
			grepnums(G.newmail,msgno);
		}

		fputs("\nMail:\n", GRAB);

		if (!strcmp(mh.replyto,"base")) {

fprintf(GRAB,"%s %s %s %s %s %s by %s - - to %s\n",
mh.number, mh.dname, mh.mname, mh.dnum, mh.time, mh.year,
mh.author, mh.recip);

		} else {

fprintf(GRAB,"%s %s %s %s %s %s by %s reply-to %s by %s\n",
mh.number, mh.dname, mh.mname, mh.dnum, mh.time, mh.year,
mh.author, mh.replyto, mh.recip);

		}
		fprintf(GRAB,"Subject: %s\n\n",mh.subject);

		free(header);

		sprintf(filename,"%s/%s/.mail/msg.%d",C.maildirs,user,msgno[0]);
		if ((BODY=fopen(filename,"r"))) {
			while (fgets(buffer,1022,BODY)) {
				if (buffer[0] == '.') {
				/* Only while putting strips ALL leading dots */	

					char newbuffer[1024];
					j = 0;
					newbuffer[j] = '.';
					while(buffer[j]) {	
						newbuffer[j+1] = buffer[j];
						j++;
					}
					newbuffer[j+1] = 0;
					strcpy(buffer,newbuffer);
				}
				fputs(buffer,GRAB);
			}
			fclose(BODY);
		} else {
			fputs("\nMessage body unreadable.\n",GRAB);
		}
		fputs("\n.\n",GRAB);
		if (mode != 'q') {
			printf(".");
		}
	}
	printf("\n");
	fclose(GRAB);
}

int mailquote (char *in) {
/* MENU COMMAND */
	char temp[21];
	char *copy = strdup(in);
	struct valid_mail *vm;

	tnt(copy);
	if (copy[0]) {
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,copy);
	} else {
		shiftword(G.comline,temp,21);
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,temp);
	}


	if (!vm) {
		free(copy);
		return 0;
	}
	workmail('v',U.id,vm->msglist);
	free(vm->msglist);
	free(vm->parse);
	free(vm);
	free(copy);
	return 1;
}

void workmail (char mode,char *user,int *msglist) {
	char filename[MAINLINE + 100];
	FILE *BODY;
	FILE *GRAB;
	FILE *FIL;
	char buffer[1024];
	struct stat statbuf;
	struct mailheader mh;
	int msgno[2];
	int i;
	char *header;

	msgno[0] = 0;
	msgno[1] = 0;

	sprintf(filename,"workpad");
	if (!(GRAB = fopen(filename,"a"))) {
		if (mode != 'q') {
			/*printf("Cannot write to workpad.\n");*/
			printf(Ustring[67],"workpad");
			printf("\n");
		}
		return;
	}

	for (i = 0;msglist[i];i++) {
		msgno[0] = msglist[i];

		header = definemail('v',U.id,msgno[0]);
		if (!header) {
			continue;
		}

		parse_mail_header(header,&mh);

		if ((C.mailmonitor == 1) && !strcmp(mh.hash,"T")) {
			if (U.mailreserves <= 0) {
				if (mode != 'q') {
					/*printf("You do not have any external mail allowance left.\n");*/
					sprintf(filename,Ustring[160],0);
					printf(Ustring[454],filename,Ustring[159]);
					printf("\n");
				}
				free(header);
				continue;
			} else {
				sprintf(filename,"%s/%s/.mail/msg.%d",C.maildirs,U.id,msgno[0]);
				if (stat(filename,&statbuf)) {
					if (mode != 'q') {
						/*printf("Message %d unreadable.\n",msgno[0]);*/
						sprintf(filename,Ustring[193],msgno[0]);
						printf(Ustring[66],filename);
						printf("\n");
					}
					free(header);
					continue;
				}
				U.mailreserves -= statbuf.st_size;
				mailreserves_write(U.id,U.mailreserves);
				strcpy(mh.hash,"#");
				sprintf(filename,"%s/%s/.mail/hdr.%d",C.maildirs,U.id,msgno[0]);
				if ((FIL = fopen(filename,"w"))) {
					fprintf(FIL,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",mh.replyto,mh.read,mh.hash,mh.number,mh.dname,mh.mname,mh.dnum,mh.time,mh.dtz,mh.year,mh.from,mh.author,mh.to,mh.recip,mh.dash,mh.subject);
					fclose(FIL);
				}
			}
		}

		fprintf(GRAB,"%s wrote:\n",mh.author);
		free(header);

		sprintf(filename,"%s/%s/.mail/msg.%d",C.maildirs,user,msgno[0]);
		if ((BODY=fopen(filename,"r"))) {
			while (fgets(buffer,1022,BODY)) {
				fprintf(GRAB,">%s",buffer);
			}
			fclose(BODY);
		} else {
			/*printf("Message unreadable.\n");*/
			sprintf(filename,Ustring[193],msgno[0]);
			printf(Ustring[66],filename);
			printf("\n");
		}
	}
	fclose(GRAB);
}

int maillist (char *in) {
/* MENU COMMAND */
	int pagerows = 0;
	int i;
	struct mailheader mh;
	struct valid_mail *vm;
	char *copy = strdup(in);
	char *header;

	tnt(copy);
	if (copy[0]) {
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,copy);
	} else {
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,G.comline);
		flushcom("");
	}

	if (!vm) {
		free(copy);
		return 0;
	}

	printf("\n");

	for (i=0;vm->msglist[i];i++) {
		header = definemail('v',U.id,vm->msglist[i]);

		if (!header) {
			continue;
		}
		parse_mail_header(header,&mh);

		if (!strcmp(vm->parse,"new") || !strcmp(vm->parse,"in")) {

printf("%s %s %s %s %s %s %s %s %s %s %s - %s\n",
mh.read,mh.hash,mh.number,mh.dname,mh.dnum,mh.mname,mh.year,mh.time,mh.dtz,
mh.from,mh.author,mh.subject);

		} else if (!strcmp(vm->parse,"out")) {

printf("%s %s %s %s %s %s %s %s %s %s %s - %s\n",
mh.read,mh.hash,mh.number,mh.dname,mh.dnum,mh.mname,mh.year,mh.time,mh.dtz,
mh.to,mh.recip,mh.subject);

		} else {

printf("%s %s %s %s %s %s %s %s %s %s %s %s %s - %s\n",
mh.read,mh.hash,mh.number,mh.dname,mh.dnum,mh.mname,mh.year,mh.time,mh.dtz,
mh.from,mh.author,mh.to,mh.recip,mh.subject);

		}
		pagerows++;
		if (pagerows > (LINES - 5)) {
			if (do_continue("")) {
				pagerows = 0;
				continue;
			} else {
				break;
			}
		}
		free(header);
	}
	free(vm->msglist);
	free(vm->parse);
	free(vm);
	free(copy);
	return 1;
}

int maildelete (char *in) {
/* MENU COMMAND */
	int confirm = 0;
	char response[2];
	struct valid_mail *vm;
	char temp[MAINLINE];
	char *copy = strdup(in);

	tnt(copy);
	if (copy[0]) {
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,copy);
	} else {
		vm = get_valid_mail('v',U.id,"",G.mailcurrent,G.comline);
		flushcom("");
	}
	
	if (!vm) {
		free(copy);
		return 0;
	}

	if (!strcmp(vm->parse,"new") || !strcmp(vm->parse,"next")) {
		/*printf("%s invalid.\n",vm->parse);*/
		printf("%s\n",Ustring[457]);
		free(vm->msglist);
		free(vm->parse);
		free(vm);
		free(copy);
		return 0;
	}

	if (vm->msglist[1]) {
		/*make_prompt("Confirm for each one? Y/n/q ");*/
		sprintf(temp,"%s %c/%c/%c",Ustring[458],G.bigyes,G.littleno,G.littlequit);
		make_prompt(temp);
		get_one_lc_char(response);
		if ((!response[0] || response[0] == G.littleyes)) {
			confirm = 1;
		} else if (response[0] == G.littleno) {
			confirm = 0;
		} else if (response[0] == G.littlequit) {
			free(vm->msglist);
			free(vm->parse);
			free(vm);
			free(copy);
			return 0;
		} else {
			confirm = 1;
		}
	}
	delete_maillist('v',confirm,vm);
	free(vm->msglist);
	free(vm->parse);
	free(vm);
	free(copy);
	return 1;
}

int delete_maillist (char mode, int confirm, struct valid_mail *vm) {
	int i;
	struct mailheader mh;
	char filename[MAINLINE + 100];
	char temp[MAINLINE + 100];
	char response[2];
	char *header;

	if (mode != 'q') {
		/*printf("Deleting... ");*/
		printf("%s ",Ustring[112]);
		fflush(stdout);
	}
	for (i=0;vm->msglist[i];i++) {
		header = definemail('q',U.id,vm->msglist[i]);
		if (!header) {
			continue;
		}
		parse_mail_header(header,&mh);

		if (!strcmp(mh.read,"U") && strcmp(vm->parse,"numberlist")) {
			free(header);
			continue;
		}

		if (confirm) {
			printf("\n");

			printf("%s %s %s %s %s %s %s %s %s %s %s %s - %s\n",
			mh.read,mh.hash,mh.number,mh.dname,mh.dnum,mh.time,
			mh.dtz,mh.year,mh.from,mh.author,mh.to,mh.recip,
			mh.subject);

			/*sprintf(temp,"Delete %d? y/N/q ",vm->msglist[i]);*/
			sprintf(temp,"%d",vm->msglist[i]);
			sprintf(filename,Ustring[108],temp);
			sprintf(temp,"%s %c/%c/%c",filename,G.littleyes,G.bigno,G.littlequit);
			make_prompt(temp);
			get_one_lc_char(response);
			if (response[0] == G.littlequit) {
				break;
			} else if (response[0] != G.littleyes) {
				continue;
			}

		}

		if (!strcmp(mh.read,"U")) {
			if (mode != 'q') {
				if (strcmp(mh.author,U.id)) {
					/*sprintf(temp,"You haven't read msg %d. Really delete?",vm->msglist[i]);*/
					sprintf(temp,Ustring[460],vm->msglist[i]);
					if (!no_yes(temp)) {
						continue;
					}									
				} else {
					/*sprintf(temp,"Remove %s's copy too?",mh.recip);*/
					sprintf(temp,Ustring[461],mh.recip);
					if (no_yes(temp)) {
						sprintf(filename,"%s/%s/.mail/hdr.%d",C.maildirs,mh.recip,vm->msglist[i]);
						remove(filename);
						sprintf(filename,"%s/%s/.mail/msg.%d",C.maildirs,mh.recip,vm->msglist[i]);
						remove(filename);
					}
				}
			}
		}
		sprintf(filename,"%s/%s/.mail/hdr.%d",C.maildirs,U.id,vm->msglist[i]);
		remove(filename);
		sprintf(filename,"%s/%s/.mail/msg.%d",C.maildirs,U.id,vm->msglist[i]);
		remove(filename);

		if (!confirm && (mode != 'q')) {
			printf("%d ",vm->msglist[i]);
		}
		free(header);
	}
	printf("\n");
	return 1;
}


int mailforward (char *in) {
/* MENU COMMAND */
	struct valid_mail *vm;
	struct mailheader mh;
	int msgno;
	int result;
        char from[MAINLINE];
	char to[51];
	char temp[MAINLINE+100];
 	char *header;
	char *final;
	char *copy = strdup(in);
                                                                            

	/* which message */
	tnt(copy);
	strshift(copy,from,MAINLINE,Ustring[501]);
	tnt(from);
	if (!from[0]) {
		strshift(G.comline,from,MAINLINE,Ustring[501]);
	}

	shiftword(copy,to,51);
	if (!to[0]) {
		shiftword(G.comline,to,51);
	}
	free(copy);
	
	tnt(from);
	if (!from[0]) {
		if ((to[0]) && (G.mailcurrent)) {
			sprintf(from,"%d",G.mailcurrent);
		}
	}

	vm = get_valid_mail('v',U.id,"",G.mailcurrent,from);
	if (!vm) {
		return 0;
	}
	msgno = vm->msglist[0];

	/* which user */
	tnt(to);
	if (!to[0] || !Dstrcmp(to,Ustring[501])) {
		/*make_prompt("Mail to whom? ");*/
		make_prompt(Ustring[462]);
		get_one_line(to);
	}
	final = mail_check_recips('v',to);

	if (!final[0]) {
		free(final);
		return 0;
	}
	
	header = definemail('q',U.id,msgno);
	parse_mail_header(header,&mh);

	sprintf(temp,"echo -e 'Forwarded by %s\n------------\n' > message",U.id);
	dsystem(temp);
	sprintf(temp,"cat %s/%s/.mail/msg.%d >> message",C.maildirs,U.id,msgno);
	dsystem(temp);
	
	while (final[0]) {
		shiftword(final,to,MAINLINE);
		fflush(stdout);
		if (result) {
			if (strchr(to,'@')) {
				/*
				U.mailreserves -= statbuf.st_size;
				mailreserves_write(U.id,U.mailreserves);
				*/
				printf("%s - %s\n",to,Ustring[502]);
			} else {
				result = sendmail('v',to,mh.author,"message","base",mh.subject,'f');
				printf("%s %d\n",Ustring[94],result);/*sending mail message*/
			}
		}
	}

	remove("message");
	free(header);
	free(final);
	return 1;
}


int mailpost (char *in) {
/* MENU COMMAND */
	char subject[MAINLINE];
	char temp[MAINLINE];
	char *final;
	char *copy = strdup(in);

	tnt(copy);
	if (copy[0]) {
		strcpy(temp,copy);
	} else {
		strcpy(temp,G.comline);
		flushcom("");
	}
	free(copy);

	tnt(temp);
	if (!temp[0] || !Dstrcmp(temp,Ustring[501])) {
		/*make_prompt("Mail to whom? ");*/
		make_prompt(Ustring[462]);
		get_one_line(temp);
	}
	final = mail_check_recips('v',temp);

	if (!final[0]) {
		free(final);
		return 0;
	}

	/*make_prompt("Subject: ");*/
	make_prompt(Ustring[92]);
	get_one_line(subject);
	if (!subject[0]) {
		strcpy(subject,"No subject");
	}

	if (!edit_special("message")) {
		return 0;
	}
	if (!send_mail_off("base","message",U.id,subject,final)) {
		free(final);
		return 0;
	}
	free(final);
	return 1;
}

/* ARGSUSED0 */
int mailreply (char *in) {
/* MENU COMMAND */
	int msgno;
	char temp[21];
	char subject[MAINLINE];
	struct mailheader mh;

	struct valid_mail *vm;
	char *copy = strdup(in);
	char *author;
	char *header;
	
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

	if ((!temp[0]) && (G.mailcurrent > 0)) {
		sprintf(temp,"%d",G.mailcurrent);
	}

	/*vm = get_valid_mail('v',U.id,"Reply to which message? ",G.mailcurrent,temp);*/
	vm = get_valid_mail('v',U.id,Ustring[89],G.mailcurrent,temp);
	if (!vm) {
		return 0;
	}
	msgno = vm->msglist[0];
	free(vm->msglist);
	free(vm->parse);
	free(vm);

	header = definemail('v',U.id,msgno);
	if (!header) {
		return 0;
	}
	parse_mail_header(header,&mh);

	author = mail_check_recips('v',mh.author);
	if (!author[0]) {
		free(author);
		free(header);
		return 0;
	}
	free(author);
	
	strcpy(subject,mh.subject);
	tnt(subject);
	/*printf("\nSubject: %s\n",subject);*/
	printf("\n%s %s\n",Ustring[92],subject);
	/*make_prompt("Press [Enter] to accept or type in new subject: ");*/
	make_prompt(Ustring[93]);
	get_one_line(temp);
	if (temp[0]) {
		strcpy(subject,temp);
	}

	if (!edit_special("message")) {
		free(header);
		return 0;
	}
	if (!send_mail_off(mh.number,"message",U.id,subject,mh.author)) {
		free(header);
		return 0;
	}
	free(header);
	return 1;
}

int send_mail_off(char *ffield,char *messagefile,char *sender, char *subject, char *mailwho) {
	char recipient[MAINLINE];
	int result;
	char *whocopy = strdup(mailwho);
	struct stat statbuf;

	while (whocopy[0]) {
		shiftword(whocopy,recipient,MAINLINE);
		/*printf("Sending mail message...");*/
		printf("%s", Ustring[94]);
		fflush(stdout);
		result = sendmail('v',recipient,sender,messagefile,ffield,subject,'f');
		if (result) {
			if (strchr(recipient,'@') && C.mailmonitor && !stat("message",&statbuf)) {
				U.mailreserves -= statbuf.st_size;
				mailreserves_write(U.id,U.mailreserves);
			}
			printf(" %d\n",result);
		}
	}
	remove(messagefile);
	free(whocopy);
	return 1;
}

char *mail_check_recips (char mode, char *params) {

	char temp[MAINLINE];
	char tempa[1024];
	struct stat statbuf;
	char filename[MAINLINE + 100];
	char flags[UFLAGMAX + 2];
	char *templist;
	char *paramcopy = strdup(params);

	tempa[0] = 0;

	while (paramcopy[0]) {
		shiftword(paramcopy,temp,MAINLINE);

		if (!strchr(temp,'@')) {
			lower_string(temp);
		}
		if (!Dstrcmp(temp,"sysop")) {
			strcpy(temp,C.sysopname);
		}

		sprintf(filename,"%s/%s",C.maildirs,temp);

		if (strchr(temp,'@')) {
			if (U.level < C.extmaillevel) {
				/*strcpy(G.errmsg,"You do not have external mail facilities");*/
				if (mode == 'v') {
					printf(Ustring[316],Ustring[463]);
					printf("\n");
				}
			} else if (!comp_flags(C.extmailmask,U.flags)) {
				/*strcpy(G.errmsg,"You do not have external mail facilities");*/
				if (mode == 'v') {
					printf(Ustring[316],Ustring[463]);
					printf("\n");
				}
			} else if ((C.mailmonitor == 1) && (U.mailreserves <= 0)) {
				/*strcpy(G.errmsg,"You have no external mail allowance left");*/
				if (mode == 'v') {
					sprintf(filename,Ustring[160],0);
					printf(Ustring[454],filename,Ustring[159]);
					printf("\n");
				}
			} else {
				templist = concat(tempa,temp);
				strncpy(tempa,templist,1024);
				tempa[1023] = 0;
				free(templist);
			}
		} else if (Dstrcmp(temp,C.sysopname) && (U.level < C.pvtmaillevel)) {
			/*strcpy(G.errmsg,"You may only post mail to the SysOp");*/
			if (mode == 'v') {
				printf("%s\n",Ustring[466]);
			}
		} else if (stat(filename,&statbuf)) {
			/*sprintf(G.errmsg,"%s has no mailbox.",temp);*/
			if (mode == 'v') {
				printf(Ustring[250],temp,Ustring[465]);
				printf("\n");
			}
		} else if ((level_read(temp) < C.pvtmaillevel) && (U.level < C.sysoplevel)) {
			/*sprintf(G.errmsg,"%s has no mailbox.",temp);*/
			if (mode == 'v') {
				printf(Ustring[250],temp,Ustring[465]);
				printf("\n");
			}
		} else if ((level_read(temp) < C.pvtmaillevel) && (U.level < C.sysoplevel)) {
			/*sprintf(G.errmsg,"%s has no mailbox.",temp);*/
			if (mode == 'v') {
				printf(Ustring[250],temp,Ustring[465]);
				printf("\n");
			}
		} else if (flags_read(temp,flags) && !comp_flags(C.pvtmailmask,flags) && (U.level < C.sysoplevel)) {
			/*sprintf(G.errmsg,"%s has no mailbox.",temp);*/
			if (mode == 'v') {
				printf(Ustring[250],temp,Ustring[465]);
				printf("\n");
			}
		} else {
			templist = concat(tempa,temp);
			strncpy(tempa,templist,1024);
			tempa[1023] = 0;
			free(templist);
		}
	}
	free(paramcopy);
	templist = strdup(tempa);
	return templist;
}

int get_unix_mail (char mode,char *user) {
	char string[MAINLINE];
	char from_value[MAINLINE];
	char subject[MAINLINE];
	FILE *MBOX; 
	FILE *MSG;
	int before_body;
	char *field; 
	char *value;

	enum {
		none = 0,
		envelope,
		return_path,
		sender,
		mail_from,
		from,
		reply_to
	} from_field = none;

	/*
	 * We want to take a copy of the unix mailbox so we can process it
	 * and let new messages arrive without confusion...
	 * The file *should* be locked during this, but as I've no idea
	 * how unix locks mailboxes, I shalln't.
	 */
	sprintf(string,"cp %s/%s mailtemp 2>&1",C.unixmaildir,user);
	if (dsystem(string)) {
		if (mode != 'q') {
			/*printf("Could not import your mail.\n");*/
			printf("%s\n",Ustring[467]);
		}
		return 0;
	}
	sprintf(string,"cp /dev/null %s/%s",C.unixmaildir,user);
	dsystem(string); /* Hum ho, just carry on regardless */

	/*
	 * Now we must split the mailbox into individual messages and
	 * process each one.
	 */
	if (!(MBOX = fopen("mailtemp","r"))) {
#if defined(DEVEL)
		perror("mailtemp");
#endif
		return 0;
	}
	string[0]=0;
	fgets(string,MAINLINE,MBOX);
	while(!feof(MBOX)) {
		if (!strncmp(string,"From ",5)) {
			break;
		}
		fgets(string,MAINLINE,MBOX);
	}
	while(!feof(MBOX)) {
		if (!(MSG = fopen("tempmail","w"))) {
#if defined(DEVEL)
			perror("tempmail");
#endif
			fclose(MBOX);
			return 0;
		}
		subject[0] = 0;
		before_body = 1;
		from_field = envelope;
		sscanf(string,"From %256s ",from_value);
		fputs(string,MSG);

		fgets(string,MAINLINE,MBOX);
		while (!feof(MBOX) && strncmp(string,"From ",5)) {
			fputs(string,MSG);
			if (before_body) {
				if (!string[1]) {
					before_body = 0;
				} else {
					if ((value = strchr(string,':'))) {
						*value = 0;
						while(isspace(*++value)) *value=0;
						field = string;
						lower_string(field);
						if (!strcmp(field,"return-path") && (from_field <= envelope)) {
							from_field = return_path;
							strcpy(from_value,value);
						} else if (!strcmp(field,"sender") && (from_field <= return_path)) {
							from_field = sender;
							strcpy(from_value,value);
						} else if (!strcmp(field,"mail-from") && (from_field <= sender)) {
							from_field = mail_from;
							strcpy(from_value,value);
						} else if (!strcmp(field,"from") && (from_field <= mail_from)) {
							from_field = from;
							strcpy(from_value,value);
						} else if (!strcmp(field,"reply-to") && (from_field <= from)) {
							from_field = reply_to;
							strcpy(from_value,value);
						} else if (!strcmp(field,"subject")) {
							strncpy(subject,value,MAINLINE - 1);
							subject[MAINLINE - 1] = 0;
						}
					}
				}
			}
			fgets(string,MAINLINE,MBOX);
		}
		fclose(MSG);
		get_unix_mail_id(from_value);
		tnt(subject);
		sendmail(mode,U.id,from_value,"tempmail","base",subject,'f');
	}
	remove("tempmail");
	fclose(MBOX);
	remove("mailtemp");
	return 1;
}

void get_unix_mail_id(char *string) {
	char out[MAINLINE];

	if (strchr(string,'<')) {
		strshift(string,out,2,"<");
		strshift(string,out,MAINLINE,">");
	} else {
		shiftword(string,out,MAINLINE);
	}
	strcpy(string,out);
}



/* ARGSUSED0 */
int any_unix_mail (char *dummy) {
/* MENU COMMAND */
/* For user to check their own mail voluntarily */
	char command[MAINLINE + MAINLINE + 100];
	struct stat statbuf;

	if (U.level >= C.extmaillevel) {
		sprintf(command,"%s/%s",C.unixmaildir,U.id);
		if (!stat(command,&statbuf) && (statbuf.st_size)) {
			/*make_prompt("Would you like your Unix mail imported? Y/n ");*/
			if (yes_no(Ustring[468])) {
				/*fputs("Processing mail, please wait...",stdout);*/
				printf("%s", Ustring[469]);
				get_unix_mail('v',U.id);
				printf("\n");
				return 1;
			}
		}
	}
	return 0;
}

int unix_mailcheck (char *dummy) {
/* MENU COMMAND */
/* For automatic checking */
	static time_t mailtouch = 0;
	char filename[MAINLINE + 100];
	struct stat statbuf;
	int result = 0;

	if (U.level >= C.extmaillevel) {
		sprintf(filename,"%s/%s",C.unixmaildir,U.id);
		if ((!stat(filename,&statbuf)) && (statbuf.st_size)) {
			if (statbuf.st_mtime > mailtouch) {
				/*printf("\n\a*** External mail arrived ***\n");*/
				printf("\n\a%s\n",Ustring[470]);
				if (any_unix_mail("")) {
					G.mupdate = 1;
					result = 1;
				}
			}
			mailtouch = statbuf.st_mtime;
		}
	}
	return result;	
}


/* ARGSUSED0 */
int mailcheck (char *dummy) {
/* MENU COMMAND */
/* For automatic checking */
	char filename[MAINLINE + 100];
	struct stat statbuf;

	if (G.mupdate) {
		G.mupdate = 0;
		sprintf(filename,"%s/%s/.mailalert",C.users,U.id);
		if (!stat(filename,&statbuf)) {
			/*printf("\n\a*** You have new mail ***\n");*/
			printf("\n\a%s\n",Ustring[471]);
			remove(filename);
			return 1;
		}
	}
	return 0;
}

#if defined(READ_COMMANDS)

int copymail (char *in) {
/* MENU COMMAND */
	char from[MAINLINE];
	char to[51];
	int result;

	struct valid_mail *vm;
	struct valid_files *vf;
	char *copy = strdup(in);

	tnt(copy);
	strshift(copy,from,MAINLINE,Ustring[501]);
	tnt(from);
	if (!from[0]) {
		strshift(G.comline,from,MAINLINE,Ustring[501]);
	}

	shiftword(copy,to,51);
	if (!to[0]) {
		shiftword(G.comline,to,51);
	}
	free(copy);
	
	tnt(from);
	if (!from[0]) {
		if ((to[0]) && (G.mailcurrent)) {
			sprintf(from,"%d",G.mailcurrent);
		}
	}

	
	vm = get_valid_mail('v',U.id,"",G.mailcurrent,from);
	if (!vm) {
		return 0;
	}


	vf = get_valid_dirs('v',1,"area",C.areasdir,to,0);
	strncpy(to,vf->files,51);
	to[50] = 0;
	free(vf->input);
	free(vf->files);
	free(vf);
	if (!to[0]) {
		free(vm->msglist);
		free(vm->parse);
		free(vm);
		return 0;
	}

	if (is_area_elig('v',to) < 2) {
		free(vm->msglist);
		free(vm->parse);
		free(vm);
		return 0;
	}

	hups_off();
	result = copy_maillist('v',to,vm->msglist);
	hups_on();
	free(vm->msglist);
	free(vm->parse);
	free(vm);
	return result;
}

int copy_maillist (char mode, char *to, int *fromlist) {
	int place = 0;
	FILE *FIL;
	struct mailheader fhs;
	char filename[MAINLINE + 100];
	char inbody[MAINLINE + 100];
	char temp[MAINLINE * 3];
	char *fromheader;

	while (fromlist[place]) {
		fromheader = definemail('q',U.id,fromlist[place]);
		if (!fromheader) {
			if ((mode != 'q') && (!G.intflag)) {
				/*printf("Msg no %d has vanished right under your nose!\n",fromlist[place]);*/
				sprintf(temp,Ustring[193],fromlist[place]);
				printf(Ustring[65],temp);
				printf("\n");
			}
			place ++;
			continue;
		}

		sprintf(filename,"%s/%s.temp",C.tmpdir,U.id);
		sprintf(inbody,"%s/%s/.mail/msg.%d",C.maildirs,U.id,fromlist[place]);
		if ((FIL = fopen(filename,"w"))) {
			fprintf(FIL,"Copied from private mail by %s\n",U.id);
			fputs("------\n\n",FIL);
			fclose(FIL);
		}
		sprintf(temp,"cat %s >> %s",inbody,filename);
		dsystem(temp);

		parse_mail_header(fromheader,&fhs);
		if (strlen(fhs.author) > 14) {
			fhs.author[14] = 0; /* this is a message area limit */
		}
		sendmess(mode,to,fhs.author,filename,fhs.subject,"BASE-MESSAGE - - - -",'B','f');
		free(fromheader);

		place++;
	}
	return 1;
}

#else 

int copymail (char *in) {
	puts("Sorry, this system does not support mail copying.");
	return 0;
}

#endif
