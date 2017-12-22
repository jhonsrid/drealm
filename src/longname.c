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

#ifndef USE_TERMCAP

#include <curses.h>
#include <term.h>

int main(int argc,char *argv[]) {
	int rc;
	use_env(FALSE);
	(void)setupterm(argv[1],1,&rc);
	if (rc == 1) {
		(void)printf("%d %d %s\n",lines,columns,longname());
		return 0;	
	} else {
		return 1;
	}
}


#else	/* TERMCAP requested - oh dear oh dear oh dear... */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>

static char *longname(const char *term);
static char *longname_file(const char *term, const char *termcap);

static char *longname(const char *term) {
	char *TC=getenv("TERMCAP");
	char *TERM=getenv("TERM");
	char *p, *q;
	int found = 0;
	
	if (!TC) return longname_file(term,"/etc/termcap");
	if (TC[0] == '/') return longname_file(term,TC);
	if (strcmp(term,TERM)) return longname_file(term,"/etc/termcap");

	q = p = TC;
	while(*p && (*p != ':') && (*p != '\n')) {
		if (*p == '|') {
			*p = '\0';
			if (!strcmp(q,term)) ++found;
			q = ++p;
		}
		++p;
	}
	*p = '\0';
	if (!strcmp(q,term)) ++found;
	if (found) return q;
	else return TERM;
}

static char *longname_file(const char *term, const char *termcap) {
	FILE *TC = fopen(termcap,"r");
	static char buffer[1024];
	char *p, *q;
	int found = 0;
	
	if (TC == NULL) return NULL;
	setvbuf(TC, NULL, _IOFBF, BUFSIZ * 256);
	while(fgets(buffer, 1024, TC)) {
		if (strchr("#\n\t ",buffer[0])) continue;
		q = p = buffer;
		while(*p && (*p != ':') && (*p != '\n')) {
			if (*p == '|') {
				*p = '\0';
				if (!strcmp(q,term)) ++found;
				q = ++p;
			}
			++p;
		}
		*p = '\0';
		if (!strcmp(q,term)) ++found;
		if (found) {
			fclose(TC);
			return q;
		}
	}
	fclose(TC);
	return NULL;
}

int main(int argc,char *argv[]) {
	char bp[1024], *TERM=(argc > 1) ? argv[1] : getenv("TERM");
	int rc = tgetent(bp, TERM);

	if (rc != 1) return 1;

	(void)printf("%d %d %s\n",tgetnum("li"),tgetnum("co"),longname(TERM));

	return 0;
}

#endif
