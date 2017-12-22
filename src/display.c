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

/* ANSI headers */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Non-ANSI headers */
#include <unistd.h>
#include <sys/ioctl.h>

#if !defined(USE_TERMCAP)
#  if defined(SVR42)
#    include <sys/ttold.h>
#  endif
#  include <curses.h>
#  include <term.h>
#else
#  include <curses.h>
#endif

#include "display.h"
#include "drealm_sig.h"

int TOT_LINES;
int LINES;
int WIDTH;
#if defined(USE_TERMCAP)
static char TC[1024];
static char ST[1024], *_ST = ST, *cursor_up, *clr_eol;
#define putp(x)	tputs(x, 1, putchar)
#endif

static void winch_handler(int sig); /*What is this? An engineering project?*/

int get_LW(int tty) {
	struct winsize winsz;
#if defined(USE_TERMCAP)
	int lines=atoi(getenv("LINES")), columns=atoi(getenv("COLUMNS"));
	char *TERM=getenv("TERM");
#endif
	/*
	 * Attempt to read terminal size from tty.
	 * If this fails, set size to zero so we can ignore it later.
	 * If it works, catch changes to window size.
	 */
	if (ioctl(1,TIOCGWINSZ,&winsz) != 0) {
		winsz.ws_row = 0;
		winsz.ws_col = 0;
	} else {
		Dsigset(SIGWINCH,winch_handler);
	}

#if !defined(USE_TERMCAP)
	(void)setupterm(NULL,1,NULL);
	LINES=(winsz.ws_row)?winsz.ws_row:((lines)?lines:24);
	WIDTH=(winsz.ws_col)?winsz.ws_col:((columns)?columns:80);
#else
	switch(tgetent(TC,TERM)) {
		case -1:
#if defined(DEVEL)
			perror("Could not open TERMCAP");
#endif
			return 0;
			/* NOTREACHED */
		case 0:
#if defined(DEVEL)
			fprintf(stderr,"'%s': Unknown terminal type.\n",TERM);
#endif
			return 0;
			/* NOTREACHED */
		default:
			break;
	}
	lines=(winsz.ws_row)?winsz.ws_row:((lines)?lines:tgetnum("li"));
	columns=(winsz.ws_col)?winsz.ws_col:((columns)?columns:tgetnum("co"));
	LINES=(lines)?lines:24;
	WIDTH=(columns)?columns:80;
	cursor_up = tgetstr("up",&_ST);
	clr_eol = tgetstr("ce",&_ST);
#endif
	return 1;
}

/*
 * BODY is the file to display
 * esc_type is -1: do not paginate
 *              0: decide yourself
 *              1: paginate
 */
int pager(FILE *BODY, int esc_type,
	const char *Continue, const char Yes, const char No) {
/* LENGTHS CHECKED */
	char reply[BUFSIZ];
	char buffer[BUFSIZ];
	char lastbyte = 0;
	unsigned int bpos;	/* offset from start of buffer to start of line */
	int esc;	/* flag: are we treating file as ANSI? */
	int lpos;	/* current screen column (hopefully) */
	unsigned int nr;	/* number of bytes read */
	unsigned int pos;	/* offset from start of line to current character */

	switch (esc_type) {
		case -1: esc = 1; break;
		case  1: esc = 0; break;
		default: esc = !LINES; break;	/* LINES == 0 means output to file, so don't page */
	}
		
	nr = fread(buffer,sizeof (char),BUFSIZ,BODY);
	if ((esc_type == 0) && (*buffer == '\033'))
		esc++;	/* First byte is ESC, so don't page */

	bpos = 0;
	lpos = 0;
	while(nr > 0) {
		while(bpos < nr) {
			pos = 0;
/* only do the while loop inside the if esc == 0 bit */
			while( ((bpos+pos) < nr) && (esc == 0) && (lpos < (WIDTH - 1)) && (buffer[bpos+pos] != '\n') ) {
				if (buffer[bpos+pos] == '\t') {
					while((lpos % 8) != 7) lpos++;
				}
				lpos++;
				pos++;
			}
			if (esc == 0) {
				/* We've got here because:
					a) We hit end of buffer
					b) We hit WIDTH
					c) We got a newline char
				 */
				lastbyte=buffer[bpos+pos];
				if ( (bpos+pos) >= nr ) {
					/* case a */
					(void)fwrite(&buffer[bpos],sizeof (char),pos,stdout);
					bpos += pos;
					continue;
				}
				if (lpos >= (WIDTH - 1)) {
					/* case b */
					int oldpos = pos;
					while(pos && isspace(buffer[bpos+pos-1]))
						pos--;
					if (pos == oldpos)
						while(pos && !isspace(buffer[bpos+pos-1]))
							pos--;
					if (!pos) pos=oldpos;
					(void)fwrite(&buffer[bpos],sizeof (char),pos,stdout);
					(void)putchar('\n');
				} else {
					/* case c */
					pos++; /* to include the newline */
					(void)fwrite(&buffer[bpos],pos,1,stdout);
				}
				bpos += pos;
				lpos = 0;
				TOT_LINES++;
				if (TOT_LINES == (LINES - 2)) {
					(void)printf("%s %c/%c ", Continue, toupper(Yes), tolower(No));
					(void)fgets(reply,BUFSIZ,stdin);
					reply[0] = tolower(reply[0]);
					if (reply[0] == No) return 1;
					if (cursor_up && clr_eol) {
						(void)putp(cursor_up);
						(void)putp(clr_eol);
					}
					TOT_LINES = 0;
				}
			} else {
				(void)fwrite(&buffer[bpos],nr - bpos,1,stdout);
				lastbyte = buffer[nr-1];
				bpos = nr;
			}
		}
		nr = fread(buffer,sizeof (char),BUFSIZ,BODY);
		bpos = 0;
	}
	if ((lastbyte != '\n') && (!esc)) (void)putchar('\n');
	return 1;
}

void page_file(char *filename,
	const char *Continue, const char Yes, const char No) {
/* LENGTHS CHECKED */
	FILE *FIL;

	if (!get_LW(1)) {
		return;
	}
	if (FIL = fopen(filename,"r")) {
		(void)pager(FIL,0,Continue,Yes,No);
		(void)fclose(FIL);
	}
}

static void winch_handler(int sig) {
	(void)get_LW(1);
}
