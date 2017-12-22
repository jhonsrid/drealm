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
#include <time.h>
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

#include "getmemb.h"

/* ------------------------------------------------------------------------- */

static int _get_valid_members(
	const char mode,			/* 'q' for quiet;
						   'v' for verbose */
	const int nr_matches,			/* max matches to return
						   (0 = unlimited) */
	const char *prompt,			/* text to put in messages */
	const char *memberlist,			/* file to search */
	struct slist **const params,		/* list of names to check */
	const int case_sensitive		/* don't ignore case in
						   compares */
);
static int glob(struct slist **const list, struct slist **const l,
	const char *memberlist, const char mode, const char *prompt,
	const int case_sensitive);
/* == START == */
static int filter(const char *memberlist,
#if defined(SVR42)
	char *regexp,
#else /* !SVR42 */
#  if defined(LINUX)
	regex_t *regexp,
#  else /* !LINUX */
	char *regexp,
#  endif
#endif
	char ***p, const int case_sensitive);
/* == END == */
/* ------------------------------------------------------------------------- */

struct valid_members *get_valid_members(const char mode, const int nr_matches,
	const char *prompt, const char *memberlist, const char *params,
	const int case_sensitive)
{
	struct valid_members *vm;
	char word[MAINLINE];
	int i;
	struct stat sb;
	FILE *L;
	char filename[MAINLINE + 100];

	/* This is where the list of members will end up */
#if !defined(LINUX)
	sprintf(filename,"%s/memblist.%ld",C.tmpdir,getpid());
#else
	sprintf(filename,"%s/memblist.%d",C.tmpdir,getpid());
#endif

	vm = (struct valid_members *)malloc(sizeof (struct valid_members));

	if (!memberlist[0]) {
		if (mode == 'v') {
			/*printf("No memberlist specified.\n");*/
			printf("%s ",Ustring[276]);
			printf(Ustring[294],Ustring[320]);
			printf("\n");
		}
		errorlog("get_valid_members: no memberlist");
		vm->input = strdup(params);
		vm->members = strdup("");
		return vm;
	}

	if (L = fopen(memberlist,"r")) {
		fclose(L);
	} else {
		if (mode == 'v') {
			/*printf("Invalid memberlist specified.\n");*/
			printf(Ustring[64],Ustring[320]);
			printf("I\n");
		}
		errorlog("get_valid_members: cannot open memberlist");
		vm->input = strdup(params);
		vm->members = strdup("");
		return vm;
	}

	if (!params[0] && (mode == 'v')) {
	/* Nothing entered and we're allowed to talk to the user */
		char *promptstring;
		char number[5];

		promptstring = (char *)malloc(15 + 12 + strlen(prompt) + 20);
		vm->input=(char *)malloc(MAINLINE);
		/*strcpy(promptstring,"Please specify ");*/
		switch(nr_matches) {
			case 0:
				/*strcat(promptstring, "one or more");*/
				 sprintf(promptstring,Ustring[288],prompt); 
				break;
			case 1:
				/*strcat(promptstring, "one");*/
				sprintf(promptstring,Ustring[289],prompt);
				break;
			default:
				sprintf(number,"%5d",nr_matches);
				tnt(number);
				/*strcat(promptstring,"up to ");*/
				/*strcat(promptstring,number);*/
				sprintf(promptstring,Ustring[290],number,prompt); 
				break;
		}
		/*
		strcat(promptstring," ");
		strcat(promptstring,prompt);
		if (nr_matches != 1) {
			strcat(promptstring,"s");
		}
		strcat(promptstring," (? for a list): ");
		*/
		
		strcat(promptstring," (");
		strcat(promptstring,Ustring[321]);
		strcat(promptstring,")");
                                       		
		/* CONSTCOND */
		while(1) {
			make_prompt(promptstring);
			get_one_line(vm->input);
			tnt(vm->input);
			if (!strcmp(vm->input,"?")) {
				list_members(memberlist);
				continue;
			} else if (!strcmp(vm->input,"q") || !strcmp(vm->input,"quit")) {
				vm->input[0] = 0;
			}
			break;
		}
	} else {
		vm->input=strdup(params);
	}

/* Now we fork and wait in the parent */
	i = fork();
	if (i < 0) {
		/* Error in fork() */
		/*printf("There is a system problem.  Please try later.\n");*/
		printf("%s %s\n",Ustring[276],Ustring[322]);
		remove(filename);
	} else if (i > 0) {
		/* in the parent - just wait for child to die */
		(void)wait(&i);
	} else {
		/* in the child, i == 0 */
		struct slist *array = 0;
		struct slist *l;
		char *p;

		tnt(vm->input);
		p = vm->input;
		while(*p) {
			while(*p && isdelim(*p)) p++;
			for(i=0;(i<MAINLINE) && *p && !isdelim(*p);i++) word[i] = *p++;
			word[i] = 0;
			push_slist(&array, word);
		}
		if (vm->input[0] && _get_valid_members(mode, nr_matches, prompt, memberlist, &array, case_sensitive)) {
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
		vm->members = (char *)malloc((unsigned)sb.st_size+1);
		vm->members[0] = 0;
		if (L = fopen(filename,"r")) {
			i = fread(vm->members,1,(unsigned)sb.st_size,L);
			vm->members[i] = 0;
			fclose(L);
		}
	} else {
		vm->members = strdup("");
	}
	remove(filename);
	return vm;
}
/* ------------------------------------------------------------------------- */

int _get_valid_members(const char mode, const int nr_matches, const char *prompt,
	const char *memberlist, struct slist **const params,
	const int case_sensitive)
{
	struct slist *l;
	int i;

	l = *params;
	tnt(Ustring[323]);
	while(l) {
		/*if (!strcmp("me",l->this)) {*/
		if (!strcmp(Ustring[323],l->this)) {
			free(l->this);
			l->this=strdup(U.id);
		}
		glob(params, &l, memberlist, mode, prompt, case_sensitive);
	}

	for(i = 0, l = *params; l; l = l->next) {
		i++;
	}
	if (nr_matches && (i > nr_matches)) {
		for(i = 0, l = *params; i < nr_matches; l = l->next) i++;
		while((l = pop_slist(params, l)));

		if (mode == 'v') {
			/*printf("Too many %ss matched, only selecting", prompt);*/
			printf("%s",Ustring[324]);
			for(l = *params; l; l = l->next)
				printf(" %s", l->this);
			printf("\n");
		}
	}
	return i;
}

static int glob(struct slist **const list, struct slist **const l,
	const char *memberlist, const char mode, const char *prompt,
	const int case_sensitive) {
	struct slist *k = (*l)->prev;
	struct slist *m = (*l)->next;
	char *pattern = strdup((*l)->this);
	char *shellpat = (char *)malloc(strlen((*l)->this) * 2 + 1);
	char **membersfound = 0;
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
	for(i = j = 0;pattern[i]; i++) {
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
	shellpat[j] = 0;

#if defined(SVR42)
	re = regcmp("^",shellpat,"$",NULL);
#else /* not SVR42, so it's LINUX here */
	free(pattern);
	pattern = (char *)malloc(strlen(shellpat) + 10);

	sprintf(pattern,"^%s$",shellpat);
	re = !regcomp(&preg,pattern,0);
#endif /* which one */
#else /* not SVR42 or LINUX */
	re = 1;
#endif

	if (re) {
		struct slist *local;

#if defined(SVR42)
		nr = filter(memberlist, re, &membersfound, case_sensitive);
		free(re);
#else /* !SVR42 */
#  if defined(LINUX)
		nr = filter(memberlist, &preg, &membersfound, case_sensitive);
		regfree(&preg);
#  else /* !LINUX */
		nr = filter(memberlist, pattern, &membersfound, case_sensitive);
#  endif
#endif
		for(i=0; i<nr; i++) {
			local = (struct slist *)malloc(sizeof (struct slist));
			local->this = membersfound[i];

			local->prev = k;
			local->next = m;

			if (k) k->next = local;
			else *list = local;
			k = local;

			if (m) {
				m->prev = local;
			}
		}
		if (membersfound) free(membersfound);
	}
	else if (mode == 'v') printf("%s - invalid pattern.\n", pattern);

	*l = m;
	if (!nr && (mode == 'v')) {
		/*printf("%s - no matching %s found.\n", pattern, prompt);*/
		printf("%s\n",Ustring[123]);
	}
	free(shellpat);
	free(pattern);
	return nr;
}

static int filter(const char *memberlist,
#if defined(SVR42)
	char *regexp,
#else /* !SVR42 */
#  if defined(LINUX)
	regex_t *regexp,
#  else /* !LINUX */
	char *regexp,
#  endif
#endif
	char ***ptr, const int case_sensitive)
{
	unsigned int i; /* can only be +ve */
	char member[MAINLINE];
	char **p;
	FILE *mlist = fopen(memberlist,"r");
	char filename[MAINLINE * 2 + 100];
	char *entry;

	if (!mlist) {
		sprintf(filename,"invalid memberlist %s",memberlist);
		errorlog(filename);
		return 0;
	}
	for(i=0; fgets(member,MAINLINE,mlist); i++);
	if (!i) {
		fclose(mlist);
		return 0;
	}

	p = (char **)malloc(i * sizeof (char *));

	rewind(mlist);
	i = 0;
	while(fgets(member,MAINLINE,mlist)) {
		entry = strdup(member);
		tnt(entry);
		if (!case_sensitive) {
			lower_string(entry);
		}
		if (
#if defined(SVR42)
			(regex(regexp,entry) != NULL)
#else /* !SVR42 */
#  if defined(LINUX)
			!regexec(regexp,entry,0,0,0)
#  else /* !LINUX */
			dos_match(regexp,entry)
#  endif
#endif
		) {
			p[i] = strdup(member);
			tnt(p[i]);
			i++;
		}
	}
	qsort(p, i, sizeof (char *), strsort);
	*ptr = p;
	fclose(mlist);
	return i;
}

int list_members(const char *memberlist) {
	struct valid_members *vm = get_valid_members('q',0,"",memberlist,"*",0);
	size_t maxlen = 0;
	int i;
	int nl = 0;

	if (!vm->members[0]) {
		/*printf("There is nothing to list.\n\n");*/
		printf(Ustring[313],Ustring[320]);
		printf("\n\n");
	} else {
		char *p = vm->members;

		while(*p) {
			while(*p && isspace(*p)) p++;
			for(i=0;*p && !isspace(*p);p++,i++);
			if (i > maxlen) maxlen = i;
		}
		maxlen+=2;
		p = (char *)malloc(maxlen);
		i = WIDTH - 1;
		while(vm->members[0]) {
			shiftword(vm->members,p,maxlen);
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
	free(vm->input);
	free(vm->members);
	free(vm);
	return maxlen-1;
}
