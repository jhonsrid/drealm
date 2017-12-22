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
#define MAXLOOP 30
#define MAINLINE 256
#define SCANMAX	100
#define AFLAGMAX 30
#define UFLAGMAX 30
#define USTRINGSIZE 64000
#define USTRINGCOUNT 525
#define MSTRINGSIZE 25501
#define MSTRINGCOUNT 100
#define MAXLANGS 12

/* maximum 30 char names */
/* Here are the Area Flags */
#define AREATRUE	1	/* in an area */
#define SIGS		2	/* area attaches sigs */
#define ALIASES		3	/* area requests aliases */
#define READONLY	4	/* only sysop/chairman can post */
#define PRIVATE		5	/* only members can join */
#define MODERATED	6	/* chairman must forward messages */


/* Here are the User Flags,  */
#define	LIVE		1	/* Actual user online*/
#define	SILENT		2	/* Silent flag */
#define	CHAIRMAN	3	/* Depends on what area you are in */
#define	GAGGED		4       /* Gagged people */
#define ABEND		5       /* Dropped line or timed out last call */
#define NODEGROUP	6	/* The line number of their terminal */




struct functab {
	const char *keyword;
	int (* func)(char *args);
};

extern struct functab keywords[];
extern struct functab funcvar[];

#if 0
struct strarraytab {
	const char *varstring;
	char **realvar;
};
extern struct strarraytab strarrayvar[];
#endif

struct stringtab1 {
	const char *varstring;
	char **realvar;
};
extern struct stringtab1 stringvar1[];

struct stringtab2 {
	const char *varstring;
	char *realvar;
};
extern struct stringtab2 stringvar2[];

struct chartab {
	const char *varstring;
	char *realvar;
};

extern struct chartab charvar[];

struct flagstrtab {
	const char *varstring;
	char *realvar;
	char (*flagname)[11];
};
extern struct flagstrtab flagstrvar[];

struct inttab {
	const char *varstring;
	int  *realvar;
};
extern struct inttab intvar[];


struct datetab {
	const char *varstring;
	time_t *realvar;
};
extern struct datetab datevar[];

struct menuload {
	char menuname[15];
	int nextmenu;
	char menubody[0];
};


struct globalvars {
	int  levelpointer;
	time_t starttime;
	int  echo;
	int  chat;
	int  chatenabled;
	int  uid;
	int  line;
	int  laston;

	int  rawlog;
	int  chatlog;
	int  hupflag;
	int  intflag;
	int  usr1flag;
	int  usr2flag;
	int  intrs;
	int  timer_on;
	int  hups;
	int  highmsg;
	int  pointer;
	int  current;
	int  whichnext;
	int  update;
	int  fupdate;
	int  mupdate;
	int  nupdate;
	char areaname[51];
	char areaflags[AFLAGMAX + 2];
	char areamask[UFLAGMAX + 2];
	char mymsgindex[4002];
	int  arealevel;
	int  chain[256];

	int  maxusers;
	int  chattoggle;
	int  randomnum;
	char errmsg[100];
	char prompt[MAINLINE];
	char comline[MAINLINE];
	char got[MAINLINE];
	char taken[MAINLINE];
	char command[21];

	int  lines;
	int  columns;
	char *envpathstring;
	char *files;
	char envtermstring[MAINLINE + 11];
	char envrowstring[21];
	char envlinestring[21];
	char envcolstring[21];
	char envwidthstring[21];

	char customvar1[MAINLINE];	
	char customvar2[MAINLINE];	
	char customvar3[MAINLINE];	
	char customvar4[MAINLINE];	
	char customvar5[MAINLINE];	
	char customvar6[MAINLINE];	
	char customvar7[MAINLINE];	
	char customvar8[MAINLINE];	
	char level[11];
	char levelstack[16][11];
	char topdir[MAINLINE + 81];
	char taildir[MAINLINE + 81];
	char backtop[MAINLINE + 81];
	char backtail[MAINLINE + 81];
	char backdir[MAINLINE + 81];
	char dir[MAINLINE + 81];
	char sitdir[MAINLINE + 81];
	int  areapointer;
	int  scan;
	char home[MAINLINE + 81];
	char dev[MAINLINE];
	char nicename[81];

	int mailplace;
	int mailcurrent;
	int newmail[256];
	int dirpointer;
	char *menucache;

	char bigyes;
	char littleyes;
	char bigno;
	char littleno;
	char bigquit;
	char littlequit;
	char yesno[5];
};




extern struct globalvars G;
extern int  Continue;
extern int  Line_matched;
extern int  Looping_counter;


struct configvars {
	int  chatstyle;
	int  admin;
	int  group;
	int  public;
	size_t siglength;

	int  newuserlevel;
	int  speaklevel;
	int  extmaillevel;
	char *extmailmask;
	int  pvtmaillevel;
	char *pvtmailmask;
	int  pvtfileslevel;
	char *pvtfilesmask;
	int  sysoplevel;

	int  canchown;
	int  canlogin;
	int  autocreate;
	int  mailmonitor;
	int  sensitive;
	int  filesensitive;
	int  commandstacking;
	int  quickreturn;
	int  uploadreport;
	int  tp;
	size_t menucache;
	int  filemode;

	char *configfile;
	char *areasdir;
	char *library;
	char *datadir;
	char *configdir;
	char *bin;
	char *menus;
	char *users;
	char *privatefiles;
	char *tmpdir;
	char *homedirs;
	char *unixmaildir;
	char *maildirs;
	size_t maxfilename;

	char *bbsname;
	char *bbsshort;
	char *sysopname;

	char *newareaflags;
	int  newarealevel;
	char *newareamask;
	char *newuserflags;

	char *startarea;
	char *newsarea;

	char *display1;
	char *display2;
	char *display3;
	char *displayname1;
	char *displayname2;
	char *displayname3;
	char *editor1;
	char *editor2;
	char *editor3;
	char *editorname1;
	char *editorname2;
	char *editorname3;

	char *path;

	char *bbsshell;
	char *filter;
	char *extras;
	char *unixmailer;
	char *awkname;

	char uflagnames[UFLAGMAX+2][11];
	char aflagnames[AFLAGMAX+2][11];
	
};

extern struct configvars C;

struct uservars {
	char id[9];			/* .drealmrc */
	char flags[UFLAGMAX + 2];
	int  level; 	
	char display[MAINLINE];
	char displayname[MAINLINE];
	char editor[MAINLINE];
	char editorname[MAINLINE];
	char languagename[MAINLINE];
	char termtype[21];
	char envterm[21];
	int  chatcolour;
	int  cols;
	int  rows;
	char erase;
	char werase;
	char kill;
	char reprint;
	int  hotkeys;
	int  recent;
	int  existrecent;
	char language[11];
	char readmode[21];
	int  readown;
	unsigned int pausetime;
	int  timeout;
	int  chat;
	int  chatsendcolour;
	int  spare2;
	int  spare3;
	int  spare4;	

	time_t expiry; 			/* .subscriber */
	int totalcalls;  		/* .totalcalls */
	int totalmessages; 		/* .totalmessages */
	int mailreserves; 		/* .mailallowance */
	time_t firstcall;		/* .firston */
	time_t lastcall;		/* .lastcall */

};	
extern struct uservars U;
extern struct uservars D;

extern char *Ustring[USTRINGCOUNT + 1];
extern char *Mstring[MSTRINGCOUNT + 1];
extern char Newareaflags[AFLAGMAX + 1];
extern char Newareamask[UFLAGMAX + 1];
extern char Newuserflags[UFLAGMAX + 1];
extern char Pvtmailmask[UFLAGMAX + 1];
extern char Pvtfilesmask[UFLAGMAX + 1];
extern char Extmailmask[AFLAGMAX + 1];

extern FILE *RAWLOG;
extern FILE *CHATLOG;
extern FILE *SCAN;
extern FILE *LASTON;
