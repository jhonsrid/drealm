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

int  set_echo(char *param);
void user1_off (void);
void user1_on (void);
void user1_handler (int sig);
void user2_off (void);
void user2_on (void);
void user2_handler (int sig);
void hup_handler (int sig);
void int_handler (int sig);
void get_yn (char *outstring);
void get_commandline(char *comline);
void get_one_anything(char *outstring);
void get_one_area (char *outstring, size_t maxln, char *instring);
void get_one_char(char *outstring);
void get_one_file(char *outstring, size_t maxln, char *instring);
void get_one_lc_char(char *outstring);
void get_one_line(char *in);
void get_one_name(char *outstring, size_t maxln, char *instring);
int  get_one_num(size_t maxln, int deflt);
void get_one_param(char *outstring, size_t maxln, char *instring);
void get_raw(int waitcr, size_t max_i, char *outstring, int restrictch);
void external_term(void);
void hups_off (void);
void hups_on (void);
void internal_term(void);
void intr_off (void);
void intr_on (void);
void new_get_one_param(char mode, char *prompt, char *instring, char *outstring, size_t maxln);
void restore_term(void);
void store_term(void);
void make_prompt (char *promptstring);
void put_prompt (void);
