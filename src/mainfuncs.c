
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
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/wait.h>

#if defined(SVR42)
#  include <libgen.h>
#else
#  if defined(LINUX)
#    include <regex.h>
#  endif
#endif


#include "drealm.h"
#include "drealmgen.h"
#include "inputfuncs.h"
#include "configfuncs.h"
#include "setupfuncs.h"
#include "genfuncs.h"
#include "display.h"
#include "getvalf.h"
#include "getmemb.h"

#include "mainfuncs.h"

#if CHAT_COMMANDS
#  include "chatfuncs.h"
#endif

#if READ_COMMANDS
#  include "readfuncs.h"
#endif

#if FILE_COMMANDS
#  include "filefuncs.h"
#endif

/* ------------------------------------------------------------- *
 * D E F I N E S   F O R   V A R I A B L E   F O R M A T T I N G *
 * ------------------------------------------------------------- */
#define F_ON_OFF	1
#define F_SET_UNSET	2
#define F_TRUE_FALSE	3
#define F_YES_NO	4


/* ------------------------------------------------------------- *
 * O T H E R   S T A T I C   V A R I A B L E S                   *
 * ------------------------------------------------------------- */
static int Hour = 0;
static int Firstfree = 0;  /*offset*/
static int Levelfloor = 0; /*offset*/


/* ------------------------------------------------------------- *
 * C O D E   S T A R T S   H E R E                               *
 * ------------------------------------------------------------- */

void preload(void) {
	FILE *PREL;
	char filename[MAINLINE + 100];
	char menuname[15];

	if (!C.menucache) {
		return;
	}
	
	sprintf(filename,"%s/config.preload",C.configdir);
	if (! (PREL = fopen(filename,"r"))) {
		return;
	}
	
	while (fgets(menuname,15,PREL)) {
		menuname[14] = 0;
		tnt(menuname);
		findorload_menu(menuname);
	}
	
	fclose(PREL);
	
	Levelfloor = Firstfree;
}

int free_menus(void) {
	Firstfree = Levelfloor;
	return 1;
}

int findincache(char *menuname) {
	int offset = 0;
	struct menuload *thismnu = (struct menuload *)&G.menucache[offset];

	while(thismnu < (struct menuload *)&G.menucache[Firstfree]) {
		if (!strcmp(menuname,thismnu->menuname)) {
			return offset;
		}
		offset = thismnu -> nextmenu;
		thismnu = (struct menuload *)&G.menucache[offset];
	}
	return -1;
}

int findorload_menu (char *menuname) {
	int numread;
	int numtoread;
	FILE *MNU;
	struct menuload *thismnu = (struct menuload *)&G.menucache[Firstfree];

	if (C.menucache && (findincache(menuname) >= 0)) {
		return 1;
	}

	if (! (MNU = open_menu(menuname))) {
		return 0;
	}

	numtoread = C.menucache - Firstfree - sizeof (struct menuload) - 1;
	if (numtoread <= 0) {
		fclose(MNU);
		return 1;
	}

	numread = fread(thismnu->menubody,1,(unsigned)numtoread,MNU);
	if (ferror(MNU))  {
		fclose(MNU);
		return 0;
	}
	
	if (feof(MNU)) {
		Firstfree += numread+1+(sizeof (struct menuload));
		strcpy(thismnu->menuname,menuname);
		thismnu->nextmenu = Firstfree;
		thismnu->menubody[numread] = 0;
	}
	fclose(MNU);
	return 1;
}
		

FILE *open_menu(char *menuname) {
	char fullname[MAINLINE+100];
	FILE *MNU;
	
	sprintf(fullname,"%s/%s",C.menus,menuname);
	if ((MNU = fopen(fullname,"r"))) {
		return MNU;
	}
	sprintf(fullname,"%s/%c/%s",C.menus,menuname[0],menuname);
	MNU = fopen(fullname,"r");
	return MNU;
}

int do_cmd(char type, char *level) {
	char menufile[MAINLINE+100];
	char copy[21];

	if (Looping_counter > MAXLOOP) {
		puts("\nSorry, there seems to be a fault in the menu system.");
		do_exit(73);
		return 0;
	}
	strncpy(copy,level,21);
	tnt(copy);

	if (copy[0] == 0) {
		strcpy(copy, G.level);
	}

	sprintf(menufile,"%s.cmd",copy);

	return menu(type, menufile);
}

int menu(char type, char *menuname) {
	FILE *MNU = NULL;
	struct menu_struct m;
	int totalshift;
	int line_no = 0;
	char line[1024];
	int shift = 0;

	strcpy(MenuFile,menuname);
	line[0] = 0;
	Hour = get_hour();

	/* If we're cacheing and it was found in the cache... */
	if (C.menucache && ((shift = findincache(menuname)) >= 0)) {
		totalshift = shift + sizeof(struct menuload);
	} else {
		if ( !(MNU = open_menu(menuname))) {
			return 0;
		}
		totalshift = 0;
	}

	while (Continue) {
		line_no++;
		if (MNU) {
			if (!get_disk_line(MNU, totalshift, line)) {
				fclose(MNU);
				/* log an error */
				return 0;
			}
		} else {
			get_memory_line(totalshift,line);
		}
		if (!parse_line(line, &m)) {
			if (MNU) {
				fclose(MNU);
			}
			/* log an error */
			return 0;
		}
		totalshift+=m.shiftcount;

		if (!compare_line(&m, type)) {
			continue;
		}

		if (type == 'a') {
			Line_matched = 1;
			if (m.menu_cont) {
				if (do_some_commands(m.action)) {
					Continue = (m.menu_cont + 1);
				} else {
					Continue = (m.menu_cont - 1);
				}
			} else {
				do_some_commands(m.action);
				Continue = m.menu_cont; /* 0 of course! */
			}
		} else if (type == 'd') {
			if (do_menulinedisplay(m.display)) {
				Continue = 1;
			} else {
				Continue = 0;
			}
		}
	}
	if (MNU) {
		fclose(MNU);
	}
	return 1;
}

int get_memory_line(int offset, char *line) {
	strncpy(line,&G.menucache[offset],1023);
	line[1023] = 0;
	return strlen(line);
}

int get_disk_line(FILE *MENU, int offset, char *line) {
	if (!fseek(MENU,offset,SEEK_SET)) {
		int numread = fread(line,1,1023,MENU);
		line[numread] = 0;
		return numread;
	} else {
		return 0;
	}
}

int parse_line(char *menuline, struct menu_struct *m) {
/*
This function is reponsible for deciding if the menu is ok or not and
it does it by returning 0 if it isn't
*/
	char field[MAINLINE];
	char temp[MAINLINE];


	/* comment field or spare space at front */
	m->shiftcount = 0;
	menushift(menuline,field,MAINLINE,"::",&m->shiftcount);


	/* able */
	menushift(menuline,field,MAINLINE,"::",&m->shiftcount);
	fieldtrans(temp,field);
	tnt(temp);
	if (! ((strcmp(temp,"0") == 0) || (strcmp(temp,"1") == 0))) {
		menuline[0] = 0;
		return 0;
	}
	m->able = atoi(temp);

	
	/* display */
	menushift(menuline,m->display,MAINLINE,"::",&m->shiftcount);


	/* keys to compare */
	menushift(menuline,m->keys,MAINLINE,"::",&m->shiftcount);


	/* menu action to perform */
	/* this fails the line if the field was too long, as a truncated
	   directory path for example could lead to security problems
	*/	  
	if (!menushift(menuline,m->action,MAINLINE,"::",&m->shiftcount)) {
		return 0;
	}


	/* earliest hour */
	menushift(menuline,temp,MAINLINE,"::",&m->shiftcount);
	tnt(temp);
	m->mintime = atoi(temp);


	/* latest hour */
	menushift(menuline,temp,MAINLINE,"::",&m->shiftcount);
	tnt(temp);
	m->maxtime = atoi(temp);


	/* lowest level */
	menushift(menuline,field,MAINLINE,"::",&m->shiftcount);
	fieldtrans(temp,field);
	tnt(temp);
	m->minlevel = atoi(temp);


	/* highest level */
	menushift(menuline,field,MAINLINE,"::",&m->shiftcount);
	fieldtrans(temp,field);
	tnt(temp);
	m->maxlevel = atoi(temp);


	/* user's flags to compare */
	menushift(menuline,field,MAINLINE,"::",&m->shiftcount);
	fieldtrans(temp,field);
	tnt(temp);
	m->userflags[0] = 0;
	strncpy(&m->userflags[1],temp,UFLAGMAX);
	m->userflags[UFLAGMAX + 1] = 0;


	/* area's flags to compare */
	menushift(menuline,field,MAINLINE,"::",&m->shiftcount);
	fieldtrans(temp,field);
	tnt(temp);
	m->areaflags[0] = 0;
	strncpy(&m->areaflags[1],temp,AFLAGMAX);
	m->areaflags[AFLAGMAX + 1] = 0;


	/* if we've run out of menuline already something is wrong! */
	if (!strlen(menuline)) {
		menuline[0] = 0;
   		return 0;
	}


	/* "continue" status */
	menushift(menuline,temp,MAINLINE,"::",&m->shiftcount);
	tnt(temp);
	if (! ((strcmp(temp,"-1") == 0) || (strcmp(temp,"0") == 0) || (strcmp(temp,"1") == 0) || (strcmp(temp,"2") == 0) )) {
		menuline[0] = 0;
		return 0;
	}
	if (temp[0] == '-') {
		m->menu_cont = (atoi(&temp[1]) * -1);
	} else {
		m->menu_cont = atoi(temp);
	}
	
	return 1;
}


int compare_line(struct menu_struct *m, char type) {
	char temp[MAINLINE];
	
	if (m->able &&
		comp_level(m->minlevel,m->maxlevel) &&
		comp_time(m->mintime,m->maxtime) &&
		comp_flags(m->userflags,U.flags) &&
		comp_flags(m->areaflags,G.areaflags)) {

		if (type == 'a') {
			fieldtrans(temp,m->keys);
			return comp_command(temp,G.command);
		} else if (type == 'd') {
			return 1;
		}
	}
	return 0;
}


int do_stop(void) {
	Continue = 1;
	findorload_menu("stop.cmd");
	return menu('a',"stop.cmd");
}

/* ARGSUSED0 */
int logoff (char *dummy) {
/* MENU COMMAND */
	char temp[MAINLINE + 100];

#if defined(READ_COMMANDS)
	area_clearup();
#endif
	(void)do_stop();

	sprintf(temp,"%s/%s/.doing",C.users,U.id);
	remove(temp);
	sprintf(temp,"%s/conf.%s",C.tmpdir,U.id);
	rem_lock(temp);
	
	rewind(LASTON);
	fprintf(LASTON,"0\n");
	fclose(LASTON);

	do_exit(0);
	return 0;
}

int do_ini(char *levelname) {
	char menufile[MAINLINE + 100];

	Continue = 1;
	sprintf(menufile,"%s.ini",levelname);
	return menu('a',menufile);
}

int do_exi(char *levelname) {
	char menufile[MAINLINE + 100];

	Continue = 1;
	sprintf(menufile,"%s.exi",levelname);
	return menu('a',menufile);
}

int do_start (void) {
	Continue = 1;
	findorload_menu("start.cmd");
	return menu('a',"start.cmd");
}

int append (char *templevel) {
/* MENU COMMAND */
	(void)do_cmd('a',templevel);
	return !Continue;
}

int do_pmp (void) {
	char menufile[MAINLINE + 100];

	Continue = 1;
	sprintf(menufile,"%s.pmp",G.level);
	return menu('a',menufile);
}


int do_some_commands (char *actions) {
	char action[MAINLINE];
	char *copy = strdup(actions);

	while (copy[0]) {
		strshift(copy,action,MAINLINE,";");
		if (!do_one_command(action)) {
			free(copy);
			return 0;
		}
	}
	free(copy);
	return 1;
}

int do_one_command (char *params) {
	char cmdstr[21];
	char action[MAINLINE];
	int i=0;

	fieldtrans(action,params);
	
	shiftword(action,cmdstr,21);
	tnt(cmdstr);
	if (!cmdstr[0]) {
		return 1;
	}

	while (keywords[i].keyword[0]) {
		if (!strcmp(keywords[i].keyword,cmdstr)) {
			return keywords[i].func(action);
		}
		i++;
	}
	sprintf(G.errmsg,"menu action '%s' invalid",cmdstr);
	errorlog(G.errmsg);
	/*printf("Command recognised but not working.\n");*/
	printf("%s %s\n",Ustring[284],Ustring[283]);
	return 0;
}

int do_menulinedisplay (char *params) {
	char result[MAINLINE];
	
	fieldtrans(result,params);
	return do_print(result);
}

int do_print (char *printparam) {
/* MENU COMMAND */
/* takes only from the menu */

	int i = 0;

	while (printparam[i]) {
		if ((printparam[i+1]) && (printparam[i] == '\\')) {
			i++;
		 	if (printparam[i] == '\\') {
		 		putchar('\\');
		 	} else if (printparam[i] == 'n') {
		 		putchar('\n');
		 	} else if (printparam[i] == 'b') {
		 		putchar('\b');
		 	} else if (printparam[i] == '!') {
		 		if (!do_continue("dummy")) {
					return 0;
				}
		 	} else if (printparam[i] == '~') {
		 		(void)press_enter("dummy");
		 	} else {
				putchar(printparam[i]);
			}
			i++;
		} else {
			putchar(printparam[i]);
			i++;
		}
	}
	return 1;
}

int comp_command (char *keys, char *g_command) {
	char t_keys[MAINLINE];
	char t_command[MAINLINE];


	sprintf(t_keys," %s ",keys);
	sprintf(t_command," %s ",g_command);

	if (strstr(t_keys," *FORCE* ")) {
		return 1;
	}
	if (strstr(t_keys," *DEFAULT* ") && !g_command[0]) {
		return 1;
	}
	if (strstr(t_keys," *NUMBER* ") && is_num(g_command)) {
		return 1;
	}

	if (!C.sensitive) {
		lower_string(t_keys);
	}
	return (strstr(t_keys,t_command) != NULL);
}

int comp_level (int minlevel, int maxlevel) {
	return((U.level >= minlevel) && (maxlevel >= U.level));
}

int comp_time (int mintime, int maxtime) {
	return((mintime <= Hour) && (maxtime >= Hour));
}


void fieldtrans (char *outfield, char *infield) {
	int i; /* menuline counter  */
	int j; /* varstring counter    */
	int k; /* transline counter  */
	int l; /* var_trans counter */
	char *vartrans;
	char varvar[MAINLINE];
	char midfield[MAINLINE];

	if (!strchr(infield,'\\')) {
		strncpy(outfield,infield,MAINLINE-1);
		outfield[MAINLINE-1] = 0;
		return;
	}
	
	/* The mstrings bit */

	i = 0;
	j = 0;
	k = 0;
	l = 0;
	while (infield[i]) {	
		if ((infield[i] == '\\') && (infield[i+1] == '@') && ((i == 0) || ((i > 0) && (infield[i-1] != '\\')))) {
			i++;
			i++;

			j = 0;
			while (infield[i] && (infield[i] != '@')) {
				if (j < MAINLINE-1) {
					varvar[j] = infield[i];
					j++;
				}
				i++;
			}
			i++;
			varvar[j] = 0;

			if ( (vartrans = transtring(varvar)) ) {
				l = 0;
				while (vartrans[l]) {
					if (k < MAINLINE-1) {
						midfield[k] = vartrans[l];
						k++;
					}
					l++;
				}
				free(vartrans);
			}
		} else {
			if (k < MAINLINE-1) {
				midfield[k] = infield[i];
				k++;
			}
			i++;
		}
	}

	midfield[k] = 0;
	
	
	i = 0;
	j = 0;
	k = 0;
	l = 0;
	/* The $vars$ bit */
	while (midfield[i]) {
		if ((midfield[i] == 10) || (midfield[i] == 13) || (midfield[i] == 9)) {
			if (k < (MAINLINE - 1)) {
				outfield[k] = ' ';
				k++;
			}
			i++;
			continue;
		}
		if ((midfield[i] == '\\') && (midfield[i+1] == '$') && ((i == 0) || ((i > 0) && (midfield[i-1] != '\\')))) {
			i++;
			i++;

			j = 0;
			while (midfield[i] && (midfield[i] != '$')) {
				if (j < MAINLINE-1) {
					varvar[j] = midfield[i];
					j++;
				}
				i++;
			}
			i++;
			varvar[j] = 0;

			if ( (vartrans = transvar(varvar)) ) {
				l = 0;
				while (vartrans[l]) {
					if (k < MAINLINE-1) {
						outfield[k] = vartrans[l];
						k++;
					}
					l++;
				}
				free(vartrans);
			}
		} else {
			if (k < MAINLINE-1) {
				outfield[k] = midfield[i];
				k++;
			}
			i++;
		}
	}
	outfield[k] = 0;
}


char *transvar (char *varvar) {
	char *field, *flag, *format, *value;
	int Subsc = 0;

	/* Syntax: varname<blank>["format"][#flag][field] */
	
	/* Find the end of the variable name */
	field = strchr(varvar,' ');
	if (field) {
		/* Terminate varvar and skip over spaces */
		while(isspace(*field)) *field++ = '\0';
	}
	
	/* See if there's a (date) format (if there was a format at all) */
	format = field ? strchr(field,'"') : field;
	if (format) {
		/* Skip up to and over open quotes */
		format++;
		field = format;
		/* Skip up to end quotes */
		while(*field && (*field != '"')) field++;
		/* Terminate format and skip field forward */
		if (*field) *field++ = '\0';
	}
	
	/* See if there's a subscript field (if there was a format at all) */
	flag = field ? strchr(field,'#') : field;
	if (flag) {
		/* Skip up to and over # */
		flag ++;
		/* Skip intervening spaces */
		while(*flag && isspace(*flag)) flag++;
		field = flag;
		/* Skip to end of flag name/number */
		while(*field && !isspace(*field)) field++;
		/* Terminate flag and skip field forward */
		if (*field) *field++ = '\0';
	}
		
	/* Now get a string with the variable's value in it. */
	/* For dates, if there is an extended format, use it. */
	/* For flags, return the full list but translate the flag name/number */
	value = lookup(varvar,flag,&Subsc,format);

	/* Now process field options */
	if (field) {
		char temp[MAINLINE];
		char *p;
		int Case=0;
		int Just=-1;
		int Width=0;
		int Flag=0;

		lower_string(field);		
		while (*field) switch(*field) {
			case 'o': Flag=F_ON_OFF; field++; break;
			case 's': Flag=F_SET_UNSET; field++; break;
			case 't': Flag=F_TRUE_FALSE; field++; break;
			case 'y': Flag=F_YES_NO; field++; break;
			case 'u': Case=1; field++; break;	/* Upper case */
			case 'c': Case=2; field++; break;	/* Initial caps */
			case 'l': Case=3; field++; break;	/* Lower case */
			case 'r': Just*=-1; field++; break;	/* Reverse justification */
			case '+': Just=-1; field++; break;	/* Left justify */
			case '-': Just=1; field++; break;	/* Right justify */
			case 'w':				/* Set field width */
				while(*field && !isdigit(*field)) field++;
				p = field;
				while(*field && isdigit(*field)) field++;
				if (*field) *field++=0;
				if (*p) Width = atoi(p);
				break;
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			case '\f':
				field++;
				break;
			default:
				/* log an error? */
				field++;
				break;
		}

		if (Subsc > 0) {
			if (Subsc <= (int)strlen(value)) {
				value[0] = value[Subsc-1];
				value[1] = '\0';
			} else {
				value[0] = '\0';
			}
		}

		strncpy(temp,value,MAINLINE);
		temp[MAINLINE-1] = '\0';
		free(value);
		value = str_format(temp,Flag,Case,Just,Width);
	}
	return value;
}

char *transtring (char *varvar) {
	char type;
	char compstring[8];
	int stringnum;
	
	tnt(varvar);

	if (strlen(varvar) < 8) {
		return strdup("");
	}

	strncpy(compstring,varvar,8);
	compstring[7] = 0;
	lower_string(compstring);
	if (!strcmp(compstring,"mstring")) {
		type = 'm';
	} else if (!strcmp(compstring,"ustring")) {
		type = 'u';
	} else {
		return strdup("");
	}
	
	if (is_num(&varvar[7])) {
		stringnum = atoi(&varvar[7]);
	} else {
		return strdup("");
	}
	
	if ((type == 'm') && (stringnum <= MSTRINGCOUNT)) {
		return strdup(Mstring[stringnum]);
	} else if ((type == 'u') && (stringnum <= USTRINGCOUNT)) {
		return strdup(Ustring[stringnum]);
	} else {	
		return strdup("");
	}	
}

char *lookup(char *varvar, char *flag, int *flag_no, char *format) {
	int i;

	/* Default for flag is to atoi() it.  This gives zero for strings. */
	/* This is fixed up for flag strings, lower down. */
	if (flag) {
		*flag_no = atoi(flag);
	} else {
		*flag_no = 0;
	}

	/* Constant strings */
	i = 0;
	while (stringvar2[i].varstring[0]) {
		if (!strcmp(stringvar2[i].varstring,varvar)) {
			return strdup(stringvar2[i].realvar);
		}
		i++;
	}

	/* Varying strings */
	i = 0;
	while (stringvar1[i].varstring[0]) {
		if (!strcmp(stringvar1[i].varstring,varvar)) {
			return strdup(*stringvar1[i].realvar);
		}
		i++;
	}

	/* Small numbers (single bytes) */
	i = 0;
	while (charvar[i].varstring[0]) {
		if (!strcmp(charvar[i].varstring,varvar)) {
			char temp[4];
			sprintf(temp,"%d",(int) *charvar[i].realvar);
			return strdup(temp);
		}
		i++;
	}

	/* Large numbers (four bytes) */
	i = 0;
	while (intvar[i].varstring[0]) {
		if (!strcmp(intvar[i].varstring,varvar)) {
			char temp[16];
			sprintf(temp,"%d",*intvar[i].realvar);
			return strdup(temp);
		}
		i++;
	}

	/* Flag strings - drop the leading # */
	/* These are just like other strings except we know what the
	   flag names mean here */
	i = 0;
	while (flagstrvar[i].varstring[0]) {
		if (!strcmp(flagstrvar[i].varstring,varvar)) {
			if (flag && !is_num(flag)) {
				*flag_no = name_to_num(flagstrvar[i].flagname,flag);
			}
			return strdup(&flagstrvar[i].realvar[1]);
		}
		i++;
	}

	/* Dates */
	i = 0;
	while (datevar[i].varstring[0]) {
		if (!strcmp(datevar[i].varstring,varvar)) {
			char temp[MAINLINE];
			if (format && strcmp(format,"DLT")) {
				(void)strftime(temp,MAINLINE,format,localtime((time_t *)(datevar[i].realvar)));
			} else {
				(void)dlt(temp,MAINLINE,localtime((time_t *)(datevar[i].realvar)));
			}
			return strdup(temp);
		}
		i++;
	}

	/* Functions that do things */
	i = 0;
	while (funcvar[i].keyword[0]) {
		if (!strcmp(funcvar[i].keyword,varvar)) {
			char temp[MAINLINE];
			strcpy(temp,format ? format : "");
			funcvar[i].func(temp);
			return strdup(temp);
		}
		i++;
	}
	return strdup("");
}

int name_to_num (char (*flagnames)[11],char *flagname) {
	int i;

	if (flagname) for(i = 1; flagnames[i][0]; i++) {
		if (!strcmp(flagnames[i],flagname)) {
			return i;
		}
	}
	return 0;
}


char *str_format(const char *value, int Flag, int Case, int Just, int Width) {
	char outformat[MAINLINE];
	char temp[MAINLINE];

	strcpy(outformat,"%");
	if (Just < 0) strcat(outformat,"-");
	if (Width)
	{
		sprintf(temp,"%d.%d",Width,Width);
		strcat(outformat,temp);
	}
	strcat(outformat,"s");
	switch(Flag) {
		case F_ON_OFF:
			sprintf(temp,outformat,(!*value || (*value=='0'))?"off":"on");
			break;
		case F_SET_UNSET:
			sprintf(temp,outformat,(!*value || (*value=='0'))?"unset":"set");
			break;
		case F_TRUE_FALSE:
			sprintf(temp,outformat,(!*value || (*value=='0'))?"false":"true");
			break;
		case F_YES_NO:
			sprintf(temp,outformat,(!*value || (*value=='0'))?"no":"yes");
			break;
		default:
			sprintf(temp,outformat,value);
			break;
	}
	switch(Case) {
		case 1: upper_string(temp); break;
		case 2: capitalise(temp); break;
		case 3: lower_string(temp); break;
	}
	return strdup(temp);
}

int timeon (char *outstring) {
	time_t t = (time(0) - G.starttime);
	char *format = strdup(outstring);

	if (format[0]) {
		(void)strftime(outstring,MAINLINE,format,gmtime(&t));
	} else {
		sprintf(outstring,"%d", (int)t/60);
	}
	free(format);
	return 1;
}


int add_user_to_list (char mode, char *type, char *filename, char *inlist) {
/* This only accepts valid things.  Unused function above would be suitable
   for any other list items 
   type is user or area
   */

	char temp[MAINLINE + 100];
	char tempa[MAINLINE + 100];
	char dir[MAINLINE + 100];
	FILE *FIL;
	struct valid_files *vd;

	if (!strcmp(type,"user")) {
		sprintf(dir,"%s",C.users);
	} else if (!strcmp(type,"area")) {
		sprintf(dir,"%s",C.areasdir);
	}

	vd = get_valid_dirs(mode,0,type,dir,inlist,0);
	
	if (!vd->files[0]) {
		free(vd->input);
		free(vd->files);
		free(vd);
		return 0;
	}

	sprintf(temp,"%s.lock",filename);
	if (!place_lock(mode,temp,1,0)) {
		free(vd->input);
		free(vd->files);
		free(vd);
		return 0;
	}

	while (vd->files[0]) {
		shiftword(vd->files,temp,MAINLINE);

		sprintf(tempa,"egrep -v '^(%s)$' %s >%s.temp 2>/dev/null",temp,filename,filename);
		dsystem(tempa);

		if (FIL = fopen(filename,"a")) {
			fprintf(FIL,"%s\n",temp);
			fclose(FIL);
		} else {
			if (mode == 'v') {
				/*puts("Sorry, cannot open list file.");*/
				printf(Ustring[67],Ustring[320]);
				printf("\n");
			}
			free(vd->input);
			free(vd->files);
			free(vd);
			break;
		}		
	}

	sprintf(temp,"%s.lock",filename);
	rem_lock(temp);
	free(vd->input);
	free(vd->files);
	free(vd);
	return 1;
}

int rem_from_list (char mode, char *filename,char *inlist) {
/* This one is list-centric.  After all users might be killed but you 
   still want to remove them from the list! */

	char temp[1024];
	char tempa[1024];
	unsigned int i;
	struct valid_members *vm;
	FILE *F;

	/*vm = get_valid_members(mode,0,"deletions",filename,inlist,0);*/
	vm = get_valid_members(mode,0,Ustring[286],filename,inlist,0);
	if (!vm->members[0]) {
		free(vm->members);
		free(vm->input);
		free(vm);
		return 0;
	}

	sprintf(temp,"%s.lock",filename);
	if (!place_lock(mode,temp,1,0)) {
		free(vm->members);
		free(vm->input);
		free(vm);
		return 0;
	}
	tempa[0]='\0';
	sprintf(temp,"%s.str",filename);
	F = fopen(temp,"w");
	if (F) {
		while(vm->members[0]) {
			shiftword(vm->members,temp,MAINLINE);
			fprintf(F,"^%s$\n",temp);
		}
		fclose(F);
	}
	sprintf(temp,"grep -vf %s.str %s > %s.temp 2>/dev/null", filename, filename, filename);
	i = (unsigned) dsystem(temp);
	if (WEXITSTATUS(i) > 1) {
		if (mode == 'v') {
			/*printf("Nothing removed from list.\n");*/
			printf("%s\n",Ustring[285]);
		}
	} else {
		sprintf(temp,"cp %s.temp %s 2>/dev/null",filename,filename);
		dsystem(temp);
	}
	sprintf(temp,"%s.temp",filename);
	remove(temp);
	sprintf(temp,"%s.str",filename);
	remove(temp);
	sprintf(temp,"%s.lock",filename);
	rem_lock(temp);
	free(vm->members);
	free(vm->input);
	free(vm);
	return 1;
}



int startlevel (char *templevel) {
/* MENU COMMAND */
	int oldpointer = G.levelpointer;

	while(G.levelpointer > -1) {
		(void)do_exi(G.levelstack[G.levelpointer]);
		G.levelpointer--;
	}
	if (!pushlevel(templevel)) {
		if (oldpointer > -1) {
			G.levelpointer = oldpointer;
		} else {
			/*printf("I can't seem to get a valid menu!\n");*/
			printf("%s %s\n",Ustring[284],Ustring[241]);
			do_exit(22);
		}
	}
	return 1;
}

int pushlevel (char *templevel) {
/* MENU COMMAND */
	char newlevel[11];
	char menuname[15];
	char *copy;
	
	if (G.levelpointer > 14) {
		/*printf("Too many levels stacked\n");*/
		sprintf(G.errmsg,"Too many levels stacked at %s %s\n",G.level, G.command);
		errorlog(G.errmsg);
		printf("%s %s\n",Ustring[284],Ustring[60]);
		return 0;
	}
	
	copy = strdup(templevel);
	tnt(copy);
	if (!copy[0]) {
		/*printf("No level specified at %s %s\n",G.level,G.command);*/
		sprintf(G.errmsg,"No level specified at %s %s\n",G.level, G.command);
		errorlog(G.errmsg);
		printf("%s %s\n",Ustring[284],Ustring[60]);
		free(copy);
		return 0;
	}
	get_one_file(newlevel,11,copy);
	free(copy);

	free_menus();
	sprintf(menuname,"%s.cmd",newlevel);
	if (!findorload_menu(menuname)) {
		/*printf("Cmd file not found for level %s\n",newlevel);*/
		sprintf(G.errmsg,"Cmd file not found for level %s\n",G.level);
		errorlog(G.errmsg);
		printf("%s %s\n",Ustring[284],Ustring[60]);
		return 0;
	}
	sprintf(menuname,"%s.pmp",newlevel);
	findorload_menu(menuname);
	sprintf(menuname,"%s.ini",newlevel);
	findorload_menu(menuname);
	sprintf(menuname,"%s.exi",newlevel);
	findorload_menu(menuname);

	/* below here it WILL have worked ok */
	G.levelpointer++;
	strcpy(G.levelstack[G.levelpointer],newlevel);
	strcpy(G.level,newlevel);

	(void)do_ini(G.level);

	return 1;
}

int poplevel (char *templevel) {
/* MENU COMMAND */
	char newlevel[11];
	char menuname[15];
	char *copy;
	
	newlevel[0] = 0;

	copy = strdup(templevel);
	tnt(copy);
	if (copy[0]) {
		get_one_file(newlevel,11,copy);
	}
	free(copy);
	
	if (newlevel[0]) {
		while((G.levelpointer >= 0) && strcmp(G.levelstack[G.levelpointer],newlevel)) {
			(void)do_exi(G.levelstack[G.levelpointer]);
			G.levelpointer--;
		}
	} else {
		if (G.levelpointer >= 0) {
			(void)do_exi(G.levelstack[G.levelpointer]);
			G.levelpointer--;
		}
	}

	if (G.levelpointer < 0) {
		(void)startlevel(newlevel[0] ? newlevel : "top");
	} else {
		free_menus();
		sprintf(menuname,"%s.cmd",G.levelstack[G.levelpointer]);
		if (findorload_menu(menuname)) {
			sprintf(menuname,"%s.pmp",G.levelstack[G.levelpointer]);
			findorload_menu(menuname);
			sprintf(menuname,"%s.ini",G.levelstack[G.levelpointer]);
			findorload_menu(menuname);
			sprintf(menuname,"%s.exi",G.levelstack[G.levelpointer]);
			findorload_menu(menuname);
			strcpy(G.level,G.levelstack[G.levelpointer]);
		} else {
			/*printf("Cmd file not found for level %s\n",G.levelstack[G.levelpointer]);*/
			sprintf(G.errmsg,"Cmd file not found for level %s\n",G.levelstack[G.levelpointer]);
			errorlog(G.errmsg);
			printf("%s %s\n",Ustring[284],Ustring[60]);
			(void)startlevel("top");
		}
	}
	return 1;
}

int swaplevel (char *templevel) {
/* MENU COMMAND */
	poplevel("");
	return pushlevel(templevel);
}

int do_menudisplay (char *templevel) {
/* MENU COMMAND */
	(void)do_cmd('d',templevel);
	return 1;
}

int do_prompt_hk (char *promptparam) {
/* MENU COMMAND */
/* gets user input respecting their hotkey choice */
	return do_prompt(0,promptparam);
}

int do_prompt_cr (char *promptparam) {
/* MENU COMMAND */
/* gets user input requiring a CR (overrides hotkeys) */
	return do_prompt(1,promptparam);
}

int do_prompt (int type,char *promptparam) {
/* MENU COMMAND */
/* called if an action was a prompt type */
	char filename[MAINLINE + 100];
	char *copy = strdup(promptparam);
	FILE *FIL;

	Looping_counter = 0;

	tnt(copy);
	trans_string(copy,filename,MAINLINE);
	free(copy);

	make_prompt(filename);

	if (type == 0) {
		get_commandline(G.comline);
	} else {
		get_one_line(G.comline);
	}
	
	sprintf(filename,"%s/%s/.force",C.users,U.id);
	if (FIL = fopen(filename,"r")) {
		fgets(G.comline,MAINLINE,FIL);
		fclose(FIL);
		tnt(G.comline);
		remove(filename);
	}
	return 1;
}



int do_get (char *promptparam) {
/* MENU COMMAND */
	char temp[MAINLINE + 10];
	char *copy = strdup(promptparam);

	tnt(copy);
	trans_string(copy,temp,MAINLINE);
	strcat(temp," ");
	make_prompt(temp);
	free(copy);

	get_one_line(G.got);
	if (G.got[0]) {
		return 1;
	} else {
		return 0;
	}
}

int do_take (char *promptparam) {
/* MENU COMMAND */
	char temp[MAINLINE + 10];
	char *copy = strdup(promptparam);

	if (G.comline[0]) {
		strcpy(G.taken,G.comline);
		flushcom("");
	} else {
		tnt(copy);
		trans_string(copy,temp,MAINLINE);
		strcat(temp," ");
		make_prompt(temp);
		get_one_line(G.taken);
	}
	free(copy);

	if (G.taken[0]) {
		return 1;
	} else {
		return 0;
	}
}

int set_customvar (char *params) {
/* MENU COMMAND */
/* For lengths, relies on the fact the menu actions field cannot be more than
   MAINLINE chars long! */
   
	char whichvar[2];
	int whichvarnum;
	char *copy = strdup(params);
	char temp[MAINLINE];
	
	shiftword(copy,whichvar,2);
	whichvarnum = atoi(whichvar);
	if ((whichvarnum < 1) || (whichvarnum > 8)) {	
		sprintf(temp,"Bad value for custom variable at %s, %s",G.level,G.command);
		errorlog(temp);
		free(copy);
		return 0;
	}
	tnt(copy);
	switch(whichvarnum) {
		case 1:
			strcpy(G.customvar1,copy);
			break;
		case 2:
			strcpy(G.customvar2,copy);
			break;
		case 3:
			strcpy(G.customvar3,copy);
			break;
		case 4:
			strcpy(G.customvar4,copy);
			break;
		case 5:
			strcpy(G.customvar5,copy);
			break;
		case 6:
			strcpy(G.customvar6,copy);
			break;
		case 7:
			strcpy(G.customvar7,copy);
			break;
		case 8:
			strcpy(G.customvar8,copy);
			break;
	}				
	free(copy);
	return 1;
}

/* ARGSUSED0 */
int press_enter (char *dummy) {
/* MENU COMMAND */
	char temp[2];

	/*make_prompt("Press Enter to continue...");*/
	make_prompt(Ustring[291]);
	get_raw(1, 1, temp, 0);
	return 1;
}

/* ARGSUSED0 */
int do_continue (char *dummy) {
/* MENU COMMAND */

/*	
	char response[2];

	make_prompt("[C]ontinue or [s]top? ");
	get_one_lc_char(response);
	return ((response[0] == 0) || (response[0] == 'c'));
*/
	return yes_no(Ustring[292]);
}

/* ARGSUSED0 */
int sure (char *dummy) {
/* MENU COMMAND */
/*
	char response[2];
	make_prompt("Are you sure? y/N ");
	get_one_lc_char(response);
	return (response[0] == 'y');
*/
	return no_yes(Ustring[293]);	
}

/* ARGSUSED0 */
int yes_no (char *params) {
/* MENU COMMAND */
	char response[2];
	char temp[MAINLINE + 20];
	char *copy = strdup(params);

	tnt(copy);
	/*sprintf(temp,"%s Y/n ",copy);*/
	sprintf(temp,"%s %c/%c ",copy,G.bigyes,G.littleno);
	free(copy);

	make_prompt(temp);

	get_yn(response); /* This only allows y, n, or return */
	/*return (response[0] != 'n');*/
	tolower(response[0]);
	return (response[0] != G.littleno);
}

/* ARGSUSED0 */
int no_yes (char *params) {
/* MENU COMMAND */
	char response[2];
	char temp[MAINLINE + 20];
	char *copy = strdup(params);	

	tnt(copy);
	/*sprintf(temp,"%s y/N ",copy);*/
	sprintf(temp,"%s %c/%c",copy,G.littleyes,G.bigno);
	free(copy);
	make_prompt(temp);

	get_yn(response);
	/*return (response[0] == 'y');*/
	tolower(response[0]);
	return (response[0] == G.littleyes);
}

int do_system (char *systemparam) {
/* MENU COMMAND */
/* From menuline only */
	int result;

	external_term();
	result = !dsystem(systemparam);
	internal_term();
	return result;
}

int do_shell (char *in) {
/* MENU COMMAND */
	int result;
	char *copy = strdup(in);

	tnt(copy);
	external_term();
	if (copy[0]) {
		result = !usystem(copy);
	} else {
		result = !usystem(G.comline);
		flushcom("");
	}
	internal_term();
	return result;
}

/* ARGSUSED0 */
int flushcom (char *dummy) {
/* MENU COMMAND */
	G.comline[0] = 0;
	return 1;
}

int edit (char *in) {
/* MENU COMMAND */
/* will not allow paths.  G.dir must be set */
	char inputfile[MAINLINE + MAINLINE];
	struct valid_files *vf;
	char *temp;
	char *copy = strdup(in);

	if (!is_dir_elig('v',G.dir)) {
		/*printf("Directory not available - abandoned.\n");*/
		printf("%s - %s\n",Ustring[245],Ustring[60]);
		sprintf(inputfile,"Directory not set at %s, %s",G.level,G.command);
		errorlog(inputfile);
		free(copy);
		return 0;
	}
	shiftword(copy,inputfile,MAINLINE);
	if (!inputfile[0]) {
		shiftword(G.comline,inputfile,MAINLINE);
	}	
	free(copy);
	
	/*vf = get_valid_files('v',1,"file",G.dir,inputfile,1);*/
	vf = get_valid_files('v',1,Ustring[249],G.dir,inputfile,1);

	if (!vf->files[0]) {
		if (vf->input[0]) {
			get_one_file(inputfile,C.maxfilename + 1,vf->input);
		}
	} else {
		strcpy(inputfile,vf->files);
	}
	free(vf->input);
	free(vf->files);
	free(vf);

	if (!inputfile[0]) {
		return 0;
	}

	temp = strdup(inputfile);
	sprintf(inputfile,"%s/%s",G.dir,temp);
	free(temp);

	if (!edit_special(inputfile)) {
		return 0;
	}
	return 1;
}

int edit_special (char *fullfile) {
/* MENU COMMAND */
/* Takes full paths, but not from G.comline! */
	FILE *FIL;
	int result;
	struct stat statbuf;
	char temp[MAINLINE + 100];
	char workpad[MAINLINE + 100];
	char *basename;
	char *copy = strdup(fullfile);

	tnt(copy);

	if (!copy[0]) {
		/*printf("No file specified\n");*/
		sprintf(temp,Ustring[294],Ustring[249]);
		printf("%s - %s\n",temp,Ustring[60]);
		sprintf(temp,"No file specified at %s, %s",G.level,G.command);
		errorlog(temp);
		free(copy);
		return 0;
	}

	basename = base_name(copy,'/');

	if (!(FIL = fopen(copy,"a+"))) {
		/*printf("Read/write access denied on %s.\n",copy);*/
		printf(Ustring[67],copy);
		printf("\n");
		return 0;
	}
	fclose(FIL);

	sprintf(temp,"cp workpad workpad~ 2>/dev/null");
	dsystem(temp);

	sprintf(workpad,"workpad");

	if (!stat(workpad,&statbuf) && (statbuf.st_size > 0)) {
		/*printf("Your workpad already contains text\n");*/
		printf("%s\n",Ustring[172]);
		result = wannasave(workpad,copy);
	} else {
		if (!stat(copy,&statbuf) && (statbuf.st_size > 0)) {
			/*printf("Loading existing %s into workpad...\n",basename);*/
			printf(Ustring[173],basename);
			printf("\n");
			sprintf(temp,"cp %s %s 2>/dev/null",copy,workpad);
			dsystem(temp);
		} else {
			/*printf("Editing new file %s...\n",basename);*/
			printf(Ustring[174],basename);
			printf("\n");
		}
		fflush(stdout);
		if ( (result = editposting(workpad,copy)) ) {
			result = wannasave(workpad,copy);
		}
	}

	sprintf(temp,"workpad~");
	remove(temp);
	free(basename);
	free(copy);
	return result;
}


/* ARGSUSED1 */
int editposting (char *workpad, char *endresult) {
	char response[2];
	char temp[MAINLINE + 100];
	char dir[MAINLINE + 100];
	char string[MAINLINE];
	struct valid_files *vf;
	char *changed;

	tnt(Ustring[297]);
	tnt(Ustring[298]);
	tnt(Ustring[299]);
	tnt(Ustring[230]);
	tnt(Ustring[231]);

	/* CONSTCOND */
	while (1) {
#if defined(FILE_COMMANDS)
		if (U.level < C.pvtfileslevel) {
			/*make_prompt("[E]dit online, [u]pload, [o]k done, or [a]bandon: ");*/
			make_prompt(Ustring[295]);
		} else {
			/*make_prompt("[E]dit online, [u]pload, use [f]ile, [o]k done, or [a]bandon: ");*/
			make_prompt(Ustring[296]);
		}
#else
		make_prompt("[E]dit, [o]k done, or [a]bandon: ");
#endif
		get_one_lc_char(response);
		if (!response[0]) {
			/*r = 'e';*/
			strncpy(response,Ustring[297],1);
			response[1] = 0;
		}

		if (menumatch(response,Ustring[301])) { /*abandon*/
			sprintf(temp,"%s~",workpad);
			remove(workpad);
			copy_file(temp,workpad,0);
			return 0;
		} else if (menumatch(response,Ustring[300])) { /*ok done*/
			return 1;
		} else if (menumatch(response,Ustring[297])) { /*edit online*/
			if (strcmp(U.editor,C.editor1) && strcmp(U.editor,C.editor2) && strcmp(U.editor,C.editor3)) {
				/*printf("\nYour selected editor is no longer available.  Please choose another.\n");*/
				printf("\n%s\n",Ustring[302]);
				return 1;
			}					
			sprintf(temp,"%s %s",U.editor,workpad);
			/*printf("Entering editor...\n");*/
			printf("%s\n",Ustring[175]);
			external_term();
			usystem(temp);
			internal_term();
			return 1;
		} else if ((U.level > C.pvtfileslevel) && (menumatch(response,Ustring[299]))) { /*use file*/
			sprintf(dir,"%s/%s",C.privatefiles,U.id);
			/*vf = get_valid_files('v',0,"files",dir,G.comline,0);*/
			vf = get_valid_files('v',0,Ustring[266],dir,G.comline,0);
			flushcom("");
			while(vf->files[0]) {
				shiftword(vf->files,string,MAINLINE);
				sprintf(temp,"cat %s/%s/%s >> %s 2>/dev/null",C.privatefiles,U.id,string,workpad);
				dsystem(temp);
			}
			free(vf->input);
			free(vf->files);
			free(vf);
			return 1;
		} else if (menumatch(response,Ustring[298])) { /*upload*/
			sprintf(dir,"%s/%s",C.privatefiles,U.id);
			changed = uploading(dir,"");
			while (changed[0]) {
				shiftword(changed,string,MAINLINE);
				if (temp[0] != '.') {
					sprintf(temp,"cat %s/%s >> %s",dir,string,workpad);
					dsystem(temp);
					/* keep it tidy */
					remove(string);
				}
			}
				
			free(changed);
			return 1;
		} else {
			/*printf("'%s' invalid.\n",response);*/
			sprintf(temp,"%s",response);
			printf(Ustring[472],temp);
			printf("\n");
		}

	}
	/* NOTREACHED */
}

int wannasave (char *workpad, char *endresult) {
	char response[2];
	char temp[MAINLINE + 100];
	char tempa[MAINLINE];
	char *basename = base_name(endresult,'/');

	tnt(Ustring[305]);
	tnt(Ustring[306]);
	tnt(Ustring[307]);
	tnt(Ustring[308]);
	tnt(Ustring[309]);
	tnt(Ustring[310]);

	/* CONSTCOND */
	while (1) {

		/*sprintf(temp,"[S]ave to %s, [e]dit, [l]ist, [?]help,\n[h]old workpad, [c]lear workpad, [a]bandon: ",basename);*/
		sprintf(tempa,Ustring[303],basename);
		sprintf(temp,"%s\n%s",tempa,Ustring[304]);
		make_prompt(temp);

		get_one_lc_char(response);
		if (!response[0]) {
			strncpy(response,Ustring[305],1);
			response[1] = 0;
		}

		if (menumatch(response,Ustring[305])) { /*save*/
			copy_file(workpad,endresult,1);
			/*printf("Saved to %s.\n",basename);*/
			printf(Ustring[311],basename);
			printf("\n");
			free(basename);
			if (yes_no(Ustring[312])) {
				remove(workpad);
			}
			return 1;
		} else if (menumatch(response,Ustring[308])) { /*hold*/
			free(basename);
			return 0;
		} else if (menumatch(response,Ustring[306])) { /*edit*/
			editposting(workpad,endresult);
		} else if (menumatch(response,"?")) {
			display_lang("savepad");
		} else if (menumatch(response,Ustring[307])) { /*list*/
			display(workpad);
		} else if (menumatch(response,Ustring[310])) { /*abandon*/
			sprintf(temp,"%s~",workpad);
			remove(workpad);
               		copy_file(temp,workpad,0);
			free(basename);
			return 0;
		} else if (menumatch(response,Ustring[309])) { /*clear*/
			remove(workpad);
			free(basename); 
			return 0; 	
		}
	}
	/* NOTREACHED */
}

int newuserlog (char *param) {
	FILE *LOG;
	int result = 0;
	char filename[MAINLINE + 100];
	char *date = shorttime(time(0));

	sprintf(filename,"%s/newuserlog",C.datadir);
	if ( (LOG=fopen(filename,"a")) ) {
		fprintf(LOG,"NEW USER: %s %s\n",date,param);
		fclose(LOG);
		result = 1;
	}
	free(date);
	return result;
}

int view (char *in) {
/* MENU COMMAND */
/* Viewing only allowed for G.dir */

	struct stat statbuf;
	char filename[MAINLINE + 100];
	int result = 0;
	struct valid_files *vf;
	char *copy;

	if (!is_dir_elig('v',G.dir)) {
		return 0;
	}
	
	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		/*vf = get_valid_files('v',1,"file",G.dir,copy,0);*/
		vf = get_valid_files('v',1,Ustring[249],G.dir,copy,0);
	} else {
		shiftword(G.comline,filename,MAINLINE);
		/*vf = get_valid_files('v',1,"file",G.dir,filename,0);*/
		vf = get_valid_files('v',1,Ustring[249],G.dir,filename,0);
	}	
	
	if (vf->files[0]) {
		sprintf(filename,"%s/%s",G.dir,vf->files);
		if (!stat(filename,&statbuf)) {
			if (statbuf.st_size) {
				result = display(filename);
			} else {
				/*printf("There is nothing in %s.\n",vf->files);*/
				printf(Ustring[313],vf->files);
				printf("\n");
			}
		}
	}
	free(vf->input);
	free(vf->files);
	free(vf);
	free(copy);
	return result;
}

/* ARGSUSED0 */
int update_check (char *dummy) {
/* MENU COMMAND */
	struct stat statbuf;
	char filename[MAINLINE + 100];

	if (G.update) {
		G.update = 0;
		sprintf(filename,"%s/%s/.updatealert",C.users,U.id);
		if (!stat(filename,&statbuf)) {
			update("");
			remove(filename);
			return 1;
		}
	}
	return 0;
}


/* ARGSUSED0 */
int finger (char *in) {
/* MENU COMMAND */
	return fingering('v',"terminal",in);
}

int fingergrab (char *in) {
/* MENU COMMAND */
	return fingering('v',"grab",in);
}

int fingerget (char *in) {
	return fingering('v',"get",in);
}

int fingering (char mode, char *type, char *in) {
	char propername[MAINLINE];
	char filename[MAINLINE + 100];
	char pad[MAINLINE + 50];
	char user[9];
	char brackets[MAINLINE];
	struct stat statbuf;
	FILE *FIL;
	struct valid_files *vf;
	char *laston;
	char *copy = strdup(in);

	tnt(copy);
	if (copy[0]) {
		/*vf = get_valid_dirs(mode,1,"user",C.users,copy,0);*/
		vf = get_valid_dirs(mode,1,Ustring[196],C.users,copy,0);
	} else {
		shiftword(G.comline,user,9);
		/*vf = get_valid_dirs(mode,1,"user",C.users,user,0);*/
		vf = get_valid_dirs(mode,1,Ustring[196],C.users,user,0);
	}

	strcpy(user,vf->files);
	free(vf->files);
	free(vf->input);
	free(vf);
	free(copy);

	if (!user[0]) {
		return 0;
	}

	laston = drealmtime(lastcall_read(user));
	get_propername(user,propername,31);

	if (propername[0] == 0) {
		brackets[0] = 0;
	} else {
		sprintf(brackets," (%s)",propername);
	}

	sprintf(filename,"%s/%s/.plan",C.privatefiles,user);
	if (!strcmp(type,"terminal")) {
		/*printf("%s%s was last on %s\n",user,brackets,laston);*/
		printf(Ustring[317],user,brackets,laston);
		printf("\n");
		if (!stat(filename,&statbuf) && (statbuf.st_size)) {
			display(filename);
		} else {
			/*printf("Planfile not available.\n");*/
			printf("%s\n",Ustring[315]);
		}
	} else if (!strcmp(type,"grab")) {
		sprintf(pad,"%s/%s/grabpad",C.privatefiles,U.id);
		if (U.level < C.pvtfileslevel) {
			if (mode == 'v') {
				/*printf("You do not have access to this facility.\n");*/
				printf(Ustring[316],Ustring[464]);
				printf("\n");
			}
			free(laston);
			return 0;
		}
		if (!(FIL = fopen(pad,"a"))) {
			if (mode == 'v') {
				/*printf("You do not have access to your grabpad.\n");*/
				printf(Ustring[67],"grabpad");
				printf("\n");
			}
			free(laston);
			return 0;
		}
		fprintf(FIL,"\nPlanfile: %s\n",user);
		/*fprintf(FIL,"%s%s was last on %s\n",user,brackets,laston);*/
		fprintf(FIL,Ustring[317],user,brackets,laston);
		fprintf(FIL,"\n");

		if (!stat(filename,&statbuf) && (statbuf.st_size)) {
			fclose(FIL);
			append_file(filename,pad,0);
			FIL = fopen(pad,"a");
		} else {
			/*fputs("Planfile not available.\n",FIL);*/
			fprintf(FIL,"%s\n",Ustring[315]);
		}
		fputs("\n.\n",FIL);
		fclose(FIL);

	} else if (!strcmp(type,"get")) {
		sprintf(pad,"workpad");
		if (U.level < C.pvtfileslevel) {
			if (mode == 'v') {
				/*printf("You do not have access to this facility.\n");*/
				printf(Ustring[316],Ustring[464]);
				printf("\n");
			}
			free(laston);
			return 0;
		}
		if (!(FIL = fopen(pad,"a"))) {
			if (mode == 'v') {
				/*printf("You do not have access to your grabpad.\n");*/
				printf(Ustring[67],"grabpad");
			}
			free(laston);
			return 0;
		}
		fprintf(FIL,"\nPlanfile: %s\n",user);
		/*fprintf(FIL,"%s%s was last on %s\n",user,brackets,laston);*/
		fprintf(FIL,Ustring[317],user,brackets,laston);
		fprintf(FIL,"\n");

		if (!stat(filename,&statbuf) && (statbuf.st_size)) {
			fclose(FIL);
			append_file(filename,pad,0);
			FIL = fopen(pad,"a");
		} else {
			/*fputs("Planfile not available.\n",FIL);*/
			fprintf(FIL,"%s\n",Ustring[315]);
		}
		fputs("\n.\n",FIL);
		fclose(FIL);

	}
	free(laston);
	return 1;
}


/* ARGSUSED0 */
int users (char *dummy) {
/* MENU COMMAND */
	char command[MAINLINE + 100];

	sprintf(command,"ls %s > view",C.users);
	dsystem(command);
	return display("view");
}

int plansearch (char *in) {
/* MENU COMMAND */

	char string[MAINLINE];
	char pattern[MAINLINE];
	FILE *FIL;
	DIR  *DIRT;
	struct dirent *d;
	char filename[MAINLINE + 100];
	char line[MAINLINE + 1];
	char caseline[MAINLINE + 1];
	int linecount = 0;
	int stopped = 0;
	int nr_found = 0;
	int i = 0;
	char *copy;
#if defined(LINUX) || defined(SVR42)
	char *shellpat;
	int j = 0;
#  if defined(LINUX)
	int re;
	regex_t preg;
#  else
	char *re;
#  endif
#endif



	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		if (copy[0] == '"') {
			strshift(copy,pattern,MAINLINE,"\"");
			strshift(copy,pattern,MAINLINE,"\"");
		} else {
			strshift(copy,pattern,MAINLINE,"from");
			tnt(pattern);
		}
	} else {
		if (G.comline[0] == '"') {
			strshift(G.comline,pattern,MAINLINE,"\"");
			strshift(G.comline,pattern,MAINLINE,"\"");
		} else {
			strshift(G.comline,pattern,MAINLINE,"from");
			tnt(pattern);
		}
	}
	
	flushcom("");
	free(copy);

	if (!pattern[0]) {
		/* CONSTCOND */
		while (1) {
			/*printf("Please type search pattern between double quotes (\"\") or ? for help.\n");*/
			printf("%s (%s)\n",Ustring[194],Ustring[200]);
			/*make_prompt("Search pattern: ");*/
			make_prompt(Ustring[119]);
			get_one_line(string);
			tnt(string);
			if (!string[0]) {
				return 0;
			}
			if (!strcmp(string,"?")) {
				do_ps_help();
				continue;
			}

			if ((string[0] == '"') && (string[strlen(string) - 1] == '"')) {
				strshift(string,pattern,21,"\"");
				strshift(string,pattern,MAINLINE,"\"");
			} else {
				strshift(string,pattern,MAINLINE,"from");
				tnt(pattern);
			}
			if (!pattern[0]) {
				return 0;
			}
			break;
		}
	}
	
	lower_string(pattern);


#if defined(LINUX) || defined(SVR42)
	shellpat = (char *)malloc(strlen(pattern) * 2 + 1);

	for(i = 0;pattern[i];i++) {
		switch(pattern[i]) {
			case '*':
				shellpat[j++] = '.';
				shellpat[j++] = '*';
				break;
			case '?':
				shellpat[j++] = '.';
				break;
			default:
				if (ispunct(pattern[i])) {
					shellpat[j++] = '\\';
				}
				shellpat[j++] = pattern[i];
		}
	}
	shellpat[j] = 0;

#  if defined(SVR42)
	re = regcmp(shellpat,NULL);
#  else /* LINUX */
	re = !regcomp(&preg,shellpat,0);
#  endif
	free(shellpat);

	if (re) {
		if (DIRT = opendir(C.users)) {
			/* LINTED */
			while ((d = readdir(DIRT)) && !stopped) {
				if (d->d_name[0] == '.') {
					continue;
				}

				sprintf(filename,"%s/%s/.plan",C.privatefiles,d->d_name);
				if (FIL = fopen(filename,"r")) {
					while (fgets(line,MAINLINE,FIL)) {
						j=strlen(line)-1;
						while(j && isspace(line[j])) {
							line[j] = 0;
							j--;
						}
						strcpy(caseline,line);
						lower_string(line);
#  if defined(LINUX)
						if (!regexec(&preg,line,0,0,0)) /*bracket below!! HAHA */
#  else
						if (regex(re,line) != NULL) /*bracket below!! HAHA */
#  endif

						{
							printf("%-14.14s: %-.60s\n",d->d_name,caseline);
							nr_found++;
							linecount++;
							if (linecount > LINES-5) {
								if (!do_continue("")) {
									stopped = 1;
									break;
								}
								linecount = 0;
							}

						}
					}
					fclose(FIL);
				}
			}
			closedir(DIRT);
		} else {
			/*printf("Error reading directories.\n");*/
                        printf(Ustring[66],Ustring[272]);
                        printf(" - %s\n",Ustring[60]);
                        
#  if defined(SVR42)
			free(re);
#  endif
			return 0;
		}
#  if defined(SVR42)
		free(re);
#  else
		regfree(&preg);
#  endif
		if (!nr_found) {
			/*printf("No matches.\n");*/
			printf("%s\n",Ustring[123]);
		}
		return 1;
	} else {
		/*printf("Regex error.\n");*/
                printf("%s Regex\n",Ustring[276]);  
		return 0;
	}
#else
	if (!C.awkname[0]) {
		/*for each users planfile */
		sprintf(filename,"%s/%s/.plan",C.privatefiles,dirname);
		if (FIL = fopen(filename,"r")) {
			while (!feof(FIL)) {
				fgets(line,MAINLINE,FIL);
				tnt(line);
				if (strstr(line,pattern)) {
					printf("%-14.14s: %-.60s\n",dirname,line);
				}
			}
			fclose(FIL);
		}
	} else {
		/*puts("Sorry, no awk script written yet! (-:");*/
	}
	return 1;
#endif
}

void do_ps_help (void) {
	display_lang("ps");
}

void do_logins (void) {
	FILE *FIL;
	char lockname[MAINLINE + 100];
	char filename[MAINLINE + 100];
	char exclusions[MAINLINE];
	char line[80];
	char *dlmdate;
	int totalshift = 0;


	sprintf(filename,"%s/config.stats",C.configdir);
	if ( (FIL = fopen(filename,"r")) ) {
		get_next_cfgfield (FIL,&totalshift,lockname,MAINLINE);
		get_next_cfgfield (FIL,&totalshift,exclusions,MAINLINE);
		tnt(exclusions);
		fclose(FIL);
	}
	while (exclusions[0]) {
		shiftword(exclusions,lockname,9);
		if (!strcmp(lockname,U.id)) { /*if this is an exclusion */
			return;
		}					
	}

	dlmdate = drealmtime(time(0));
	sprintf(line,"%s %s on %s\n",U.id,dlmdate,G.nicename);
	free(dlmdate);

	sprintf(lockname,"%s/logins.lock",C.datadir);
	if (place_lock('q',lockname,1,0)) {
		sprintf(filename,"%s/logins",C.datadir);

		if (FIL = fopen(filename,"a")) {
	 		fputs(line,FIL);
			fclose(FIL);
		}
		rem_lock(lockname);
	} else {
		sprintf(filename,"Could not place line in logins: %s",line);
		errorlog(filename);
	}
}

int set_doing(char *params) {
/* MENU COMMAND */
	FILE *TMP;
	char temp[MAINLINE + 100];

	sprintf(temp,"%s/%s/.doing",C.users,U.id);
	if (TMP = fopen(temp,"w")) {
		char *copy = strdup(params);
		tnt(copy);
		fputs(copy,TMP);
		fclose(TMP);
		free(copy);
		return 1;
	} else {
		return 0;
	}
}

void do_exit(int exit_code) {
#if defined(CHAT_COMMANDS)
	chatdisable("");
#endif
	if (exit_code) {
		char message[MAINLINE];
		sprintf(message,"do_exit(%d)",exit_code);
		errorlog(message);
	}
	intr_on();
	external_term();
	restore_term();
	exit(exit_code);
}

/* ARGSUSED0 */
int noticecheck (char *dummy) {
/* MENU COMMAND */
/* For automatic checking */
	char filename[MAINLINE + 100];
	struct stat statbuf;

	if (G.nupdate) {
		G.nupdate = 0;
		sprintf(filename,"%s/%s/.notice",C.users,U.id);
		if (!stat(filename,&statbuf)) {
			/*printf("\n\a*** Special Notice for you %s ***\n",U.id);*/
			printf("\n\a");
			printf(Ustring[318],U.id);
			printf("\n");
			display(filename);
			remove(filename);
			return 1;
		}
	}
	return 0;
}

/* ARGSUSED0 */
int rawlog_on (char *dummy) {
/* MENU COMMAND */

	char filename[MAINLINE + 100];

	if (G.rawlog) {
		return 1;
	}
				
	sprintf(filename,"%s/inputlog",C.datadir);
	if (RAWLOG = fopen(filename,"a")) {
		G.rawlog = 1;
		return 1;
	} else {
		G.rawlog = 0;
		return 0;
	}
}	

/* ARGSUSED0 */
int rawlog_off (char *dummy) {
/* MENU COMMAND */

	if (!G.rawlog) {
		return 1;
	}
	fclose(RAWLOG);
	G.rawlog = 0;
	return 1;
}	

/* ARGSUSED0 */
int chatlog_on (char *dummy) {
/* MENU COMMAND */

	char filename[MAINLINE + 100];

	if (G.chatlog) {
		return 1;
	}
				
	sprintf(filename,"%s/chatlog",C.datadir);
	if (CHATLOG = fopen(filename,"a")) {
		G.chatlog = 1;
		return 1;
	} else {
		G.chatlog = 0;
		return 0;
	}
}	

/* ARGSUSED0 */
int chatlog_off (char *dummy) {
/* MENU COMMAND */

	if (!G.chatlog) {
		return 1;
	}
	fclose(CHATLOG);
	G.chatlog = 0;
	return 1;
}	

int notify_user (char *in) {
/* MENU COMMAND */
	char user[9];
	char notice[MAINLINE];
	char filename[MAINLINE + 100];
	FILE *FIL;
	char *copy = strdup(in);

	shiftword(copy,filename,9);
	if (!filename[0]) {
		shiftword(G.comline,filename,9);
	}	

	if (!ask_for_user(filename,user)) {
		free(copy);
		return 0;
	}

	tnt(copy);
	if (copy[0]) {
		strcpy(notice,copy);
	} else {
		strcpy(notice,G.comline);
		flushcom("");
	}
	free(copy);
		
	if (!notice[0]) {
		sprintf(filename,"Notice for %s: ",user);
		make_prompt(filename);
		get_one_line(notice);
	}

	if (!notice[0]) {
		return 0;
	}

	sprintf(filename,"%s/%s/.notice",C.users,user);
	if (FIL = fopen(filename,"a")) {
		fprintf(FIL,"%s\n",notice);
		fclose(FIL);
		sprintf(filename,"touch %s/%s",C.users,user);
		dsystem(filename);
		return 1;
	} else {
		return 0;
	}
}

int is_line (char *params) {
	
	char *copy = strdup(params);
	tnt(copy);	
	if (!copy[0]) {
		free(copy);
		return 0;
	}
	if (atoi(copy) == G.line) {
		free(copy);
		return 1;
	} else {
		free(copy);
		return 0;
	}
}


int check_interval (char *params) {
	char *copy = strdup(params);
	int minutes;

	tnt(copy);	
	if (!copy[0]) {
		free(copy);
		return 0;
	}
	minutes = atoi(copy);
	free(copy);
	if (!minutes) {
		return 0;
	}

	if ( ( (time(0) - G.laston) / 60) < minutes) {
		return 1;
	} 
	return 0;
}

int dlm_log (char *param) {
/* MENU COMMAND */
/* CHECKED */
	FILE *LOG;
	int result = 0;
	char *date;
	char filename[MAINLINE + 100];

	date = shorttime(time(0));

	sprintf(filename,"%s/log",C.datadir);
	if ( (LOG=fopen(filename,"a")) ) {
		fprintf(LOG,"%s %s\n",date,param);
		fclose(LOG);
		result = 1;
	}
	free(date);
	return result;
}

int logfile (char *params) {
/* MENU COMMAND */
	char filename[MAINLINE + 100];
	FILE *LOG;
	int result = 0;
	char *copy = strdup(params);

	shiftword(copy,filename,(MAINLINE + 50));
	if ( (LOG=fopen(filename,"a")) ) {
		fprintf(LOG,"%s\n",copy);
		fclose(LOG);
		result = 1;
	}
	free(copy);
	return result;
}

int randomnum (char *maxnumstring) {
	double num;

	tnt(maxnumstring);
	if (is_num(maxnumstring)) {
		num = atoi(maxnumstring);
		srand((unsigned)time(0));
		G.randomnum = (int)((num*rand())/RAND_MAX)+1;
		return 1;
	} else {
		G.randomnum = 0;
		errorlog("No maximum random number specified.");
		return 0;
	}		
}


int display (char *params) {
/* MENU COMMAND PLUS */

	return display_it(params,0);
}




int display_lang (char *params) {
/* MENU COMMAND PLUS */
	return display_it(params,1);
}


int display_it (char *params,int type) {
/* MENU COMMAND PLUS */
/* type is 0 for taking whole filename, 1 for just taking base */
	char command[MAINLINE * 2 + 50];
	char filename[MAINLINE + 100];
	struct stat s;	
	char temp[MAINLINE];
	char *copy = strdup(params);
	

	shiftword(copy,temp,MAINLINE);
	free(copy);

	/* this copies temp back over filename even if type is 1
	if ((type == 1) && (!whichlangfile(temp,filename))) {
		return 0;
	} else {
		strcpy(filename,temp);
	}
	*/

	if (type == 1) { 
		if (!whichlangfile(temp,filename)) {
			return 0;
		}
	} else {
		strcpy(filename,temp);
	}

	if ((!stat(filename,&s)) && (s.st_size > 0)) {
		if (strcmp(U.display,C.display1) && strcmp(U.display,C.display2) && strcmp(U.display,C.display3)) {
			/*printf("\nYour selected display program is no longer available.  Please choose another.\n");*/
			printf("\n%s\n",Ustring[314]);
			return 0;
		}					

		sprintf(command,"view");
		if (strcmp(command,params)) {
			sprintf(command,"cat %s 2>/dev/null > view",filename);
			dsystem(command);
		}

		sprintf(command,"%s view",U.display);
		external_term();
		usystem(command);
		internal_term();
		remove("view");
		return 1;
	}
	return 0;
}


int whichlangfile (char *fileroot, char *filename) {
	FILE *FIL;
	
	sprintf(filename,"%s/%s.%s",C.library,fileroot,U.language);
	if (FIL = fopen(filename,"r")) {
		fclose(FIL);
		return 1;
	}
		
	sprintf(filename,"%s/%s.%s",C.library,fileroot,"txt");
	if (FIL = fopen(filename,"r")) {
		fclose(FIL);
		return 1;
	}

	sprintf(filename,"%s/%s.%s",C.library,fileroot,D.language);
	if (FIL = fopen(filename,"r")) {
		fclose(FIL);
		return 1;
	}

	sprintf(G.errmsg,"%s in suitable language not found",fileroot);
	errorlog(G.errmsg);
	filename[0] = '\0';
	return 0;
}
