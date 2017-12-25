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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <unistd.h>
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

#include "drealm.h"
#include "drealmgen.h"
#include "mainfuncs.h"
#include "genfuncs.h"

#include "timerfuncs.h"

struct message {
	int signal;
	int content;
};

int daytimer_start (char *params) {
/* MENU COMMAND */
/* Only takes from the menu */
	char *copy;
	FILE *FIL;
	char filename[MAINLINE + 100];
	char compare_date[9];
	char time_today_string[80];
	int time_today = 0;
	int time_left;
	int allday_time;
	char today_date[9];
	time_t t = time(0);
	
	copy = strdup(params);
	tnt(copy);
	if (is_num(copy)) {
		allday_time = atoi(copy);
		free(copy);
	} else {
		/* ERROR - means wrong params passed */
		free(copy);
		return 0;
	}
	
	sprintf(filename,"%s/%s/.timed",C.users,U.id);
	
	compare_date[0] = 0;
	time_today_string[0] = 0;

	if ((FIL = fopen(filename,"r"))) {
/* fgets() didn't work here */
		fscanf(FIL, " %s ", compare_date);
		fscanf(FIL, " %s ", time_today_string);
		fclose(FIL);
	}
	
	tnt(compare_date);
	tnt(time_today_string);
	time_today = atoi(time_today_string);
		
	(void)strftime(today_date,9,"%Y%m%d",localtime(&t));	

	if (strcmp(compare_date,today_date)) {
		time_today = 0;
		time_left = allday_time;
		if ((FIL = fopen(filename,"w"))) {
			fprintf(FIL,"%s\n",today_date);
			fprintf(FIL,"%d\n",time_today);
			fclose(FIL);
		}
	} else {
		time_left = allday_time - time_today;
	}

		
	if (time_left > 0) {
		/*printf("You have %d minute/s remaining today.\n",time_left);*/
		sprintf(filename,Ustring[280],time_left);
		printf("%s ",Ustring[386]);
		printf(Ustring[383],filename);
		printf("\n");
		if (G.timer_on) {
			return change_timer(time_left);
		} else {
			G.timer_on = 1;
			return start_timer(time_left);
		}
	} else {
		/*printf("You have used up all your time for today.\n");*/
		printf("%s\n",Ustring[395]);
		logoff("");
		return 1;
	}
}


int timer_start (char *params) {
/* MENU COMMAND */
/* Only takes minutes out of the menu */

	char temp[MAINLINE];
	char *copy;
	int minutes;
	
	copy = strdup(params);
	tnt(copy);
	
	if (copy[0] && (atoi(copy) > 0)) {
		minutes = atoi(copy);
		free(copy);
		/*printf("You have %d minute/s remaining for this session.\n",minutes);*/
		sprintf(temp,Ustring[280],minutes);
		printf("%s ",Ustring[319]);
		printf(Ustring[433],temp);
		printf("\n");
		if (G.timer_on) {
			return change_timer(minutes);
		} else {
			G.timer_on = 1;
			return start_timer(minutes);
		}
	} else {
		errorlog("No time given for the timer");
		free(copy);
		return 0;
	}
}

int timer_stop (char *dummy) {
/* MENU COMMAND */
	
	timersend(1,0);
	G.timer_on = 0;
	return 1;
}

int timer_suspend (char *dummy) {
/* MENU COMMAND */
	timersend(2,0);
	return 1;
}

int timer_resume (char *dummy) {
/* MENU COMMAND */
	timersend(3,0);
	return 1;
}

int start_timer (int minutes) {
	pid_t pid = getpid();
	char temp[MAINLINE + 100];
	FILE *FIL;
	int i = 0;

#if !defined(LINUX)
	sprintf(temp,"timer %d %s %s %ld %s",minutes,C.tmpdir,U.id,pid,C.users);
#else
	sprintf(temp,"timer %d %s %s %d %s",minutes,C.tmpdir,U.id,pid,C.users);
#endif
	dsystem(temp);
	sprintf(temp,"%s/time.%s",C.tmpdir,U.id);
	while(((FIL = fopen(temp,"r+")) == NULL) && (i++ < 3)) {
		sleep(1);
	}
	if (FIL) fclose(FIL);
	return 1; /* timer in the background is always true */
}


int change_timer (int minutes) {
	timersend(4,minutes);
	return 1;
}



#if defined(SVR42) || defined(LINUX_WITH_DGRAMS)
void timersend(const unsigned int msgtype, int content) {
	struct message msg;
	int sock_out;
	struct sockaddr_un dgram_out;

	if ((sock_out=socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		sprintf(G.errmsg,"Problems with timer. %s. Please consult Author for solution!",strerror(errno));
		errorlog(G.errmsg);
		/*printf("Problems with timer.  Please tell sysop.\n");*/
		printf("%s %s\n",Ustring[276],Ustring[277]);
		return;
	}

	msg.signal = msgtype;
	msg.content = content;

	dgram_out.sun_family = AF_UNIX;
	sprintf(dgram_out.sun_path,"%s/time.%s",C.tmpdir,U.id);

#if defined(SVR42)
	/* LINTED *//* casts to (char *) and (caddr_t) */
	if (sendto(sock_out, (char *) &msg, sizeof msg, 0,(caddr_t) &dgram_out,strlen(dgram_out.sun_path) + sizeof dgram_out.sun_family) < 0)
#else
	if (sendto(sock_out, (char *) &msg, sizeof msg, 0,(struct sockaddr *) &dgram_out,strlen(dgram_out.sun_path) + sizeof dgram_out.sun_family) < 0)
#endif
	{
		if (errno != ENOENT) {
			sprintf(G.errmsg,"Problems with timer socket. %s. Please consult Author for solution!",strerror(errno));
			errorlog(G.errmsg);
			/*printf("Problems with timer.  Please tell sysop.\n");*/
			printf("%s %s\n",Ustring[276],Ustring[277]);
			return;
		}
	}
	close(sock_out);
}
#else
#  if defined(LINUX)
void timersend(const unsigned int msgtype, int content) {
	struct message msg;
	char endpoint[108];
	int outgoing;

	sprintf(endpoint,"%s/time.%s",C.tmpdir,U.id);
	outgoing=open(endpoint,O_WRONLY | O_NONBLOCK,0);
	if (!outgoing) {
		if ((errno != ENOENT) && (errno != EAGAIN)) {
			sprintf(G.errmsg,"Problems with timer. %s. Please consult Author for solution!",strerror(errno));
			errorlog(G.errmsg);
			/*printf("Problems with timer.  Please tell sysop.\n");*/
			printf("%s %s\n",Ustring[276],Ustring[277]);
		} else {
			sprintf(G.errmsg,"Got ``%s'' trying to open '%s'.  Is timer running?",strerror(errno),endpoint);
			errorlog(G.errmsg);
		}
		return;
	}

	msg.signal = msgtype;
	msg.content = content;

	if (write(outgoing,&msg, sizeof msg) < sizeof msg) {
		sprintf(G.errmsg,"Problems with timer. %s. Please consult Author for solution!",strerror(errno));
		errorlog(G.errmsg);
		/*printf("Problems with timer.  Please tell sysop.\n");*/
		printf("%s %s\n",Ustring[276],Ustring[277]);
		return;
	}
	close(outgoing);
}
#  else
/* ARGSUSED0 */
void timersend(const unsigned int msgtype, int content) {
#error This system has nothing we can use as a socket
	(void)printf("Timer not supported on this system.\n");
}
#  endif
#endif
