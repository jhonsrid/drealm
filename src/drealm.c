
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

#define _XOPEN_SOURCE

/* ANSI headers */
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Non-ANSI headers */
#include <unistd.h>
#include <pwd.h>
/* sys/stat.h is needed for the umask call */
#include <sys/stat.h>
#include <time.h>

/* Local headers */
#include "drealm.h"
#include "mainfuncs.h"
#include "drealmgen.h"
#include "inputfuncs.h"
#include "configfuncs.h"
#include "setupfuncs.h"
#include "genfuncs.h"
#if 0
#include "key.h"
#endif
#if defined(CHAT_COMMANDS)
#include "chatfuncs.h"
#endif
#if defined(READ_COMMANDS)
#include "readfuncs.h"
#endif
#if defined(MAIL_COMMANDS)
#include "mailfuncs.h"
#endif
#if defined(FILE_COMMANDS)
#include "filefuncs.h"
#include "olrfuncs.h"
#endif
#if defined(TIMER_COMMANDS)
#include "timerfuncs.h"
#endif
#if defined(OUR_COMMANDS)
#include "ourfuncs.h"
#endif

struct uservars U;
struct uservars D;


/* ------------------------------------------------------------- */
/* global variables */

struct configvars C;




/* ------------------------------------------------------------- *
 * G L O B A L   V A R I A B L E S                               *
 * ------------------------------------------------------------- */
struct globalvars G;

int  Continue = 1;
int  Line_matched = 0;
int  Looping_counter = 0;
static int  Quickreturn = 0;
static char Backtolevel[11];



/* ------------------------------------------------------------- *
 * K E Y W O R D   T A B L E S                                   *
 * ------------------------------------------------------------- */

/*
 * Menu commands
 */
struct functab keywords[] = {

/* MENU NAVIGATION */
	"start_level",		startlevel,
	"push_level",		pushlevel,
	"pop_level",		poplevel,
	"swap_level",		swaplevel,
	"append",		append,
	"get",			do_get,
	"take",			do_take,
	"display_menu",		do_menudisplay,
	"log_off", 		logoff,
	"set_echo",		set_echo,
	"prompt",		do_prompt_hk,
	"prompt_cr",		do_prompt_cr,
	"print",  		do_print,
	"continue",   		do_continue,
	"press_enter",		press_enter,
	"sure",			sure,
	"yes_no",		yes_no,
	"no_yes",		no_yes,
	"flush_comline",	flushcom,
	"set_doing",		set_doing,
	"log",			dlm_log,
	"log_to_file",		logfile,
	"notify_user",		notify_user,
	"check_notice",		noticecheck,
	"check_expiry",		check_expiry,
	"check_interval",	check_interval,
	"system",		do_system,
	"shell",		do_shell,
	"input_log_on",		rawlog_on,
	"input_log_off",	rawlog_off,
	"chat_log_on",		chatlog_on,
	"chat_log_off",		chatlog_off,
	"set_customvar",	set_customvar,

	"finger",		finger,
	"grab_finger",		fingergrab,
	"quote_finger",		fingerget,
	"list_users",		users,
	"plan_search",		plansearch,

	"edit_choice",		edit,
	"edit",			edit_special,
	"view_choice",		view,
	"display",		display,
	"display_lang",		display_lang,
	"random",		randomnum,


/* SETUP COMMANDS */

	"set_terminal",		setmy_terminal,
	"set_erase",		setmy_erase,
	"set_werase",		setmy_werase,
	"set_kill",		setmy_kill,
	"set_reprint",		setmy_reprint,
	"set_hotkeys",		setmy_hotkeys,
	"set_readown",		setmy_readown,
	"set_timeout",		setmy_timeout,
	"set_recent",		setmy_recent,
	"set_rows",		setmy_rows,
	"set_pausetime",	setmy_pausetime,
	"set_password",		setmy_password,
	"set_columns",		setmy_cols,
	"set_editor",		setmy_editor,
	"set_readmode",		setmy_readmode,
	"set_display",		setmy_display,
	"set_language",		setmy_lang,
	"set_chat",		setmy_chat,
	"set_in_colour",	setmy_chatreccolour,
	"set_out_colour",	setmy_chatsendcolour,
	"edit_sig",		editmy_sig,
	"edit_plan",		editmy_plan,
	"give_real_name",	setmy_name,
	"give_address",		setmy_address,
	"give_dob",		setmy_dob,
	"give_phone",		setmy_phone,
	"give_aka",		setmy_propername,
	"maintain_me",		change_me,
	"maintain_user",	change_user,
	"update_me",		update,
	"force_update",		force_update,
	"reset_my_defaults",	resetmy_userdefaults,
	"reset_user_defaults",	resethis_userdefaults,
	"reset_my_flags",	resetmy_flags,
	"reset_user_flags",	resethis_flags,
	"set_my_flag",		setmy_flag,
	"set_user_flag",	sethis_flag,
	"set_my_level",		setmy_level,
	"set_user_level",	sethis_level,
	"reset_guest",		guest_reset,
	"make_my_account",	self_make,
	"make_user_account",	makehis_account,
	"set_my_expiry",	adjustmy_expiry,
	"set_user_expiry",	adjusthis_expiry,
	"set_my_reserves",	adjustmy_reserves,
	"set_user_reserves",	adjusthis_reserves,
	"force_user",		force,
	"destroy_account",	kill_user,
	"zap_user",		zap_user,
	"set_my_title",		setmy_title,
	"set_user_title",	sethis_title,

#if defined(CHAT_COMMANDS)
	"say",			say,
	"emote",		emote,
	"fx",			fx,
	"broadcast",		broadcast,
	"whos_on",		whoson,
	"chat_on",		chaton,
	"chat_off",		chatoff,
	"disable_chat",		chatdisable,
	"enable_chat",		chatenable,
	"queue_chat",		chatqueue,
	"release_chat",		chatrelease,
	"whisper",		whisper,
	"who_is",		whois,

#endif
#if defined(READ_COMMANDS)
	"news",			news,
	"quote_msg",		quotemsg,
	"read_msg",		readmsg,
	"edit_scanlist",	scanlist_edit,
	"change_area",		area_change,
	"scan_areas",		area_scan,
	"drop_area",		area_drop,
	"add_area",		area_add,
	"create_area",		area_create,
	"destroy_area",		area_destroy,
	"list_areas",		area_list,
	"set_arealevel",	setarea_level,
	"set_areaflag",		area_flagset,
	"set_maskflag",		mask_flagset,
	"add_members",		add_members,
	"add_chairmen",		add_chairmen,
	"add_gagged",		add_gagged,
	"rem_members",		rem_members,
	"rem_chairmen",		rem_chairmen,
	"rem_gagged",		rem_gagged,
	"show_members",		show_members,
	"show_chairmen",	show_chairmen,
	"show_gagged",		show_gagged,
	"show_info",		show_info,
	"post_msg",		post,
	"reply_to_msg",		reply,
	"comment_to_msg",	comment,
	"describe_area",	describe,
	"delete_msg",		delete,
	"copy_msg",		copymsg,
	"link_msg",		linkmsg,
	"skip",			skip,
	"jump",			jump,
	"flag_msg",		flagmsg,
	"grab_msg",		grab,
	"unlink_msg",		unlinkmsg,
	"text_search",		textsearch,
	"list_headers",		listheaders,
	"edit_info",		edit_info,
	"area_status",		area_status,
	"store",		store,
	"restore",		restore,
	"olr",			olr_here,
	"participants",		participants,
	"areaflags",		areaflag_menu,
	"maskflags",		mask_menu,
	"moderate",		pendput,
	"vote",			vote,
	"create_vote",		createvotemsg,
#endif
#if defined(MAIL_COMMANDS)
	"collect_unix_mail",	any_unix_mail,
	"post_mail",		mailpost,
	"copy_mail",		copymail,
	"forward_mail",		mailforward,
	"reply_to_mail",	mailreply,
	"delete_mail",		maildelete,
	"list_mail",		maillist,
	"grab_mail",		mailgrab,
	"quote_mail",		mailquote,
	"read_mail",		mailread,
	"check_local_mail",	mailcheck,
	"check_unix_mail",	unix_mailcheck,
	"unread",		unread,
	"restore_mail",		restore_mail,
	"store_mail",		store_mail,

#endif
#if defined(FILE_COMMANDS)
	"create_dir",		dir_create,
	"remove_dir",		dir_remove,
	"list_dir",		list_dir,
	"describe_file",	do_describefile,
	"file_to_user",		file_to_user,
	"file_to_public",	file_to_public,
	"delete_file",		file_del,
	"rename_file",		file_rename,
	"catalogue",		catalogue,
	"upload",		up_file,
	"download",		down_file_bf,
	"download_ub",		down_file_ubf,
	"download_special",	down_special_bf,
	"download_special_ub",	down_special_ubf,
	"clear_dir",		cleardir,
	"start_dir",		startdir,
	"push_pfu",		pushpfu,
	"pop_pfu",		poppfu,
	"nest_dir",		nestdir,
	"parent_dir",		parentdir,
	"search_filenames",	search_filenames,
	"search_filedescs",	search_filedescs,
#endif

#if defined(TIMER_COMMANDS)
	"start_daytimer",	daytimer_start,
	"start_timer",		timer_start,
	"stop_timer",		timer_stop,
	"suspend_timer",	timer_suspend,
	"resume_timer",		timer_resume,
#endif

#if defined(OUR_COMMANDS)
	"filtercast",		filtercast,
#endif

	"",			NULL
};

/* --------------------------------------------------------------* 
 * Pseudo-variables
 *---------------------------------------------------------------*/
struct functab funcvar[] = {
	"today",		nicedate,
	"now",			nicetime,
	"elapsed",		timeon,
	"",			0,
};
/* ------------------------------------------------------------- *
 * S T R I N G   A R R A Y S                                     *
 * ------------------------------------------------------------- */
#if 0
struct strarraytab strarrayvar[] = {
	"G.levelstack",		G.levelstack,
	"",			0,
};
#endif
/* ------------------------------------------------------------- *
 * S T R I N G S                                                 *
 * ------------------------------------------------------------- */
/*
 * Table of pointers to `pointers to strings'
 */

struct stringtab1 stringvar1[] = {
	"areasdir",		&C.areasdir,
	"bbsname",		&C.bbsname,
	"bbsshell",		&C.bbsshell,
	"bbsshort",		&C.bbsshort,
	"bin",			&C.bin,
	"configfile",		&C.configfile,
	"configdir",		&C.configdir,
	"datadir",		&C.datadir,
	"homedirs",		&C.homedirs,
	"library",		&C.library,
	"maildirs",		&C.maildirs,
	"menudir",		&C.menus,
	"newsarea",		&C.newsarea,
	"privatefiles",		&C.privatefiles,
	"startarea",		&C.startarea,
	"sysopname",		&C.sysopname,
	"tmpdir",		&C.tmpdir,
	"users",		&C.users,
	"",			0,
};

/*
 * Table of pointers to strings
 */
struct stringtab2 stringvar2[] = {
	"areamaskflag",		G.areamask,
	"areaname",		G.areaname,
	"comline",		G.comline,
	"command",		G.command,
	"customvar1",		G.customvar1,
	"customvar2",		G.customvar2,
	"customvar3",		G.customvar3,
	"customvar4",		G.customvar4,
	"customvar5",		G.customvar5,
	"customvar6",		G.customvar6,
	"customvar7",		G.customvar7,
	"customvar8",		G.customvar8,
	"dir",			G.dir,
	"displayname",		U.displayname,
	"editorname",		U.editorname,
	"extmailmask",		Extmailmask,
	"got",			G.got,
	"id",			U.id,
	"menulevel",		G.level,
	"readmode",		U.readmode,
	"taildir",		G.taildir,
	"taken",		G.taken,
	"terminal",		U.envterm,
	"topdir",		G.topdir,
	"node",			G.dev,
	"display",		U.display,
	"editor",		U.editor,
	"language",		U.language,
	"languagename",		U.languagename,
	"newareaflags",		Newareaflags,
	"newareamask",		Newareamask,
	"newuserflags",		Newuserflags,
	"pvtfilesmask",		Pvtfilesmask,
	"pvtmailmask",		Pvtmailmask,
	"",			0,
};
/* ------------------------------------------------------------- *
 * C H A R A C T E R S                                           *
 * ------------------------------------------------------------- */

struct chartab charvar[] = {
	"erasekey",		&U.erase,
	"killkey",		&U.kill,
	"reprintkey",		&U.reprint,
	"werasekey",		&U.werase,
	"",			0,
};
/* ------------------------------------------------------------- *
 * F L A G   S T R I N G S                                       *
 * ------------------------------------------------------------- */
struct flagstrtab flagstrvar[] = {
	"areaflag",		G.areaflags, 	C.aflagnames,
	"myflag",		U.flags,	C.uflagnames,
	"",			0,		0,
};
/* ------------------------------------------------------------- *
 * I N T E G E R S   V A R I A B L E                             *
 * ------------------------------------------------------------- */
struct inttab intvar[] = {
	"arealevel",		&G.arealevel,
	"autocreate",		&C.autocreate,
	"casesensitive",	&C.sensitive,
	"chatdefault",		&U.chat,
	"chatincolour",		&U.chatcolour,
	"chatnow",		&G.chat,
	"chatoutcolour",	&U.chatsendcolour,
	"commandstacking",	&C.commandstacking,
	"cols",			&U.cols,
	"currentmsg",		&G.current,
	"currentmail",		&G.mailcurrent,
	"extmaillevel",		&C.extmaillevel,
	"filesensitive",	&C.filesensitive,
	"groupnum",		&C.group,
	"highestmsg",		&G.highmsg,
	"hotkeys",		&U.hotkeys,
	"mailmonitor",		&C.mailmonitor,
	"mailreserves",		&U.mailreserves,
	"msgpointer",		&G.pointer,
	"mylevel",		&U.level,
	"newuserlevel",		&C.newuserlevel,
	"pausetime",		&U.pausetime,
	"pvtfileslevel",	&C.pvtfileslevel,
	"pvtmaillevel",		&C.pvtmaillevel,
	"quickreturn",		&C.quickreturn,
	"readown",		&U.readown,
	"recent",		&U.recent,
	"rows",			&U.rows,
	"siglength",		&C.siglength,
	"sysoplevel",		&C.sysoplevel,
	"timeout",		&U.timeout,
	"totalcalls",		&U.totalcalls,
	"totalmessages",	&U.totalmessages,
	"whichnext",		&G.whichnext,
	"random",		&G.randomnum,
	"",			0,
};
/* ------------------------------------------------------------- *
 * D A T E S                                                     *
 * ------------------------------------------------------------- */
struct datetab datevar[] = {
	"firstcall",		&U.firstcall,
	"lastcall",		&U.lastcall,
	"starttime",		&G.starttime,
	"subsexpiry",		&U.expiry,
	"",			0,
};


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/*
 * Global files.  These are opened/closed from the menus.
 */
FILE *RAWLOG;
FILE *CHATLOG;
FILE *SCAN;
FILE *LASTON;

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* PROGRAM BEGINS */

int main(int argc, char *argv[]) {
/* LENGTHS CHECKED  - nothing adds up to > 1024 */
	int result;
	char temp[1024];
	char smalltemp[80];
	FILE *TMP;
	struct line_details *ld;
	struct stat statbuf;
	time_t last_touch = 0;
	char *checkdir;
	char *thinga;

	user1_off();
	user2_off();
	intr_off();



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * CHECK WE CAN RUN IT OK!
 * All this is to find out C.configfile 
 */
	if (argc > 1) {
		fprintf(stderr,"drealmBBS: Too many arguments\n");
		do_exit(1);
	}

	G.maxusers = 0;
	printf("drealmBBS Release 2.1\n"
	       "Copyright (C) 1994, 1995, 1996  Inge Cubitt and Peter Jones\n");
	/*
	 * If this program checked to see if it had been paid for, it
	 * would do something like
	 * key(&G.maxusers)
	 * at this point (having set G.maxusers to -1, not zero, above).
	 */

	C.configfile = strdup("/drealm/config.drealm");
	if (!G.maxusers) {
		if (argv[0][0] == '/') {
			thinga = dir_name(argv[0],'/');
			sprintf(temp,"%s/config.drealm",thinga);
			free(thinga);
			C.configfile = strdup(temp);
		}
	}

	G.starttime = time(0);
	
	if ( (TMP = fopen(C.configfile,"r")) ) {
		fclose(TMP);
	} else {
		printf("Config file not read.\n");
		/*printf(Ustring[64],Ustring[231]);*/
		/*printf("\n");*/
		do_exit(2);
	}
	
	if (!cfgdrealm_read()) {
		/*Error message is printed from cfgdrealm_read*/
		do_exit(3);
	}



/* Yes, we can run it ok */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
	/* put menucache away */
	if (C.menucache) {
		G.menucache = (char *)malloc(C.menucache);
	} else {
		G.menucache = 0;
	}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

	/*
	 * Initialise flag name variables
	 */
	uflagnames();
	aflagnames();

/*==================================================================*/
/* Some pretty IMPORTANT starters ASAP please */

	strcpy(U.id,cuserid(NULL));

	if (!U.id[0]) {
		printf("Cannot determine who you are.\n");
		/*printf("%s\n",Ustring[232]);*/
		do_exit(40);
	}

	sprintf(temp,"%s/%s",C.tmpdir,U.id);
	if (TMP = fopen(temp,"w")) {
		fclose(TMP);
		remove(temp);
	} else {
		sprintf(temp,"Cannot log in - $tmpdir$ (%s) unusable or missing",C.tmpdir);
		errorlog(temp);
		printf("%s\n",temp);
		do_exit(3);
	}		

	sprintf(temp,"PATH=%s:%s",C.bin,C.path);
	G.envpathstring = strdup(temp);
	putenv(G.envpathstring);

	/*----------------------------------------------------------*/
	/* Get user interface safe now we know it's worth it */
	store_term();
	internal_term();
	setvbuf(stdout,NULL,_IONBF,0);
	setvbuf(stderr,NULL,_IOLBF,BUFSIZ);

/*==================================================================*/


	

	G.uid = getuid();

	/* ---------------------------------------------------------*/
	/* Sort out USER variables */

	strcpy(G.dev,ttyname(0));

	if (C.tp) {
		char *newdev = (char *)malloc(MAINLINE);
		gettp(G.dev,newdev);
		strcpy(G.dev,newdev);
		free(newdev);
	}

	ld = get_line_details(G.dev);
	G.line = ld->ln;
	if (!is_line_elig(ld,U.id)) {
		sleep(U.pausetime);
		do_exit(4);
	} else {
		strcpy(G.nicename,ld->nicename);
	}
	free(ld);

/* Umask needed before locks are created, or no one will be able to open them */
#if defined(SVR42) || defined(LINUX)
	umask(7);
#else
	printf("umask not available - file creation mask not set.\n");
#endif

	printf("\n\n");
	firstset();

	sprintf(temp,"%s/conf.%s",C.tmpdir,U.id);
	result = 1;
	if (!place_lock('q',temp,0,1)) {
		result = 0;
		printf("There is already a %s using %s",U.id,C.bbsname);
		/*printf(Ustring[233],U.id,C.bbsname);*/
		printf("\n");

		if (C.canlogin) {
			sprintf(temp,"%s/%s/.shunt",C.users,U.id);
			if (TMP = fopen(temp,"r")) {
				while (fgets(temp,80,TMP)) {
					shiftword(temp,smalltemp,9);
					sprintf(temp,"%s/conf.%s",C.tmpdir,smalltemp);
					if (place_lock('q',temp,0,1)) {
						fclose(TMP);
						result = 1;
						printf("Attempting auto-login as %s...\n",smalltemp);
						/*printf(Ustring[234],smalltemp);*/
						printf("\n");
						rem_lock(temp);
						execlp("login","login",smalltemp,0);
						printf("Auto-login failed.  Please log off fully and return as %s.",smalltemp);
						/*printf(Ustring[235],smalltemp);*/
						printf("\n");
						do_exit(25);
					}
				}
				fclose(TMP);
			}
		}
	}
	if (result != 1) {
		printf("Duplicates not allowed. Session abandoned.\n");
		/*printf("%s\n",Ustring[236]);*/
		do_exit(25);
	}

	sprintf(temp,"%s/chat.%s",C.tmpdir,U.id);
	remove(temp);
	sprintf(temp,"%s/time.%s",C.tmpdir,U.id);
	remove(temp);
	sprintf(temp,"%s/hear.%s",C.tmpdir,U.id);
	remove(temp);
	sprintf(temp,"%s/%s/.doing",C.users,U.id);
	remove(temp);
	if (C.chatstyle) {
		set_chatdoing("*chat disabled*");
		/*set_chatdoing(Ustring[201]);*/
	}
	result = 1;
	if (!is_bbs_account(U.id)) {
		result = 0;
		if (C.autocreate == 1) {
			if (make_bbs_account(U.id,"")) {
				result = 1;
			}
		}
	}
	if (result != 1) {
		printf("You do not have an account on %s.",C.bbsname);
		/*printf(Ustring[237],C.bbsname);*/
		printf("\n");

		printf("Please contact %s to arrange access.",C.sysopname);
		/*printf(Ustring[238],C.sysopname);*/
		printf("\n");
		logoff("");
	}


	G.laston = lastend_read(U.id);
	sprintf(G.home,"%s/%s",C.homedirs,U.id);
	result = totalcalls_read(U.id);
	result++;
	totalcalls_write(U.id,result);

	set_linegroup_flag();
	set_abend_flag();

	sprintf(temp,"%s/%s/.laston",C.users,U.id);
	LASTON = fopen(temp,"w+");
	fprintf(LASTON,"1\n");
	fflush(LASTON);

	lastcall_write(U.id,time(0));

	do_logins();

/*======================================================*/


	if (!update("")) {
		printf("Your own configuration was faulty.  Using default settings.\n");
		/*printf("%s %s\n",Ustring[239],Ustring[179]);*/
	}

	printf("Reading strings files...");
	/*printf(Ustring[240],U.language);*/
	whichstrings();
	printf("\n");
	
	/* --------------------------------------------------------------- */
	/* Initialise globals */

	G.randomnum = 0;
	G.rawlog = 0;	
	G.chatlog = 0;	
	G.chat = U.chat;
	G.chatenabled = 0;
	G.chattoggle = 0;
	G.command[0] = 0;
	G.intflag = 0;
	G.timer_on = 0;
#if defined(READ_COMMANDS)
	read_init();
#endif
	G.echo = 1;		/* 0,1,or 2 changed by app for passwords etc	*/
	G.update = 1;
	G.mupdate = 1; /*mail*/
	G.fupdate = 1; /*force*/
	G.nupdate = 1; /*notice*/
	G.levelpointer = -1;
	G.comline[0] = 0;
	G.prompt[0] = 0;
	G.got[0] = 0;
	G.taken[0] = 0;
	G.customvar1[0] = 0;
	G.customvar2[0] = 0;
	G.customvar3[0] = 0;
	G.customvar4[0] = 0;
	G.customvar5[0] = 0;
	G.customvar6[0] = 0;
	G.customvar7[0] = 0;
	G.customvar8[0] = 0;
	G.dir[0] = 0;
	G.topdir[0] = 0;
	G.taildir[0] = 0;
	G.backdir[0] = 0;
	G.backtail[0] = 0;
	G.backtop[0] = 0;
	G.newmail[0] = 0;
	G.files = strdup("");
	Looping_counter = 0;

	sprintf(temp,"%s/%s",C.users,U.id);
	checkdir = strdup(temp);
	sprintf(G.sitdir,"%s/%s",C.privatefiles,U.id);


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* MAIN LOOP BEGINS */
/* This section is to do with executing the menu structure */

	preload();

	do_start();

	G.comline[0] = 0;
	(void)startlevel("top");
	while (Looping_counter < MAXLOOP) {
		Line_matched = 0;
		stat(checkdir,&statbuf);
		if (statbuf.st_mtime > last_touch) {
			last_touch = statbuf.st_mtime;
			G.update = 1;
			G.mupdate = 1;
			G.fupdate = 1;
			G.nupdate = 1;
		}
		Looping_counter++;
		update_check("");

		if (G.comline[0] == 0) {
			if (Quickreturn && C.quickreturn) {
				poplevel(Backtolevel);
				Backtolevel[0] = 0;
			}
			Continue = 1;
			do_pmp();
			Quickreturn = 0;
			strcpy(Backtolevel,G.level);
		} else {
			Quickreturn = 1;
		}
		tnt(G.comline);
		if (ispunct(G.comline[0])) {
			G.command[0] = G.comline[0];
			G.command[1] = 0;
			strcpy(G.comline,&G.comline[1]);
		} else {
			shiftword(G.comline,G.command,21);
		}
		if (!C.sensitive) {
			lower_string(G.command);
		}
		Continue = 1;
		Line_matched = 0;
		chatqueue("");
		do_cmd('a',G.level);
		chatrelease(""); /* let some out before the prompt comes */
		G.chattoggle = 1;
		if (!Line_matched || !C.commandstacking) {
			flushcom("");
		}
	}
	errorlog("More than allowed number of cycles without a `prompt' action");
	/*puts("\nSorry, there seems to be a fault in the menu system.");*/
	printf("\n%s\n",Ustring[241]);
	do_exit(3);
	return(0);
}
