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
struct message {
	int	signal;		/* operation code */
	char	text[1024];	/* text buffer */
};

int   chatin (char *recipient, char *msg);
int   chatmess (char *type);
int   is_online (char *id,char type);
char *onlist (char *indicator);
int   set_chatdoing(char *chat_string);
int   show_chat (char *dummy);
void  socketsend(const char *recipient, const int msgtype, const char *text);
int   sub_chatoff (void);
int   sub_chaton (void);
void  chatlog(char *msg);
int   check_faculties(void);


int   broadcast (char *line);
int   chatdisable (char *dummy);
int   chatenable (char *dummy);
int   chatoff (char *dummy);
int   chaton (char *dummy);
int   chatqueue (char *dummy);
int   chatrelease (char *dummy);
int   emote (char *dummy);
int   fx (char *dummy);
int   say (char *dummy);
int   whisper (char *dummy);
int   whois (char *in);
int   whoson (char *dummy);
