
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
#include <limits.h>
#if defined(LINUX)
#  include <signal.h>
#endif

/* Non-ANSI headers */
#include <unistd.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>

/* Local headers */
#include "genfuncs.h"

#define MAXLINE 256


int trans_string(const char *input, char *output, size_t output_length) {
/*
The purpose is to copy the input to the output, replacing \ sequences
with the appropriate character, up to output_length.

Returns true.
*/

	char *copy;
	char *p;
	char *q;
	int out_ptr = 0;

	output_length --;
	if (! (p = strchr(input,'\\'))) {
		strncpy(output, input, output_length);
		output[output_length] = '\0';
		return 1;
	}
	
	copy = strdup(input);

	q = copy;
	while ((out_ptr < output_length) && (p = strchr(q,'\\'))) {
		if (p[1]) {
			switch (p[1]) {
				case 'n': *p = '\n'; break;
				case 'b': *p = '\b'; break;
				default:  *p = p[1]; break;
			}
			p[1] = '\0';
			p = &p[2];
			strncpy(&output[out_ptr],q,output_length - out_ptr);
			output[output_length] = '\0';
			out_ptr += strlen(&output[out_ptr]);
		} else {
			p = &p[1];
		}
		q = p;
	}

	strncpy(&output[out_ptr],q,output_length - out_ptr);
	output[output_length] = '\0';
	free(copy);
	return 1;
}

/*
The purpose is to copy the end of the string after the divider and
return a pointer to the copy
*/
char *base_name(const char *string, const char divider) {
	char *p = strrchr(string, divider);
	
	if (p) {
		return strdup(++p);
	} else {
		return strdup(string);
	}
}

char *dir_name (const char *path, const char divider) {
/* CHECKED */
/* gets rid of the actual filename out of a full path and puts dir bit in temp*/
	char *temp = strdup(path);
	char *p = strrchr(temp, divider);

	if (p) {
		*p = '\0';
	} else {
		temp[0] = 0;
	}
	
	return temp;
}




int is_in_list (const char *filename,const char *value) {
/* CHECKED */
	FILE *FIL;
	char temp[MAXLINE];

	if ( (FIL = fopen(filename,"r")) ) {
		while (fgets(temp,MAXLINE,FIL)) {
			temp[MAXLINE - 1] = 0;
			tnt(temp);
			if (!strcmp(temp,value)) {
				fclose(FIL);
				return 1;
			}
		}
		fclose(FIL);
	}
	return 0;
}





int *string_to_list (const char *string) {
/* CHECKED */
/* turns a string of space-separated numbers into an integer array */
	char numstring[5];
	char *tempstring;
	int i; /* number count */
	int j; /* movement along the string */
	int k; /* backwards counter */
	int l; /* other counter */
	int *list;

	for(k=(strlen(string)-1);k && (string[k] == ' ');k--); /* Find out last solid character */
	i = 0;
	for(j=0;string[j] && (string[j] == ' ');j++) {
		if (string[j] == ' ') {
			i++; /* for each number */
		}
	}

	i = 0;
	for(;string[j] && (j < k);j++) {
		if (string[j] == ' ') {
			i++; /* for each number */
		}
	}
	i++; /* Cos there is no space at the end. i now represents number of numbers */ 

	list = (int *)malloc((i + 1) * sizeof (int)); /* add one for the null */

	tempstring = strdup(string);
	l = 0;
	while (tempstring[0] && (l < i)) {
		shiftword(tempstring,numstring,5);
		list[l]=atoi(numstring);
		l++;
	}
	list[l]=0;

	free(tempstring);
	return list;
}

int shiftnum (int *list) {
/* CHECKED */
/* returns first number shifted off front of list and reduces list*/
	int i;
	int result;

	result = list[0];
	if (result) {
		i = 0;
		while (list[i]) {
			list[i] = list[i + (unsigned)1];
			i++;
		}
	}
	return result;
}

int *combinenums (const int *firstlist, const int *secondlist) {
/* CHECKED */
/* makes new list out of 1st list then 2nd list */
	int i;
	int j;
	int *templist;

	j = 0;
	for (i = 0;firstlist && firstlist[i];i++) {
		j++;
	}
	for (i = 0;secondlist && secondlist[i];i++) {
		j++;
	}
	j++;  /* Incremented to allow for null */
	templist = (int *)malloc(j * (sizeof (int))); 

	j = 0;
	for(i=0; firstlist && firstlist[i];i++) {
		templist[j] = firstlist[i];
		j++;
	}
	for(i=0; secondlist && secondlist[i];i++) {
		templist[j] = secondlist[i];
		j++;
	}
	templist[j] = 0;
	return templist;
}


void grepnums (int *mainlist,const int *sublist) {
/* CHECKED */
/* removes numbers in sublist from mainlist */
	int i;
	int j;
	int k;
	int found;
	int *templist;

	if (!mainlist[0]) {
		return;
	}

	for (i = 0;mainlist && mainlist[i];i++);
	i++; /* Increment to allow for null */

	templist = (int *)malloc(i * (sizeof (int)));

	k = 0;
	for(i=0; mainlist && mainlist[i];) {
		found = 0;
		for(j=0; sublist && sublist[j]; j++) {
			if (mainlist[i] == sublist[j]) {
				found++;
				break;
			}
		}
		if (!found) {
			templist[k] = mainlist[i];
			k++;
		}
		i++;
	}
	templist[k] = 0;
	for(i=0; mainlist && templist[i];) {
		mainlist[i] = templist[i];
		i++;
	}
	if (mainlist) {
		mainlist[i] = 0;
	}
	free(templist);
}


/*
The purpose of strshift() is to remove all characters from the input up to
a given separator, and put them in the output.  The remainder of the input
after the separator is shifted to the start and the number of characters
removed is returned.  If the number of characters removed is greater than
the given length for the output, the output is truncated.
*/
int strshift(char *input, char *output, const size_t output_length, const char *separator) {
	char *p = (char *)strstr(input, separator);
	int shifted = 0;

	if (!p) {
		/*
		 * no separator found
		 * move input to output, up to output_length
		 */
		strncpy(output, input, output_length-1);
		output[output_length-1] = '\0';
		shifted = strlen(input);
		input[0] = '\0';
	} else {
		/*
		 * p points to the separator.  We can clobber it, which is
		 * useful.
		 */
		*p = '\0';
		strncpy(output, input, output_length-1);
		output[output_length-1] = '\0';
		shifted = strlen(input) + strlen(separator);
		strcpy(input,&p[strlen(separator)]);
	}
	return shifted;
}

/*
The purpose of menushift() is to remove all characters from the input up to
a given separator, and put them in the output.  The remainder of the input
after the separator is shifted to the start.  If the number of characters
removed is greater than the given length for the output, the output is
truncated and FALSE is returned.  Otherwise true.
*/
int menushift(char *input, char *output, const size_t output_length, const char *separator, size_t *shifted) {
	char *p = (char *)strstr(input, separator);
	int overflow = 0;

	if (!p) {
		/*
		 * no separator found
		 * move input to output, up to output_length
		 */
		strncpy(output, input, output_length-1);
		output[output_length-1] = '\0';
		*shifted += strlen(input);
		overflow = (strlen(output) < strlen(input));
		input[0] = '\0';
	} else {
		/*
		 * p points to the separator.  We can clobber it, which is
		 * useful.
		 */
		*p = '\0';
		strncpy(output, input, output_length-1);
		output[output_length-1] = '\0';
		*shifted += strlen(input) + strlen(separator);
		overflow = (strlen(output) < strlen(input));
		strcpy(input,&p[strlen(separator)]);
	}
	return (overflow? 0 : 1);
}

/*
shiftword() removes leading spaced from the input
then removes the next string up to a space or end of input.
Up to the specified number of characters are put into output.
*/
int shiftword(char *input, char *output, size_t output_length) {
	char *p;
	int i;
	
	/* count leading space */
	for(p=input;*p && isspace(*p);p++);
	i = (p - input);

	/* skip them, if present */
	if (i) strcpy(input, p);

	/* find next space */
	for(p=input;*p && !isspace(*p);p++);
	i += (p - input);

	if (strlen(p) == 0) {
		/* p points to the end of input */
		strncpy(output,input,output_length-1);
		output[output_length-1] = '\0';
		input[0] = '\0';
	} else {
		/* i.e. we haven't reached the end of input */
		*p++ = '\0';
		i++; /* skip the space we reached */
		strncpy(output,input,output_length-1);
		output[output_length-1] = '\0';
		strcpy(input,p);
	}
	return i;
}

void tnt (char *string) {
/* CHECKED - Result cannot get larger than original */
/* Strips off leading and trailing blanks of all sorts from string
*/
	int i = strlen(string)-1;
	char *tempstring;

	if (i < 0) {
		return;
	}

	tempstring = strdup(string);

	while (i && isspace(tempstring[i])) {
		tempstring[i] = 0;
		i--;
	}
	i = 0;
	while (tempstring[i] && isspace(tempstring[i])) {
		i++;
	}
	strcpy(string,&tempstring[i]);
	free(tempstring);
}

int intsort(const void *a, const void *b) {
/* CHECKED */
	const int *x = (const int *)a;
	const int *y = (const int *)b;

	return *x - *y;
}

char *dir_search (const char *filename) {
/* CHECKED */
/* searches path for specified filename - puts directory it was in in temp */

	char *testname = (char *)malloc(strlen(filename) + 257);
	char *path = strdup(getenv("PATH"));
	char temp[MAXLINE];


	while (path[0]) {
		(void)strshift(path,temp,256,":");
		if (temp[0]) {
			/* This is to expand an empty field into cwd, but be
			aware that a final empty field will not be
			discovered so we might need to do an explicit search
			in cwd where desired */
			/* EMPTY */
		}
		sprintf(testname,"%s/%s",temp,filename);
		/* Test for execute/search permission */
		if (!access(testname,X_OK)) {
			break;
		}
	}
	free(path);
	free(testname);
	return strdup(temp);
}

void lower_string (char *string) {
/* CHECKED - Cannot exceed limits */
	int i;
	for(i=0; string[i]; i++) string[i] = tolower(string[i]);
}

void upper_string (char *string) {
/* CHECKED - Cannot exceed limits */
	int i;
	for(i=0; string[i]; i++) string[i] = toupper(string[i]);
}

void capitalise(char *string) {
/* CHECKED - Cannot exceed limits */
	int p = 0;
	while (string[p]) {
		while ((string[p] == ' ') || (ispunct(string[p]))) {
			p++;
		}
		string[p] = toupper(string[p]);
		p++;
		while (string[p] && (string[p] != ' ') && (! ispunct(string[p]))) {
			string[p] = tolower(string[p]);
			p++;
		}
	}
}

int is_num (const char *string) {
/* CHECKED - Cannot exceed limits */
	int i = 0;
	if (!string[0]) {
		return 0;
	}
	for(i=0; string[i]; i++) {
		if (!isdigit(string[i])) {
			return 0;
		}
	}
	return 1;
}

void numtostr(char *string) {
/*
 * string contains a list of space separated numbers
 * this is turned into the ascii characters for those numbers
 * Used by setupsubs - not to be confused with string_to_list
 */
	char num[MAXLINE];
	char buffer[MAXLINE];
	int i;

	i = 0;
	while(string[0] && (i < MAXLINE - 1)) {
		shiftword(string, num, MAXLINE);
		buffer[i] = (char)strtol(num,0,0);
		if (buffer[i]) {
			i++;
		}
	}
	buffer[i] = 0;
	strcpy(string,buffer);
	
}

int rem_lock(const char *file) {
/* CHECKED */
	return remove(file);
}

char *concat(char *const params, char *const comline) {
/* CHECKED */
	char *string;

	string = (char *)malloc(strlen(comline) + strlen(params) + 2);
	strcpy(string,params);
	if (comline[0] && params[0]) {
		strcat(string," ");
	}
	strcat(string,comline);
	return string;
}

char *getfield(const char *filename, const char *field) {
/* CHECKED */
/* suitable for a password file maybe? <shrug> */
	char string[MAXLINE];
	char *value;
	FILE *MSG;
	if (!(MSG = fopen(filename,"r"))) {
		return NULL;
	}
	while(fgets(string,MAXLINE,MSG)) {
		if (!string[1]) {
			break;
		}
		if (!(value = strchr(string,':'))) {
			continue;
		}
		*value = '\0';
		while(isspace(*++value)) *value='\0';
		lower_string(string);
		if (strcmp(string,field)) {
			continue;
		}
		fclose(MSG);
		return strdup(value);
	}
	fclose(MSG);
	return NULL;
}

int string_in_file (const char *string,const char *file) {
/* CHECKED */
	FILE *FIL;
	char line_in[MAXLINE];

	if ( (FIL = fopen(file,"r")) ) {
		while(fgets(line_in,MAXLINE,FIL)) {
			line_in[MAXLINE - 1] = 0;
			if (strstr(line_in,string)) {
				fclose(FIL);
				return 1;
			}
		}
		fclose(FIL);
	}
	return 0;
}

int dos_match(char *pattern, char *name) {
/* CHECKED - No assignments made */
	int i = 0;
	int j = 0;

	for(i=0;pattern[i];i++) {
		switch (pattern[i]) {
			case '*':
				while(name[j] && (name[j] != '.')) {
					j++;
				}
				break;
			case '?':
				j++;
				break;
			default:
				if (name[j] && (pattern[i] == name[j])) {
					j++;
				} else {
					return 0;
				}
				break;
		}
	}
	return 1;
}

int gettp(char *tpdev, char *realdev) {
#if defined(SVR42)
/* This is for the trusted paths rubbish.  Not used unless trusted paths. */
	char wanted[MAXLINE];
	char found[MAXLINE];
	char dummy[MAXLINE];
	char filename[MAXLINE];
	char line[MAXLINE];
	DIR *SAF;
	struct dirent *saf;
	FILE *LOG;
#endif

	strcpy(realdev,tpdev);
#if defined(SVR42)
	if (tpdev[0] == '/') {
		strcpy(wanted,tpdev);
	} else if (!strncmp(tpdev,"tp/",3)) {
		sprintf(wanted,"/dev/%s",tpdev);
	} else {
		return 0;
	}

	if (SAF = opendir("/var/saf")) {
		while(saf = readdir(SAF)) {
			sprintf(filename,"/var/saf/%s/log",saf->d_name);
			if (LOG = fopen(filename,"r")) {
				while(fgets(line,256,LOG)) {
					line[255] = 0;
					if (strstr(line,"INFO: ")) {
						continue;
					}
					strshift(line,dummy,1,";");
					strshift(line,dummy,1,";");
					sscanf(line,"Starting service %*s on tp device %s muxed under tty device %s\n",found,dummy);
					if (!strcmp(wanted,found)) {
						strcpy(realdev,dummy);
					}
				}
				fclose(LOG);
			}
		}
		closedir(SAF);
	}
#endif
	return 1;
}

int strsort(const void *x, const void *y) {
	const char * const *a = (const char * const *)x;
	const char * const *b = (const char * const *)y;
	return strcmp(*a, *b);
}


void random_init(void) {
	/* call _once_ at start of program */
	unsigned short seed[3];
	union _xlong {
		unsigned short x[2];
		unsigned long  y;
	} var;
	struct timeval tv;
	struct timezone tz;
	
	gettimeofday(&tv, &tz);
	
	var.y = tv.tv_usec;
	seed[0] = var.x[0];
	seed[1] = var.x[1];
	
	var.y = tv.tv_sec;
	seed[1] += var.x[1];
	seed[2] = var.x[0];
	

	(void)seed48(seed);
}

inline unsigned int get_random(unsigned int upper) {
	/* return in range 0<=x<upper) */
	return drand48() * upper;
}

int get_int_from_file (char *filename) {
	FILE *CFG;
	int i = 0;

	if (!(CFG = fopen(filename,"r"))) {
		return 0;
	}

	fscanf(CFG," %d ",&i);
	fclose(CFG);
	return i;
}