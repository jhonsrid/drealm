
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
struct details {
	char *changedate;
	char *id;
	char *realname;
	char *phone;
	char *address;
	char *dob;
	char *propername;
};


int 	make_acc_dirs (char *filename);
int 	lastend_write (char *user, int value);
time_t 	lastend_read (char *user);
int 	abend_read (char *user);
void 	set_linegroup_flag (void);
void 	set_abend_flag (void);
int 	sub_set_userflag (char *user, int flag, char value);
int 	destroy_bbs_account (char *id);
int 	destroy_unix_account (char *id);
int 	adjusthis2_reserves (char *user, char *amountstring);
int 	adjusthis2_expiry (char *user,char *daystring);
int 	sethis2_level (char *user,char *levelstring);
int	foundterminfo (char *tempterm,char *tempdesc, int *temprows, int *tempcols);
int 	usermaint (char *user);
int 	flags_write (char *filename, char *flags);
int 	ask_for_user (char *params, char *user);
int 	defaults_read (char *user, struct uservars *userbuf);
int 	defaults_write (char *user, struct uservars *userbuf);
int 	defaultsmenu (char *user);
int 	set_name (char *user);
int 	set_address (char *user);
int 	set_dob (char *user);
int 	set_phone (char *user);
int 	set_propername (char *user);
int 	edit_sig (char *user);
int 	edit_plan (char *user);
int 	set_hotkeys (char *user);
int 	set_display (char *user);
int 	set_lang (char *user);
int 	set_editor (char *user);
int 	set_terminal (char *user);
int 	set_cols (char *user);
int 	set_rows (char *user);
int 	set_recent (char *user);
int 	set_readmode (char *user);
int 	set_readown (char *user);
int 	set_pausetime (char *user);
int 	set_timeout (char *user);
int 	set_chat (char *user);
int 	set_userflag (char *user,char *flagstring, char *valuestring);
int 	set_userlevel (char *user,char *levelstring);
int 	set_chatreccolour (char *user, int colour);
int 	set_chatsendcolour (char *user, int colour);
int 	is_bbs_account (char *user);
int 	is_shell_account (char *user);
int 	get_home (char *user,char *homestring);
int 	flags_read (char *user,char *flags);
int 	make_unix_account (char *id,char *realname);
int 	make_bbs_account (char *id, char *password);
int 	put_int_in_userfile (char *file, char *user, int value);
int 	get_int_from_userfile (char *file, char *user);
int 	drealmrc_read (char *filename, struct uservars *userbuf, int firstread);
int 	drealmrc_write (FILE *CFG, struct uservars *userbuf);
int 	firstset (void);
int 	flagmenu (char *user);
void	free_details (struct details *d);
int 	details_write (char mode, char *user, struct details *de);
struct 	details *details_read (char mode, char *user);
void 	get_propername (char *user, char *propername, size_t namelen);
int 	level_read (char *user);
time_t 	expiry_read (char *user);
time_t 	firstcall_read (char *user);
time_t 	lastcall_read (char *user);
int 	totalcalls_read (char *user);
int 	totalmessages_read (char *user);
int 	mailreserves_read (char *user);
int 	level_write (char *user, int value);
int 	expiry_write (char *user, time_t value);
int 	firstcall_write (char *user, time_t value);
int 	lastcall_write (char *user, int value);
int 	totalcalls_write (char *user, int value);
int 	totalmessages_write (char *user, int value);
int 	mailreserves_write (char *user, int value);
int 	do_stty(char *user,char *parm,char *explanation);
int 	reset_userdefaults (char *id);
int 	reset_flags(char *user);
int	update_force (char *user);
int     set_title(char *user, char *titlestring);
int	is_in_passwd(char *user);	
void 	init_ustrings (void);
void 	init_mstrings (void);
int  	stringsread (char *filename,int stringcount,int stringsize,char type);
void  	whichstrings (void);

/* MENU COMMANDS */
int     setmy_title (char *params);
int     sethis_title (char *in);
int 	force (char *in);
int 	force_update (char *in);
int	guest_reset(char *dummy);
int	self_make(char *dummy);
int 	resetmy_userdefaults (char *dummy);
int 	resethis_userdefaults (char *in);
int 	update (char *dummy);
int 	change_user (char *in);
int 	change_me (char *dummy);
int 	setmy_password (char *dummy);
int 	sethis_password (char *in);
int 	setmy_name (char *dummy);
int 	sethis_name (char *in);
int 	setmy_address (char *dummy);
int 	sethis_address (char *in);
int 	setmy_dob (char *dummy);
int 	sethis_dob (char *in);
int 	setmy_phone (char *dummy);
int 	sethis_phone (char *in);
int 	setmy_propername (char *dummy);
int 	sethis_propername (char *in);
int 	editmy_sig (char *dummy);
int 	edithis_sig (char *in);
int 	editmy_plan (char *dummy);
int 	edithis_plan (char *in);
int 	kill_user (char *in);
int 	makehis_account (char *in);
int	resetmy_flags (char *dummy);
int	resethis_flags (char *in);
int 	setmy_hotkeys (char *dummy);
int 	sethis_hotkeys (char *in);
int 	setmy_display (char *dummy);
int 	sethis_display (char *in);
int 	setmy_lang (char *dummy);
int 	sethis_lang (char *in);
int 	setmy_editor (char *dummy);
int 	sethis_editor (char *in);
int 	setmy_terminal (char *dummy);
int 	sethis_terminal (char *in);
int 	setmy_cols (char *dummy);
int 	sethis_cols (char *in);
int 	setmy_rows (char *dummy);
int 	sethis_rows (char *in);
int 	setmy_erase (char *dummy);
int 	sethis_erase (char *in);
int 	setmy_werase (char *dummy);
int 	sethis_werase (char *in);
int 	setmy_kill (char *dummy);
int 	sethis_kill (char *in);
int 	setmy_reprint (char *dummy);
int 	sethis_reprint (char *in);
int 	setmy_recent (char *dummy);
int 	sethis_recent (char *in);
int 	setmy_readmode (char *dummy);
int 	sethis_readmode (char *in);
int 	setmy_readown (char *dummy);
int 	sethis_readown (char *in);
int 	setmy_pausetime (char *dummy);
int 	sethis_pausetime (char *in);
int 	setmy_timeout (char *dummy);
int 	sethis_timeout (char *in);
int 	setmy_chat (char *dummy);
int 	sethis_chat (char *in);
int 	setmy_flag (char *dummy);
int 	sethis_flag (char *in);
int 	setmy_level (char *dummy);
int 	sethis_level (char *in);
int 	setmy_chatreccolour (char *dummy);
int 	sethis_chatreccolour (char *in);
int 	setmy_chatsendcolour (char *dummy);
int 	sethis_chatsendcolour (char *in);
int	zap_user (char *in);
int 	check_expiry (char *params);
int	adjusthis_expiry (char *in);
int	adjustmy_expiry (char *params);
int	adjusthis_reserves (char *in);
int	adjustmy_reserves (char *params);
void 	find_colour_string (int colourcode,char *colourstring);
