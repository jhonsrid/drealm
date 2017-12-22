
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
/* This is needed to pick up POSIX extensions to ANSI */
#if !defined(LINUX)
#define _POSIX_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <pwd.h>
#include <errno.h>


#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#if defined(SVR42)
#  include <libgen.h>
#else
#  if defined(LINUX)
#    include <regex.h>
#  endif
#endif

#include "drealm.h"
#include "configfuncs.h"
#include "genfuncs.h"
#include "drealmgen.h"


#define	FALSE	0
#define	TRUE	1


int comp_flags (char *m_flags, char *flags) {
	int i = 1;

	while (m_flags[i]) {
		if ((m_flags[i] != '?') && (flags[i] != '?') && (m_flags[i] != flags[i])) {
			return 0;
		}
		i++;
	}
	return 1;
}

int get_hour (void) {
	time_t tim;
	struct tm *hour;

	tim = time(0);
	hour = localtime(&tim);
	return hour->tm_hour;
}

int nicedate (char *outstring) {
/* outstring must be preset to at least MAINLINE long */
	time_t t = time(0);

	(void)strftime(outstring,16,"%a %b %e %Y",localtime(&t));
	return 1;
}

int nicetime (char *outstring) {
/* outstring must be preset to at least MAINLINE long */
	time_t t = time(0);
	char *format = strdup(outstring);

	if (format[0]) {
		if (!strcmp(format,"DLT")) {
			(void)dlt(outstring,MAINLINE,localtime(&t));
		} else {
			(void)strftime(outstring,MAINLINE,format,localtime(&t));
		}
	} else {
		(void)strftime(outstring,6,"%R",localtime(&t));
	}
	return 1;
}

int is_line_elig (struct line_details *ld,char *user) {
	char filename[MAINLINE + 100];
	struct passwd *pword;
	FILE *CFG;
	int f = 0;
	int i = 0;

	if (ld->ln == 0) {
		printf("%s\n",ld->errorstring);
		return 0;
	}
	if (ld->exc > 0) {
		sprintf(filename,"%s/exc.%d",C.configdir,ld->ln);
		if (is_in_list(filename,user)) {
			if (ld->exc == 1) {
				printf("%s\n",ld->errorstring);
			}
			return 0;
		}
	}
	if (ld->only > 0) {
		sprintf(filename,"%s/only.%d",C.configdir,ld->ln);
		if (!is_in_list(filename,user)) {
			if (ld->only == 1) {
				printf("%s\n",ld->errorstring);
			}
			return 0;
		}
	}
	if (ld->inc > 0) {
		sprintf(filename,"%s/inc.%d",C.configdir,ld->ln);
		if (is_in_list(filename,user)) {
			return 1;
		}
	}
	if (ld->level > 0) {
		sprintf(filename,"%s/level.%d",C.configdir,ld->ln);
		if (CFG = fopen(filename,"r")) {
			fscanf(CFG," %d ",&i);
			fclose(CFG);
			sprintf(filename,"%s/%s/.level",C.users,user);
			if (CFG = fopen(filename,"r")) {
				fscanf(CFG," %d ",&f);
				fclose(CFG);
				if (f < i) {
					if (ld->level == 1) {
						printf("%s\n",ld->errorstring);
					}
					return 0;
				}
			}
		} else {
			if (ld->level == 1) {
				printf("%s\n",ld->errorstring);
			}
			return 0;
		}
	}

	if (ld->shells > 0) {
		pword = getpwnam(user);
		sprintf(filename,"%s/shells.%d",C.configdir,ld->ln);
		if (!is_in_list(filename,pword->pw_shell)) {
			if (ld->shells == 1) {
				printf("%s\n",ld->errorstring);
			}
			return 0;
		}
	}
	return 1;
}

struct line_details *get_line_details (char *dev) {
/* THIS COULD LOCK THE SYSTEM OUT IF IT IS USED AS PART OF A LOGIN - beware */
	int lntemp;
	int i;
	char temp[1024];
	char smalltemp[81];
	char filename[MAINLINE + 100];
	FILE *FIL;
	struct line_details *ld;

	ld = (struct line_details *)malloc(sizeof (struct line_details));

	ld->ln = 0;
	strcpy(ld->dev,dev);
	strcpy(ld->nicename,"Unknown");
	strcpy(ld->errorstring,"");
	strcpy(ld->promptstring,"");
	strcpy(ld->screenfile,"");
	ld->exc = 0;
	ld->only = 0;
	ld->inc = 0;
	ld->level = 0;
	ld->shells = 0;

	sprintf(filename,"%s/config.nodes",C.configdir);
	if (FIL = fopen(filename,"r")) {
		i = 0;
		while (fgets(temp,1024,FIL) && (!G.maxusers || (i < G.maxusers))) {
			tnt(temp);
			if (!((temp[0] == ':') && (temp[1] == ':'))) {
				continue;
			}
			i++;
			strshift(temp,smalltemp,81,"::");

			/* LINENUM */
			strshift(temp,smalltemp,81,"::");
			tnt(smalltemp);
			lntemp = atoi(smalltemp);

			/* DEV */
			strshift(temp,smalltemp,81,"::");
			tnt(smalltemp);
			if (!strcmp(smalltemp,dev) || ((!G.maxusers) && !strcmp(smalltemp,"*DEFAULT*"))) {
				ld->ln = lntemp;
			} else {
				continue;
			}


			/* NICENAME */
			strshift(temp,smalltemp,81,"::");
			tnt(smalltemp);
			if (smalltemp[0]) {
				strcpy(ld->nicename,smalltemp);
			}

			/* EXC */
			strshift(temp,smalltemp,81,"::");
			tnt(smalltemp);
			ld->exc = atoi(smalltemp);

			/* ONLY */
			strshift(temp,smalltemp,81,"::");
			tnt(smalltemp);
			ld->only = atoi(smalltemp);

			/* INC */
			strshift(temp,smalltemp,81,"::");
			tnt(smalltemp);
			ld->inc = atoi(smalltemp);

			/* LEVEL */
			strshift(temp,smalltemp,81,"::");
			tnt(smalltemp);
			ld->level = atoi(smalltemp);

			/* SHELLS */
			strshift(temp,smalltemp,81,"::");
			tnt(smalltemp);
			ld->shells = atoi(smalltemp);

			/* ERRORSTRING */
			/* only shown if the security mode in the failed test was 1 */
			/* 2 suppresses it */
			strshift(temp,smalltemp,81,"::");
			tnt(smalltemp);
			if (smalltemp[0]) {
				strcpy(ld->errorstring,smalltemp);
			}

			/* PROMPTSTRING */
			/* Not used in this version */
			strshift(temp,smalltemp,81,"::");
			tnt(smalltemp);
			if (smalltemp[0]) {
				strcpy(ld->promptstring,smalltemp);
			}

			/* LOGIN SCREEN */
			strshift(temp,smalltemp,81,"::");
			tnt(smalltemp);
			if (smalltemp[0]) {
				strcpy(ld->screenfile,smalltemp);
			}
			break;
		}
		fclose(FIL);
	} else {
		errorlog("Could not read config.nodes");
	}
	return ld;
}


int copy_file (char *origfile, char *targetfile, int filter) {
	int result = 0;
	struct stat statbuf;
	char temp[MAINLINE + MAINLINE + 10];

	if (stat(origfile,&statbuf)) {
		return 0;
	}

	if (!strcmp(origfile,targetfile)) {
		return 1;
	}

	if (filter && C.filter[0]) {
		sprintf(temp,"%s < %s > %s 2>/dev/null",C.filter,origfile,targetfile);
	} else {
		sprintf(temp,"cp %s %s 2>/dev/null",origfile,targetfile);
	}
	result = dsystem(temp);
	chmod(targetfile,0660);

	return !result;
}

int append_file (char *origfile, char *targetfile, int filter) {
	char temp[MAINLINE + MAINLINE + 10];

	if (!strcmp(origfile,targetfile)) {
		return 1;
	}

	if (filter && C.filter[0]) {
		sprintf(temp,"%s < %s >> %s 2>/dev/null",C.filter,origfile,targetfile);
	} else {
		sprintf(temp,"cat %s >> %s 2>/dev/null",origfile,targetfile);
	}
	return !dsystem(temp);
}

char *drealmtime (time_t t) {
	char outstring[MAINLINE];

	(void)dlt(outstring,MAINLINE,localtime(&t));
	return strdup(outstring);
}

static char *create_lock(const char *lockname, const pid_t pid) {
	static char my_lock[MAINLINE + 105];
	FILE *mine;
	char *p;

	(void)strncpy(my_lock,lockname, MAINLINE + 105);
	my_lock[MAINLINE + 104] = 0;

	if (p=strrchr(my_lock,'/')) {
		p++;
	} else {
		p=my_lock;
	}


	(void)sprintf(p,"L%d.LCK",(int)pid);

	if ((mine=fopen(my_lock,"w")) == NULL) {
		return(NULL);
	}

#if !defined(LINUX)
	(void)fprintf(mine,"%ld\n",pid);
#else
	(void)fprintf(mine,"%d\n",pid);
#endif
	(void)fclose(mine);

	return(my_lock);
}

static int read_lock(const char *lockname)
{
	FILE *old;
	int oldpid=0;

	old=fopen(lockname,"r");
	(void)fscanf(old,"%d\n",&oldpid);
	return(oldpid);
}

int place_lock(const char mode, const char *lockname, int must_wait, int force) {

/* force=0 no burst */
/* force=1 burst if my own */
/* force=2 burst after wait if wait enabled or else immediately */
/* force=3 (not implemented) kill the other process! */

	pid_t pid = getpid();
	char *my_lock=create_lock(lockname,pid);
	int waited=0;

	if (my_lock == NULL) {
#if defined(DEVEL)
		(void)fputs("place_lock: create_lock failed\n",stderr);
#endif
		return(FALSE);
	}

	while (link(my_lock, lockname) < 0) {
#if defined(SVR42) || defined(LINUX)
		int oldpid;

		oldpid = read_lock(lockname);

		if (!oldpid || ((force == 1) && (oldpid == pid)) || (kill(oldpid,0) && (errno == ESRCH)) ) {

			if (remove(lockname)) {
				if (mode != 'q') {
					/*(void) printf("Could not remove old lock %s\n");*/
					(void) printf("%s\n",Ustring[242]);
				}
				remove(my_lock);
				return FALSE;
			}
			continue;
		}
#else
		/* kill not available.  Have to just time out. */
#endif

		if (mode != 'q') {
			switch (waited) {
				case 25:
					/*(void) printf("Could not create %s...\n", lockname);*/
					(void) printf("%s\n",Ustring[243]);
					break;
				case 12:
					/*(void) printf("Still waiting to create %s...(25 sec max wait)\n", lockname);*/
					(void) printf(Ustring[244],25);
					(void) printf("\n");
					break;
				case 7:
					/*(void) printf("Still waiting to create %s...\n", lockname);*/
					(void) printf(Ustring[244],25);
					(void) printf("\n");
					break;
				default:
					break;
			}
		}


		if (!must_wait || (waited == 25)) {
			if (force == 2) {
				if (remove(lockname)) {
					if (mode != 'q') {
					/*(void) printf("Could not remove lock %s\n",lockname);*/
					(void) printf("%s\n",Ustring[242]);
					}
					remove(my_lock);
					return FALSE;
				}
				continue;
			}
			(void)remove(my_lock);
			return(FALSE);
		}
		(void)sleep(1);
		waited++;
	}
	(void)remove(my_lock);
	return(TRUE);
}

int dsystem(const char *incommand) {
/* NB This returns its values the right way round unlike system() */
	pid_t x;

	x = fork();
	if (x < 0) {
		/* fork() failed */
		return -1;
	} else if (x > 0) {
		/* parent */
		int i;
		(void)wait(&i);
		return i;
	} else {
		/* child */
		char *argv[4];
#if defined(SVR42)
		argv[0] = strdup("/sbin/sh");
#else
		argv[0] = strdup("sh");
#endif
		argv[1] = strdup("-cp");
		argv[2] = strdup(incommand);
		argv[3] = 0;

		(void)execvp(argv[0],argv);
#if defined(DEVEL)
		perror(argv[0]);
#endif

		exit(1);
	}
	/* NOTREACHED */
}

int usystem(const char *incommand) {
/* NB This returns its values the right way round unlike system */
	pid_t x;

	x = fork();
	if (x < 0) {
		/* fork() failed */
		return -1;
	} else if (x > 0) {
		/* parent */
		int i;
		(void)wait(&i);
		return i;
	} else {
		/* child */
		char *argv[4];
#if defined(SVR42)
		(void)setgid(getgid());
		(void)setuid(getuid());
		argv[0] = strdup("/sbin/sh");
#else
#  if defined(LINUX)
		setregid(getgid(),getgid());
		setreuid(getuid(),getuid());
		argv[0] = strdup("sh");

#  endif
#endif
		argv[1] = strdup("-cp");
		argv[2] = strdup(incommand);
		argv[3] = 0;

		(void)execvp(argv[0],argv);
#if defined(DEVEL)
		perror(argv[0]);
#endif

		exit(1);
	}
	/* NOTREACHED */
}

size_t dlt(char *s, size_t maxsize, const struct tm *timeptr) {
	char zed[11];
	char temp[6];

	strftime(s,maxsize,"%Z",timeptr);
	shiftword(s,zed,11);
	if (!zed[0]) {
		strcpy(zed,"DLT");
	}


	strftime(temp,6," %Y",timeptr);
	strftime(s,(maxsize - 17),"%a %b %e %R ",timeptr);
	strcat(s,zed);
	strcat(s,temp);
	return strlen(s);

#if defined(ALL_FOOLS_DAY)
	/* A bit of code useful for All Fools Day instead of 4 lines above */
	strcpy(s,"Sat Apr 1 11:59:59 BST 1995");
	return strlen(s);
#endif
}

void run_err (const char *report) {
	char *date;

	if (report && report[0]) {
		date = shorttime(time(0));
#if defined(DEVEL)
		fprintf(stderr,"%s %s\n",date,report);
#endif
		free(date);
	}
}

char *shorttime (time_t t) {
	char outstring[MAINLINE];

	(void)strftime(outstring,MAINLINE,"%x %X",localtime(&t));
	return strdup(outstring);
}

int Dstrcmp (const char *string1, const char *string2) {
/* This returns false if they match in the same way as strcmp */
	char *string1a = strdup(string1);
	char *string2a = strdup(string2);
	int result = 0;
	
	tnt(string1a);
	tnt(string2a);
	
	if (!C.sensitive) {
		lower_string(string1a);
		lower_string(string2a);
	}
	
	result = strcmp(string1a,string2a);
	free(string1a);
	free(string2a);
	return result;
}
	
int menumatch (char *input, char *option) {
/* This is a quick way to match a user's input with a SINGLE character option
   menu.  It find the first printable character of the menu option and the 
   first of the user's input, and converts them into same case if the value
   of C.casesensitive is 0.
*/   
	char cinput[MAINLINE];
	char coption[MAINLINE];
	
	strcpy(cinput,input);
	strcpy(coption,option);
	
	tnt(cinput);
	tnt(coption);
	
	if (C.sensitive) {
		return (cinput[0] == coption[0]);
	} else {
		return (tolower(cinput[0]) == tolower(coption[0]));
	}
}	

void errorlog (const char *param) {
	FILE *LOG;
	char *date;
	char filename[MAINLINE + MAINLINE + 100];

	date = shorttime(time(0));
	
	sprintf(filename,"%s/errorlog",C.datadir);
	if ( (LOG=fopen(filename,"a")) ) {
		fprintf(LOG,"ERROR: %s %s, command %s, level %s\n",param,date,G.command,G.level);
		fclose(LOG);
	}

/*
	sprintf(filename,"echo ERROR: %s %s, command %s, level %s > %/errorlog",param,date,G.command,G.level,C.datadir);
	dsystem(filename);
*/
	free(date);

}
