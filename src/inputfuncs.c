
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#if defined(SVR42)
#  include <sys/select.h>
int select(int nfds, fd_set *r, fd_set *w, fd_set *e, struct timeval *tv);
#endif
#include <ctype.h>
#include <termios.h>

#include "drealm_sig.h"

#include "drealm.h"
#include "drealmgen.h"
#include "mainfuncs.h"
#include "inputfuncs.h"
#include "configfuncs.h"
#include "setupfuncs.h"
#include "genfuncs.h"
#include "chatfuncs.h"

static int Timed_out = 0;
static struct termios storedterm;

int set_echo(char *input) {
/* MENU COMMAND */
	int mode = atoi(input);
	if (mode >= 0 && mode <= 2) {
		G.echo = mode;
		return 1;
	} else {
		return 0;
	}
}

void make_prompt (char *promptstring) {
	strncpy(G.prompt,promptstring,MAINLINE);
	G.prompt[MAINLINE -1] = 0;
}

void put_prompt (void) {
	printf("%s ",G.prompt);
}

void store_term(void) {
	tcgetattr(0,&storedterm);
}

void restore_term(void) {
	if (storedterm.c_cflag & CBAUD) {
		tcsetattr(0,TCSAFLUSH,&storedterm);
	}
}


void internal_term(void) {
#if defined(SVR42)
	system("stty min 1 time 0 -icanon -echo susp '^-' dsusp '^-' quit '^-'");
#else
#  if defined(LINUX)
	system("stty min 1 time 0 -icanon -echo susp '^-' quit '^-'");
#  endif
#endif
}

void external_term(void) {
	system("stty icanon echo");
}

/* ARGSUSED0 */
void user1_handler (int sig) {
	G.usr1flag++;
}

void user1_on (void) {
	Dsigset(SIGUSR1,user1_handler);
	G.usr1flag = 0;
}

void user1_off (void) {
	Dsigset(SIGUSR1,SIG_IGN);
	G.usr1flag = 0;
}

/* ARGSUSED0 */
void user2_handler (int sig) {
	G.usr2flag++;
}

void user2_on (void) {
	Dsigset(SIGUSR2,user2_handler);
	G.usr2flag = 0;
}

void user2_off (void) {
	Dsigset(SIGUSR2,SIG_IGN);
	G.usr2flag = 0;
}

/* ARGSUSED0 */
void int_handler (int sig) {
	G.intflag++;
}

void intr_on (void) {
	Dsigset(SIGINT,int_handler);
	G.intflag = 0;
	G.intrs = 1;
}

void intr_off (void) {
	Dsigset(SIGINT,SIG_IGN);
	G.intflag = 0;
	G.intrs = 0;
}

/* ARGSUSED0 */
void hup_handler (int sig) {
	G.hupflag++;
}

void hups_on (void) {
	if (G.hupflag) {
		do_exit(1);
	}
	Dsigset(SIGHUP,SIG_DFL);
	G.hups = 1;
}

void hups_off (void) {
	Dsigset(SIGHUP,hup_handler);
	G.hupflag = 0;
	G.hups = 0;
}

static int drealm_getchar(int *timeout) {
	fd_set ri;
	int i;

#if defined(SVR42)
	/* LINTED *//* casts to (char *) */
	(void)FD_ZERO(&ri);
#else
	FD_ZERO(&ri);
#endif
	/* LINTED *//* fd_mask is signed for no apparent reason */
	FD_SET(0,&ri);
	Timed_out = 0;
	G.usr1flag = 0;
	G.intflag = 0;
	for(;;) {
		if (*timeout) {
			struct timeval tv = { *timeout, 0 };

			/* tv must be updated by select!! */
			i = select(31,&ri,NULL,NULL,&tv);
			*timeout = tv.tv_sec;
			if (*timeout < 0) {
				*timeout = 0;
			}
		} else {
			i = select(31,&ri,NULL,NULL,NULL);
		}
		if (i < 0) {
			return -1;
		} else if (i == 0) {
			Timed_out++;
			return 0;
		} else {
			int j;
			j = read(0,&i,1);
			if (j < 0) {
				return -1;
			} else if (j == 0) {
				return 0;
			} else {
				return i;
			}
		}
	}
}

static void reprint_input(char *string) {
	putchar('\n');
	put_prompt();
	if (G.echo == 1) {
		fputs(string, stdout);
	} else if (G.echo == 2) {
		int j;
		for (j=0;string[j];j++) {
			putchar('*');
		}
	}
#if defined(LINUX)
	fflush(stdout); /* this should not be needed - stdout is unbuffered! */
#endif
}

void get_raw (int waitcr, size_t max_i, char *outstring, int restrictch) {
/*
 * waitcr	Wait for newline if non-zero, otherwise hotkey
 * max_i	Max number of characters to accept
 * outstring	Where to put the characters accepted
 * restrictch	0 - all keys allowed
 *		1 - isprint() plus stuff in C.extras
 *		2 - isdigit() only
 *		3 - y/n only
 *	       -1 - all keys allowed and do not translate editing characters
 */
	char temp[MAINLINE];
	int c;
	int i = 0;
	int times_out = 0;
	int remaining = 0;

	max_i--;

	if (! outstring) {
		strcpy(G.errmsg,"No output pointer for get_raw.  Please report bug to author\n");
		errorlog(G.errmsg);
		/*printf("\n!!!Program error!!! Ask SysOp to check error log.\n");*/
		printf("\n%s %s\n",Ustring[276],Ustring[277]);
		return;
	}

	outstring[0] = 0;
	remaining = (U.timeout > 30)? U.timeout - 30 : U.timeout;

	user1_on();
	put_prompt();
	while (waitcr || (i < max_i)) {
		while (G.usr1flag) {
			G.usr1flag = 0;
			outstring[i] = 0;
			reprint_input(outstring);
		}

		c = drealm_getchar(&remaining);

		if (c < 0) {
			if (G.intflag) { 
				i = 0;
				break;
			}
			continue;	/* mustn't drop thru */
		} else if (c == 0) {
			if (Timed_out) {
				if (!times_out) {
					/*printf("\nYou have nearly been timed out.\n\a");*/
					sprintf(temp,Ustring[279],30);
					printf("\n%s ",Ustring[282]);
					printf(Ustring[278],temp);
					printf("\n\a");
					outstring[i] = 0;
					reprint_input(outstring);
					times_out++;
					remaining = 30;
				} else {
					/*printf("\nYou have been timed out.\n");*/
					printf("\n%s %s\n\a",Ustring[282],Ustring[281]);
					restore_term();
					exit(0); /* this should clear up */
				}
			}
			continue;	/* mustn't drop thru! */
		}

		/*
		 * c > 0 - i.e. we read something
		 */
		times_out = 0;
		remaining = (U.timeout > 30)? U.timeout - 30 : U.timeout;

		if (c == '\n') {
			break;
		}

		/* If we're doing editing, handle it now */
		if (restrictch != -1) {
			if ((c == U.erase) || (c == 8) || (c == 127)) {
				if (i) {
					i--;
					if (G.echo) printf("\b \b");
				}
				continue;
			} else if (c == U.werase) {
				while(i && (outstring[i-1] == ' ')) {
					i--;
					if (G.echo) printf("\b \b");
				}
				while(i && (outstring[i-1] != ' ')) {
					i--;
					if (G.echo) printf("\b \b");
				}
				continue;
			} else if (c == U.kill) {
				while(i) {
					i--;
					if (G.echo) printf("\b \b");
				}
				continue;
			} else if (c == U.reprint) {
				outstring[i] = 0;
				reprint_input(outstring);
				continue;
			}
			/* else drop through */
		}

		/* okay, handle everything else as typed */
		if (i < max_i) {
			if ( 
				   (restrictch < 1)
				|| ((restrictch == 1) && (isprint(c) || strchr(C.extras,c)))
				|| ((restrictch == 2) && isdigit(c))
				|| ((restrictch == 3) && strchr(G.yesno,c))
				|| (restrictch > 3)
			) {
				outstring[i++] = (char)c;
				if (G.echo == 1) {
					putchar(c);
				} else if (G.echo == 2) {
					putchar('*');
				}
			}
		}
	}
	user1_off();
	outstring[i] = 0;
	putchar('\n');
	if (G.rawlog) {
		char *date = shorttime(time(0));
		fprintf(RAWLOG,"%s %s %s %s\n",date,U.id,G.prompt,outstring);
		fflush(RAWLOG);
		free(date);
	}
}

void get_commandline (char *comline) {
	unsigned int max = (U.hotkeys ? 2 : MAINLINE);
	int wait = (U.hotkeys ? 0 : 1);

	get_raw(wait,max,comline,1);
}


void get_one_line (char *in) {
	get_raw(1,MAINLINE,in,1);
}


int get_one_num (size_t maxln, int deflt) {
	char instring[11];
	instring[0] = 0;

	if (maxln > 10) {
		maxln = 10;
	}
	get_raw(1,maxln+1,instring,2);
	return (instring[0] ? atoi(instring) : deflt);
}

void get_one_param (char *outstring, size_t maxln, char *temp) {
/* get first word out of string, return lc */

	int i = 0; /* input offset */
	int j = 0; /* output offset */
	char *instring = (char *)malloc(maxln);

	if (temp[0] == 0) {
		get_raw(1,maxln,instring,1);
	} else {
		strncpy(instring,temp,maxln);
		instring[maxln - 1] = 0;
	}

	while (instring[i] && (instring[i] == ' ')) {
		i++;
	}
	while (instring[i] && (instring[i] != ' ') && (j < (maxln-1))) {
		if (isgraph(instring[i])) {
			outstring[j] = tolower(instring[i]);
			j++;
		}
		i++;
	}
	outstring[j] = 0;
	free(instring);
}


void new_get_one_param (char mode, char *prompt, char *in_instring, char *outstring, size_t maxln) {
/* get first word out of string, return tnt and lc */

	int i = 0; /* input offset */
	int j = 0; /* output offset */
	char *instring = strdup(in_instring);

	shiftword(instring,outstring,maxln);
	free(instring);

	if ((outstring[0]) || (mode != 'v')) {
		lower_string(outstring);
		return;
	}
	make_prompt(prompt);
	get_raw(1,maxln,outstring,1);
	if (outstring[0] == 0) {
		return;
	}

	while (outstring[i] && (outstring[i] == ' ')) {
		i++;
	}
	while (outstring[i] && (outstring[i] != ' ') && (j < (maxln-1))) {
		if (isgraph(outstring[i])) {
			outstring[j] = tolower(outstring[i]);
			j++;
		}
		i++;
	}
	outstring[j] = 0;
}

void get_one_name (char *outstring, size_t maxln, char *temp) {
/* allows lowercase alphanumeric, and _  */
	int i = 0; /* input offset */
	int j = 0; /* output offset */
	char *instring = (char *)malloc(maxln);

	if (temp[0] == 0) {
		get_raw(1,maxln,instring,1);
	} else {
		strncpy(instring,temp,maxln);
		instring[maxln - 1] = 0;
	}

	while (instring[i] && (instring[i] == ' ')) {
		i++;
	}
	while (instring[i] && (instring[i] != ' ') && (j < (maxln-1))) {
		if (isalnum(instring[i]) || (instring[i] == '_')) {
			outstring[j] = tolower(instring[i]);
			j++;
		}
		i++;
	}
	outstring[j] = 0;
	free(instring);
}

void get_one_area (char *outstring, size_t maxln, char *temp) {
/* allows lowercase alphanumeric, . and _  */
	int i = 0; /* input offset */
	int j = 0; /* output offset */
	char *instring = (char *)malloc(maxln);

	if (temp[0] == 0) {
		get_raw(1,maxln,instring,1);
	} else {
		strncpy(instring,temp,maxln);
		instring[maxln - 1] = 0;
	}
	while (instring[i] && (instring[i] == ' ')) {
		i++;
	}
	while (instring[i] && (instring[i] != ' ') && (j < (maxln-1))) {
		if (isalnum(instring[i]) || (instring[i] == '_') || ((i > 0) && (instring[i] == '.'))) {
			outstring[j] = tolower(instring[i]);
			j++;
		}
		i++;
	}
	outstring[j] = 0;
	free(instring);
}

void get_one_file (char *outstring, size_t maxln, char *temp) {
/* allows all alphanumeric upper and lower, _ . - ! ~  */
	int i = 0; /* input offset */
	int j = 0; /* output offset */
	char *instring = (char *)malloc(maxln);

	if (temp[0] == 0) {
		get_raw(1,maxln,instring,1);
	} else {
		strncpy(instring,temp,maxln);
		instring[maxln - 1] = 0;
	}

	while (instring[i] &&
		 (
		   ( (instring[i] == ' ') || (instring[i] == '/') )
		   ||
		   ( (instring[i] == '.') && (U.level < C.sysoplevel) )
		 )
		) {
		i++;
	}
	while (instring[i] && (instring[i] != ' ') && (j < (maxln-1))) {
		if (isalnum(instring[i]) || strchr("_-.!~",instring[i])) {
			outstring[j] = instring[i];
			j++;
		}
		i++;
	}
	outstring[j] = 0;
	free(instring);
}

void get_one_char (char *outstring) {
	get_raw(U.hotkeys ? 0 : 1, 2, outstring, 1);
}

void get_one_lc_char (char *outstring) {
	get_raw(U.hotkeys ? 0 : 1, 2, outstring, 1);
	lower_string(outstring);
}

void get_yn (char *outstring) {
	get_raw(U.hotkeys ? 0 : 1, 2, outstring, 3);
	lower_string(outstring);
}

void get_one_anything (char *outstring) {
	get_raw(U.hotkeys ? 0 : 1, 2, outstring, 0);
}


