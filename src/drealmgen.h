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
struct line_details {
	int ln;
	char dev[MAINLINE + 50];
	char nicename[81];
	int exc;
	int only;
	int inc;
	int level;
	int shells;
	char errorstring[MAINLINE];	
	char promptstring[MAINLINE];
	char screenfile[MAINLINE + 50];
};

void errorlog (const char *param);
int comp_flags (char *m_flags, char *flags);
int get_hour (void);
int nicedate (char *outstring);
int nicetime (char *outstring);
int is_line_elig (struct line_details *ld,char *user);
struct line_details *get_line_details (char *dev);
int copy_file (char *origfile, char *targetfile, int filter);
int append_file (char *origfile, char *targetfile, int filter);
char *drealmtime (time_t t);
int place_lock(const char mode, const char *lockname,int must_wait,int force);
int dsystem(const char *incommand);
int usystem(const char *incommand);
size_t dlt(char *s, size_t maxsize, const struct tm *timeptr);
void  run_err (const char *report);
char *shorttime (time_t t);
int Dstrcmp (const char *string1, const char *string2);
int menumatch (char *option, char *input);
