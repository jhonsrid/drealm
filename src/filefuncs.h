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
/* */
#define MAXPROTO 8

struct prototab {
	char *proto;
	char *protodln;
	char *protouln;
	char *protonarr;
	int protobatch;
};


int cfgfiles_parse (FILE *CFG, char F[][6][MAINLINE]);
void cfgfiles_read (char F[][6][MAINLINE]);
int   copy_desc (char *fromparent, char *toparent, char *file);
int   describe_file (char *dir,char *file);
int   dirmake (char *newdir);
int   is_dir_elig (char mode, char *params);
int   oktosend (char *wholefilename);
int   downloading (int bf, char *dir, char *files); 
char *uploading (char *dir, char *params);
int   dirrm (char *newdir);
int   down_file (int bf, char *in);

/*=====================================================*/
/* MENU COMMANDS */

int   catalogue (char *dummy);    /* Lists files in G.dir plus descriptions */
int   cleardir (char *dummy);     /* Gets rid of ALL directory information */
int   dir_create (char *in);      /* Parent dir in menu.  Name from either */
int   dir_remove (char *in);	  /* Only removes if no files */
int   do_describefile (char *in); /* Uses G.dir.  Name from input */
int   down_file_bf (char *in);	  /* Uses G.dir.  Name from input */
int   down_file_ubf (char *in);	  /* Uses G.dir.  Name from input */
int   down_special_bf (char *files);	  /* Full name from menu */
int   down_special_ubf (char *files);	  /* Full name from menu */
int   file_del (char *in);	  /* Uses G.dir.  Name from input */
int   file_rename (char *in);     /* Uses G.dir.  Name from input */ 
int   file_to_public (char *params); /* Parent of destination dir in menu.  Fromdir is G.dir. Name from input */
int   file_to_user (char *params);    /* As above but C.users is parent dir. */
int   list_dir(char *params);     /* Lists G.dir.  Type c,l,or x from input, defaults to 's' */
int   nestdir (char *in);         /* Uses G.dir as parent.  User chooses child to change to*/
int   parentdir (char *dummy);    /* Cd back one to parent. Won't pass starting directory */
int   poppfu (char *dummy);       /* Quickly return from private dir to previous G.dir*/
int   pushpfu (char *dummy);      /* Quickly cd to private dir */
int   startdir (char *params);    /* Replace whole dir stack with new starting dir */
#if 0
int   take_files (char *in);	  /* Get a file list either from G.comline or prompt */
#endif
int   up_file (char *in);         /* Upload a file to G.dir */

 
/*==========================================================*/
int search_filedescs (char *in);
int search_filenames (char *in);
void do_fds_help (void);
void do_fns_help (void);
void search_names (char recurse, int *stopped,int *nr_found,int *linecount, char *topdir, char *restdir, char *pattern);
void search_descs (char recurse,int *stopped,int *nr_found,int *linecount, char *topdir, char *restdir, char *pattern);
void recurse_dirs (char recurse, int *linecount,int *stopped, int *nr_found, char type, char *topdir, char *restdir, char *pattern);
int filesearch (char type, char recurse, char *copy);
