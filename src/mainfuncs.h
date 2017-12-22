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
struct indexlist {
	char areaname[15];
	int  pointer;
};

struct menu_struct {
	size_t shiftcount;
	int  able;
	char display[MAINLINE];
	char keys[MAINLINE];
	char action[MAINLINE];
	int  mintime;
	int  maxtime;
	int  minlevel;
	int  maxlevel;
	char userflags[UFLAGMAX + 2];
	char areaflags[AFLAGMAX + 2];
	int  menu_cont;
};

int display_it (char *params,int type);
int whichlangfile (char *fileroot, char *filename);
int get_memory_line(int offset, char *line);
int get_disk_line(FILE *MENU, int offset, char *line);
int parse_line(char *menuline, struct menu_struct *m);
int compare_line(struct menu_struct *m, char type);
int findincache(char *menuname);
int findorload_menu (char *menuname);
FILE *open_menu(char *menuname);
int free_menus(void);
void fieldtrans (char *outfield, char *infield);
char *transvar (char *varvar);
char *transtring (char *varvar);
char *str_format(const char *invalue, int Flag, int Case, int Just, int Width);
char *lookup(char *varvar, char *flag, int *flag_no, char *format);
int check_interval (char *params);
int name_to_num (char (*flagnames)[11],char *flagname);
void do_ps_help (void);
int fingering (char mode, char *type, char *params);
int menu_parse (char *menuline, char type);
int menu(char type, char *menufile);
int menufile_read (FILE *MNU, char type);
int do_some_commands (char *actions);
int do_one_command (char *action);
int do_menulinedisplay (char *display);
int comp_command (char *keys, char *g_command);
int comp_level (int minlevel, int maxlevel);
int comp_time (int mintime, int maxtime);
char MenuFile[MAINLINE];
void do_exit (int exit_code);
int newuserlog (char *param);
void do_logins (void);
int wannasave (char *workpad, char *endresult);
int editposting (char *workpad, char *endresult);
int rem_from_list (char mode, char *filename,char *value);
int add_user_to_list (char mode, char *type, char *filename, char *inlist);
int do_ini(char *levelname);
int do_exi(char *levelname);
int do_start(void);
int do_stop(void);
int do_cmd(char type, char *level);
int do_pmp(void);
int update_check (char *dummy);
int timeon (char *outstring);	

int is_line (char *params);    	/* sees if the line is a particular number */
int noticecheck (char *dummy); 	/* checks for .notice and displays it */
int plansearch (char *in); 	/* look for string in peoples planfiles */
int users (char *dummy); 	/* list all users */
int finger (char *in);         	/* finger user by input line */
int fingergrab (char *in);     	/* grab planfile to grabpad */
int fingerget (char *in);      	/* grab planfile to workpad */
int sure (char *dummy);		/* Ask user if sure */
int view (char *in);		/* view file from G.dir.  Name from input */
int display(char *params);	/* display filepath from menuline */
int display_lang(char *params);	/* display library file from basename only */
int set_doing (char *params);	/* string for current user to have in .doing */
int do_continue(char *dummy);  	/* ask continue or stop */
int press_enter(char *dummy);	/*   */
int do_menudisplay(char *dummy);/* show display fields in current menu */
int do_print(char *printparam);	/* print what comes next */
int do_prompt_hk(char *promptparam);/* print out param from menu, then prompt for command line using hotkey choice*/
int do_prompt_cr(char *promptparam);/* print out param from menu, then prompt for command line forcing CR to end*/
int do_prompt(int type,char *promptparam);/* called from both above */
int do_system(char *systemparam);/* get system to execute command which is on menuline */
int do_shell (char *in);	/* take system command from input */ 
int edit(char *in);		/* edit file from G.dir, file named in input */
int edit_special(char *fullfile);/* edit file fully named in menuline, any dir */
int flushcom(char *dummy);	/* clears command line */
int logoff(char *dummy); 	/* logs off, do stop menu */
int do_take (char *promptparam);/* same as do_get but tries to take from comline first*/
int do_get (char *promptparam); /* prints prompt specified on menuline and gets $got */
int append(char *templevel);   	/* branch off to another menu. returns true if line was matched */
int poplevel(char *templevel); 	/* take a level off the stack, or take levels off till you get to named. do .exi for each popped*/
int pushlevel(char *templevel);	/* push a level on the stack, do .ini */
int startlevel(char *templevel);/* pop through stack, doing all .exis and start from new level */ 
int swaplevel(char *templevel); /* pop one level and push another */
int no_yes (char *params);	/* defaults to no. True if yes */
int yes_no (char *params);	/* defaults to yes. True if yes */
int rawlog_on (char *dummy);
int rawlog_off (char *dummy);
int chatlog_on (char *dummy);
int chatlog_off (char *dummy);
int notify_user (char *in);

int set_customvar (char *params);
int dlm_log (char *param);
int logfile (char *params);
void preload(void);
int randomnum (char *maxnumstring);

