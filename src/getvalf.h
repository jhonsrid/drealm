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
struct valid_files {
	char *input;		/* What the user typed in (or passed) */
	char *files;		/* What the input expanded to */
};

struct valid_files *get_valid_entries(
	const char mode,	/* 'q' for quiet; 'v' for verbose */
	const int nr_matches,	/* max matches to return (0 = unlimited) */
	const char *prompt,	/* text to put in messages */
	const char *dirname,	/* directory to search */
	const char *params,	/* list of files to check */
	const char valid_what,	/* 'f' for regular files, 'd' for directories,
				   anything else gets everything */
	const int case_sensitive	/* don't ignore case */
);
struct valid_files *get_valid_files(char mode,int many,char *word,char *thisdir,char *fromstring, int ignore); /* returns only regular files */
struct valid_files *get_valid_dirs(char mode,int many,char *word,char *thisdir,char *fromstring, int ignore); /* returns only directories */
struct valid_files *get_valid_both(char mode,int many,char *word,char *thisdir,char *fromstring, int ignore); /* returns anything at all */
int list_files(char *dirname);
int list_dirs(char *dirname);
int list_both(char *dirname);
