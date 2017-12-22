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
int   	trans_string (const char *in_line, char *outline, size_t outlength);
char 	*base_name (const char *path, const char divider);
void  	capitalise(char *string);
int  	*combinenums (const int *firstlist, const int *secondlist);
char 	*concat(char *const params, char *const comline);
char 	*dir_name (const char *path, const char divider);
char 	*dir_search(const char *filename);
int   	dos_match(char *pattern, char *name);
char 	*getfield(const char *filename, const char *field);
int   	gettp(char *tpdev, char *realdev);
void  	grepnums (int *mainlist,const int *sublist);
int   	intsort(const void *a, const void *b);
int   	is_in_list (const char *filename,const char *value);
int   	is_num(const char *string);
void  	lower_string(char *string);
void  	numtostr(char *string);
int   	rem_lock(const char *file);
int   	shiftnum (int *list);
int   	shiftword(char *bigstring, char *smallstring, const size_t smallmax);
int   	string_in_file(const char *string,const char *file);
int  	*string_to_list (const char *instring);
int   	strshift(char *bigstring, char *smallstring, const size_t smallmax, const char *splitstring);
int   	menushift(char *bigstring, char *smallstring, const size_t smallmax, const char *splitstring, size_t *got_to);
void  	tnt(char *string);
void  	upper_string(char *string);
int	strsort(const void *x, const void *y);
void  	random_init(void);
int 	get_int_from_file (char *filename);

#define isdelim(x)	(isspace(x) || (x == ','))
