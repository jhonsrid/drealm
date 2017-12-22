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

#include <sys/stat.h>
#if defined(SVR42) || defined(LINUX_WITH_DGRAMS)
#  include <sys/socket.h>
#  include <sys/un.h>
#else
#  if defined(LINUX)
#    include <fcntl.h>
#  endif
#endif
#if defined(SVR42)
#  include <sys/select.h>
int socket(int domain, int type, int protocol);
int bind(int sockfd, caddr_t sockname, int namelen);
int select(int nfds, fd_set *r, fd_set *w, fd_set *e, struct timeval *tv);
#endif

#include "drealm_sig.h"

#include "colour.h"

struct message {
	int	signal;		/* operation code */
	char	text[1024];	/* text buffer */
};

struct msg_queue {
	struct msg_queue *next_msg;
	int signal;
	char *message;
};

static int chatout(struct message *msg);
static int chatout_init(enum colour_names rcolour,char *tmpdir, char *id);
static int check_parent(void);
static void printmsg(enum colour_names colour, char *text);
static void closedown(int sig);
static int incoming;
static int parent = 0;
static enum colour_names rec_colour = plain;
static char endpoint[108];	/* see sys/un.h */
static struct msg_queue root;

int main(int argc, char *argv[]) {
/* LENGTHS CHECKED */
#if defined(SVR42) || defined(LINUX)
#  if defined(SVR42) || defined(LINUX_WITH_DGRAMS)
	struct sockaddr_un dgram_in;
#  endif
	struct message msg;
	fd_set fdset;
	int done = 0;
	struct timeval tv;
	pid_t p;

	if (argc != 4) {
#if defined(DEVEL)
		(void)fputs("chatout: Invalid parameters.\n",stderr);
#endif
		exit(1);
	}

	if ((strlen(argv[2]) + strlen(argv[3]) + (size_t) 7) > (size_t) 108) {
#if defined(DEVEL)
		(void)fputs("chatout: Parameters too long.\n",stderr);
#endif
		exit(1);
	}

	if (!chatout_init((enum colour_names)atoi(argv[1]),argv[2],argv[3])) {
		/* Only reason is endpoint exists - this is "okay" */
#if defined(DEVEL)
		(void)fputs("chatout: already running?\n",stderr);
#endif
		exit(0);
	}

#  if defined(SVR42) || defined(LINUX_WITH_DGRAMS)
	if ((incoming=socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
#if defined(DEVEL)
		perror("chatout: socket");
#endif
		(void)unlink(endpoint);
		exit(1);
	}

	dgram_in.sun_family = AF_UNIX;
	(void)strcpy(dgram_in.sun_path, endpoint);
	if (bind(incoming,
		(const struct sockaddr *) &dgram_in,
		sizeof (short) + 
		/* LINTED */
		strlen(dgram_in.sun_path)))
	{
#if defined(DEVEL)
		perror("chatout: bind");
#endif
		(void)close(incoming);
		(void)unlink(endpoint);
		exit(1);
	}
#  else
	if (mknod(endpoint,010666,0) < 0) {
#if defined(DEVEL)
		perror("chatout: mknod");
#endif
		(void)unlink(endpoint);
		exit(1);
	}

	if ((incoming=open(endpoint,O_RDONLY | O_NONBLOCK,0)) < 0) {
#if defined(DEVEL)
		perror("chatout: open");
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
		perror("chatout: fork");
		close(incoming);
		(void)unlink(endpoint);
		exit(1);
	} else if (p > 0) {
		exit(0);
	}

	while(!done) {
#if defined(SVR42)
		/* LINTED */
		(void)FD_ZERO(&fdset);
#else
		FD_ZERO(&fdset);
#endif
		/* LINTED */
		FD_SET(incoming,&fdset);

		/* select() may reset these */
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		if (select(31,&fdset,NULL,NULL,&tv) > 0) {
			if (read(incoming, &msg, sizeof msg) >= 0) {
				done = chatout(&msg);
			} else {
#if defined(DEVEL)
				perror("chatout: read");
#endif				
				(void)close(incoming);
				(void)unlink(endpoint);
				exit(1);
			}
		} else {
			done = check_parent();
		}
	}

	closedown(0);
	exit(0);
#else
	(void)fputs("Chat not supported on this system.\n",stdout);
	exit(2);
#endif

/* NOTREACHED */
}

/* ARGSUSED0 */
static void closedown(int sig) {
/* LENGTHS CHECKED */
	(void)close(incoming);
	(void)unlink(endpoint);
	exit(0);
}

static int chatout_init (enum colour_names rcolour, char *tmpdir, char *id) {
/* LENGTHS CHECKED */
	struct stat buf;
	FILE *F;

	(void)Dsigset(SIGHUP,closedown);
	(void)Dsigset(SIGTERM,closedown);
	(void)Dsigset(SIGINT,SIG_IGN);

	root.next_msg = NULL;
	root.message = NULL;

	/* If there is a conf.<user>, get the pid from it */
	(void)sprintf(endpoint,"%s/conf.%s",tmpdir,id);
	if (F = fopen(endpoint,"r")) {
		(void)fscanf(F," %d ",&parent);
		(void)fclose(F);
	} else {
		parent = 0;
	}

 	switch (rcolour - 20) {
		case red:
		case green:
		case yellow:
		case blue:
		case magenta:
		case cyan:
		case bold:
		case multi:
		case plain:
			rec_colour = (rcolour - 20);
			break;
		default:
			rec_colour = plain;
			break;
	}

	(void)sprintf(endpoint,"%s/chat.%s",tmpdir,id);
	return(stat(endpoint,&buf));
}

static void queue_msg(struct msg_queue *mq, struct message *msg) {
/* LENGTHS CHECKED */
	if (mq->next_msg != NULL) {
		queue_msg(mq->next_msg,msg);
	} else {
		mq->next_msg = malloc(sizeof (struct msg_queue));
		mq = mq->next_msg;
		mq->next_msg = NULL;
		mq->signal = msg->signal;
		mq->message = (char *)malloc(strlen(msg->text)+2);
		(void)strcpy(mq->message,msg->text);
	}
}

static void dequeue_msg(struct msg_queue *mq,int silent) {
/* LENGTHS CHECKED */
	if (mq->message != NULL) {
		if (! silent) printmsg((enum colour_names)mq->signal,mq->message);
		free(mq->message);
		mq->message = NULL;
	}
	if (mq->next_msg != NULL) {
		dequeue_msg(mq->next_msg,silent);
		free(mq->next_msg);
		mq->next_msg = NULL;
	}
}

static void printmsg(enum colour_names msg_colour, char *text) {
/* LENGTHS CHECKED */
	enum colour_names colour;

	if (rec_colour == multi) {
		colour = msg_colour;
	} else {
		colour = rec_colour;
	}
	
	 switch (colour) {
		case red:     (void)printf("\r%c[K%c[1;31m%s%c[0m",27,27,text,27); break;
		case green:   (void)printf("\r%c[K%c[1;32m%s%c[0m",27,27,text,27); break;
		case yellow:  (void)printf("\r%c[K%c[1;33m%s%c[0m",27,27,text,27); break;
		case blue:    (void)printf("\r%c[K%c[1;34m%s%c[0m",27,27,text,27); break;
		case magenta: (void)printf("\r%c[K%c[1;35m%s%c[0m",27,27,text,27); break;
		case cyan:    (void)printf("\r%c[K%c[1;36m%s%c[0m",27,27,text,27); break;
		case bold:    (void)printf("\r%c[K%c[1m%s%c[0m",27,27,text,27); break;
		default:      (void)printf("\n%s",text); break;
	}
	(void)fflush(stdout);
	if (parent) (void)kill((pid_t)parent,SIGUSR1);
}

static int chatout(struct message *msg) {
/* LENGTHS CHECKED */
	static int queue = 0;

	if (msg->signal == 1) {	/* close down link */
		return 1;
	} else if (msg->signal == 2) {	/* enable message queuing */
		if (parent) (void)kill((pid_t)parent,SIGUSR2);
		queue = 1;
	} else if (msg->signal == 3) {	/* display queue and disable queuing */
		if (queue) dequeue_msg(&root,0);
		if (parent) (void)kill((pid_t)parent,SIGUSR2);
		queue = 0;
	} else if (msg->signal == 4) {	/* display queue */
		if (queue) dequeue_msg(&root,0);
		if (parent) (void)kill((pid_t)parent,SIGUSR2);
	} else if (msg->signal == 5) {	/* lose the queue */
		if (queue) dequeue_msg(&root,1);
		queue = 0;
	} else if ((msg->signal >= 30) && (msg->signal < 40)) {	/* set receive colour */
		rec_colour = (msg->signal - 20);
	} else { /* incoming message, possibly coloured */
		if (! queue) {
			printmsg((enum colour_names) msg->signal,msg->text);
			if (parent) (void)kill((pid_t)parent,SIGUSR2);
		} else {
			queue_msg(&root,msg);
		}
	}
	return 0;
}

static int check_parent(void) {
	if (parent) 
		return kill((pid_t)parent,0);
	else
		return 0;
}
