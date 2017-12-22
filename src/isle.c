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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "drealm.h"
#include "genfuncs.h"
#include "display.h"

int Ln;
char Bs;
int Linesmade = 0;
char last_char;
char *La[1001]; 	/* each of these will be a pointer to one line */
char *Holdlines[1001];

int line_input (FILE *FIL);
int reformat_text (void);
int list_text (void);
int gob_get_raw (int waitcr, int max_i, char *outstring, int restrict);
int read_from_disk (char *filename);
int write_to_disk (char *filename);
int line_ed_menu (void);
int append_text (void);
int insert_text (void);
int set_gobble_modes(void);
int set_exit_modes(void);
int delete_text (void);
int substitute_text (void);


int main (int argc, char *argv[]) {
	char filename[256];
	int size;
	FILE *FIL;

	Ln = 1;

	if (!argc) {
		printf("isle: No filename given.\n");
		exit(14);
	}
	if (strlen(argv[1]) > 255) {
		printf("isle: Filename too long.\n");
		exit(12);
	}
	sprintf(filename,"%s",argv[1]);
	filename[255] = 0;

	if (!(FIL = fopen(filename,"a"))) {
		printf("isle: %s - Permission denied.\n",filename);

		exit(91);
	} else {
		size = ftell(FIL);
		fclose(FIL);
	}


	if (!(FIL = fopen(filename,"r"))) {
		printf("isle: %s - Permission denied.\n",filename);
		exit(91);
	} else {
		fclose(FIL);
	}

	(void)get_LW(1);

	Bs = atoi(argv[4]);
	if (!Bs) {
		Bs = 8;
	}

	set_gobble_modes();
	if (size) {
		read_from_disk(filename);
		(void)line_ed_menu();
	} else {
		Ln = 1;
		(void)append_text();
		(void)line_ed_menu();
	}
	(void)write_to_disk(filename);
	(void)set_exit_modes();
	exit(0);
	/* NOTREACHED */
}

int line_input (FILE *FIL) {
	char whole_line[256];
	char next_line[256];
	char c;
	int Cc = 0;
	int i;
	int j;

	next_line[0] = 0;

	printf("%3d>",Ln);
	while (fread(&c,1,1,FIL)) {
		if (isprint(c)) {
			if ((Cc > (WIDTH - 6)) && (c == ' ')) {
				c = '\n';

			} else {
				if (Cc > (WIDTH - 6)) {
					whole_line[Cc] = 0;
					i = Cc;
					while (i) {
						i--;
						if (whole_line[i] == ' ') {
							whole_line[i] = 0;
							break;
						}
					}
					if (i > 0) {
						j = Cc - 1;
						while (j > i) {
							putchar('\b');
							putchar(' ');
							putchar('\b');
							j--;
						}
						next_line[0] = 0;
						strcpy(next_line,&whole_line[i + 1]);
					}


					La[Ln] = strdup(whole_line);
					Linesmade++;
					Ln++;

					printf("\n%3d>",Ln);

					Cc = 0;
					whole_line[0] = 0;
					while (next_line[Cc]) {
						putchar(next_line[Cc]);
						whole_line[Cc] = next_line[Cc];
						Cc++;
					}
					next_line[0] = 0;
				}
				whole_line[Cc] = c;
				putchar(c);
				Cc ++;
			}

		} else if (c == Bs) {
			if (Cc) {
				Cc --;
				putchar('\b');
				putchar(' ');
				putchar('\b');
			}
		}
		if (c == '\n') {
			putchar(c);
			whole_line[Cc] = 0;

			if (!strcmp(whole_line,".")) {
				return 1;
			}

			La[Ln] = strdup(whole_line);
			Linesmade++;
			Ln++;

			printf("%3d>",Ln);
			Cc = 0;
		}
		last_char = c;
	}
	if (last_char != '\n') {
		putchar('\n');
		whole_line[Cc + 1] = 0;
		La[Ln] = strdup(whole_line);
		Linesmade++;
		Ln++;
		Cc = 0;
	} else {
		printf("\b\b\b\b    \b\b\b\b");
	}
	return 1;
}

int reformat_text (void) {
	int i = 1;
	char temp[20];
	FILE *FIL;

	sprintf(temp,"%s.edit",getenv("LOGNAME"));

	if (!(FIL = fopen(temp,"w"))) {
		printf("isle: %s - Write permission denied.\n",temp);
		return 0;
	}
	while (i < Linesmade) {
		if ((La[i])[0] == 0) {
			fputs("\n",FIL);
		} else {
			fputs(La[i],FIL);
			if (La[i + (unsigned)1][0] == 0) {
				fputc('\n',FIL);
			} else {
				fputc(' ',FIL);
			}
		}
		i++;
	}
	fputs(La[i],FIL);
	fputs("\n",FIL);
	fclose(FIL);
	Ln = 1;
	Linesmade = 0;
	read_from_disk(temp);
	return 1;
}


int list_text (void) {
	int line_counter = 0;
	int i = 0;
	char temp[3];
	char opt;

	if (Linesmade) {
		putchar('\n');
		while (i < Linesmade) {
			i++;
			line_counter++;

			if (line_counter > (LINES - 5)) {
				printf("[C]ontinue or [s]top? ");
				(void)gob_get_raw(0, 2, temp, 1);
				opt = tolower(temp[0]);
				if (opt == 's') {
					return 1;
				}
				line_counter = 0;
			}
			printf("%3d>",i);
			printf("%s",La[i]);
			putchar('\n');
		}
	} else {
		printf("No text in file.\n");
	}
	return 1;
}

int gob_get_raw (int waitcr, int max_i, char *outstring, int restrict) {
	char c;
	int i = 0;

	max_i--;

	if (! outstring) {
		(void)set_exit_modes();
		exit(1);
	}
	while (waitcr || (i < max_i)) {
		c = getchar();
		if (c == '\n') {
			break;
		} else if (c == Bs) {
			if (i) {
				i--;
				printf("\b \b");
			}
		} else {
			if ((restrict == 1) && isprint(c)) {
				outstring[i++] = c;
				putchar(c);
			} else if ((restrict == 2) && isdigit(c)) {
				outstring[i++] = c;
				putchar(c);
			}
		}
	}
	outstring[i] = 0;
	putchar('\n');
	return 1;
}



int read_from_disk (char *filename) {
	int size;
	FILE *FIL;

	if (!(FIL = fopen(filename,"a"))) {
		printf("isle: %s - Permission denied.\n",filename);
		exit(91);
	} else {
		size = ftell(FIL);
		fclose(FIL);
	}
	if (size) {
		if (!(FIL = fopen(filename,"r+"))) {
			printf("isle: %s - Permission denied.\n",filename);
			exit(91);
		}
		line_input(FIL);
		fclose(FIL);
	} else {
		printf("No text in file.\n");
	}
	return 1;
}


int write_to_disk (char *filename) {
	FILE *FIL;

	int i = 0;

	if (!(FIL = fopen(filename,"w"))) {
		printf("isle: %s - Permission denied.\n",filename);
		exit(91);
	}
	while (i < Linesmade) {
		i++;
		fputs(La[i],FIL);
		fputc('\n',FIL);
	}
	fclose(FIL);
	return 1;
}

int line_ed_menu (void) {
	char temp[15];
	char opt = 'x';
	while (opt != 'f') {
		fputs("\n[a]ppend, [i]nsert, [d]elete, [s]ubstitute\n",stdout);
		fputs("[r]eformat, [l]ist, [F]inish: ",stdout);
		(void)gob_get_raw(0, 2, temp, 1);
		opt = tolower(temp[0]);
		if (!opt) {
			opt = 'f';
		}

		putchar('\n');

		switch(opt) {
			case 'f':
				break;
			case 'd':
				(void)delete_text();
				break;
			case 'a':
				(void)append_text();
				break;
			case 'i':
				(void)insert_text();
				break;
			case 'l':
				(void)list_text();
				break;
			case 'r':
				(void)reformat_text();
				break;
			case 's':
				(void)substitute_text();
				break;
			default:
				printf("That input was not understood.\n");
		}
	}
	return 1;
}



int append_text (void) {
	printf("\n-------------------------------------------------------------------------------\n");
	printf("Finish with a full stop on a line by itself.\n\n");
	line_input(stdin);
	return 1;
}

int insert_text (void) {
	char temp[5];
	int num;
	int i; /* La counter */
	int j; /* Holdlines counter */
	int holdlinesmade;

	printf("Insert before which line? ");
	(void)gob_get_raw(1, 5, temp, 2);
	if (temp[0] == 0) {
		return 0;
	}
	num = atoi(temp);

	if ((num > Linesmade) || (num < 1)) {
		printf("Invalid line number.\n");
		return 0;
	}


	i = num - 1;
	j = 0;
	holdlinesmade = j;
	while (i < Linesmade) {
		i++;
		j++;
		Holdlines[j] = La[i];
		holdlinesmade = j;
	}

	Ln = num;
	(void)append_text();

	i = Ln - 1;
	j = 0;
	while (j < holdlinesmade) {
		i++;
		j++;
		La[i] = Holdlines[j];
		Linesmade = i;
	}
	Ln = Linesmade + 1;
	return 1;
}



int set_gobble_modes(void) {
	return !system("stty min 1 time 0 -icanon -echo");
}

int set_exit_modes(void) {
	return !system("stty icanon echo");
}


int delete_text (void) {
	char temp[5];
	int num;
	int j;
	int start_line;
	int end_line;
	int number_of_lines;

	printf("Delete starting from which line? ");
	(void)gob_get_raw(1, 5, temp, 2);
	if (temp[0] == 0) {
		return 0;
	}
	num = atoi(temp);

	if ((num > Linesmade) || (num < 1)) {
		printf("Invalid line number.\n");
		return 0;
	}

	start_line = num;

	printf("Delete ending at what line? ");

	(void)gob_get_raw(1, 5, temp, 2);
	if (temp[0] == 0) {
		return 0;
	}
	num = atoi(temp);
	end_line = num;

	if ((num > Linesmade) || (end_line < start_line)) {
		printf("Invalid line number.\n");
		return 0;
	}
	end_line = num;
	number_of_lines = ((end_line - start_line) + 1);

	j = start_line - 1;
	Linesmade = Linesmade - number_of_lines;
	Ln = Linesmade + 1;
	while (j < Linesmade) {
		j++;
		La[j] = La[j + number_of_lines];
	}
	return 1;
}


int substitute_text (void) {
	char oldtext[80];
	char newtext[255];
	char temp[255];
	char *wtdf;
	unsigned int  pointer;

	int num;
	int line;

	printf("Substitute text on which line? ");
	(void)gob_get_raw(1, 5, temp, 2);
	if (temp[0] == 0) {
		return 0;
	}
	num = atoi(temp);
	if ((num > Linesmade) || (num < 1)) {
		printf("Invalid line number.\n");
		return 0;
	}
	line = num;

	printf("%3d>%s\n",line,La[line]);

	printf("Enter text to replace: ");
	(void)gob_get_raw(1, 80, temp, 1);
	if (temp[0] == 0) {
		return 0;
	}
	strcpy(oldtext,temp);

	wtdf = strstr(La[line],oldtext);
	if (wtdf == NULL) {
		printf("String not found on line %d.\n",line);
		return 0;
	}

	pointer = (int)(wtdf - La[line]);

	printf("Enter replacement text: ");
	(void)gob_get_raw(1, 255, temp, 1);
	strcpy(newtext,temp);

	wtdf = (char *)malloc(strlen(La[line]) - strlen(oldtext) + strlen(newtext));
	if (pointer) {
		strncpy(wtdf,La[line],pointer);
	}
	wtdf[pointer]  = 0;
	strcat(wtdf,newtext);

	strcat(wtdf,&La[line][pointer+strlen(oldtext)]);
	free(La[line]);
	La[line] = wtdf;

	printf("%3d>%s\n",line,La[line]);

	return 1;
}
