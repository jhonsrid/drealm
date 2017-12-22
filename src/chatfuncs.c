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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Non-ANSI headers */
#include <unistd.h>
#include <utmp.h>
#include <sys/stat.h>
#if defined(SVR42) || defined(LINUX_WITH_DGRAMS)
#  include <sys/socket.h>
#  include <sys/un.h>
#  if defined(SVR42)
int sendto(int s, char *msg, int len, int flags, caddr_t to, int tolen);
int socket(int domain, int type, int protocol);
#  endif
#else
#  if defined(LINUX)
#    include <fcntl.h>
#  endif
#endif
#include <sys/wait.h>

/* Local headers */
#include "drealm.h"
#include "drealmgen.h"
#include "mainfuncs.h"
#include "inputfuncs.h"
#include "configfuncs.h"
#include "setupfuncs.h"
#include "genfuncs.h"
#include "getvalf.h"

#include "chatfuncs.h"


/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
 * NOTES:
 * U.chat means decision about whether to start off with chat on
 * G.chat means current choice of whether to receive chat when allowed
 * G.chatenabled means SysOp's current choice of whether to allow chatting
 * Only chat.user will actually tell whether this person is actually getting
 * chat right now.
 * $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
 */


void chatlog (char *msg) {
/* CHECKED */
	if (G.chatlog) {
		char *newmsg = strdup(msg);
		char *date = shorttime(time(0));

		tnt(newmsg);
		fprintf(CHATLOG,"%s %s: %s\n",date,U.id,newmsg);
		fflush(CHATLOG);
		free(date);
		free(newmsg);
	}
}


/* ARGSUSED0 */
int chatenable (char *dummy) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
/*
 * Used by the program, during file transfers etc.  Also for the menu to
 * turn a user's chat on after being off.
 * Will not turn on chat for a person who has deliberately turned it off.
 */
	if (C.chatstyle) {
		if (G.chatenabled == 0) { /* Only if it's not already enabled */
			if (G.chat == 1) { /* Only if the user wants it */
				if (sub_chaton()) { /* Only if chat reception not already on */
					/*printf("Chat enabled.\n");*/
					printf("%s\n",Ustring[218]);
				}
			}
		}
		G.chatenabled = 1; /* SysOp allows it anyway */
		return 1; /* Refers to global state of enablement */
	}
	return 0;
}

/* ARGSUSED0 */
int chaton (char *dummy) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
/* This is for a user to turn their own chat on */
	if (C.chatstyle) {
		G.chat = 1; /* Means they'd prefer to get it and will when it is enabled */
		if (G.chatenabled) { /* Only if it's enabled globally */
			if (sub_chaton()) { /* Only if chat reception was not already on */
				printf("%s\n",Ustring[219]);
				/*printf("Chat reception now turned on.\n");*/
			} else {
				/*printf("Chat reception already on.\n");*/
				printf("%s\n",Ustring[220]);
			}
			return 1; /* Refers to chat being currently in ON state */
		}
		/*printf("Chat not enabled at this point.\n");*/
		printf("%s\n",Ustring[221]);
	}
	return 0;
}

int sub_chaton (void) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
/* 'doing' lines are appropriate here, as these are not set from above, 
 *  do not copy method into sub_chatoff.
 */
	char temp[MAINLINE + 100];
	struct stat statbuf;

	sprintf(temp,"%s/chat.%s",C.tmpdir,U.id);
	if (stat(temp,&statbuf)) {  /* Only if chat is not currently in reception */
		sprintf(temp,"%d",(U.chatcolour + 20));
		chatin("",temp);
		/*set_chatdoing("*listening for chat*");*/
		set_chatdoing(Ustring[202]);
		return 1; /* If chat state was altered */
	}
	return 0; /* If chat state was already on */
}

int chatdisable (char *dummy) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
/* Used by the program, during file transfers etc.  Also for the menu to
   turn a user's chat off */
	if (C.chatstyle) {
		if (G.chatenabled == 1) { /* Only if not already disabled */
			if (sub_chatoff()) { /* Only if user had been receivingchat */
				/*set_chatdoing("*chat disabled*");*/
				set_chatdoing(Ustring[201]);
				/*printf("Chat disabled.\n");*/
				printf("%s\n",Ustring[222]);
			}
		}
		G.chatenabled = 0; /* SysOp wants it off no matter what happens */
		return 1;
	}
	return 0;
}

/* ARGSUSED0 */
int chatoff (char *dummy) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
/* This is for the user to turn his own chat off */
	if (C.chatstyle) {
		G.chat = 0; /* User has decided not to bother with chat no matter what comes next */

		if (sub_chatoff()) { /* Only if not already off */
			/*set_chatdoing("*chat disabled*");*/
			set_chatdoing(Ustring[201]);
			/*printf("Chat reception now turned off.\n");*/
			printf("%s\n",Ustring[223]);
		} else {
			/*printf("Chat reception already off.\n");*/
			printf("%s\n",Ustring[224]);
		}
		return 1; /* Refers to chat being now in OFF state */
	}
	return 0; /* Chat cannot be done anything to */
}

int sub_chatoff (void) {
/* LENGTHS CHECKED */
/* Do not use set_chatdoing in here */
	char temp[MAINLINE + 100];
	struct stat statbuf;

	sprintf(temp,"%s/chat.%s",C.tmpdir,U.id);
	if (!stat(temp,&statbuf)) { /* Only if chat reception in progress */
		chatin("-1","");
		return 1; /* Chat state was altered */
	}
	return 0; /* Chat state unaltered */
}

int chatqueue (char *dummy) {
/* LENGTHS CHECKED */
	char temp[MAINLINE + 100];
	struct stat statbuf;
	int j = 0;
	int result = 0;
	
	if (C.chatstyle) {
		sprintf(temp,"%s/chat.%s",C.tmpdir,U.id);
		if (!stat(temp,&statbuf)) { /* If chat is actually ON */
#if 0
			/*too slow for general use*/
			/*set_chatdoing("*chat queuing*");*/
			set_chatdoing(Ustring[203]);
#endif
			user2_on();
			result = chatin("-2","");
			while (!G.usr2flag && (j < 5)) {
				sleep(1);
				j++;
			}
			user2_off();
		}
	}
	return result;
}

/* ARGSUSED0 */
int chatrelease (char *dummy) {
/* LENGTHS CHECKED */
	char temp[MAINLINE + 100];
	struct stat statbuf;
	int result;
	int j = 0;

	if (C.chatstyle) {
		sprintf(temp,"%s/chat.%s",C.tmpdir,U.id);
		if (!stat(temp,&statbuf)) { /* Only if chat is actually ON */
#if 0
			/*set_chatdoing("*listening for chat*");*/
			set_chatdoing(Ustring[202]);
#endif
			G.usr2flag = 0;
			user2_on();
			result = chatin("-3","");
			while (!G.usr2flag && (j < 5)) {
				sleep(1);
				j++;
			}
			user2_off();
			return result;
		}
	}
	return 0;
}

/*===================================================================*/

int check_faculties (void) {
/* LENGTHS CHECKED */
	if (G.chat != 1) {
		/*printf("You must turn your chat on before you can converse.\n");*/
		printf("%s\n",Ustring[225]);
		return 0;
	}
	if (G.chatenabled != 1) {
		/*printf("Chat not enabled at this point.\n");*/
		printf("%s\n",Ustring[221]);
		return 0;
	}
	return 1;
}

/* ARGSUSED0 */
int say (char *dummy) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
	return chatmess("say");
}

/* ARGSUSED0 */
int emote (char *dummy) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
	return chatmess("emote");
}

/* ARGSUSED0 */
int fx (char *dummy) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
	return chatmess("fx");
}

/* ARGSUSED0 */
int whisper (char *dummy) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
	char temp[MAINLINE + 100];
	char tempa[MAINLINE + 100];
	char recipient[9];

	char *params = strdup(G.comline);
	flushcom("");

	if (!check_faculties()) {
		free(params);
		return 0;
	}

	shiftword(params,recipient,9);
	if (!recipient[0]) {
		/*make_prompt("Private message to whom? ");*/
		make_prompt(Ustring[208]);
		get_one_line(tempa);
		shiftword(tempa,recipient,9);
	}
	if (!recipient[0]) {
		/*printf("No user named.\n");*/
		printf(Ustring[294],Ustring[196]);
		printf("\n");
		free(params);
		return 0;
	}
	lower_string(recipient);

	if (! is_online(recipient,'c')) {
		/*printf("%s is either not listening or not logged on.\n",recipient);*/
		printf(Ustring[210],recipient);
		printf("\n");
		free(params);
		return 0;
	}

	if (!params[0]) {
		/*make_prompt("private message> ");*/
		make_prompt(Ustring[204]);
		get_one_line(tempa);
	} else {
		strcpy(tempa,params);
	}
	free(params);
	
	if (tempa[0] == 0) {
		/*printf("No message to send);*/
		printf(Ustring[294],Ustring[69]);
		printf("\n");
		return 0;
	}
	/*sprintf(temp,"[%s whispers] %s\n",U.id,tempa);*/
	sprintf(temp,Ustring[214],U.id,tempa);
	strcat(temp,"\n");
	chatin(recipient,temp);
	chatlog(temp);

	return 1;
}

int broadcast (char *line) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
	char recipient[9];
	char *broadcastlist;
	char newline[MAINLINE + 100];

	if (line[0] == 0) {
		return 0;
	}

	trans_string(line,newline,MAINLINE);

	broadcastlist = onlist("chat");

	while(broadcastlist[0]) {
		shiftword(broadcastlist,recipient,9);
		chatin(recipient,newline);
	}
	chatlog(newline);
	free(broadcastlist);
	return 1;
}

/* ARGSUSED0 */
int whoson (char *dummy) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
	char temp[MAINLINE + 100];
	char brackets[MAINLINE];
	char doing[MAINLINE];
	char chatdoing[80];
	char id[9];
	char propername[MAINLINE];
	char title[MAINLINE];
	char *list;
	FILE *TMP;

	list = onlist("conf");

	printf("\n");
	while(list[0]) {
		shiftword(list,id,9);

		title[0] = 0;
		sprintf(temp,"%s/%s/.title",C.users,id);
		if ((TMP = fopen(temp,"r"))) {
			if (fgets(title,31,TMP)) {
			/* 79 because doing is only 80 and has a space on */
				title[30] = 0;
				tnt(title);
			}
			fclose(TMP);
		}

		propername[0] = 0;
		get_propername(id,propername,31);

		brackets[0] = 0;
		if (propername[0] && title[0]) {
			sprintf(brackets," (%s - %s)",propername,title);
		} else if (title[0]) {
			sprintf(brackets," (%s)",title);
		} else if (propername[0]) {
			sprintf(brackets," (%s)",propername);
		}

		doing[0] = 0;
		sprintf(temp,"%s/%s/.doing",C.users,id);
		if ((TMP = fopen(temp,"r"))) {
			if (fgets(temp,79,TMP)) {
			/* 79 because doing is only 80 and has a space on */
				temp[79] = 0;
				tnt(temp);
				if (temp[0]) {
					sprintf(doing,"%s ",temp);
				}
			}
			fclose(TMP);
		}

		sprintf(temp,"%s/%s/.chatdoing",C.users,id);
		if ((TMP = fopen(temp,"r"))) {
			if (!fgets(chatdoing,79,TMP)) {
				chatdoing[0] = 0;
			}
			chatdoing[79] = 0;
			fclose(TMP);
		} else {
			chatdoing[0] = 0;
		}

		/*printf("%s%s %s%s\n",id,brackets,doing,chatdoing);*/
		printf("%s%s %s%s\n",id,brackets,doing,chatdoing);
	}
	free(list);
	return 1;
}

/* ARGSUSED0 */
int show_chat (char *dummy) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
/* displays whatever chat has accumulated */
	if (G.chat == 1) {
		return chatin("-4","");
	}
	return 0;
}

int chatmess (char *type) {
/* LENGTHS CHECKED */
/* Called by say, emote and fx */
	char chatstring[MAINLINE + 100];
	char temp[MAINLINE + 100];
	char *blist;
	char recipient[9];

	if (!check_faculties()) {
		return 0;
	}
	if (strcmp(type,"say") == 0) {
		if (!G.comline[0]) {
			/*make_prompt("message> ");*/
			make_prompt(Ustring[205]);
			get_one_line(temp);
		} else {
			strcpy(temp,G.comline);
			flushcom("");
		}
		if (temp[0] == 0) {
			/*printf("No message to send - abandoned.\n");*/
			printf(Ustring[294],Ustring[69]);
			printf("\n");
			return 0;
		}
		/*sprintf(chatstring,"[%s says] %s\n",U.id,temp);*/
		sprintf(chatstring,Ustring[215],U.id,temp);
		strcat(chatstring,"\n");
	} else if (strcmp(type,"emote") == 0) {
		if (!G.comline[0]) {
			/*make_prompt("emote to all> ");*/
			make_prompt(Ustring[206]);
			get_one_line(temp);
		} else {
			strcpy(temp,G.comline);
			flushcom("");
		}
		if (temp[0] == 0) {
			/*printf("No action to emote - abandoned.\n");*/
			printf(Ustring[294],Ustring[212]);
			printf("\n");
			return 0;
		}
		/*sprintf(chatstring,"<%s %s>\n",U.id,temp);*/
		sprintf(chatstring,Ustring[217],U.id,temp);
		strcat(chatstring,"\n");
	} else if (strcmp(type,"fx") == 0) {
		if (!G.comline[0]) {
			/*make_prompt("FX> ");*/
			make_prompt(Ustring[207]);
			get_one_line(temp);
		} else {
			strcpy(temp,G.comline);
			flushcom("");
		}
		if (temp[0] == 0) {
			/*printf("No effect to send - abandoned.\n");*/
			printf(Ustring[294],Ustring[213]);
			printf("\n");
			return 0;
		}
		/*sprintf(chatstring,"<FX: %s>\n",temp);*/
		sprintf(chatstring,Ustring[216],temp);
		strcat(chatstring,"\n");
	} else {
		flushcom("");
		return 0;
	}


	blist = onlist("chat");
	while(blist[0]) {
		shiftword(blist,recipient,9);
		chatin(recipient,chatstring);
	}

	chatlog(chatstring);
	free(blist);

	return 1;
}

int set_chatdoing(char *chat_string) {
/* LENGTHS CHECKED */
	FILE *TMP;
	char temp[MAINLINE + 100];

	sprintf(temp,"%s/%s/.chatdoing",C.users,U.id);
	if (TMP = fopen(temp,"w")) {
		fprintf(TMP,"%s",chat_string);
		fclose(TMP);
		return 1;
	}
	return 0;
}

int is_online (char *id,char type) {
/* LENGTHS CHECKED */
	char tempid[9];
	struct utmp *u;
	int found = 0;
	struct stat statbuf;
	char temp[MAINLINE + 100];

	
	while(u = getutent()) {
		if (u->ut_type == USER_PROCESS) {
			strncpy(tempid,u->ut_user,8);
			tempid[8]=0;
			tnt(tempid);
			if (!strcmp(id,tempid)) {
				if (type == 'c') {
					sprintf(temp,"%s/chat.%s",C.tmpdir,id);
				} else {
					sprintf(temp,"%s/conf.%s",C.tmpdir,id);
				}
				if (stat(temp,&statbuf)) {
					continue;
				} else {
					found++;
					break;
				}
			}
		}
	}
	endutent();
	return found;
}


int whois (char *in) {
/* LENGTHS CHECKED */
/* MENU COMMAND */
	struct valid_files *vf;
	char person[9];
	char propername[MAINLINE];
	char *copy = strdup(in);
	
	shiftword(copy,person,9);
	free(copy);
	if (!person[0]) {
		shiftword(G.comline,person,9);
	}

	/*vf = get_valid_dirs('v',1,"user",C.users,person,0);*/
	vf = get_valid_dirs('v',1,Ustring[196],C.users,person,0);
	strcpy(person,vf->files);
	free(vf->input);
	free(vf->files);
	free(vf);

	if (!person[0]) {
		return 0;
	}

	get_propername(person,propername,MAINLINE);

	if (propername[0]) {
		/*printf("%s - a.k.a %s.\n",person,propername);*/
		printf("%s - %s %s.\n",person,Ustring[337],propername);
	} else {
		/*printf("%s - No other name given.\n",person);*/
		printf("%s - %s\n",person,Ustring[226]);
	}
	return 1;
}

char *onlist (char *indicator) {
/* LENGTHS CHECKED */
	char person[9];
	char tempa[1024];
	struct utmp *utp;
	struct valid_files *vf;
	char temp[MAINLINE];


	if (!strcmp(indicator,"chat")) {
		/*vf = get_valid_entries('q',0,"users",C.tmpdir,"chat.*",' ',0);*/
		vf = get_valid_entries('q',0,Ustring[287],C.tmpdir,"chat.*",' ',0);
	} else {
		/*vf = get_valid_entries('q',0,"users",C.tmpdir,"conf.*",' ',0);*/
		vf = get_valid_entries('q',0,Ustring[287],C.tmpdir,"conf.*",' ',0);
	}	

	tempa[0] = 0;
	while(vf->files[0]) {
		shiftword(vf->files,temp,MAINLINE);
		setutent();		
		
		while (utp = getutent()) {
			if (utp->ut_type != USER_PROCESS) {
				continue;
			}
			strncpy(person,utp->ut_user,8);
			person[8] = 0;
			tnt(person);
			if (!strcmp(person,&temp[5])) {
				strcat(tempa,person);
				strcat(tempa," ");
				break;
			}
		}		
	}
	endutent();
	free(vf->files);
	free(vf->input);
	free(vf);
	tnt(tempa);
	return strdup(tempa);
}


int chatin (char *recipient, char *msg) {
/* LENGTHS CHECKED */
	char temp[MAINLINE + 100];
	struct stat statbuf;

	if (C.chatstyle == 2) {
	/* Sockety chat suitable for linux and svr42 */
		if (!recipient[0]) {	/* Start chatout */
			sprintf(temp,"chatout %s %s %s",msg,C.tmpdir,U.id);
			dsystem(temp);
			return 1; /* chatout in the background is always true */
		} else if (!msg[0]) {	/* Send signal */

#if defined(LINUX) || defined(SVR42)
			socketsend(U.id,atoi(recipient) * -1,NULL);
			return 1;
#else
			/*printf("Chat not supported for your system.\n");*/
			printf("%s\n",Ustring[228]);
			return 0;
#endif
		} else {		/* Send chat message */
#if defined(LINUX) || defined(SVR42)
			socketsend(recipient,U.chatsendcolour,msg);
			return 1;
#else
			/*printf("Chat not supported for your system.\n");*/
			printf("%s\n",Ustring[228]);
			return 0;
#endif
		}
	} else if (C.chatstyle == 1) {
	/* Plainfile-based */
		char temps[MAINLINE + 100];
		char tempa[MAINLINE + 100];
		char command[MAINLINE + MAINLINE + 100];
		FILE *TMP;

		if (strcmp(recipient,"-1") == 0) {
			sprintf(temps,"%s/chat.%s",C.tmpdir,U.id);
			(void)remove(temps);
			sprintf(temps,"%s/hear.%s",C.tmpdir,U.id);
			(void)remove(temps);
			return 1;
		} else if (recipient[0] == 0) {
			sprintf(temps,"%s/chat.%s",C.tmpdir,U.id);
			TMP = fopen(temps,"w");
			fclose(TMP);
			return 1;
		} else if (strcmp(recipient,"-4") == 0) {
			sprintf(temps,"%s/hear.%s",C.tmpdir,U.id);
			sprintf(tempa,"%s/heard.%s",C.tmpdir,U.id);
			rename(temps,tempa);
			sprintf(command,"cat %s",tempa);
			dsystem(command);
			remove(tempa);
			putchar('\n');
			return 1;
		} else if (strcmp(recipient,"-5") == 0) {
			sprintf(temps,"%s/hear.%s",C.tmpdir,U.id);
			(void)remove(temps);
			return 1;
		} else {
			sprintf(temps,"%s/chat.%s",C.tmpdir,recipient);
			if (!stat(temps,&statbuf)) {
				sprintf(temps,"%s/hear.%s",C.tmpdir,recipient);
				if (TMP = fopen(temps,"a")) {
					fprintf(TMP,"%s",msg);
					fclose(TMP);
					return 1;
				}
			}
			return 0;
		}
	} else if (C.chatstyle == 0) {
	/* No chat at all */
		return 0;
	} else {
	/* Still no chat */
		return 0;
	}
}

#if defined(SVR42) || defined(LINUX_WITH_DGRAMS)
void socketsend(const char *recipient, const int msgtype, const char *text) {
/* LENGTHS CHECKED */
	struct message msg;
	int sock_out;
	struct sockaddr_un dgram_out;

	if ((sock_out=socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		sprintf(G.errmsg,"Problems with socket chat. %s. Please consult Author for solution!",strerror(errno));
		errorlog(G.errmsg);
		/*printf("Problems with chat.  Please tell sysop.\n");*/
		printf("%s\n",Ustring[229]);
		return;
	}

	msg.signal = msgtype;
	if (text) {
		strncpy(msg.text,text,255);
		msg.text[255] = 0;
	}

	dgram_out.sun_family = AF_UNIX;
	sprintf(dgram_out.sun_path,"%s/chat.%s",C.tmpdir,recipient);

#if defined(SVR42)
	/* LINTED *//* casts to (char *) and (caddr_t) */
	if (sendto(sock_out, (char *)&msg, sizeof msg, 0, (caddr_t) &dgram_out, strlen(dgram_out.sun_path) + sizeof dgram_out.sun_family) < 0)
#else
	if (sendto(sock_out, (char *)&msg, sizeof msg, 0, (struct sockaddr *) &dgram_out, strlen(dgram_out.sun_path) + sizeof dgram_out.sun_family) < 0)
#endif
	{
		if (errno != ENOENT) {
			close(sock_out);
			sprintf(G.errmsg,"Problems with socket chat. %s. Please consult Author for solution!",strerror(errno));
			errorlog(G.errmsg);
			/*printf("Problems with chat.  Please tell sysop.\n");*/
			printf("%s\n",Ustring[229]);
			return;
		}
	}
	close(sock_out);
}
#else
#  if defined(LINUX)
void socketsend(const char *recipient, const int msgtype, const char *text) {
	struct message msg;
	char endpoint[108];
	int outgoing;

	sprintf(endpoint,"%s/chat.%s",C.tmpdir,recipient);
	if ((outgoing=open(endpoint,O_WRONLY | O_NONBLOCK,0)) < 0) {
		switch(errno) {
			case ENOENT:
			case ENXIO:
			case EAGAIN:
				/*
				 * these can happen if the "other" end
				 * exits unexpectedly: it's not a problem.
				 */
				break;
			default:
				sprintf(G.errmsg,"Problems with chat. %s. Please consult Author for solution!",strerror(errno));
				errorlog(G.errmsg);
				/*printf("Problems with chat.  Please tell sysop.\n");*/
				printf("%s\n",Ustring[229]);
		}
		return;
	}

	msg.signal = msgtype;
	if (text) {
		strncpy(msg.text,text,255);
		msg.text[255] = 0;
	}

	if (write(outgoing,&msg, sizeof msg) < sizeof msg) {
		switch(errno) {
			case ENOENT:
			case ENXIO:
			case EAGAIN:
				/*
				 * these can happen if the "other" end
				 * exits unexpectedly: it's not a problem.
				 */
				break;
			default:
				sprintf(G.errmsg,"Problems with chat. %s. Please consult Author for solution!",strerror(errno));
				errorlog(G.errmsg);
				/*printf("Problems with chat.  Please tell sysop.\n");*/
				printf("%s\n",Ustring[229]);
		}
		close(outgoing);
		return;
	}
	close(outgoing);
}
#  else
void socketsend(const char *recipient, const int msgtype, const char *text) {
#error This system has nothing we can use as a socket
	/*(void)printf("Chat not supported on this system.\n");*/
	(void)printf("%s\n",Ustring[228]);
}
#  endif
#endif
