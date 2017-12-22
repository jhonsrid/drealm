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
struct valid_messages {
	char *lock;
	char *parse;
	int  *msglist;
};
struct areaheader {
	char *flag;	/* '#' normally; 'A' for ANSI; 'V' for voting */
	char *number;	/* Message number for display */
	char *dname;	/* Day name */
	char *mname;	/* Month name */
	char *dom;	/* Day of month */
	char *time;	/* Time of day */
	char *tzname;	/* Timezone */
	char *year;	/* Year */
	char *from;	/* "from" */
	char *author;	/* name of sender */
	char *narrative;/* "BASE-MESSAGE", "reply-to", etc */
	char *hash;	/* "-" or "#" */
	char *parent;	/* Message this is a reply to */
	char *by;	/* "-" or "by" */
	char *parentby;	/* Name of sender of parent */
	char *base;	/* Message number of base of thread */
	char *next;	/* Next in thread (or "-" if last in thread) */
	char *prev;	/* Previous in thread (or last in thread if base) */
	char *footer;	/* Space-separated numbers of replies */
};

struct msgfunctab {
	char **keyword;
	char *parse;
	int *(* func)(const char mode, const char *area, const int current);
};


int     pendput (char *dummy);
int	send_now (const char mode, const char *sender, const char *area,
		const char cmd, const int parent, const char *subject,
		const char *messagefile);
int	wait_mod (const char mode, const char *sender, const char *area,
		const char cmd, const int parent, const char *subject,
		const char *messagefile);
void	summarise(int msgno);
int 	get_next_area (char *areascout);
void 	close_scan (void);
void 	open_scan (void);
int 	list_areas(char *wantedflags, int level, char *umaskflags);
int 	set_maskflag (char *area, int flag, char value);
int 	areamask_read (char *area,char *flags);
void 	get_area_message(char *area, int msgno);
int 	destroy_area (char *areascout);
int 	list_one (char *area,int msgno);
void 	do_ts_help (void);
int 	point (char *params);
char   *compose_msgbody (char *area, int msgno, char *msgfile);
int 	write_main (char cmd,char *in);
int 	writemsg (char cmd, char *in);
int 	set_areaflag (char *area, int flag, char value);
int 	scan_area (char *params);
int 	create_area (char *areascout,char *desc);
int 	copy_list (char mode, char *from, char *to, int *fromlist);
int 	set_pointers (char *areascout);
int 	change_area (char mode,char *params);
int 	delete_list (char mode,char *area,int *msglist);
int 	countmsgs (char *area, int msgno);
int 	area_clearup (void);
int 	check_area(const char mode,const char *areaname);
int 	displaymsg (const char *area, const int msgno);
int 	is_msg_elig (int msgno);
int 	attach_messages (char *area, int link_to, int *sublist);
void 	msgdesclist (void);
int 	moretoread (int highest);
int    *definethread (const char mode, const char *area, const int msgno);
int    *definetree (const char mode, const char *area, const int msgno);
int    *findtreewise(const char mode, const char *area, const int msgno);
int    *findthreadwise(const char mode, const char *area, const int msgno);
int    *findnumeric(const char mode, const char *area, const int msgno);
int    *findnext(const char mode, const char *area, const int msgno);
int    *findforward(const char mode, const char *area, const int msgno);
int    *findback(const char mode, const char *area, const int msgno);
int    *findonthread(const char mode, const char *area, const int msgno);
int    *findbackthread(const char mode, const char *area, const int msgno);
int    *findupchain(const char mode, const char *area, const int msgno);
int    *findbase(const char mode, const char *area, const int msgno);
int    *findfirst(const char mode, const char *area, const int msgno);
int    *findlast(const char mode, const char *area, const int msgno);
int    *findnumber(const char mode, const char *area, const int msgno);
int 	msgindex(const char *area, const char value, const int *offsets);
void 	mco (const char updown, const int *offsets);
void 	mlo (char value, const int *offsets);
int 	relink_thread (const char *area, int *msglist);
struct	valid_messages *get_valid_messages (const char mode, const char *area,	const char *inprompt, const int current, const char *inparams,	const int threadlock);
int     parse_area_header(char *header, struct areaheader *h);
char   *definemsg(const char mode, const char *area, const int msgno);
int     is_area_elig (char mode, char *area);
int 	read_init(void);
int 	areaflags_read (char *area,char *flags);
void	grab_area_message(char mode, char *area, int msgno);
int 	add_a(char *whichlist,char *in);
int 	rem_a(char *whichlist,char *in);
int 	show_a(char *whichfile);
int 	edit_scanlist(char *user);
int 	areaflagmenu (char *area);
int 	maskmenu (char *area);
int	detach_messages (char *area, int msgno, int *sublist);
int	set_arealevel(const char *area, const char *levelstring);
int	arealevel_write(const char *area, const int level);
int	arealevel_read(const char *area);



int 	add_chairmen (char *in);/* Takes list from input	*/
int 	add_gagged (char *in);	/* Takes list from input	*/
int 	add_members (char *in);	/* Takes list from input*/
int 	area_add(char *in);	/* One area only*/
int 	area_change(char *in);	/* Takes one area from input */
int 	area_create(char *in);	/* Creates area in C.areasdir*/
int     area_destroy (char *in);/* Destroys area from C.areasdir.  Checks eligibility*/
int 	area_drop(char *in);	/* One area only*/
int 	mask_menu (char *dummy);
int 	areaflag_menu (char *dummy);
int 	area_flagset(char *params);/* flag number and value */
int 	area_list (char *params);/* Takes optional areaflag pattern to match */
int 	area_scan(char *in);	/* changes area to next one with new messages. True if found one */
int     area_status(char *dummy);/* some data about current area*/
int 	comment(char *in);	/* sort of reply.  Can take message no*/
int 	copymsg(char *in);	/* copy <msgdesc> to area */
int	createvotemsg(char *in);
int 	delete(char *in);	/* delete <msgdesc> */
int 	describe(char *dummy);	/* describe current area one-line for list */
int     edit_info(char *dummy);	/* info file ccurrent area */
int 	flagmsg (char *in);	/* mark <msgdesc> for non-pagination*/
int     quotemsg (char *in);	/* grab message to workpad*/
int 	grab (char *in);	/* grab message to grabpad*/
int	jump (char *in);	/* jump TO a position */
int 	linkmsg(char *in);	/* link <msg> to <msg>*/
int     listheaders(char *in);	/* takes all arguments from input*/
int     mask_flagset (char *in);/* flag number and value */
int     news (char *dummy);	/* just runs news */
int 	participants (char *dummy);/* participant checkup */
int 	post(char *in);		/* post */
int 	reply(char *in);	/* reply */
int 	readmsg(char *params);	/* read <msg> */
int 	rem_chairmen (char *in);/* list from input*/
int 	rem_gagged (char *in);	/* "" */
int 	rem_members (char *in);	/* "" */
int     restore (char *dummy);	/* restore pointers*/
int 	scanlist_edit(char *dummy);/* edit your own scanlist*/
int 	show_chairmen (char *dummy);/* */
int 	show_gagged (char *dummy);/* */
int 	show_info (char *dummy);/* */
int 	show_members (char *dummy);/* */
int 	skip(char *in);		/* skip over <msgs> */
int     store (char *dummy);	/* store pointers */
int     textsearch(char *in);	/* input pattern */
int 	unlinkmsg(char *in);	/* unlink <msg> */
int	vote (char *in);
int	setarea_level(char *params); /* set area level */
