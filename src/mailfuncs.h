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

struct mailheader {
	char *replyto;	/* Message this is a reply to */
	char *read;	/* 'R' for read, 'U' for unread */
	char *hash;	/* "#" */
	char *number;	/* Message number for display */
	char *dname;	/* Day name */
	char *mname;	/* Month name */
	char *dnum;	/* Day of month */
	char *time;	/* Time of day */
	char *dtz;	/* Timezone */
	char *year;	/* Year */
	char *from;	/* "from" */
	char *author;	/* name of sender */
	char *to;	/* "to" */
	char *recip;	/* Name of sender of parent */
	char *dash;	/* "-" */
	char *subject;	/* multi-word subject */
};
struct valid_mail {
	char *parse;
	int *msglist;
};
 
int    copy_maillist (char mode, char *to, int *fromlist);
char  *definemail(char mode, char *user, int msgno);
int    delete_maillist (char mode, int confirm, struct valid_mail *vm);
int    displaymail (char *user, int msgno);
int   *findnextmail(char mode, char *user, int current);
int   *findorigmail(char mode, char *user, int current);
int   *findspecmail(char mode, char *user, int current);
int   *get_mail(const char *opt, const char *recip);
int    get_unix_mail (char mode,char *user);
void   grabmail(char mode,char *user,int *msglist);
void   workmail(char mode,char *user,int *msglist);
char  *mail_check_recips(char mode, char *params);
void   maildesclist(void);
void   parse_mail_header(char *header, struct mailheader *mh);
int    send_mail_off(char *ffield,char *messagefile,char *sender, char *subject, char *mailwho);
struct valid_mail *get_valid_mail(char mode, char *user, char *inprompt, int current, char *inparams);
void   get_unix_mail_id(char *string);
int    markunread (int msgno);
void   markasread (int msgno);

int    any_unix_mail(char *dummy); /* Voluntary checking for unix mail */
int    copymail (char *in);     /* Copies mail list to area */
int    mailforward (char *in);	/* Copies mail to a person */
int    mailcheck(char *dummy); 	/* Auto-checking for mailalert */
int    maildelete(char *in);	/* Takes a number or list from input */
int    mailgrab(char *in);	/* grabs to grabpad */
int    mailquote(char *in);	/* grabs to workpad */
int    maillist(char *in);	/* lists by parameter */
int    mailpost(char *in);	/* post */
int    mailread(char *in);	/* read by input */
int    mailreply(char *in);	/* reply */
int    restore_mail (char *dummy);
int    store_mail (char *dummy);
int    unix_mailcheck (char *dummy); /* Auto-checking for JUST ARRIVED unix mail */
int    unread (char *in);
