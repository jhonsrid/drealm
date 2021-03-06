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
struct valid_members {
	char *input;		/* What the user typed in (or passed) */
	char *members;		/* What the input expanded to */
};

struct valid_members *get_valid_members(
	const char mode,		/* 'q' for quiet; 'v' for verbose */
	const int nr_matches,		/* max matches to return
					   (0 = unlimited) */
	const char *prompt,		/* text to put in messages */
	const char *memberlist,		/* file to search */
	const char *params,		/* list of names to check */
	const int case_sensitive	/* true to make case matter */
);

int list_members(const char *memberlist);
