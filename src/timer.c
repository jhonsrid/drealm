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
#include <unistd.h>
#if defined(SVR42)
#  include <time.h>
#else
#  include <sys/time.h>
#endif

#include <unistd.h>
#include <sys/stat.h>
#if defined(SVR42) || defined(LINUX_WITH_DGRAMS)
#  include <sys/socket.h>
#  include <sys/un.h>
#else
#  if defined(LINUX)
#    include <sys/ioctl.h>
#    include <termios.h>
#    include <fcntl.h>
#  endif
#endif
#if defined(SVR42)
#  include <sys/select.h>
int socket(int domain, int type, int protocol);
int bind(int sockfd, caddr_t sockname, int namelen);
int select(int nfds, fd_set *r, fd_set *w, fd_set *e, struct timeval *tv);
#  define _POSIX_SOURCE
#endif

#include "drealm_sig.h"


struct message {
	int msgtype;
	int content;
};

static int timeout_init(char *minutes, char *tmpdir, char *id, char *pid, char *usersdir);
static void accumulate_time(void);
static void tidy(void);
static void closedown(int sig);
static void hangup(int sig);
static void warning(int sig);
static int timeout(struct message *msg);
static int check_parent(void);
static void (*sigwas(int signum))(int);

static int incoming;
static char *udir;
static char *uid;
static int parent = 0;
static char endpoint[108];	/* see sys/un.h */
static pid_t pid_to_go;

static unsigned int stored_timeout = 0;


int main(int argc, char *argv[]) {
#if defined(SVR42) || defined(LINUX)
#  if defined(SVR42) || defined(LINUX_WITH_DGRAMS)
	struct sockaddr_un dgram_in;
#  endif
	struct message msg;
	fd_set fdset;
	int done = 0;
	struct timeval tv;
	pid_t p;

	if (argc != 6) {
#if defined(DEVEL)
		(void)fputs("timer: Invalid parameters.\n",stderr);
#endif
		exit(1);
	}

	if ((strlen(argv[2]) + strlen(argv[3]) + (size_t) 7) > (size_t) 108) {
#if defined(DEVEL)
		(void)fputs("timer: Parameters too long.\n",stderr);
#endif
		exit(1);
	}

	if (!timeout_init(argv[1],argv[2],argv[3],argv[4],argv[5])) {
		/* Only reason is endpoint exists - this is "okay" */
#if defined(DEVEL)
		(void)fputs("timer: already running?\n",stderr);
#endif
		exit(0);
	}

#  if defined(SVR42) || defined(LINUX_WITH_DGRAMS)
	if ((incoming=socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
#if defined(DEVEL)
		perror("timer: socket");
#endif
		(void)unlink(endpoint);
		exit(1);
	}

	dgram_in.sun_family = AF_UNIX;
	(void)strcpy(dgram_in.sun_path, endpoint);
	if (bind(incoming,
		(caddr_t) &dgram_in,
		/* LINTED */
		sizeof (short) + strlen(dgram_in.sun_path)))
	{
#if defined(DEVEL)
		perror("timer: bind");
#endif
		(void)close(incoming);
		(void)unlink(endpoint);
		exit(1);
	}
#  else
	if (mknod(endpoint,010666,0) < 0) {
#if defined(DEVEL)
		perror("timer: mknod");
#endif
		(void)unlink(endpoint);
		exit(1);
	}

	if ((incoming=open(endpoint,O_RDONLY | O_NONBLOCK,0)) < 0) {
#if defined(DEVEL)
		perror("timer: open");
#endif
		(void)unlink(endpoint);
		exit(1);
	}
#  endif


	/* process is in the foreground to this point.  We have successfully
	   started up.  We can now fork the main loop into the background
	   and return to the caller.
	 */
        
	p = fork();
	if (p < 0) {
		perror("timer: fork");
		close(incoming);
		(void)unlink(endpoint);
		exit(1);
	} else if (p > 0) {
		exit(0);
	}


	/*
	 * Now we're in the background, we can start the timer... not before!
	 */

	if (stored_timeout <= 120) {
		(void)Dsigset(SIGALRM,hangup);
	} else {
		(void)Dsigset(SIGALRM,warning);
	}
	(void)alarm(stored_timeout);


	while(!done) {
#  if defined(SVR42)
		/* LINTED */
		(void)FD_ZERO(&fdset);
#  else
		FD_ZERO(&fdset);
#  endif
		/* LINTED */
		FD_SET(incoming,&fdset);

		/* select() may reset these */
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		if (select(31,&fdset,NULL,NULL,&tv) > 0) {
			if (read(incoming, &msg, sizeof msg) >= 0) {
				done = timeout(&msg);
			} else {
#if defined(DEVEL)
				perror("timer: read");
#endif
				(void)close(incoming);
				(void)unlink(endpoint);
				exit(1);
			}
		} else {
			done = check_parent();
		}
	}

	tidy();
	exit(0);
#else
	(void)fputs("Timer not supported on this system.\n",stdout);
	if (parent) (void)kill((pid_t)parent,SIGUSR1);
	exit(2);
#endif

/* NOTREACHED */
}

static int timeout_init (char *minutes, char *tmpdir, char *id, char *pid, char *usersdir) {
	struct stat buf;
	FILE *F;
	unsigned int i;

	(void)Dsigset(SIGHUP,closedown);
	(void)Dsigset(SIGTERM,closedown);
	(void)Dsigset(SIGINT,SIG_IGN);

	/* If there is a conf.<user>, get the pid from it */
	(void)sprintf(endpoint,"%s/conf.%s",tmpdir,id);
	if (F = fopen(endpoint,"r")) {
		(void)fscanf(F," %d ",&parent);
		(void)fclose(F);
	} else {
		parent = 0;
	}

	i = abs(atoi(minutes));
	if (!i) return 0;
	stored_timeout = (i <= 2 ? i : i-2) * 60;

	pid_to_go = atoi(pid);
	if (!pid_to_go) return 0;

	udir = strdup(usersdir);
	uid = strdup(id);
	(void)sprintf(endpoint,"%s/time.%s",tmpdir,id);
	return stat(endpoint,&buf);
}


static void accumulate_time(void) {
	unsigned int remaining = alarm(0);
	unsigned int used = stored_timeout - remaining;
	char file_date[80];
	char file_time[80];
	char filename[1024];
	FILE *TIMED;
	

	sprintf(filename,"%s/%s/.timed",udir,uid);
	if (!(TIMED = fopen(filename,"r+"))) return;
	fscanf(TIMED, " %s ", file_date);
	fscanf(TIMED, " %s ", file_time);

	rewind(TIMED);
	fprintf(TIMED,"%s\n", file_date);	/* only drealmBBS updates this */
	fprintf(TIMED,"%d\n", (((atoi(file_time) * 60) + used + 30) / 60));
	fclose(TIMED);
}

static void tidy(void) {
	accumulate_time();
	(void)close(incoming);
	(void)unlink(endpoint);
}


/* ARGSUSED0 */
static void closedown(int sig) {
	tidy();
	exit(0);
}

/* ARGSUSED0 */
static void hangup(int sig) {
	(void)Dsigset(SIGALRM,SIG_IGN);
	tidy();
	(void)fputs("\n\nYou have been timed out.\n\n",stdout);
	/* we need to pass these in at some point
	printf("\n\n");
	printf("%s %s",Ustring[319],Ustring[281]);
	printf("\n\n");
	*/	
	if (parent) (void)kill((pid_t)parent,SIGUSR1);
	(void)kill(pid_to_go,SIGHUP);
	usleep(1000000);
	exit(0); /* if we get this far */
}

/* ARGSUSED0 */
static void warning(int sig) {
	(void)Dsigset(SIGALRM,SIG_IGN);
	(void)alarm(0);
	accumulate_time();

	(void)Dsigset(SIGALRM,hangup);
	stored_timeout = 120;
	(void)alarm(stored_timeout);

	(void)fputs("\n\aYou will be timed out in a further two minutes.\n",stdout);
	/* we need to pass these in at some point
	printf("\n\n");
	printf("%s ",Ustring[319]);
	printf(Ustring[278],2,Ustring[280]);
	printf("\n\n");
	*/
	if (parent) (void)kill((pid_t)parent,SIGUSR1);
}

static int timeout(struct message *msg) {
	static int paused = 0;
	unsigned int used;
	unsigned int new_timeout;
	unsigned int old_timeout;
	static unsigned int remaining = 0;

	switch(msg->msgtype) {
		case 1:		/* turn timer off */
			return 1;
		case 2:		/* suspend timer */
			if (!paused) {
				paused = 1;
				remaining = alarm(0);
			}
			break;
		case 3:		/* release timer */
			if (paused) {
				paused = 0;
				(void)alarm(remaining);
			}
			break;
		case 4:		/* think about adjusting time */
			if (!paused) remaining = alarm(0);
			used = stored_timeout - remaining;
			if (sigwas(SIGALRM) == warning) {
				old_timeout = stored_timeout + 120;
			} else {
				old_timeout = stored_timeout;
			}
			new_timeout = msg->content * 60;

			/* Don't ever increase... */
			if (new_timeout > old_timeout) break;

			if (new_timeout - used <= 0) hangup(SIGALRM);

			if (new_timeout >= 120) new_timeout -= 120;
			remaining = new_timeout - used;
			stored_timeout = new_timeout;

			if ((remaining <= 120) && (sigwas(SIGALRM) == warning)) warning(SIGALRM);
			if (!paused) (void)alarm(remaining);
			break;
	}
	return 0;
}

static int check_parent(void) {
	if (parent) 
		return kill((pid_t)parent,0);
	else
		return 0;
}

static void (*sigwas(int signum))(int) {
	struct sigaction sa;
	return sigaction(signum, NULL, &sa) ? SIG_ERR : sa.sa_handler;
}
