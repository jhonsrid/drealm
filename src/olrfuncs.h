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
/* MENU OPTIONS */

int   olr_here (char *dummy); /* The whole thing */

/*====================================*/

void  do_message_error(char *msg, char *filename);
void  do_put_error(char *msg, FILE *PUT);
void  do_run_error(char *msg);
int   grab_all_areas(char mode);
int   grab_all_mail(char mode);
int   is_msgsep(char *line);
int   make_message(FILE *PUT, char *filename);
int   olrput(char mode);
int   putareacomment(char mode,char *line,FILE *PUT);
int   putareamain(char cmd,char mode,char *params,FILE *PUT);
int   putareapost(char mode,char *line,FILE *PUT);
int   putareareply(char mode,char *line,FILE *PUT);
int   putmailpost(char mode,char *line,FILE *PUT);
int   putmailreply(char mode,char *line,FILE *PUT);
FILE *write_subject(char *msg);
