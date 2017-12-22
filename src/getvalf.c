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
#include <time.h>

/* Non-ANSI headers */
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#if defined(SVR42)
#  include <libgen.h>
#else /* !SVR42 */
#  if defined(LINUX)
#    include <regex.h>
#  endif /* LINUX */
#endif

/* Local headers */
#include "drealm.h"
#include "drealmgen.h"
#include "mainfuncs.h"
#include "inputfuncs.h"
#include "configfuncs.h"
#include "setupfuncs.h"
#include "genfuncs.h"
#include "display.h"
#include "slist.h"

#include "getvalf.h"

/* ------------------------------------------------------------------------- */

static int _get_valid_files(
	const char mode,			/* 'q' for quiet;
						   'v' for verbose */
	const int nr_matches,			/* max matches to return
						   (0 = unlimited) */
	const char *prompt,			/* text to put in messages */
	const char *dirname,			/* directory to search */
	struct slist **const params,		/* list of files to check */
	const char valid_what,			/* 'd' for directories;
						   'f' for regular files;
						   other gets everything */
	const int case_sensitive		/* don't ignore case in
						   compares */
);
static int glob(struct slist **const list, struct slist **const l,
	const char *dirname, const char mode, const char *prompt,
	const char valid_what, const int case_sensitive);
/* == START == */
static int sortdir(const char *dirname,
#if defined(SVR42)
	char *regexp,
#else /* !SVR42 */
#  if defined(LINUX)
	regex_t *regexp,
#  else /* !LINUX */
	char *regexp,
#  endif
#endif
	char ***p, const char valid_what, const int case_sensitive);
/* == END == */
static int list_entries(const char *dirname, const char what);
/* ------------------------------------------------------------------------- */

struct valid_files *get_valid_entries(const char mode, const int nr_matches,
	const char *prompt, const char *dirname, const char *params,
	const char valid_what, const int case_sensitive)
{
	struct valid_files *vf;
	DIR *dir;
	char word[MAINLINE];
	int i;
	struct stat sb;
	FILE *L;
	char filename[MAINLINE + 100];

	/* This is where the list of files will end up */
#if !defined(LINUX)
	sprintf(filename,"%s/filelist.%ld",C.tmpdir,getpid());
#else
	sprintf(filename,"%s/filelist.%d",C.tmpdir,getpid());
#endif

	vf = (struct valid_files *)malloc(sizeof (struct valid_files));

	if (!dirname[0]) {
		if (mode == 'v') {
			/*printf("No directory specified\n");*/
			printf(Ustring[294],Ustring[247]);
			printf("\n");
		}
		errorlog("get_valid_entries: no directory name");
		vf->input = strdup(params);
		vf->files = strdup("");
		return vf;
	}

	if (dir = opendir(dirname)) {
		closedir(dir);
	} else {
		if (mode == 'v') {
			/*printf("Invalid directory specified\n");*/
			printf("%s\n",Ustring[245]);
		}
		vf->input = strdup(params);
		vf->files = strdup("");
		return vf;
	}

	if (!params[0] && (mode == 'v')) {
	/* Nothing entered and we're allowed to talk to the user */
		char *promptstring;
		char number[5];

		/*promptstring = (char *)malloc(15 + 12 + strlen(prompt) + 20);*/
		vf->input=(char *)malloc(MAINLINE);
		/*strcpy(promptstring,"Please specify ");*/
		switch(nr_matches) {
			case 0:
				promptstring = (char *)malloc(strlen(Ustring[288]) + strlen(prompt) + 30);
				/*strcat(promptstring, "one or more");*/
				sprintf(promptstring,Ustring[288],prompt);
				break;
			case 1:
				promptstring = (char *)malloc(strlen(Ustring[289]) + strlen(prompt) + 30);
				/*strcat(promptstring, "one");*/
				sprintf(promptstring,Ustring[289],prompt);
				break;
			default:
				promptstring = (char *)malloc(strlen(Ustring[290]) + strlen(prompt) + 30);
				sprintf(number,"%5d",nr_matches);
				tnt(number);
				/*strcat(promptstring,"up to ");*/
				sprintf(promptstring,Ustring[290],number,prompt);
				break;
		}
		/*
		strcat(promptstring," ");
		strcat(promptstring,prompt);
		if (nr_matches != 1) {
			strcat(promptstring,"s");
		}
		*/
		strcat(promptstring," (");
		strcat(promptstring,Ustring[321]);
		strcat(promptstring,")");

		/* CONSTCOND */
		while(1) {
			make_prompt(promptstring);
			get_one_line(vf->input);
			tnt(vf->input);
			if (!strcmp(vf->input,"?")) {
				list_entries(dirname, valid_what);
				continue;
			}
			break;
		}
	} else {
		vf->input=strdup(params);
	}

/* Now we fork and wait in the parent */
	i = fork();
	if (i < 0) {
		/* Error in fork() */
		printf("There is a system problem.  Please try later.\n");
		remove(filename);
	} else if (i > 0) {
		/* in the parent - just wait for child to die */
		(void)wait(&i);
	} else {
		/* in the child, i == 0 */
		struct slist *array = 0;
		struct slist *l;
		char *p;

		tnt(vf->input);
		p = vf->input;
		while(*p) {
			while(*p && isdelim(*p)) p++;
			for(i=0;(i<MAINLINE) && *p && !isdelim(*p);i++) word[i] = *p++;
			word[i] = 0;
			push_slist(&array, word);
		}
		if (vf->input[0] && _get_valid_files(mode, nr_matches, prompt, dirname, &array, valid_what, case_sensitive)) {
			l = array;
			i = 0;
			if (L = fopen(filename,"w")) {
				while(l) {
					i += strlen(l->this) + 1;
					fputs(l->this,L);
					if (l->next) fputs(" ",L);
					l = l->next;
				}
				fclose(L);
			}
		} else {
			remove(filename); /* Just in case */
		}
		free_slist(&array);
		exit(0);
	}
/* This is after the child has ended */

	if (!stat(filename,&sb)) {
		vf->files = (char *)malloc((unsigned)sb.st_size+1);
		vf->files[0] = 0;
		if (L = fopen(filename,"r")) {
			i = fread(vf->files,1,(unsigned)sb.st_size,L);
			vf->files[i] = 0;
			fclose(L);
		}
	} else {
		vf->files = strdup("");
	}
	remove(filename);
	return vf;
}
/* ------------------------------------------------------------------------- */

int _get_valid_files(const char mode, const int nr_matches, const char *prompt,
	const char *dirname, struct slist **const params,
	const char valid_what, const int case_sensitive)
{
	struct slist *l;
	int i;

	l = *params;
	while(l) {
		if (!strcmp("me",l->this)) {
			free(l->this);
			l->this=strdup(U.id);
		}
		if (l->this[0] == '.') {
			if (mode == 'v') {
				/*printf("%s - please do not use . at the start of a %s name.\n",l->this, prompt);*/
				printf(Ustring[325],".", prompt);
				printf("\n");
			}
			l = pop_slist(params, l);
			continue;
		}
		if (l->this[0] == '~') {
			if (mode == 'v') {
				/*printf("%s - please do not use ~ at the start of a %s name.\n",l->this, prompt);*/
				printf(Ustring[325],"~", prompt);
				printf("\n");
			}
			l = pop_slist(params, l);
			continue;
		}
		if (strchr(l->this,'/')) {
			if (mode == 'v') {
				/*printf("%s - please do not use / in a %s name.\n",l->this, prompt);*/
				printf(Ustring[325],"/", prompt);
				printf("\n");
			}
			l = pop_slist(params, l);
			continue;
		}
		glob(params, &l, dirname, mode, prompt, valid_what, case_sensitive);
	}

	for(i = 0, l = *params; l; l = l->next) {
		i++;
	}
	if (nr_matches && (i > nr_matches)) {
		for(i = 0, l = *params; i < nr_matches; l = l->next) i++;
		while((l = pop_slist(params, l)));

		if (mode == 'v') {
			/*printf("Too many %ss matched, only selecting", prompt);*/
			printf(Ustring[324],(*params)->this);
			printf("\n");
		}
	}
	return i;
}

static int glob(struct slist **const list, struct slist **const l,
	const char *dirname, const char mode, const char *prompt,
	const char valid_what, const int case_sensitive) {
	struct slist *k = (*l)->prev;
	struct slist *m = (*l)->next;
	char *pattern = strdup((*l)->this);
	char *mixed_case = strdup(pattern);
	char *shellpat = (char *)malloc(strlen((*l)->this) * 2 + 2 + 1);
	char **filesfound = 0;
	int i, j, nr = 0;
#if defined(SVR42)
	char *re;
#else /* !SVR42 */
#  if defined(LINUX)
	int re;
	regex_t preg;
#  endif
#endif

	(void)pop_slist(list, *l);

	if (!case_sensitive) {
		lower_string(pattern);
	}

#if defined(SVR42) || defined(LINUX)
	j = 0;
	shellpat[j++] = '^';
	for(i = 0;pattern[i]; i++) {
		switch(pattern[i])
		{
			case '*':
				shellpat[j++] = '.';
				shellpat[j++] = pattern[i];
				break;
			case '?':
				shellpat[j++]='.';
				break;
			default:
				if (ispunct(pattern[i]))
					shellpat[j++] = '\\';
				shellpat[j++] = pattern[i];
		}
	}
	shellpat[j++] = '$';
	shellpat[j] = 0;

#  if defined(SVR42)
	re = regcmp(shellpat,NULL);
#  else /* not SVR42, so it's LINUX here */
	re = !regcomp(&preg,shellpat,0);
#  endif /* which one */
#else /* not SVR42 or LINUX */
	re = 1;
#endif

	if (re) {
		struct slist *local;

#if defined(SVR42)
		nr = sortdir(dirname, re, &filesfound, valid_what, case_sensitive);
		free(re);
#else /* !SVR42 */
#  if defined(LINUX)
		nr = sortdir(dirname, &preg, &filesfound, valid_what, case_sensitive);
		regfree(&preg);
#  else /* !LINUX */
		nr = sortdir(dirname, pattern, &filesfound, valid_what, case_sensitive);
#  endif
#endif
		for(i=0; i<nr; i++) {
			local = (struct slist *)malloc(sizeof (struct slist));
			local->this = filesfound[i];

			local->prev = k;
			local->next = m;

			if (k) k->next = local;
			else *list = local;
			k = local;

			if (m) {
				m->prev = local;
			}
		}
		if (filesfound) {
			free(filesfound);
		}
	} else if (mode == 'v') {
		/*printf("%s - invalid pattern.\n", mixed_case);*/
		printf(Ustring[472],mixed_case);
		printf("\n");
	}
	*l = m;
	if (!nr && (mode == 'v')) {
		/*printf("%s - no matching %s found.\n", mixed_case, prompt);*/
		printf("%s\n",Ustring[123]);
	}
	free(shellpat);
	free(mixed_case);
	free(pattern);
	return nr;
}

static int sortdir(const char *dirname,
#if defined(SVR42)
	char *regexp,
#else /* !SVR42 */
#  if defined(LINUX)
	regex_t *regexp,
#  else /* !LINUX */
	char *regexp,
#  endif
#endif
	char ***ptr, const char valid_what, const int case_sensitive)
{
	int i;
	struct dirent *ent;
	char **p;
	DIR *dirp = opendir(dirname);
	char filename[MAINLINE * 2 + 100];
	struct stat statbuf;
	char *entry;

	if (!dirp) {
		sprintf(filename,"invalid directory %s",dirname);
		errorlog(filename);
		return 0;
	}
	for(i=0; readdir(dirp); i++);
	if (!i) {
		closedir(dirp);
		return 0;
	}

	p = (char **)malloc(i * sizeof (char *));

	rewinddir(dirp);
	i = 0;
	while((ent = readdir(dirp))) {
		entry = strdup(ent->d_name);
		if (!case_sensitive) {
			lower_string(entry);
		}
		if ((ent->d_name[0] != '.') &&
			(strcmp("lost+found",ent->d_name)) &&
			(strlen(ent->d_name) < MAINLINE)
#if defined(SVR42)
			&& (regex(regexp,entry) != NULL)
#else /* !SVR42 */
#  if defined(LINUX)
			&& !regexec(regexp,entry,0,0,0)
#  else /* !LINUX */
			&& dos_match(regexp,entry)
#  endif
#endif
		) {
			sprintf(filename,"%s/%s",dirname,ent->d_name);
			switch (valid_what) {
				case 'f':
					stat(filename,&statbuf);
					if (!S_ISREG(statbuf.st_mode)) {
						continue;
					}
					break;
				case 'd':
					stat(filename,&statbuf);
					if (!S_ISDIR(statbuf.st_mode)) {
						continue;
					}
					break;
			}
			p[i++] = strdup(ent->d_name);
		}
	}
	qsort(p, (unsigned)i, sizeof (char *), strsort);
	*ptr = p;
	closedir(dirp);
	return i;
}

static int list_entries(const char *dirname, const char what) {
	struct valid_files *vf = get_valid_entries('q',0,"",dirname,"*",what,0);
	size_t maxlen = 0;
	int i;
	int nl = 0;

	if (!vf->files[0]) {
		/*printf("There is nothing to list.\n\n");*/
		printf(Ustring[313],Ustring[320]);
		printf("\n\n");
	} else {
		char *p = vf->files;

		while(*p) {
			while(*p && isspace(*p)) p++;
			for(i=0;*p && !isspace(*p);p++,i++);
			if (i > maxlen) maxlen = i;
		}
		maxlen+=2;
		p = (char *)malloc(maxlen);
		i = WIDTH - 1;
		while(vf->files[0]) {
			shiftword(vf->files,p,maxlen);
			printf("%*s",(signed)maxlen * -1,p);
			i -= maxlen;
			if (i < maxlen) {
				printf("\n");
				i = WIDTH - 1;
				if (++nl > (LINES - 2)) {
					if (!do_continue("")) {
						break;
					}
					nl = 0;
				}
			}
		}
		free(p);
		if (i < WIDTH - 1) {
			printf("\n");
		}
	}
	free(vf->input);
	free(vf->files);
	free(vf);
	return maxlen-1;
}

/*
 * This function receives a parameter, `ignore', and ignores it.
 */
struct valid_files *get_valid_files(char mode,int many,char *word,char *thisdir,char *fromstring, int ignore) {
 	return get_valid_entries(mode,many,word,thisdir,fromstring,'f',C.filesensitive);
}

struct valid_files *get_valid_dirs(char mode,int many,char *word,char *thisdir,char *fromstring, int ignore) {
 	return get_valid_entries(mode,many,word,thisdir,fromstring,'d',ignore);
}

struct valid_files *get_valid_both(char mode,int many,char *word,char *thisdir,char *fromstring, int ignore) {
 	return get_valid_entries(mode,many,word,thisdir,fromstring,' ',ignore);
}

int list_files(char *dirname) {
	return list_entries(dirname,'f');
}

int list_dirs(char *dirname) {
	return list_entries(dirname,'d');
}

int list_both(char *dirname) {
	return list_entries(dirname,' ');
}
