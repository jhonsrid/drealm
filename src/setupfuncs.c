
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
#include <pwd.h>
#include <errno.h>


#include "drealm.h"
#include "drealmgen.h"
#include "mainfuncs.h"
#include "inputfuncs.h"
#include "configfuncs.h"
#include "genfuncs.h"
#include "getvalf.h"
#include "colour.h"
#include "setupfuncs.h"
#include "display.h"


#if defined(READ_COMMANDS)
#include "readfuncs.h"
#endif
#if defined(CHAT_COMMANDS)
#include "chatfuncs.h"
#endif

static char ustrings[USTRINGSIZE];
static char mstrings[MSTRINGSIZE];


int ask_for_user (char *params, char *user) {
	struct valid_files *vf;
	
	/*vf = get_valid_dirs('v',1,"user",C.users,params,0);*/
	vf = get_valid_dirs('v',1,Ustring[196],C.users,params,0);
	strcpy(user,vf->files);
	free(vf->input);
	free(vf->files);
	free(vf);
	if (user[0]) {
		return 1;
	} else {
		return 0;
	}
}


int setmy_password (char *dummy) {
/* MENU COMMAND */
	flushcom("");
	external_term();
	dsystem("passwd");
	internal_term();
	return 1;	
}

int sethis_password (char *in) {
/* MENU COMMAND */
	int result;
	char user[9];
	char command[MAINLINE + 100];
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		

	sprintf(command,"setpwd %s",user);
	external_term();
	result = !dsystem(command);
	internal_term();
	if (!result) {
		/*printf("Password not changed.\n");*/
		printf("%s\n",Ustring[329]);
	}
	return result;
}

int editmy_plan (char *dummy) {
/* MENU COMMAND */
	flushcom("");
	return edit_plan(U.id);
}

int edithis_plan (char *in) {
/* MENU COMMAND */
	char user[9];
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		

	return edit_plan(user);
}

int edit_plan (char *user) {
	char filename[MAINLINE + 100];
	sprintf(filename,"%s/%s/.plan",C.privatefiles,user);
	return edit_special(filename);
}


int editmy_sig (char *dummy) {
/* MENU COMMAND */
	flushcom("");
	return edit_sig(U.id);
}

int edithis_sig (char *in) {
/* MENU COMMAND */
	char user[9];
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		

	return edit_sig(user);
}

int edit_sig (char *user) {
	char filename[MAINLINE + 100];
	sprintf(filename,"%s/%s/.sig",C.privatefiles,user);
	return edit_special(filename);
}



/*========================================================================*/
/* PERSONAL DETAILS */

int setmy_address (char *dummy) {
/* MENU COMMAND */
	flushcom("");
	return set_address(U.id);
}

int sethis_address (char *in) {
/* MENU COMMAND */
	char user[9];
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		

	return set_address(user);
}

int set_address (char *user) {
	char temp[MAINLINE];
	char tempa[MAINLINE];
	struct details *dt;
	
	if (!user[0]) {
		return 0;
	}

	dt = details_read('q',user);
	
	if (dt->address[0]) {
		/*printf("\nCurrent address: %s\n",dt->address);*/
		printf("\n");
		printf(Ustring[391],Ustring[334],dt->address);
		printf("\n");
		/*make_prompt("New Address: ");*/
		
		sprintf(tempa,Ustring[400],Ustring[334]);
		make_prompt(tempa);
	} else {
		/*printf("\nAddress all on one line please\n");*/
		printf("\n%s\n",Ustring[332]);
		/*make_prompt("Full Address: ");*/
		sprintf(tempa,Ustring[399],Ustring[334]);
		make_prompt(tempa);
	}
	get_one_line(temp);
	printf("\n");
	if (!temp[0]) {
		free_details(dt);
		return 0;
	}
	free(dt->address);
	dt->address = strdup(temp);
	details_write('v',user,dt);
	free_details(dt);
	return 1;
}

int setmy_dob (char *dummy) {
/* MENU COMMAND */
	flushcom("");
	return set_dob(U.id);
}

int sethis_dob (char *in) {
/* MENU COMMAND */
	char user[9];
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	return set_dob(user);
}



int set_dob (char *user) {
	char temp[MAINLINE];
	char tempa[MAINLINE];
	struct details *dt;

	if (!user[0]) {
		return 0;
	}

	dt = details_read('q',user);
	if (dt->dob[0]) {
		/*printf("\nCurrent date of birth: %s\n",dt->dob);*/
		printf("\n");
		printf(Ustring[391],Ustring[335],dt->dob);
		printf("\n");
		/*make_prompt("New date of birth: ");*/
		sprintf(tempa,Ustring[400],Ustring[335]);
		make_prompt(tempa);
	} else {
		/*make_prompt("\nDate of birth (any format): ");*/
		make_prompt(Ustring[333]);
	}	
	get_one_line(temp);
	printf("\n");
	if (!temp[0]) {
		free_details(dt);
		return 0;
	}
	free(dt->dob);
	dt->dob = strdup(temp);
	details_write('v',user,dt);
	free_details(dt);
	return 1;
}

int setmy_name (char *dummy) {
/* MENU COMMAND */
	flushcom("");
	return set_name(U.id);
}

int sethis_name (char *in) {
/* MENU COMMAND */
	char user[9];
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	return set_name(user);
}



int set_name (char *user) {
	char temp[MAINLINE];
	char tempa[MAINLINE];
	struct details *dt;

	if (!user[0]) {
		return 0;
	}
	dt = details_read('q',user);

	if (dt->realname[0]) {
		/*printf("Current real name: %s\n",dt->realname);*/
		printf("\n");
		printf(Ustring[391],Ustring[336],dt->realname);
		printf("\n");
		/*make_prompt("New name: ");*/
		sprintf(tempa,Ustring[400],Ustring[336]);
		make_prompt(tempa);
	} else {
		/*make_prompt("Real name: ");*/
		sprintf(tempa,Ustring[399],Ustring[336]);
		make_prompt(tempa);
	}
	get_one_line(temp);
	printf("\n");
	if (!temp[0]) {
		free_details(dt);
		return 0;
	}
	free(dt->realname);
	dt->realname = strdup(temp);
	details_write('v',user,dt);
	free_details(dt);
	return 1;
}

int setmy_propername (char *dummy) {
/* MENU COMMAND */
	flushcom("");
	return set_propername(U.id);
}

int sethis_propername (char *in) {
/* MENU COMMAND */
	char user[9];
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	return set_propername(user);
}


int set_propername (char *user) {
	int i;
	int j;
	char command[MAINLINE];
	char propername[MAINLINE];
	char newproper[MAINLINE];


	get_propername(user,propername,MAINLINE);

	if (propername[0]) {
		/*printf("\nCurrent full name: %s\n",propername);*/
		printf("\n");
		printf(Ustring[391],Ustring[337],propername);
		printf("\n");
		sprintf(command,Ustring[400],Ustring[337]);
		/*make_prompt("New full name: ");*/
		make_prompt(command);
	} else {
		/*make_prompt("\nGive the full name you wish to be known by: ");*/
		sprintf(command,Ustring[399],Ustring[337]);
		make_prompt(command);
	}
	get_one_line(newproper);
	printf("\n");

	tnt(newproper);
	if (!newproper[0]) {
		sprintf(command,Ustring[158],Ustring[337]);/*Clear AKA?*/
		if (!propername[0] || !no_yes(command)) {
			return 0;
		}
	}
	
	i = 0;
	j = 0;
	while (newproper[i]) {
		if ((newproper[i] != ':') && (newproper[i] != ',')) {
			propername[j] = newproper[i];
			j++;
		}
		i++;
	}
	propername[j] = 0;
		
	sprintf(command,"username %s \"%s\"",user,propername);
	if (!dsystem(command)) {
		return 1;
	} else {
		/*printf("Please ask %s about this facility.\n",C.sysopname);*/
		printf(Ustring[316],Ustring[464]); /* says not available */
		printf("\n");
	}
	return 0;
}

int setmy_phone (char *dummy) {
/* MENU COMMAND */
	flushcom("");
	return set_phone(U.id);
}

int sethis_phone (char *in) {
/* MENU COMMAND */
	char user[9];
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	return set_phone(user);
}

int set_phone (char *user) {
	char temp[MAINLINE];
	char tempa[MAINLINE];
	struct details *dt;

	if (!user[0]) {
		return 0;
	}
	dt = details_read('q',user);

	if (dt->phone[0]) {
		/*printf("\nCurrent phone number: %s\n",dt->phone);*/
		printf("\n");
		printf(Ustring[391],Ustring[338],dt->phone);
		printf("\n");
		/*make_prompt("New number: ");*/
		sprintf(tempa,Ustring[400],Ustring[338]);
		make_prompt(tempa);
	} else {
		/*make_prompt("\nVoice phone number: ");*/
		sprintf(tempa,Ustring[399],Ustring[338]);
		make_prompt(tempa);
	}
	get_one_line(temp);
	printf("\n");
	if (!temp[0]) {
		free_details(dt);
		return 0;
	}
	free(dt->phone);
	dt->phone = strdup(temp);
	details_write('v',user,dt);
	free_details(dt);
	return 1;
}

/* END of Personal Details */
/*=========================================================================*/

/*=========================================================================*/
/* USAGE DEFAULTS */


int setmy_erase (char *dummy) {
/* MENU COMMAND */
	int result;
	/*if (result = do_stty(U.id,"erase","delete")) {*/
	if ((result = do_stty(U.id,"erase",Ustring[340]))) {
		update("");
	}
	flushcom("");
	return result;
}	

int sethis_erase (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		

	/*if (result = do_stty(user,"erase","erase")) {*/
	if ((result = do_stty(user,"erase",Ustring[340]))) {
		update_force(user);
	}
	return result;
}

int setmy_werase (char *dummy) {
/* MENU COMMAND */
	int result;
	/*if (result = do_stty(U.id,"werase","word erase")) {*/
	if ((result = do_stty(U.id,"werase",Ustring[341]))) {
		update("");
	}
	flushcom("");
	return result;
}	

int sethis_werase (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	/*if (result = do_stty(user,"werase","word erase")) {*/
	if ((result = do_stty(user,"werase",Ustring[341]))) {
		update_force(user);
	}
	return result;
}

int setmy_kill (char *dummy) {
/* MENU COMMAND */
	int result;
	/* if (result = do_stty(U.id,"kill","line kill")) { */
	if ((result = do_stty(U.id,"kill",Ustring[342]))) {
		update("");
	}
	flushcom("");
	return result;
}	

int sethis_kill (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	/*if (result = do_stty(user,"kill","line kill")) {*/
	if ((result = do_stty(user,"kill",Ustring[342]))) {
		update_force(user);
	}
	return result;
}

int setmy_reprint (char *dummy) {
/* MENU COMMAND */
	int result;
	/*if (result = do_stty(U.id,"reprint","line redraw")) {*/
	if ((result = do_stty(U.id,"reprint",Ustring[343]))) {
		update("");
	}
	flushcom("");
	return result;
}	

int sethis_reprint (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	/*if (result = do_stty(user,"reprint","line redraw")) {*/
	if ((result = do_stty(user,"reprint",Ustring[343]))) {
		update_force(user);
	}
	return result;
}



int do_stty(char *user,char *parm,char *explanation) {
	char key[2];
	char temp[MAINLINE];
	struct uservars u;
	int result = 0;

	if (defaults_read(user,&u)) {
		/*sprintf(temp,"Please press the key you wish to use for %s: ",explanation);*/
		sprintf(temp,Ustring[339],explanation);
		make_prompt(temp);
		get_raw(0,2,key,-1);
		if (key[0]) {
			if (!strcmp(parm,"erase")) {
				u.erase = key[0];
			} else if (!strcmp(parm,"werase")) {
				u.werase = key[0];
			} else if (!strcmp(parm,"kill")) {
				u.kill = key[0];
			} else if (!strcmp(parm,"reprint")) {
				u.reprint = key[0];
			}
			if (defaults_write(user,&u)) {
				result = 1;
			} else {
				/*printf("Unable to write new defaults - abandoned.\n");*/
				printf(Ustring[67],Ustring[344]);
				printf(" - %s\n",Ustring[60]);
			}
		} else {
			/*printf("Your %s key has not been changed.\n",explanation);*/
			sprintf(temp,Ustring[346],explanation);
			printf(Ustring[345],temp);
			printf("\n");
		}
	} else {
		/*printf("Unable to read current defaults file - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
}




int setmy_chat (char *dummy) {
/* MENU COMMAND */
	int result;
	if ((result = set_chat(U.id))) {
		update("");
	}
	flushcom("");
	return result;
}

int sethis_chat (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	result = set_chat(user);
	if (result) {
		update_force(user);
	}
	return result;
}

int set_chat (char *user) {
#if defined(CHAT_COMMANDS)
	struct uservars u;
	int result = 0;

	if (defaults_read(user,&u)) {
		if (u.chat) {
			/*printf("By default chat messages will not be received.\n");*/
			printf("%s\n",Ustring[347]);
			u.chat = 0;
			if (!strcmp(user,U.id)) {
				chatoff("");
			}
		} else {
			u.chat = 1;
			/*printf("By default chat messages will be received.\n");*/
			printf("%s\n",Ustring[348]);
			if (!strcmp(user,U.id)) {
				chaton("");
			}
		}
		if (defaults_write(user,&u)) {
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		/*printf("Unable to read current defaults file - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
#else
	/*printf("Chat not supported on this system.\n");*/
	printf("%s\n",Ustring[228]);
	return 0;
#endif
}



int setmy_hotkeys (char *dummy) {
/* MENU COMMAND */
	int result;

	if ((result = set_hotkeys(U.id))) {
		update("");
	}
	flushcom("");
	return result;
}

int sethis_hotkeys (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_hotkeys(user))) {
		update_force(user);
	}
	return result;
}

int set_hotkeys (char *user) {
	struct uservars u;
	int result = 0;

	if (defaults_read(user,&u)) {
		if (u.hotkeys) {
			u.hotkeys = 0;
			/*printf("Hot-keys now turned OFF.\n");*/
			printf(Ustring[387],Ustring[351],Ustring[349]);
			printf("\n");
		} else {
			/*printf("Hot-keys now turned ON.\n");*/
			printf(Ustring[387],Ustring[351],Ustring[350]);
			printf("\n");
			u.hotkeys = 1;
		}
		if (defaults_write(user,&u)) {
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
}


int setmy_readown (char *dummy) {
/* MENU COMMAND */
	int result;
	if ((result = set_readown(U.id))) {
		update("");
	}
	flushcom("");
	return result;
}

int sethis_readown (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_readown(user))) {
		update_force(user);
	}
	return result;
}

int set_readown (char *user) {
	struct uservars u;
	int result = 0;

	if (defaults_read(user,&u)) {
		if (u.readown) {
			u.readown = 0;
			/*printf("Not now receiving own messages during normal reading.\n");*/
			printf("%s\n",Ustring[352]);
		} else {
			u.readown = 1;
			/*printf("Now receiving own messages in natural sequence.\n");*/
			printf("%s\n",Ustring[353]);
		}
		if (defaults_write(user,&u)) {
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		printf("Unable to read current defaults - abandoned.\n");
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
}



int setmy_pausetime (char *dummy) {
/* MENU COMMAND */
	int result;
	if ((result = set_pausetime(U.id))) {
		update("");
	}
	flushcom("");
	return result;
}

int sethis_pausetime (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_pausetime(user))) {
		update_force(user);
	}
	return result;
}

int set_pausetime (char *user) {
	struct uservars u;
	int result = 0;
	int response;
	char temp[MAINLINE];
	char tempa[MAINLINE];

	if (defaults_read(user,&u)) {

		/*sprintf(temp,"Enter number of seconds to pause before a flush [%d]: ",u.pausetime);*/
		sprintf(tempa,Ustring[356],u.pausetime);
		sprintf(temp,"%s [%s]",Ustring[354],tempa);
		sprintf(tempa,Ustring[399],temp);
		make_prompt(temp);
		response = get_one_num (4, (signed)u.pausetime);

		if (response == u.pausetime) {
			/*printf("Pausetime remains unchanged at %d seconds.\n",response);*/
			sprintf(temp,Ustring[356],response);
			printf(Ustring[394],Ustring[354],temp);
			printf("\n");
		} else {
			/*printf("Pausetime now set to %d seconds.\n",response);*/
			sprintf(temp,Ustring[356],response);
			printf(Ustring[387],Ustring[354],temp);
			printf("\n");
			u.pausetime = response;
		}
		if (defaults_write(user,&u)) {
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
}



int setmy_timeout (char *dummy) {
/* MENU COMMAND */
	int result;
	if ((result = set_timeout(U.id))) {
		update("");
	}
	flushcom("");
	return result;
}

int sethis_timeout (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_timeout(user))) {
		update_force(user);
	}
	return result;
}

int set_timeout (char *user) {
	struct uservars u;
	int result = 0;
	int response;
	char temp[MAINLINE];
	char tempa[MAINLINE];

	if (defaults_read(user,&u)) {
		/*sprintf(temp,"Enter inactivity timeout in seconds (0 to disable) [%d]: ",u.timeout);*/

		sprintf(tempa,Ustring[356],u.timeout);
		sprintf(temp,"%s [%s]",Ustring[355],tempa);
		sprintf(tempa,Ustring[399],temp);
		make_prompt(temp);
		response = get_one_num (4, u.timeout);

		if (response == u.timeout) {
			/*printf("Timeout remains unchanged at %d seconds.\n",response);*/
			sprintf(temp,Ustring[356],response);
			printf(Ustring[394],Ustring[355],temp);
			printf("\n");
		} else {
			/*printf("Timeout now set to %d seconds.\n",response);*/
			sprintf(temp,Ustring[356],response);
			printf(Ustring[387],Ustring[355],temp);
			printf("\n");
			u.timeout = response;
		}
		if (defaults_write(user,&u)) {
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
}






int setmy_rows (char *dummy) {
/* MENU COMMAND */
	int result;
	if ((result = set_rows(U.id))) {
		update("");
	}
	flushcom("");
	return result;
}

int sethis_rows (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_rows(user))) {
		update_force(user);
	}
	return result;
}

int set_rows (char *user) {
	struct uservars u;
	int result = 0;
	int response;
	char temp[MAINLINE];
	char tempa[MAINLINE];

	if (defaults_read(user,&u)) {
		/*sprintf(temp,"Enter screen length in rows [%d]: ",u.rows);*/
		sprintf(tempa,Ustring[359],u.rows);
		sprintf(temp,"%s [%s]",Ustring[357],tempa);
		sprintf(tempa,Ustring[399],temp);
		make_prompt(tempa);
		response = get_one_num (4, u.rows);

		if (response == u.rows) {
			/*printf("Screen length remains unchanged at %d rows.\n",response);*/
			sprintf(temp,Ustring[359],response);
			printf(Ustring[394],Ustring[357],temp);
			printf("\n");
		} else {
			/*printf("Screen length now set to %d rows.\n",response);*/
			sprintf(temp,Ustring[359],response);
			printf(Ustring[387],Ustring[357],temp);
			printf("\n");
			u.rows = response;
		}
		if (defaults_write(user,&u)) {
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
}


int setmy_cols (char *dummy) {
/* MENU COMMAND */
	int result;
	if ((result = set_cols(U.id))) {
		update("");
	}
	flushcom("");
	return result;
}

int sethis_cols (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_cols(user))) {
		update_force(user);
	}
	return result;
}

int set_cols (char *user) {
	struct uservars u;
	int result = 0;
	int response;
	char temp[MAINLINE];
	char tempa[MAINLINE];
	
	if (defaults_read(user,&u)) {
		/*sprintf(temp,"Enter screen width in number of characters [%d]: ",u.cols);*/
		sprintf(tempa,Ustring[360],u.cols);
		sprintf(temp,"%s [%s]",Ustring[358],tempa);
		sprintf(tempa,Ustring[399],temp);
		make_prompt(tempa);
		response = get_one_num (4, u.cols);

		if (response == u.cols) {
			/*printf("Width remains unchanged at %d characters.\n",response);*/
			sprintf(temp,Ustring[360],response);
			printf(Ustring[394],Ustring[358],temp);
			printf("\n");
		} else {
			/*printf("Width now set to %d characters.\n",response);*/
			sprintf(temp,Ustring[360],response);
			printf(Ustring[387],Ustring[358],temp);
			printf("\n");
			u.cols = response;
		}
		if (defaults_write(user,&u)) {
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
}

int setmy_recent (char *dummy) {
/* MENU COMMAND */
	int result;
	if ((result = set_recent(U.id))) {
		update("");
	}
	flushcom("");
	return result;
}

int sethis_recent (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_recent(user))) {
		update_force(user);
	}
	return result;
}

int set_recent (char *user) {
	struct uservars u;
	int result = 0;
	int response;
	char temp[MAINLINE];
	char tempa[MAINLINE];

	if (defaults_read(user,&u)) {
		/*sprintf(temp,"Enter number of old msgs to read in newly joined areas [%d]: ",u.recent);*/
		sprintf(tempa,Ustring[362],u.recent);
		sprintf(temp,"%s [%s]",Ustring[361],tempa);
		sprintf(tempa,Ustring[399],temp);
		make_prompt(temp);
		response = get_one_num (4, u.recent);

		if (response == u.recent) {
			/*printf("Autorecent remains unchanged at %d messages.\n",response);*/
			sprintf(temp,Ustring[362],response);
			printf(Ustring[394],Ustring[361],temp);
			printf("\n");
		} else {
			/*printf("Autorecent now set to %d messages.\n",response);*/
			sprintf(temp,Ustring[362],response);
			printf(Ustring[387],Ustring[361],temp);
			printf("\n");
			u.recent = response;
		}
		if (defaults_write(user,&u)) {
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
}



void find_colour_string (int colourcode,char *colourstring) {
	switch (colourcode) {	
		case 10:
			strcpy(colourstring,Ustring[376]);/*plain*/
			break;
		case 11:
			strcpy(colourstring,Ustring[377]);/*bold*/
			break;
		case 12:
			strcpy(colourstring,Ustring[370]);/*yellow*/
			break;
		case 13:
			strcpy(colourstring,Ustring[371]);/*red*/
			break;
		case 14:
			strcpy(colourstring,Ustring[372]);/*green*/
			break;
		case 15:
			strcpy(colourstring,Ustring[373]);/*blue*/
			break;
		case 16:
			strcpy(colourstring,Ustring[374]);/*cyan*/
			break;
		case 17:
			strcpy(colourstring,Ustring[375]);/*magenta*/
			break;
		case 18:
			strcpy(colourstring,Ustring[378]);/*multi*/
			break;
		default:
			strcpy(colourstring,Ustring[368]);/*unknown*/
			break;

	}
}

int setmy_chatsendcolour (char *in) {
/* MENU COMMAND */
	int result;
	char colourstring[11];
	char *copy = strdup(in);

	shiftword(copy,colourstring,11);
	if (!colourstring[0]) {
		flushcom("");
	}
	free(copy);
	if ((result = set_chatsendcolour(U.id,atoi(colourstring)))) {
		update("");
	}
	return result;
}

int sethis_chatsendcolour (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char colourstring[11];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	shiftword(copy,colourstring,11);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_chatsendcolour(user,atoi(colourstring)))) {
		update_force(user);
	}
	return result;
}

int set_chatsendcolour (char *user, int colour) {
#if defined(CHAT_COMMANDS)
	struct uservars u;
	int result = 0;
	char response[2];
	char temp[MAINLINE];

	if (defaults_read(user,&u)) {
	        if ((colour < 11) || (colour > 17)) {
			/*
			printf("Available colours\n");
			printf("[a] - uncoloured\n");
			printf("[b] - yellow\n");
			printf("[c] - red\n");
			printf("[d] - green\n");
			printf("[e] - blue\n");
			printf("[f] - cyan\n");
			printf("[g] - magenta\n");
			*/
			printf("%s\n",Ustring[379]);
			printf("[1] - %s\n",Ustring[369]);
			printf("[2] - %s\n",Ustring[370]);
			printf("[3] - %s\n",Ustring[371]);
			printf("[4] - %s\n",Ustring[372]);
			printf("[5] - %s\n",Ustring[373]);
			printf("[6] - %s\n",Ustring[374]);
			printf("[7] - %s\n",Ustring[375]);
			printf("\n");
			/*make_prompt("Choose colour for your outgoing chat messages: ");*/
			find_colour_string(u.chatsendcolour,temp);
        		printf(Ustring[391],Ustring[380],temp);
			printf("\n");
        		sprintf(temp,Ustring[393],Ustring[380]);
        		make_prompt(temp);

			get_one_lc_char(response);
			if (!response[0]) {
				return 0;
			}
			switch (response[0]) {		
				case '1':
					u.chatsendcolour = bold;
					break;
				case '2':
					u.chatsendcolour = yellow;
					break;
				case '3':
					u.chatsendcolour = red;
					break;
				case '4':
					u.chatsendcolour = green;
					break;
				case '5':
					u.chatsendcolour = blue;
					break;
				case '6':
					u.chatsendcolour = cyan;
					break;
				case '7':
					u.chatsendcolour = magenta;
					break;
				default:
					/*printf("'%s' invalid.\n",response);*/
					printf(Ustring[472],response);
					printf("\n");
					return 0;			
			}
		} else {
			u.chatsendcolour = colour;
		}		
		if (defaults_write(user,&u)) {
			find_colour_string(u.chatsendcolour,temp);
			/*printf("Chat colour set.\n");*/
			printf(Ustring[387],Ustring[380],temp);
			printf("\n");
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
#else
	printf("Chat not available on this system.\n");
	return 0;
#endif
}

int setmy_chatreccolour (char *in) {
/* MENU COMMAND */
	int result;
	char colourstring[11];
	char *copy = strdup(in);
	
	shiftword(copy,colourstring,11);
	if (!colourstring[0]) {
		flushcom("");
	}
	free(copy);

	if ((result = set_chatreccolour(U.id,atoi(colourstring)))) {
		update("");
	}
	return result;
}

int sethis_chatreccolour (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char colourstring[11];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	shiftword(copy,colourstring,11);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_chatreccolour(user,atoi(colourstring)))) {
		update_force(user);
	}
	return result;
}


int set_chatreccolour (char *user, int colour) {
#if defined(CHAT_COMMANDS)
	struct uservars u;
	int result = 0;
	char response[2];
	char temp[MAINLINE];
	
	if (defaults_read(user,&u)) {
		if ((colour < 10) || (colour > 18)) {
			/*
			printf("Available colours\n");
			printf("[a] - plain text\n");
			printf("[b] - bold\n");
			printf("[c] - yellow\n");
			printf("[d] - red\n");
			printf("[e] - green\n");
			printf("[f] - blue\n");
			printf("[g] - cyan\n");
			printf("[h] - magenta\n");
			printf("[i] - multi-coloured as sent\n");
			*/
			printf("%s\n",Ustring[379]);
			printf("[1] - %s\n",Ustring[376]);
			printf("[2] - %s\n",Ustring[377]);
			printf("[3] - %s\n",Ustring[370]);
			printf("[4] - %s\n",Ustring[371]);
			printf("[5] - %s\n",Ustring[372]);
			printf("[6] - %s\n",Ustring[373]);
			printf("[7] - %s\n",Ustring[374]);
			printf("[8] - %s\n",Ustring[375]);
			printf("[9] - %s\n",Ustring[378]);
			printf("\n");

			/*make_prompt("Choose colour for your incoming chat messages: ");*/
			find_colour_string(u.chatcolour,temp);
        		printf(Ustring[391],Ustring[381],temp);
			printf("\n");
        		sprintf(temp,Ustring[393],Ustring[381]);
        		make_prompt(temp);
	
			get_one_lc_char(response);
			if (!response[0]) {
				return 0;
			}
			switch (response[0]) {		
				case '1':
					u.chatcolour = plain;
					break;
				case '2':
					u.chatcolour = bold;
					break;
				case '3':
					u.chatcolour = yellow;
					break;
				case '4':
					u.chatcolour = red;
					break;
				case '5':
					u.chatcolour = green;
					break;
				case '6':
					u.chatcolour = blue;
					break;
				case '7':
					u.chatcolour = cyan;
					break;
				case '8':
					u.chatcolour = magenta;
					break;
				case '9':
					u.chatcolour = multi;
					break;
				default:
					/*printf("'%s' invalid.\n",response);*/
					printf(Ustring[472],response);
					printf("\n");
					return 0;			
			}
		} else {
			u.chatcolour = colour;
		}
		if (defaults_write(user,&u)) {
			find_colour_string(u.chatcolour,temp);
			/*printf("Chat colour set.\n");*/
			printf(Ustring[387],Ustring[381],temp);
			printf("\n");
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
#else
	printf("Chat not available on this system.\n");
	return 0;
#endif
}



int setmy_display (char *dummy) {
/* MENU COMMAND */
	int result;
	if ((result = set_display(U.id))) {
		update("");
	}
	flushcom("");
	return result;
}

int sethis_display (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_display(user))) {
		update_force(user);
	}
	return result;
}

int set_display (char *user) {
	struct uservars u;
	int result = 0;
	int response;
	int i,j;
	char temp[MAINLINE];
	struct {
		char **prog;
		char **name;
	} display_set[3] = {
		&C.display1,	&C.displayname1,
		&C.display2,	&C.displayname2,
		&C.display3,	&C.displayname3,
	};

	if (defaults_read(user,&u)) {

		i = 0;
		j = 0;
		while (i < 3) {
			if ((*(display_set[i].prog))[0]) {
				printf("[%d] - %s\n",i+1,*(display_set[i].name));
			}
			if (!strcmp(u.display,*(display_set[i].prog))) j=i;
			i++;
		}
		printf("\n");
		/*make_prompt("Choose display program: ");*/
		sprintf(temp,Ustring[393],Ustring[385]);
		make_prompt(temp);

		response = get_one_num(1,j+1) - 1;
		if (((response < 0) || (response > 2)) || !(*(display_set[response].prog))[0]) {
			/*printf("Invalid choice - display remaining unchanged.\n");*/
			printf(Ustring[472],response);
			printf("\n");
			return 0;
		} else {
			strcpy(u.display, *(display_set[response].prog));
			strcpy(u.displayname, *(display_set[response].name));
			/*("Display set to %s.\n", *(display_set[response].name));*/
			printf(Ustring[387],Ustring[385],*(display_set[response].name));
			printf("\n");
		}

		if (defaults_write(user,&u)) {
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
}

int setmy_editor (char *dummy) {
/* MENU COMMAND */
	int result;
	if ((result = set_editor(U.id))) {
		update("");
	}
	flushcom("");
	return result;
}

int sethis_editor (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_editor(user))) {
		update_force(user);
	}
	return result;
}

int set_editor (char *user) {
	struct uservars u;
	int result = 0;
	int response;
	int i,j;
	char temp[MAINLINE];
	struct {
		char **prog;
		char **name;
	} editor[3] = {
		&C.editor1,	&C.editorname1,
		&C.editor2,	&C.editorname2,
		&C.editor3,	&C.editorname3,
	};

	if (defaults_read(user,&u)) {
		i = 0;
		j = 0;
		while (i < 3) {
			if ((*(editor[i].prog))[0]) {
				printf("[%d] - %s\n",i+1,*(editor[i].name));
			}
			if (!strcmp(u.editor,*(editor[i].prog))) j=i;
			i++;
		}
		printf("\n");
		/*make_prompt("Choose your editor program: ");*/
		sprintf(temp,Ustring[393],Ustring[384]);
		make_prompt(temp);
	
		response = get_one_num(1,j+1) - 1;
		if (((response < 0) || (response > 2)) || !(*(editor[response].prog))[0]) {
			/*printf("Invalid choice");*/
			printf(Ustring[472],response);
			printf("\n");
			return 0;
		} else {
			strcpy(u.editor, *(editor[response].prog));
			strcpy(u.editorname, *(editor[response].name));
			/*printf("Editor set to %s.\n", *(editor[response].name));*/
			printf(Ustring[387],Ustring[384],*(editor[response].name));
			printf("\n");
		}
	
		if (defaults_write(user,&u)) {
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
}


int setmy_readmode (char *dummy) {
/* MENU COMMAND */
	int result;
	if ((result = set_readmode(U.id))) {
		update("");
	}
	flushcom("");
	return result;
}

int sethis_readmode (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_readmode(user))) {
		update_force(user);
	}
	return result;
}

int set_readmode (char *user) {
	struct uservars u;
	int result = 0;
	int response;
	char temp[MAINLINE];
	char tempa[10];
	
	if (defaults_read(user,&u)) {
		/*
		printf("[1] - REFERENCE  - Read message replies chain-fashion\n");
		printf("[2] - THREADWISE - Read messages replies numerically by subject\n");
		printf("[3] - NUMERIC    - Read messages strictly numerically\n");
		*/
		printf("[1] - REFERENCE  - %s\n",Ustring[388]);
		printf("[2] - THREADWISE - %s\n",Ustring[389]);
		printf("[3] - NUMERIC    - %s\n",Ustring[390]);
		printf("\n");

		/*printf("Reading mode is currently %s.\n",u.readmode);*/
		printf(Ustring[391],Ustring[190],u.readmode);
		printf("\n");
		/*make_prompt("Enter message reading mode preferred: ");*/
		sprintf(temp,Ustring[393],Ustring[190]);
		make_prompt(temp);

		response = get_one_num(1,0);
		if (!response) {
			/*printf("Reading mode remaining as %s.\n",u.readmode);*/
			printf(Ustring[394],Ustring[190],u.readmode);
			printf("\n");
			return 0;
		} else if (response == 1) {
			strcpy(temp,"REFERENCE");
		} else if (response == 2) {
			strcpy(temp,"THREADWISE");
		} else if (response == 3) {
			strcpy(temp,"NUMERIC");
		} else {
			/*printf("'%d' invalid.\n",response);*/
			sprintf(tempa,"%d",response);
			printf(Ustring[472],tempa);
			return 0;
		}
		if (strcmp(temp,u.readmode)) {
			strcpy(u.readmode,temp);
			/*printf("Reading mode now set to %s.\n",u.readmode);*/
			printf(Ustring[387],Ustring[190],u.readmode);
			printf("\n");
			if (defaults_write(user,&u)) {
				result = 1;
			} else {
				/*printf("Unable to write new defaults - abandoned.\n");*/
				printf(Ustring[67],Ustring[344]);
				printf(" - %s\n",Ustring[60]);
			}
		} else {
			printf("Reading mode remaining as %s.\n",u.readmode);
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
}






int setmy_terminal (char *dummy) {
/* MENU COMMAND */
	int result;
	if ((result = set_terminal(U.id))) {
		update("");
	}
	flushcom("");
	return result;
}

int sethis_terminal (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_terminal(user))) {
		update_force(user);
	}
	return result;
}

int set_terminal (char *user) {
	char filename[MAINLINE + 50];
	char tempdesc[MAINLINE];
	char tempterm[81];
	struct uservars u;
	struct stat statbuf;
	int tempcols;
	int temprows;

	if (defaults_read(user,&u)) {
		sprintf(filename,"%s/termtypes.txt",C.library);

		printf("\n");
		if (!stat(filename,&statbuf) && (statbuf.st_size > 0)) {
			display(filename);
		} else {
			strcpy(filename,"Library file termtypes.txt missing or empty.");
			errorlog(filename);
			/*printf("\nCannot find terminal list but you may try one you know.\n");*/
			printf("\n%s\n",Ustring[396]);
		}
		/*printf("Terminal type is currently %s.\n",u.envterm);*/
		printf(Ustring[391],Ustring[397],u.envterm);
		printf("\n\n");
		/*make_prompt("Please type in name of desired terminal: ");*/
		sprintf(tempdesc,Ustring[400],Ustring[397]);
		make_prompt(tempdesc);
		get_one_line(filename);
		printf("\n");

		shiftword(filename,tempterm,81);

		if (tempterm[0] == 0) {
			/*printf("Terminal remaining as %s.\n",u.envterm);*/
			printf(Ustring[394],Ustring[397],u.envterm);
			printf("\n");
			return 0;
		}

		/*NOW CHECK TERMINFO */
		if (foundterminfo (tempterm,tempdesc,&temprows,&tempcols)) {
			/*printf("\nDescription of %s:\n%s\n",tempterm,tempdesc);*/
			printf(Ustring[256],tempterm);
			printf(":\n%s\n",tempdesc);
#if !defined(TERMCAP)
			/*printf("Lines and columns may be configured separately.\n");*/
			printf("%s\n",Ustring[258]);

			/*
			sprintf(filename,"Continue with setting up %s? Y/n",tempterm);
			make_prompt(filename);
			get_one_lc_char(response);
			if (response[0] && (response[0] != 'y')) {
				return 0;
			}			
			*/

			if (!yes_no(Ustring[292])) {
				return 0;
			}
			/*printf("%s has %d rows display by default.\n",tempterm,temprows);*/
			sprintf(filename,Ustring[359],temprows);
			printf(Ustring[392],tempterm,filename);
			printf("\n");
			/*sprintf(temp,"Enter screen length in rows [%d]: ",temprows);*/
			sprintf(filename,Ustring[359],temprows);
			sprintf(tempdesc,"%s [%s]",Ustring[357],filename);
			sprintf(filename,Ustring[399],tempdesc);
			make_prompt(filename);
			u.rows = get_one_num (4, temprows);

			/*printf("%s has %d column display by default.\n",tempterm,tempcols);*/
			sprintf(filename,Ustring[360],tempcols);
			printf(Ustring[392],tempterm,filename);
			printf("\n");
			/*sprintf(temp,"Enter screen width in number of characters [%d]: ",tempcols);*/
			sprintf(filename,Ustring[360],tempcols);
			sprintf(tempdesc,"%s [%s]",Ustring[358],filename);
			sprintf(filename,Ustring[399],tempdesc);
			make_prompt(filename);
			u.cols = get_one_num (4, u.cols);
#endif
			strcpy(u.envterm,tempterm);
			if (defaults_write(user,&u)) {
				printf("Terminal set to %s.\n",u.envterm);
				return 1;
			} else {
				/*printf("Unable to write new defaults - abandoned.\n");*/
				printf(Ustring[67],Ustring[344]);
				printf(" - %s\n",Ustring[60]);
			}
		} else {
			/*printf("'%s' not available - terminal remaining as %s.\n",tempterm,u.envterm);*/
			printf(Ustring[65],tempterm);
			printf(" - ");
			printf(Ustring[394],Ustring[397],u.envterm);
			printf("\n");
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return 0;
}

int foundterminfo (char *name,char *desc,int *rows,int *cols) {
	char command[MAINLINE];
	FILE *FIL;	
	int result = 0;
		
	sprintf(command,"longname %s > term 2>/dev/null",name);
	if (!dsystem(command)) {
		if ((FIL = fopen("term","r"))) {
			fscanf(FIL,"%d %d ",rows,cols);
			fgets(desc,128,FIL);
			fclose(FIL);
		} 
		if (desc[0] && rows && cols) {
			result = 1;
		}
	} 		
	remove("term");
	return result;
}


/* END of usage defaults */
/*===================================================================*/

/*===================================================================*/
/* FLAGS and LEVELS */

int setmy_flag (char *params) {
/* MENU COMMAND */
	int result;
	char flagstring[MAINLINE];
	char valuestring[MAINLINE];
	char *copy = strdup(params);	

	shiftword(copy,flagstring,MAINLINE);
	tnt(flagstring);
	shiftword(copy,valuestring,MAINLINE);
	free(copy);
	
	if ((result = set_userflag(U.id,flagstring,valuestring))) {
		update("");
	}
	return result;
}



int sethis_flag (char *in) {
/* MENU COMMAND */
/* should be passed flag offset, and value.*/
/* may also be passed user at end */
/* Really meant for a sysop to change a user flag specifically */
/* User is last parameter, cos you are meant to put the flag and value*/
/* on the menu line and just select a victim */

	char flagstring[MAINLINE];
	char valuestring[MAINLINE];
	char user[9];
	int result = 0;
	struct valid_files *user_names;
	char *params = strdup(in);
	char temp[MAINLINE];
	
	shiftword(params,flagstring,MAINLINE);
	if (!flagstring[0]) {
		shiftword(G.comline,flagstring,MAINLINE);
		if (!flagstring[0]) {
			/*make_prompt("Which flag? ");*/
			sprintf(temp,Ustring[393],Ustring[71]);		
			make_prompt(temp);
			get_one_line(flagstring);
			if (!flagstring[0]) {
				/*printf("No flag specified - abandoned.\n");*/
				printf(Ustring[294],Ustring[71]);
				printf(" - %s\n",Ustring[60]);
				free(params);
				return 0;
			}
		}
	}
	
	shiftword(params,valuestring,MAINLINE);
	if (!valuestring[0]) {
		shiftword(G.comline,valuestring,MAINLINE);
		if (!valuestring[0]) {
			make_prompt("What value? ");
			get_one_line(valuestring);
			if (!valuestring[0]) {
				/*printf("No value specified - abandoned.\n");*/
				printf(Ustring[294],Ustring[161]);
				printf(" - %s\n",Ustring[60]);

				free(params);
				return 0;
			}
		}
	}
	
	tnt(params);
	if (params[0]) {
		/*user_names = get_valid_dirs('v',0,"users",C.users,params,0);*/
		user_names = get_valid_dirs('v',0,Ustring[287],C.users,params,0);
	} else {
		/*user_names = get_valid_dirs('v',0,"users",C.users,G.comline,0);*/
		user_names = get_valid_dirs('v',0,Ustring[287],C.users,G.comline,0);
		flushcom("");
	}

	shiftword(user_names->files,user,9);	
	if (!(result = set_userflag(user,flagstring,valuestring))) {
		/*printf("Unable to set flag for %s.  Check validity of flag and value.\n",user);*/
		sprintf(temp,Ustring[67],Ustring[71]);
		printf("%s %s - %s\n",Ustring[196],user,temp);
	} else {		
		update_force(user);
		/*printf("Flag set for %s.\n",user);*/
		sprintf(temp,Ustring[387],Ustring[71],valuestring);
		printf("%s %s - %s\n",Ustring[196],user,temp);
		while (user_names->files[0]) {
			shiftword(user_names->files,user,9);	

			if ((result = set_userflag(user,flagstring,valuestring))) {
				update_force(user);
				/*printf("Flag set for %s.\n",user);*/
				sprintf(temp,Ustring[387],Ustring[71],valuestring);
				printf("%s %s - %s\n",Ustring[196],user,temp);
			} else {
				/*printf("Unable to set flag for %s.\n",user);*/
				sprintf(temp,Ustring[67],Ustring[71]);
				printf("%s %s - %s\n",Ustring[196],user,temp);
			}
		}
	}
	free(user_names->input);
	free(user_names->files);
	free(user_names);
	free(params);
	return result;
}


int set_userflag (char *user, char *inflagstring, char *invaluestring) {
	char uflags[UFLAGMAX + 2];
	char temp[MAINLINE + 50];
	char value;
	int flag = 0;
	char flagstring[MAINLINE];
	char valuestring[MAINLINE];

	if (!user[0]) {
		strcpy(temp,"No user specified.\n");
		errorlog(temp);
		return 0;
	}

	strcpy(flagstring,inflagstring);
	tnt(flagstring);
	if (!flagstring[0]) {
		strcpy(temp,"No flag specified.\n");
		errorlog(temp);
		return 0;
	}

	if (is_num(flagstring)) {
		flag = atoi(flagstring);
	} else {
		flag = name_to_num(C.uflagnames,flagstring);
	}
	
	if ((flag < 1) || (flag > UFLAGMAX)) {
		sprintf(temp,"Invalid flag number specified '%d'.\n",flag);
		errorlog(temp);
		return 0;
	}

	strcpy(valuestring,invaluestring);
	tnt(valuestring);
	if (!valuestring[0]) {
		strcpy(temp,"No value specified.\n");
		errorlog(temp);
		return 0;
	}

	value = valuestring[0];
	lower_string(valuestring);

	flags_read(user,uflags);

	if (!Dstrcmp(valuestring,"off")) {
		value = '0';
	} else if (!Dstrcmp(valuestring,"on")) {
		value = '1';
	} else if (!Dstrcmp(valuestring,"toggle")) {
		if (uflags[flag] == '0') {
			value = '1';
		} else {
			value = '0';
		}
	}
	
	uflags[flag] = value;

	if (flags_write(user,uflags)) {
		return 1;
	}
	return 0;
}

int sub_set_userflag (char *user, int flag, char value) {
	char uflags[UFLAGMAX + 2];

	if (!user[0]) {
		strcpy(G.errmsg,"No user specified.\n");
		errorlog(G.errmsg);
		return 0;
	}
	if ((flag < 1) || (flag > UFLAGMAX)) {
		sprintf(G.errmsg,"Invalid flag number specified '%d'.\n",flag);
		errorlog(G.errmsg);
		return 0;
	}
	if (!value) {
		strcpy(G.errmsg,"No value specified.\n");
		errorlog(G.errmsg);
		return 0;
	}	

	flags_read(user,uflags);
	uflags[flag] = value;

	if (flags_write(user,uflags)) {
		return 1;
	}
	return 0;
}


void set_abend_flag (void) {
	char temp[32];
	int i;
	
	i = abend_read(U.id);
	sprintf(temp,"%0d",i);
	sub_set_userflag(U.id,ABEND,temp[0]);
}

void set_linegroup_flag (void) {
	char temp[32];
	
	sprintf(temp,"%0d",G.line);
	sub_set_userflag(U.id,NODEGROUP,temp[0]);
}


int resetmy_flags (char *dummy) {
/* MENU COMMAND */
	int result;

	if ((result = reset_flags(U.id))) {
		update("");
	}
	return result;
}



int resethis_flags (char *in) {
/* MENU COMMAND */

	char userstring[MAINLINE];
	char user[9];
	int result;
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,MAINLINE);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		

	result = reset_flags(user);
	if (result) {
		update_force(user);
	}
	return result;
}

int reset_flags (char *user) {
	char temp[MAINLINE + 100];
	FILE *FIL;
	
	sprintf(temp,"%s/%s/.flags",C.users,user);
	remove(temp);
	
	if ((FIL = fopen(temp,"w"))) {
		fprintf(FIL,"%s\n",&C.newuserflags[1]);
		fclose(FIL);
		sprintf(temp,"chmod 0770 %s/%s/.flags",C.users,user);
		dsystem(temp);

		return 1;
	} else {
		sprintf(temp,"Failed to make %s/%s/.flags",C.users,user);
		errorlog(temp);
		return 0;
	}
}




int setmy_level (char *params) {
/* MENU COMMAND */
	int result;
	char levelstring[MAINLINE];
	char *copy = strdup(params);
	
	shiftword(copy,levelstring,MAINLINE);
	free(copy);
	tnt(levelstring);

	if ((result = set_userlevel(U.id,levelstring))) {
		update("");
	}
	return result;
}

int sethis_level (char *in) {
/* MENU COMMAND */
/* Meant to have level in the menuline, but will ask later if not */
	char levelstring[MAINLINE];
	char userstring[MAINLINE];
	char user[9];
	char *copy = strdup(in);
	
	shiftword(copy,levelstring,MAINLINE);
	shiftword(copy,userstring,MAINLINE);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,MAINLINE);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	return sethis2_level (user,levelstring);
}

int sethis2_level (char *inuser,char *inlevelstring) {
	int result = 0;
	char user[9];
	char levelstring[5];
	char temp[MAINLINE];

	/*new_get_one_param('v',"Which user? ",inuser,user,9);*/
	sprintf(temp,Ustring[393],Ustring[196]);
	new_get_one_param('v',temp,inuser,user,9);
	if (!user[0]) {
		/*printf("No user specified - abandoned.\n");*/
		printf(Ustring[294],Ustring[196]);
		printf(" - %s\n",Ustring[60]);
		return 0;
	}

	/*new_get_one_param('v',"To what level? ",inlevelstring,levelstring,5);*/
	sprintf(temp,Ustring[399],Ustring[167]);
	new_get_one_param('v',temp,inlevelstring,levelstring,5);
	if (!levelstring[0]) {
		/*printf("No level specified - abandoned.\n");*/
		printf(Ustring[294],Ustring[167]);
		printf(" - %s\n",Ustring[60]);
		return 0;
	}

	if ((result = set_userlevel(user,levelstring))) {
		update_force(user);
		/*printf("Level set.\n");*/
		sprintf(temp,Ustring[387],Ustring[167],levelstring);
		printf("%s %s - %s\n",Ustring[196],user,temp);
	} else {
		/*printf("Unable to set level.\n");*/
		sprintf(temp,Ustring[67],Ustring[167]);
		printf("%s %s - %s\n",Ustring[196],user,temp);
	}
	return result;
}

int set_userlevel (char *user,char *levelstring) {
/* MENU COMMAND */
	int level;
	char temp[MAINLINE];
	char *copy;

	if (!user[0]) {
		strcpy(temp,"No user specified.\n");
		errorlog(temp);
		return 0;
	}
	
	copy = strdup(levelstring);
	tnt(copy);
	if (copy[0] && is_num(copy)) {
		level = atoi(copy);
	} else {
		strcpy(G.errmsg,"Invalid parameters for setting level.\n");
		errorlog(G.errmsg);
		free(copy);
		return 0;
	}	
	free(copy);			
	if (level_write(user,level)) {
		return 1;
	}
	strcpy(G.errmsg,"Unable to set level.\n");
	errorlog(G.errmsg);
	return 0;
}

int setmy_title (char *params) {
/* MENU COMMAND */
	int result;
	char *copy = strdup(params);
	
	tnt(copy);
	result = set_title(U.id,copy);
	free(copy);
	return result;
}

int sethis_title (char *in) {
/* MENU COMMAND */
/* Meant to have title in the menuline, but will ask later if not */
	int result;
	char userstring[MAINLINE];
	char user[9];
	char temp[MAINLINE];
	char *copy = strdup(in);
	
	tnt(copy);
	shiftword(copy,userstring,MAINLINE);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,MAINLINE);
	}
	if (!ask_for_user(userstring,user)) {
		free(copy);
		return 0;
	}		
	if (copy[0]) {
		result = set_title(user,copy);
	} else if (G.comline[0]) {
		result = set_title(user,G.comline);
	} else {
		/*make_prompt("Enter required title: ");*/
		sprintf(temp,Ustring[399],Ustring[156]);
		make_prompt(temp);
		get_one_line(userstring);
		tnt(userstring);
		if (!userstring[0]) {
			sprintf(temp,Ustring[158],Ustring[156]);
			if (!no_yes(temp)) { /*"Clear title?" */
				free(copy);
				return 0;
			}
		}
		result = set_title(user,userstring);
	}				
	free(copy);
	return result;
}



int set_title(char *user, char *titlestring) {
/* MENU COMMAND */
	FILE *TMP;
	char temp[MAINLINE + 100];

	if (!user[0]) {
		return 0;
	}

	sprintf(temp,"%s/%s/.title",C.users,user);
	if ((TMP = fopen(temp,"w"))) {
		fputs(titlestring,TMP);
		fclose(TMP);
		return 1;
	} else {
		return 0;
	}
}



/* END of Flags and Levels */
/*====================================================================*/
/*====================================================================*/
/* DATES and MAIL */


int adjustmy_reserves (char *params) {
/* MENU COMMAND */
	char amountstring[MAINLINE];
	char amount[MAINLINE];
	int result = 0;
	signed int reserves;
	signed int bytes;
	char *copy = strdup(params);

	shiftword(copy,amountstring,MAINLINE);
	free(copy);
	tnt(amountstring);

	reserves = mailreserves_read(U.id);
	
	if (!amountstring[0]) {
		return 0;
	} else if (!((amountstring[0] == '+') || (amountstring[0] == '-'))) {
		sprintf(amount,"+%s",amountstring);
	} else {
		strcpy(amount,amountstring);
	}

	bytes = atoi(amount);
	if (!bytes) {
		return 0;
	}
	
	reserves += bytes;

	if ((result = mailreserves_write(U.id,reserves))) {
		update("");
	}
	return result;
}



int adjusthis_reserves (char *in) {
/* MENU COMMAND */
	char amountstring[MAINLINE];
	char userstring[MAINLINE];
	char user[9];
	char *copy = strdup(in);

	shiftword(copy,amountstring,MAINLINE);
	shiftword(copy,userstring,MAINLINE);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,MAINLINE);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		

	return adjusthis2_reserves (user,amountstring);
}


int adjusthis2_reserves (char *user, char *amountstring) {
	char temp[MAINLINE];
	char tempa[MAINLINE];
	char sign[2];

	signed int reserves;
	int result = 0;
	signed int bytes;
	char amount[MAINLINE];

	if (!user[0]) {
		/*printf("No user specified - abandoned.\n");*/
		printf(Ustring[294],Ustring[196]);
		printf("- %s\n",Ustring[60]);
		return 0;
	}


	reserves = mailreserves_read(user);
	
	if (!amountstring[0]) {
		/*printf("Current mail reserves %d.\n",reserves);*/
		sprintf(temp,Ustring[160],reserves);
		printf(Ustring[391],Ustring[159],temp);
		printf("\n");
		/*make_prompt("Increase(+) or decrease(-)? +/-: ");*/
		make_prompt(Ustring[162]);
		get_one_lc_char(sign);
		if ((sign[0] != '+') && (sign[0] != '-')) {
			/*printf("Abandoned.\n");*/
			printf("%s\n",Ustring[60]);
			return 0;
		}

		/*sprintf(temp,"Adjust by %s how many: ",sign);*/
		sprintf(temp,Ustring[163],sign);
		make_prompt(temp);
		bytes = get_one_num(12,0);
		if (bytes == 0) {
			/*printf("Abandoned.\n");*/
			printf("%s\n",Ustring[60]);
			return 0;
		}
		sprintf(amount,"%s%d",sign,bytes);
	} else if (!((amountstring[0] == '+') || (amountstring[0] == '-'))) {
		sprintf(amount,"+%s",amountstring);
	} else {
		strcpy(amount,amountstring);
	}

	bytes = atoi(amount);
	if (!bytes) {
		return 0;
	}
	
	reserves += bytes;

	/*sprintf(temp,"Adjust to %d? y/N ",reserves);
	make_prompt(temp);
	get_one_lc_char(response);
	if (response[0] != 'y') {
		printf("Abandoned.\n");
		return 0;
	}
	*/
	sprintf(tempa,"%d",reserves);	
	sprintf(temp,Ustring[164],tempa);
	if (!no_yes(temp)) {
		printf("%s\n",Ustring[60]);
		return 0;
	}		
		
	if ((result = mailreserves_write(user,reserves))) {
		update_force(user);
	}
	return result;
}



int adjustmy_expiry (char *params) {
/* MENU COMMAND */
	int result = 0;	
	char daystring[MAINLINE];
	char days[MAINLINE];
	int time_1;
	int time_2;
	signed int realdays;
	char *copy = strdup(params);

	shiftword(copy,daystring,MAINLINE);
	free(copy);
	tnt(daystring);

	time_1 = expiry_read(U.id);
	if (!time_1) {
		time_1 = time(0);
	}
	
	if (!daystring[0]) {
		return 0;
	} else if (!((daystring[0] == '+') || (daystring[0] == '-'))) {
		sprintf(days,"+%s",daystring);
	} else {
		strcpy(days,daystring);
	}

	realdays = atoi(days);
	if (!realdays) {
		return 0;
	}
	
	time_2 = time_1 + (realdays * 24 * 60 * 60);
	if ((result = expiry_write(U.id,time_2))) {
		update("");
	}
	return result;
}

int adjusthis_expiry (char *in) {
/* MENU COMMAND */
	char daystring[MAINLINE];
	char userstring[MAINLINE];
	char user[9];
	char *copy = strdup(in);

	shiftword(copy,daystring,MAINLINE);
	shiftword(copy,userstring,MAINLINE);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,MAINLINE);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		

	return adjusthis2_expiry (user,daystring);
}


int adjusthis2_expiry (char *user,char *daystring) {
	int result = 0;	
	char days[MAINLINE];
	char temp[MAINLINE];
	char newdaystring[21];
	int time_1;
	int time_2;
	signed int realdays;
	char sign[2];

	if (!user[0]) {
		/*printf("No user specified - abandoned.\n");*/
		printf(Ustring[294],Ustring[196]);
		printf("- %s\n",Ustring[60]);
		return 0;
	}
	time_1 = expiry_read(user);
	if (!time_1) {
		time_1 = time(0);
	}
	

	if (!daystring[0]) {
		/*printf("Current expiry date %s\n",drealmtime(time_1));*/
		printf(Ustring[391],Ustring[170],drealmtime(time_1));
		printf("\n");
		/*make_prompt("Make date later(+) or earlier(-)? +/-: ");*/
		make_prompt(Ustring[162]);
		get_one_lc_char(sign);
		if ((sign[0] != '+') && (sign[0] != '-')) {
			/*printf("Abandoned.\n");*/
			printf("%s\n",Ustring[60]);
			return 0;
		}
		/*sprintf(days,"Adjust by %s how many days? ",sign);*/
		sprintf(days,Ustring[163],sign);
		new_get_one_param('v',days,"",newdaystring,5);
		if (!newdaystring[0]) {
			/*printf("Abandoned.\n");*/
			printf("%s\n",Ustring[60]);
			return 0;
		}

		sprintf(days,"%s%s",sign,newdaystring);
	} else if (!((daystring[0] == '+') || (daystring[0] == '-'))) {
		sprintf(days,"+%s",daystring);
	} else {
		strcpy(days,daystring);
	}

	realdays = atoi(days);
	if (!realdays) {
		return 0;
	}
	time_2 = time_1 + (realdays * 24 * 60 * 60);

	/*sprintf(temp,"Adjust to %s?",drealmtime(time_2));*/
	sprintf(temp,Ustring[164],drealmtime(time_2));
	if (!(yes_no(temp))) {
		/*printf("Abandoned.\n");*/
		printf("%s\n",Ustring[60]);
		return 0;
	}
	if ((result = expiry_write(user,time_2))) {
		update_force(user);
	}
	return result;
}


/* END of Dates and Mail */
/* ===============================================================*/

int force_update (char *in) {
/* MENU COMMAND */
	char user[9];
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	return update_force(user);
}


int update_force (char *user) {
	char filename[MAINLINE + 100];
	FILE *FIL;
	struct valid_files *vf;

	/*vf = get_valid_dirs('q',1,"user",C.users,user,0);*/
	vf = get_valid_dirs('q',1,Ustring[196],C.users,user,0);

	if (vf->files[0]) {
		sprintf(filename,"%s/%s/.updatealert",C.users,vf->files);
		if ( (FIL = fopen(filename,"w")) ) {
			fclose(FIL);
		}
		sprintf(filename,"touch %s/%s",C.users,vf->files);
		dsystem(filename);
	}
	free(vf->input);
	free(vf->files);
	free(vf);
	return 1;
}

int force (char *in) {
/* MENU COMMAND */
	char filename[MAINLINE + 50];
	char *params;
	char user[9];
	FILE *FIL;
	int result = 0;
	char userstring[41];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		free(copy);
		return 0;
	}		

	tnt(copy);
	if (copy[0]) {
		params = strdup(copy);
	} else {
		params = strdup(G.comline);
		flushcom("");
	}
	if (!params[0]) {
		free(params);
		/*sprintf(userstring,"Force %s to do what? ",user);*/
		sprintf(userstring,Ustring[176],user);
		make_prompt(userstring);
		get_one_line(filename);
		params = strdup(filename);
	}
	if (params[0]) {
		sprintf(filename,"%s/%s/.force",C.users,user);
		if ( (FIL = fopen(filename,"w")) ) {
			fputs(params,FIL);
			fclose(FIL);
			sprintf(filename,"touch %s/%s",C.users,user);
			dsystem(filename);
			result = 1;
		}
	}
	free(params);
	free(copy);	
	return result;
}




int level_read (char *user) {
	return get_int_from_userfile(".level",user);
}

time_t expiry_read (char *user) {
	return (time_t)get_int_from_userfile(".expiry",user);
}

int check_expiry (char *params) {
/* MENU COMMAND */
/* params are a string to make as an announcement */
	time_t expdate;
	time_t nowdate;
	int diffdays;
	char *copy = strdup(params);
	
	expdate = expiry_read(U.id);

	if (!expdate) {
		free(copy);
		return 0;
	}			
	
	tnt(copy);

	nowdate = time(0);
	if (nowdate > expdate) {
		if (copy[0]) {
			/*printf("Your %s has expired.\n",copy);*/
			printf(Ustring[177],copy);
			printf("\n");
		}
		free(copy);
		return 1;
	}
		
	diffdays = ((expdate - nowdate) / 24 / 60 / 60);
	if (copy[0]) {
		/*printf("Your %s expires in %d days time.\n",copy,diffdays);*/
		printf(Ustring[178],copy,diffdays);
		printf("\n");
	}
	free(copy);
	return 0;
}

time_t firstcall_read (char *user) {
	return (time_t)get_int_from_userfile(".firston",user);
}

time_t lastcall_read (char *user) {
	return get_int_from_userfile(".lastcall",user);
}

time_t lastend_read (char *user) {
	struct stat statbuf;
	char filename[MAINLINE + 100];

	sprintf(filename,"%s/%s/.laston",C.users,user);
	if (!stat(filename,&statbuf)) {
		return statbuf.st_mtime;
	} else {
		return 0;
	}
}

int abend_read (char *user) {
	return get_int_from_userfile(".laston",user);
}

int totalcalls_read (char *user) {
	return get_int_from_userfile(".totalcalls",user);

}

int totalmessages_read (char *user) {
	return get_int_from_userfile(".totalmessages",user);

}

int mailreserves_read (char *user) {
	return get_int_from_userfile(".mailallowance",user);

}

int level_write (char *user, int value) {
	int result;
	result = put_int_in_userfile(".level",user,value);
	return result;
}

int expiry_write (char *user, time_t value) {
	return put_int_in_userfile(".expiry",user,(int)value);
}

int firstcall_write (char *user, time_t value) {
	return put_int_in_userfile(".firston",user,(int)value);
}

int lastcall_write (char *user, int value) {
	return put_int_in_userfile(".lastcall",user,value);
}

int lastend_write (char *user, int value) {
	fflush(LASTON);
	rewind(LASTON);
	return fprintf(LASTON,"%d\n",value);
}

int totalcalls_write (char *user, int value) {
	return put_int_in_userfile(".totalcalls",user, value);
}

int totalmessages_write (char *user, int value) {
	return put_int_in_userfile(".totalmessages",user,value);
}


int mailreserves_write (char *user, int value) {
	return put_int_in_userfile(".mailallowance",user,value);
}



int put_int_in_userfile (char *file, char *user, int value) {
	FILE *CFG;
	char filename[MAINLINE + 100];
	char temp[MAINLINE + 100];

	sprintf(filename,"%s/%s/%s",C.users,user,file);
	if (! (CFG = fopen(filename,"w"))) {
		sprintf(temp,"Error in writing to %s\n",filename);
		errorlog(temp);
		return 0;
	}
	fprintf(CFG,"%d\n",value);
	fclose(CFG);
	return 1;
}


int get_int_from_userfile (char *file, char *user) {

	FILE *CFG;
	char filename[MAINLINE];
	int i = 0;

	sprintf(filename,"%s/%s/%s",C.users,user,file);
	if (!(CFG = fopen(filename,"r"))) {
/* This comes out very annoyingly even where absence of file did not matter very much*/
/*		printf("Error in reading %s\n",filename);*/
		return 0;
	}

	fscanf(CFG," %d ",&i);
	fclose(CFG);
	return i;
}

void get_propername (char *user, char *propername, size_t namelen) {
/* LENGTHS CHECKED */
/* propername is deliberately updated */
	char *temp;
	struct passwd *pword;

	if ( (pword = getpwnam(user)) ) {
#if defined(SVR42)
		temp = strdup(pword->pw_comment);
#else
#  if defined(LINUX)
		temp = strdup(pword->pw_gecos);
#  endif
#endif
		(void)strshift(temp,propername,namelen,",");
		free(temp);
		tnt(propername);
	} else {
		propername[0] = 0;
	}
}




struct details *details_read (char mode, char *user) {
	char filename[MAINLINE + 100];
	char temp[MAINLINE];
	FILE *FIL;
	struct details *d = (struct details *)malloc(sizeof (struct details));
	temp[0]=0;

	sprintf(filename,"%s/%s/.details",C.users,user);
	if ((FIL = fopen(filename,"r"))) {
		if (fgets(temp,MAINLINE,FIL)) {
			tnt(temp);
			temp[30] = 0;
			d->changedate = strdup(temp);
		}

		if (fgets(temp,MAINLINE,FIL)) {
			tnt(temp);
			temp[8] = 0;
			d->id = strdup(temp);
		}

		if (fgets(temp,MAINLINE,FIL)) {
			tnt(temp);
			temp[80] = 0;
			d->realname = strdup(temp);
		}

		if (fgets(temp,MAINLINE,FIL)) {
			tnt(temp);
			temp[80] = 0;
			d->phone = strdup(temp);
		}

		if (fgets(temp,MAINLINE,FIL)) {
			tnt(temp);
			temp[MAINLINE-1] = 0;
			d->address = strdup(temp);
		}

		if (fgets(temp,MAINLINE,FIL)) {
			tnt(temp);
			temp[80] = 0;
			d->dob = strdup(temp);
		}
		fclose(FIL);
	} else {
		if (mode != 'q') {
			/*printf("Could not open %s, using null values.\n",filename);*/
			printf(Ustring[66],Ustring[198]);
			printf("- %s\n",Ustring[192]);
		}
		d->changedate = strdup("");
		d->id = strdup("");
		d->realname = strdup("");
		d->phone = strdup("");
		d->address = strdup("");
		d->dob = strdup("");

	}
	d->propername = (char *)malloc(31);
	get_propername(user,d->propername,31);
	return d;
}

void free_details (struct details *d) {
	free(d->propername);
	free(d->dob);
	free(d->address);
	free(d->phone);
	free(d->realname);
	free(d->id);
	free(d->changedate);
	free(d);
}


int details_write (char mode, char *user, struct details *de) {
	char filename[MAINLINE + 100];
	char pastlife[MAINLINE + 100];
	FILE *FIL;
	time_t t = time(0);


	(void)dlt(filename,MAINLINE,localtime(&t));
	free(de->changedate);
	de->changedate = strdup(filename);

	sprintf(filename,"%s/%s/.details",C.users,user);
	sprintf(pastlife,"%s/%s/.pastlife",C.users,user);
	copy_file(filename,pastlife,0);

	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%s\n",de->changedate);
		fprintf(FIL,"%s\n",user);
		fprintf(FIL,"%s\n",de->realname);
		fprintf(FIL,"%s\n",de->phone);
		fprintf(FIL,"%s\n",de->address);
		fprintf(FIL,"%s\n",de->dob);
		fclose(FIL);
		return 1;
	} else {
		if (mode == 'v') {
			/*printf("Unable to write to %s details file,\n",user);*/
			printf(Ustring[67],Ustring[198]);
			printf("\n");
		}
		return 0;
	}
}


int flags_read (char *user,char *flags) {
/* flags is deliberately updated */
	char filename[MAINLINE + 100];
	FILE *CFG;
	char c;
	int i = 1;

	flags[0] = '#';
	sprintf(filename,"%s/%s/.flags",C.users,user);
	if ((CFG = fopen(filename,"r"))) {
		while ((i <= UFLAGMAX) && !feof(CFG)) {
			fread(&c,1,1,CFG);
			if (!isgraph(c)) {
				continue;
			}
			flags[i] = c;
			i++;
		}
		flags[i] = 0;
		fclose(CFG);
	} else {
		while (i <= UFLAGMAX) {
			flags[i] = '0';
			i++;
		}
		flags[i] = 0;
		return 0;
	}
	return 1;
}

int flags_write (char *user, char *flags) {
	char filename[MAINLINE + 100];
	FILE *FIL;

	sprintf(filename,"%s/%s/.flags",C.users,user);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%s\n",&flags[1]);
		fclose(FIL);
	} else {
		/*printf("Flag file not written.\n");*/
		printf(Ustring[67],Ustring[168]);
		printf("\n");
		return 0;
	}
	return 1;
}


int change_me (char *dummy) {
/* MENU COMMAND */
	flushcom("");
	update("");
	return usermaint(U.id);
}

int change_user (char *in) {
/* MENU COMMAND */
	char user[9];
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	return usermaint(user);
}


int usermaint (char *user) {
	char filename[MAINLINE + 100];
	int level;
	int calls;
	int messages;
	int reserves;
	char tempuser[15];
	char *nice_firstcall;
	char *nice_lastcall;
	char *nice_expiry;
	struct details *dr;
	char choice[2];

	choice[0] = 0;

	if (!user[0]) {
		return 0;
	}

	while (choice[0] != 'q') {
		dr = details_read('v',user);
		level = level_read(user);
		calls = totalcalls_read(user);
		messages = totalmessages_read(user);
		reserves =  mailreserves_read(user);
		nice_expiry = drealmtime(expiry_read(user));
		nice_firstcall = drealmtime(firstcall_read(user));
		nice_lastcall = drealmtime(lastcall_read(user));
		
/*=======================================================================
IGNORE THIS BIT
printf("\n--------------------------------------------------------------------\n");
printf("User id: %s        (details last updated %s)\n",user,dr->changedate);
printf("First on: %s    Last on: %s\n",nice_firstcall,nice_lastcall);
printf("Calls Made: %-6d    Msgs posted: %-6d\n",calls,messages);
printf("[a] Real Name: %s\n",dr->realname);
printf("[b] Address  : %s\n",dr->address);
printf("[c] Phone    : %s\n",dr->phone);
printf("[d] D.O.B.   : %s\n",dr->dob);
printf("[e] A.K.A.   : %s\n",dr->propername);
printf("[f] Security Level      : %-6d\n",level);
printf("[g] Subscription Expires: %s\n",nice_expiry);

#if defined(MAIL_COMMANDS)
printf("[h] Ext. Mail Reserves  : %d\n",reserves);
#endif

printf("[i] View planfile         [j] Edit planfile\n");
printf("[k] View sig              [l] Edit sig\n");

#if defined(READ_COMMANDS)
printf("[m] View scanlist         [n] Edit scanlist\n");
#endif

if (U.level >= C.sysoplevel) {
	printf("[o] View notes            [p] Edit notes\n");
	printf("[r] User defaults         [s] Password change\n");
	printf("[t] User Flags\n");
}
printf("[q] Quit\n");
===================================================================*/


printf("\n--------------------------------------------------------------------\n");
printf(Ustring[401],user,dr->changedate);printf("\n");
printf(Ustring[402],nice_firstcall,nice_lastcall);printf("\n");
printf(Ustring[403],calls,messages);printf("\n");
printf(Ustring[404],dr->realname);printf("\n");
printf(Ustring[405],dr->address);printf("\n");
printf(Ustring[406],dr->phone);printf("\n");
printf(Ustring[407],dr->dob);printf("\n");
printf(Ustring[408],dr->propername);printf("\n");
printf(Ustring[409],level);printf("\n");
printf(Ustring[410],nice_expiry);printf("\n");

#if defined(MAIL_COMMANDS)
printf(Ustring[411],reserves);printf("\n");
#endif

printf("%s\n", Ustring[412]);
printf("%s\n", Ustring[413]);

#if defined(READ_COMMANDS)
printf("%s\n", Ustring[414]);
#endif

if (U.level >= C.sysoplevel) {
	printf("%s\n", Ustring[415]);
	printf("%s\n", Ustring[416]);
	printf("%s\n", Ustring[417]);
}
printf("%s\n", Ustring[418]);



		printf("\n");
		/*make_prompt("Change: ");*/
		make_prompt(Ustring[419]);
		get_one_lc_char(choice);
		
		strcpy(tempuser,user);
	
		switch (choice[0]) {
			case 'a':
				set_name(tempuser);
				break;
			case 'b':
				set_address(tempuser);
				break;
			case 'c':
				set_phone(tempuser);
				break;
			case 'd':
				set_dob(tempuser);
				break;
			case 'e':
				set_propername(tempuser);
				break;
			case 'f':
				if (U.level >= C.sysoplevel) {
					sethis2_level(tempuser,"");
				} else {
					/*printf("You can't do this");*/
					printf("%s\n",Ustring[126]);	
				}
				
				break;
			case 'g':
				if (U.level >= C.sysoplevel) {
					adjusthis2_expiry(tempuser,"");
				} else {
					/*printf("You can't do this");*/
					printf("%s\n",Ustring[126]);	
				}
				break;
#if defined(MAIL_COMMANDS)
			case 'h':
				if (U.level >= C.sysoplevel) {
					adjusthis2_reserves(tempuser,"");
				} else {
					/*printf("You can't do this");*/
					printf("%s\n",Ustring[126]);	
				}
				break;
#endif
			case 'i':
				sprintf(filename,"%s/%s/.plan",C.privatefiles,tempuser);
				display(filename);
				press_enter("");
				break;
			case 'j':
				edit_plan(tempuser);
				break;
			case 'k':
				sprintf(filename,"%s/%s/.sig",C.privatefiles,tempuser);
				display(filename);
				press_enter("");
				break;
			case 'l':
				edit_sig(tempuser);
				break;
#if defined(READ_COMMANDS)
			case 'm':
				sprintf(filename,"%s/%s/.scanlist",C.users,user);
				display(filename);
				press_enter("");
				break;
			case 'n':
				if (U.level >= C.sysoplevel) {
					edit_scanlist(tempuser);
				} else {
					scanlist_edit("");
				}
				break;
#endif
			case 'o':
				if (U.level >= C.sysoplevel) {
					sprintf(filename,"%s/%s/.notes",C.users,tempuser);
					display(filename);
					press_enter("");
				} else {
					/*printf("You can't do this");*/
					printf("%s\n",Ustring[126]);	
				}
				break;
			case 'p':
				if (U.level >= C.sysoplevel) {
					sprintf(filename,"%s/%s/.notes",C.users,tempuser);
					edit_special(filename);
				} else {
					/*printf("You can't do this");*/
					printf("%s\n",Ustring[126]);	
				}
				break;
			case 'r':
				if (U.level >= C.sysoplevel) {
					defaultsmenu(tempuser);
				} else {
					/*printf("You can't do this");*/
					printf("%s\n",Ustring[126]);	
				}
				break;
			case 's':
				if (U.level >= C.sysoplevel) {
					if (sure("")) {
						sethis_password(tempuser);
					}
				} else {
					/*printf("You can't do this");*/
					printf("%s\n",Ustring[126]);	
				}
				break;
			case 't':
				if (U.level >= C.sysoplevel) {
					flagmenu(tempuser);
				} else {
					/*printf("You can't do this");*/
					printf("%s\n",Ustring[126]);	
				}
				break;
		}

		free(nice_lastcall);
		free(nice_firstcall);
		free(nice_expiry);
		free_details(dr);

	}
	return 1;
}

int flagmenu (char *user) {
	char uflag[UFLAGMAX + 2];
	char filename[MAINLINE + 100];
	char newvalue[2];
	char tempuser[15];
	char flagstring[3];
	char temp[MAINLINE];
	int choice;

	sprintf(filename,"%s/%s/.flags",C.users,user);

	/* CONSTCOND */
	while (1) {
		if (!flags_read(user,uflag)) {
			/*make_prompt("Can't read flag file. Create from scratch? y/N ");*/

			sprintf(temp,Ustring[66],Ustring[168]);
			sprintf(filename,"%s %s",temp,Ustring[165]);
			if (!no_yes(filename)) {
				return 0;
			}
			reset_flags(user);
			continue;
		}

/*printf("Flags\n");*/
printf("%s\n",Ustring[168]);
/*printf("No.     Name        No.     Name        No.     Name\n");*/
printf("%s\n",Ustring[169]);

printf("[1]  %c  %-10s   [11]  %c  %-10s   [21]  %c  %-10s\n",uflag[1],C.uflagnames[1],uflag[11],C.uflagnames[11],uflag[21],C.uflagnames[21]);
printf("[2]  %c  %-10s   [12]  %c  %-10s   [22]  %c  %-10s\n",uflag[2],C.uflagnames[2],uflag[12],C.uflagnames[12],uflag[22],C.uflagnames[22]);
printf("[3]  %c  %-10s   [13]  %c  %-10s   [23]  %c  %-10s\n",uflag[3],C.uflagnames[3],uflag[13],C.uflagnames[13],uflag[23],C.uflagnames[23]);
printf("[4]  %c  %-10s   [14]  %c  %-10s   [24]  %c  %-10s\n",uflag[4],C.uflagnames[4],uflag[14],C.uflagnames[14],uflag[24],C.uflagnames[24]);
printf("[5]  %c  %-10s   [15]  %c  %-10s   [25]  %c  %-10s\n",uflag[5],C.uflagnames[5],uflag[15],C.uflagnames[15],uflag[25],C.uflagnames[25]);
printf("[6]  %c  %-10s   [16]  %c  %-10s   [26]  %c  %-10s\n",uflag[6],C.uflagnames[6],uflag[16],C.uflagnames[16],uflag[26],C.uflagnames[26]);
printf("[7]  %c  %-10s   [17]  %c  %-10s   [27]  %c  %-10s\n",uflag[7],C.uflagnames[7],uflag[17],C.uflagnames[17],uflag[27],C.uflagnames[27]);
printf("[8]  %c  %-10s   [18]  %c  %-10s   [28]  %c  %-10s\n",uflag[8],C.uflagnames[8],uflag[18],C.uflagnames[18],uflag[28],C.uflagnames[28]);
printf("[9]  %c  %-10s   [19]  %c  %-10s   [29]  %c  %-10s\n",uflag[9],C.uflagnames[9],uflag[19],C.uflagnames[19],uflag[29],C.uflagnames[29]);
printf("[10] %c  %-10s   [20]  %c  %-10s   [30]  %c  %-10s\n",uflag[10],C.uflagnames[10],uflag[20],C.uflagnames[20],uflag[30],C.uflagnames[30]);
/*printf("[0]  quit\n");*/
printf("[0]  %s\n",Ustring[63]);

		printf("\n");

		sprintf(temp,Ustring[393],Ustring[71]);
		make_prompt(temp);
		choice = get_one_num(2,0);

		if (!choice) {
			break;
		}
		strcpy(tempuser,user);

		if ((choice > 0) || (choice <= UFLAGMAX)) {
			sprintf(flagstring,"%d",choice);
			/*make_prompt("New value: ");*/
			sprintf(temp,Ustring[400],Ustring[161]);
			make_prompt(temp);
			get_one_char(newvalue);
			if (newvalue[0]) {
				set_userflag(tempuser,flagstring,newvalue);
				update_force(tempuser);
			} else {
				/*printf("Flag unchanged.\n");*/
				printf(Ustring[394],Ustring[71],uflag[choice]);
				printf("\n");
			}
		} else if (choice > UFLAGMAX) {
			/*printf("No such flag.\n");*/
			printf(Ustring[472],choice);
			printf("\n");
                                                 
		}
	}
	return 1;
}

int defaultsmenu (char *user) {
	char choice[2];
	struct uservars u;
	char tempuser[15];

	while (choice[0] != 'q') {
		if (defaults_read(user,&u)) {

/*=============================================================
printf("[a] - hotkeys (%d)       [b] - readmode (%s)\n",u.hotkeys,u.readmode);
printf("[c] - readown (%d)       [d] - recent (%d)\n",u.readown,u.recent);
printf("[e] - rows (%2d)         [f] - columns (%d)\n",u.rows,u.cols);
printf("[g] - erase (%2d)        [h] - word erase (%d)\n",u.erase,u.werase);
printf("[i] - kill (%2d)         [j] - reprint (%d)\n",u.kill,u.reprint);
printf("[k] - pausetime (%2d)    [l] - timeout (%d)\n",u.pausetime,u.timeout);
printf("[m] - chatcolour IN (%2d)[n] - chatcolour OUT(%d)\n",u.chatcolour,u.chatsendcolour);
printf("[o] - chat on at login (%d)\n",u.chat);
printf("[p] - editor (%s)\n",u.editorname);
printf("[r] - display (%s)\n",u.displayname);
printf("[s] - terminal (%s)\n",u.envterm);
printf("[q] - quit\n");
===============================================================*/

printf(Ustring[420],u.hotkeys,u.readmode);printf("\n");
printf(Ustring[421],u.readown,u.recent);printf("\n");
printf(Ustring[422],u.rows,u.cols);printf("\n");
printf(Ustring[423],u.erase,u.werase);printf("\n");
printf(Ustring[424],u.kill,u.reprint);printf("\n");
printf(Ustring[425],u.pausetime,u.timeout);printf("\n");
printf(Ustring[426],u.chatcolour,u.chatsendcolour);printf("\n");
printf(Ustring[427],u.chat);printf("\n");
printf(Ustring[428],u.editorname);printf("\n");
printf(Ustring[429],u.displayname);printf("\n");
printf(Ustring[430],u.envterm);printf("\n");
printf(Ustring[431],u.languagename);printf("\n");
printf("%s\n", Ustring[432]);
		
			printf("\n");
			/*make_prompt("Change: ");*/
			make_prompt(Ustring[419]);
			get_one_lc_char(choice);
			strcpy(tempuser,user);

			switch (choice[0]) {
				case 'a':
					sethis_hotkeys(tempuser);
					break;
				case 'b':
					sethis_readmode(tempuser);
					break;
				case 'c':
					sethis_readown(tempuser);
					break;
				case 'd':
					sethis_recent(tempuser);
					break;
				case 'e':
					sethis_rows(tempuser);
					break;
				case 'f':
					sethis_cols(tempuser);
					break;
				case 'g':
					sethis_erase(tempuser);
					break;
				case 'h':
					sethis_werase(tempuser);
					break;
				case 'i':
					sethis_kill(tempuser);
					break;
				case 'j':
					sethis_reprint(tempuser);
					break;
				case 'k':
					sethis_pausetime(tempuser);
					break;
				case 'l':
					sethis_timeout(tempuser);
					break;
				case 'm':
					sethis_chatreccolour(tempuser);
					break;
				case 'n':
					sethis_chatsendcolour(tempuser);
					break;
				case 'o':
					sethis_chat(tempuser);
					break;
				case 'p':
					sethis_editor(tempuser);
					break;
				case 'r':
					sethis_display(tempuser);
					break;
				case 's':
					sethis_terminal(tempuser);
					break;
				case 't':
					sethis_lang(tempuser);
					break;
				default :
					break;
			}
		} else {
			/*printf("Unable to read current defaults - abandoned.\n");*/
			printf(Ustring[66],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
			return 0;
		}
	}
	return 1;
}


int get_home (char *user,char *homestring) {
	struct passwd *pw = getpwnam(user);
	
	if (pw)	{
		strcpy(homestring,getpwnam(user)->pw_dir);
		return 1;
	} else {
		homestring[0] = 0;
		return 0;
	}
}


int is_bbs_account (char *user) {
	char filename[MAINLINE + 100];
	struct stat statbuf;

	sprintf(filename,"%s/%s",C.users,user);
	if (!stat(filename,&statbuf)) {
		return 1;
	} else {
		return 0;
	}
}

int is_shell_account (char *user) {
	struct passwd *pword;

	if (!(pword = getpwnam(user))) {
		return 0;
	}

	if (!strcmp(pword->pw_shell,C.bbsshell)) {
		return 0;
	} else {
		return 1;
	}
}

int is_in_passwd (char *user) {
	return (getpwnam(user) != NULL);
}



int guest_reset (char *dummy) {
/* MENU COMMAND */
/* U.id sparks it off by going through a menu option */
	char temp[MAINLINE * 2];

	totalmessages_write(U.id,0);
	totalcalls_write(U.id,1);

	sprintf(temp,"rm -f %s/%s/.tmpareas/*",C.users,U.id);
	dsystem(temp);
	sprintf(temp,"rm -f %s/%s/.areas/*",C.users,U.id);
	dsystem(temp);
	sprintf(temp,"echo 0 > %s/%s/.newshigh",C.users,U.id);
	dsystem(temp);
	sprintf(temp,"rm -f %s/%s/*",C.privatefiles,U.id);
	dsystem(temp);
	sprintf(temp,"rm -f %s/%s/.mail/*",C.maildirs,U.id);
	dsystem(temp);
	sprintf(temp,"rm -f %s/%s/.scanlist",C.users,U.id);
	dsystem(temp);

	reset_userdefaults(U.id);
	update("");	
	return 1;
}


int self_make (char *dummy) {
/* MENU COMMAND */
	char newid[9];
	char temp[MAINLINE];
	char filename[MAINLINE + 100];
	int z = 0;
	struct stat sb;

	
	sprintf(filename,"%s/rules",C.library);
	if (!stat(filename,&sb) && (sb.st_size > 0)) {
		/*printf("These are the rules for use of %s.\n",C.bbsname);*/
		printf(Ustring[209],C.bbsname);
		printf("\n");
		press_enter("");
		display(filename);
		
		/*make_prompt("Do you agree to observe these rules? Y/n ");*/
		if (!yes_no(Ustring[211])) {
			/*printf("Account creation abandoned\n");*/
			printf("%s\n",Ustring[60]);
			return 0;
		}
	}		
	
	while (z == 0) {
		/*make_prompt("Please type in a unique user ID of 8 characters or less: ");*/
		make_prompt(Ustring[227]);
		get_one_name(newid,9,"");
		if (!newid[0]) {
			/*printf("Account creation abandoned\n");*/
			printf("%s\n",Ustring[60]);
			return 0;
		}
		if (is_num(newid)) {
			/*printf("All numeric IDs not allowed, please choose again.\n");*/
			sprintf(temp,Ustring[472],newid);
			printf(Ustring[503],temp);
			printf("\n");
			continue;
		}
		if (is_bbs_account(newid)) {
			/*printf("There is already a '%s', please choose again.\n",newid);*/
			sprintf(temp,Ustring[253],C.bbsname,newid);
			printf(Ustring[503],temp);
			printf("\n");
			continue;
		}
		if (is_in_passwd(newid)) {
			/*sprintf(temp,"'%s' is already in the password file.  Is that your account? y/N ",newid);*/
			sprintf(temp,Ustring[248],newid);
			if (!no_yes(temp)) {
				continue;
			}
		}
		z = 1;
	}
	printf("\n");	
	/*sprintf(temp,"Set up account under name '%s'? Y/n ",newid);*/
	sprintf(temp,Ustring[264],newid);	
	if (!yes_no(temp)) {
		/*printf("Account creation abandoned\n");*/
		printf("%s\n",Ustring[60]);
		return 0;
	}


	if (!make_bbs_account(newid,"")) {
		sprintf(G.errmsg,"ACTION FAILURE:Could not complete creation of BBS files for %s, unix account and some files may have been created and now need removing.",newid);
		errorlog(G.errmsg);
		sprintf(G.errmsg,"BBS account creation failed for %s, more details in errorlog.",newid);
		newuserlog(G.errmsg);
		display_lang("nocreate");		
		return 0;
	}

	if (!(is_in_passwd(newid) || make_unix_account(newid,""))) {
		sprintf(G.errmsg,"ACTION FAILURE:System refused unix account for %s.",newid);
		errorlog(G.errmsg);
		sprintf(G.errmsg,"Could not create system account for %s",newid);
		newuserlog(G.errmsg);
		display_lang("nocreate");		
		return 0;
	}

	sprintf(filename,"%s created BBS account. Completed.",newid);
	newuserlog(filename);
	if (C.canlogin) {
		/*printf("Attempting auto-login as %s...\n",newid);*/
		printf(Ustring[330],newid);
		printf("\n");
		external_term();
		execlp("login","login",newid,(char *) NULL);
		internal_term();
		/*printf("Auto-login failed.  Please log in again manually as %s.\n",newid);*/
		printf(Ustring[331],newid);
		printf("\n");
	} else {
		/*printf("Please log in again now as %s.\n",newid);*/
		printf(Ustring[363],newid);
		printf("\n");
	}
	press_enter("");
	logoff("");
	return 1;
}

int makehis_account (char *in) {
/* MENU COMMAND */
	char newid[9];
	char temp[MAINLINE];
	char filename[MAINLINE + 100];
	char *copy = strdup(in);

	shiftword(copy,newid,9);
	free(copy);
	if (!newid[0]) {
		shiftword(G.comline,newid,9);
	}

	if (!newid[0]) {	
		/*make_prompt("Please type in a unique user ID of 8 characters or less: ");*/
		make_prompt(Ustring[227]);
		get_one_name(newid,9,"");
	}
	if (!newid[0]) {
		/*printf("Account creation abandoned\n");*/
		printf("%s\n",Ustring[60]);
		return 0;
	}
	if (is_bbs_account(newid)) {
		/*printf("There is already a '%s', please choose again.\n",newid);*/
		printf(Ustring[253],C.bbsname,newid);
		printf("\n");
		return 0;
	}
	if (is_in_passwd(newid)) {
		/*sprintf(temp,"'%s' is an existing system user.  Is that ok? y/N ",newid);*/
		sprintf(temp,Ustring[364],newid);
		if (!no_yes(temp)) {
			/*printf("Abandoned.\n");*/
			printf("%s\n",Ustring[60]);
			return 0;
		}
	}

	/*sprintf(temp,"\nSet up account under name '%s'? Y/n ",newid);	*/
	sprintf(temp,Ustring[264],newid);	
	if (!yes_no(temp)) {
		/*printf("Account creation abandoned\n");*/
		printf("%s\n",Ustring[60]);
		return 0;
	}

	if (!make_bbs_account(newid,"")) {
		sprintf(G.errmsg,"ACTION FAILURE:Could not complete creation of BBS files for %s, unix account and some files may have been created and now need removing.",newid);
		errorlog(G.errmsg);
		/*printf("Sorry, unable to create account for %s.\n",newid);*/
		printf(Ustring[270],newid);
	
		printf("\n");
		return 0;
	}

	if (!(is_in_passwd(newid) || make_unix_account(newid,""))) {
		/*printf("Automatic Unix account creation not available.\n");*/
		/*printf("Please ask your Superuser to complete this addition.\n");*/
		sprintf(G.errmsg,"ACTION FAILURE:System refused unix account for %s.",newid);
		errorlog(G.errmsg);
		printf(Ustring[365],newid);
		printf("\n");
		return 0;
	}

	sprintf(filename,"BBS account created by %s for %s.",U.id,newid);
	newuserlog(filename);
	printf("Account created.\n");
	return 1;
}

int make_bbs_account (char *id, char *password) {
	char filename[MAINLINE + 100];
	char temp[MAINLINE * 2 + 200];
	FILE *FIL;

	sprintf(filename,"%s/%s",C.privatefiles,id);
	if (!make_acc_dirs(filename)) {
		return 0;
	}

	sprintf(filename,"%s/%s",C.users,id);
	if (!make_acc_dirs(filename)) {
		return 0;
	}

	sprintf(filename,"%s/%s/.areas",C.users,id);
	if (!make_acc_dirs(filename)) {
		return 0;
	}

	sprintf(filename,"%s/%s",C.maildirs,id);
	if (!make_acc_dirs(filename)) {
		return 0;
	}

	sprintf(filename,"%s/%s/.mail",C.maildirs,id);
	if (!make_acc_dirs(filename)) {
		return 0;
	}

	sprintf(filename,"%s/%s/.flags",C.users,id);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%s\n",&C.newuserflags[1]);
		fclose(FIL);
		chmod(filename,0770);
	} else {
		sprintf(G.errmsg,"CONFIG ERROR:During user creation failed to write %s",filename);
		errorlog(G.errmsg);
	}

	sprintf(filename,"%s/%s/.firston",C.users,id);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%ld\n",time(0));
		fclose(FIL);
		chmod(filename,0770);
	} else {
		sprintf(G.errmsg,"CONFIG ERROR:During user creation failed to write %s",filename);
		errorlog(G.errmsg);
	}

	sprintf(filename,"%s/%s/.lastcall",C.users,id);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%ld\n",time(0));
		fclose(FIL);
		chmod(filename,0770);
	} else {
		sprintf(G.errmsg,"CONFIG ERROR:During user creation failed to write %s",filename);
		errorlog(G.errmsg);
	}


	sprintf(filename,"%s/%s/.newshigh",C.users,id);
	if ((FIL = fopen(filename,"w"))) {
		fputs("0\n",FIL);
		fclose(FIL);
		chmod(filename,0770);
	} else {
		sprintf(G.errmsg,"CONFIG ERROR:During user creation failed to write %s",filename);
		errorlog(G.errmsg);
	}

	sprintf(filename,"%s/%s/.totalmessages",C.users,id);
	if ((FIL = fopen(filename,"w"))) {
		fputs("0\n",FIL);
		fclose(FIL);
		chmod(filename,0770);
	} else {
		sprintf(G.errmsg,"CONFIG ERROR:During user creation failed to write %s",filename);
		errorlog(G.errmsg);
	}

	sprintf(filename,"%s/%s/.totalcalls",C.users,id);
	if ((FIL = fopen(filename,"w"))) {
		fputs("0\n",FIL);
		fclose(FIL);
		chmod(filename,0770);
	} else {
		sprintf(G.errmsg,"CONFIG ERROR:During user creation failed to write %s",filename);
		errorlog(G.errmsg);
	}

	sprintf(filename,"%s/%s/.level",C.users,id);
	if ((FIL = fopen(filename,"w"))) {
		fprintf(FIL,"%d\n",C.newuserlevel);
		fclose(FIL);
		chmod(filename,0770);
	} else {
		sprintf(G.errmsg,"CONFIG ERROR:During user creation failed to write %s",filename);
		errorlog(G.errmsg);
	}

	sprintf(filename,"%s/%s/.drealmrc",C.users,id);
	sprintf(temp,"cp %s/config.user %s",C.configdir,filename);
	if (!dsystem(temp)) {
		chmod(filename,0770);
	} else {
		sprintf(G.errmsg,"CONFIG ERROR:During user creation failed to write %s",filename);
		errorlog(G.errmsg);
	}
	return 1;
}


int make_acc_dirs (char *filename) {
	struct stat sb;
	char temp[MAINLINE];
	
	if (mkdir(filename,0770)) {
		if (errno != EEXIST) {
			sprintf(G.errmsg,"CONFIG ERROR:During BBS account creation, could neither get to nor make %s",filename);
			/*sprintf(temp,Ustring[68],filename);*/
			errorlog(temp);
			return 0;
		}
		/*IT EXISTS BUT WE DIDN'T MAKE IT*/

		if (stat(filename,&sb)) {
			sprintf(G.errmsg,"CONFIG ERROR:During BBS account creation, could neither get to nor make %s",filename);
			errorlog(G.errmsg);
			return 0;
		} else {
			/*if it's group BBS and group can r/w/x it) or (its not group bbs and other can r/w/x it*/
			if (!(( (sb.st_gid == C.group) 
			&& (((sb.st_mode) & S_IRGRP) == S_IRGRP) 
			&& (((sb.st_mode) & S_IWGRP) == S_IWGRP) 
			&& (((sb.st_mode) & S_IXGRP) == S_IXGRP) )
			|| ( (((sb.st_mode) & S_IROTH) == S_IROTH) 
			&& (((sb.st_mode) & S_IWOTH) == S_IWOTH) 
			&& (((sb.st_mode) & S_IXOTH) == S_IXOTH) ))) {
				sprintf(G.errmsg,"CONFIG ERROR:During BBS account creation, %s found to have unsuitable permissions for BBS use.",filename);
				errorlog(G.errmsg);
				return 0;
			}
		}
	}
	return 1;
}


int zap_user (char *in) {
/* MENU COMMAND */
	char command[MAINLINE + MAINLINE];
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	sprintf(command,"zapuser %s %s",user,C.tmpdir);
	result = !dsystem(command);	
	if (result) {
		/*printf("Unable to kill user's session.\n");*/
		printf(Ustring[366],user);
		printf("\n");
	}
	return result;
}
	

int kill_user (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);	

	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	result = destroy_bbs_account(user);
	if (result) {
		FILE *LOG;
		char filename[MAINLINE + 100];
		char *date = shorttime(time(0));

		sprintf(filename,"%s/newuserlog",C.datadir);
		if ( (LOG=fopen(filename,"a")) ) {
			fprintf(LOG,"REMOVED: %s %s\n",date,user);
			fclose(LOG);
		}
		free(date);
	}
	return result;
}

int destroy_bbs_account (char *id) {
	char filename[MAINLINE + 100];
	char homedir[MAINLINE + 100];
	char command[MAINLINE * 2];
	struct passwd *pword;


	if ((pword = getpwnam(id))) {
		sprintf(homedir,"%s",pword->pw_dir);
	} else {
		homedir[0] = 0;
	}

	sprintf(filename,"%s/%s",C.users,id);
	if (strcmp(homedir,filename)) {
		sprintf(command,"rm -rf %s",filename);
		dsystem(command);
	}
	sprintf(filename,"%s/%s",C.privatefiles,id);
	if (strcmp(homedir,filename)) {
		sprintf(command,"rm -rf %s",filename);
		dsystem(command);
	}
	sprintf(filename,"%s/%s/.mail",C.maildirs,id);
	if (strcmp(homedir,filename)) {
		sprintf(command,"rm -rf %s",filename);
		dsystem(command);
	}

	sprintf(filename,"%s/%s",C.homedirs,id);	
	if (strcmp(homedir,filename)) {
	/* Don't get rid of real home directory, he'll either be needing it */
	/* or the system command to kill users will do it */

		sprintf(command,"rm -rf %s",filename);
		dsystem(command);
	}

	if (!is_shell_account(id)) { /*if not normal shell user */
		if (!destroy_unix_account(id)) {
			sprintf(G.errmsg,"Unable to remove %s's unix account.\n",id);
			errorlog(G.errmsg);
			/*printf("Unable to remove %s's unix account.\n",id);*/
			printf(Ustring[268],Ustring[367]);
			printf("\n");
		}
	} else {
		printf("%s is a shell user, Unix facilities retained.\n",id);
		printf(Ustring[382],id);
		printf("\n");
	}	
	return 1;
}




int make_unix_account (char *id,char *realname) {
	char temp[MAINLINE * 2];
	int result = 0;

	if (is_in_passwd(id)) {
		return 1;
	}
	sprintf(temp,"adduser %s %s '%s' %d %s '%s'",C.homedirs,C.bbsshell,realname,C.public,id,"You must choose a password for this account");
	external_term();
	result = !dsystem(temp);
	internal_term();
	return result;
}



int destroy_unix_account (char *id) {
	char temp[MAINLINE * 2];

	if (!is_in_passwd(id)) {
		return 1;
	}
	sprintf(temp,"remuser %s",id);
	return !dsystem(temp);
}


/*================================================================*/
/* DREALMRC stuff */


int firstset (void) {
/* reads things from the main config.user */
	char filename[MAINLINE + 100];
	int i;
	int rc;

	U.level = 0;

	strcpy(G.envtermstring,"TERM=");
	putenv(G.envtermstring);
	strcpy(G.envlinestring,"LINES=24");
	putenv(G.envlinestring);
	strcpy(G.envcolstring,"COLUMNS=80");
	putenv(G.envcolstring);

	U.flags[0] = '#';
	for (i=1;i<=UFLAGMAX;i++) {
		U.flags[i] = '0';
	}
	U.flags[UFLAGMAX + 1] = 0;

	U.expiry = (time_t)0;
	U.firstcall = (time_t)0;
	U.lastcall = (time_t)0;
	U.totalcalls = 0;
	U.totalmessages = 0;
	U.mailreserves = 0;

	sprintf(filename,"%s/config.user",C.configdir);
	rc = drealmrc_read(filename, &U, 1);
	if (rc) {
		sprintf(G.envtermstring,"TERM=%s",U.envterm);
		sprintf(G.envlinestring,"LINES=%d",U.rows);
		sprintf(G.envcolstring,"COLUMNS=%d",U.cols);
	}
	return rc;
}


int update (char *dummy) {
/* MENU COMMAND */
/* reads main U_things out of user's own drealmrc */
	char filename[MAINLINE + 100];
	int rc;

	sprintf(filename,"%s/%s/.updatealert",C.users,U.id);
	remove(filename);

	U.level = level_read(U.id);
	U.expiry = expiry_read(U.id);
	U.firstcall = firstcall_read(U.id);
	U.lastcall = lastcall_read(U.id);
	U.totalcalls = totalcalls_read(U.id);
	U.totalmessages = totalmessages_read(U.id);
	U.mailreserves = mailreserves_read(U.id);

	flags_read(U.id,U.flags);

	rc = defaults_read (U.id,&U);
	if (!rc) {
		defaults_write(U.id,&U);
	}
	sprintf(G.envtermstring,"TERM=%s",U.envterm);
	sprintf(G.envlinestring,"LINES=%d",U.rows);
	sprintf(G.envcolstring,"COLUMNS=%d",U.cols);
	(void)get_LW(1);

#if defined(CHAT_COMMANDS)
	if (G.chat && G.chatenabled) {
		sprintf(filename,"-%d",(U.chatcolour + 20));
		chatin(filename,"");
	}
#endif
	return 1;
}

int defaults_read (char *user, struct uservars *userbuf) {
/* reads anyone's .drealmrc into any buffer */

	char filename[MAINLINE + 100];
	int result = 0;

	sprintf(filename,"%s/%s/.drealmrc",C.users,user);
	result = drealmrc_read(filename,userbuf,0);
	return result;
}	

int drealmrc_read (char *filename, struct uservars *userbuf, int firstread) {
/* puts stuff into ready_prepared buffers but could be for anyone */
/* OR, more importantly, for any file! */
/* 'firstread' is true if reading from config.user */

	char result[MAINLINE];
	int i;
	int totalshift = 0;
	FILE *CFG;


	if (!(CFG = fopen(filename,"r"))) {
		sprintf(G.errmsg,"Could not read %s",filename);
		errorlog(G.errmsg);
		return 0;
	}

	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if ((sscanf(result, " %d ", &userbuf->hotkeys) != 1) || ((userbuf->hotkeys != 0) && (userbuf->hotkeys != 1))) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for hotkeys in %s",filename);
			errorlog(G.errmsg);
			userbuf->hotkeys = 0;
		} else {
			userbuf->hotkeys = D.hotkeys;
		}
	}
	if (firstread) {
		D.hotkeys = userbuf->hotkeys;
	}	
		

	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	tnt(result);
	if ((int)strlen(result) > (MAINLINE - 1)) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for display in %s",filename);
			errorlog(G.errmsg);
			strcpy(userbuf->display,"");
		} else {
			strcpy(userbuf->display,D.display);
		}
	} else {
		strcpy(userbuf->display,result);
	}
	if (firstread) {
		strcpy(D.display,userbuf->display);
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	tnt(result);
	if ((int)strlen(result) > (MAINLINE - 1)) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for displayname in %s",filename);
			errorlog(G.errmsg);
			strcpy(userbuf->displayname,"");
		} else {
			strcpy(userbuf->displayname,D.displayname);
		}
	} else {
		strcpy(userbuf->displayname,result);
	}
	if (firstread) {
		strcpy(D.displayname,userbuf->displayname);
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	tnt(result);
	if ((int)strlen(result) > (MAINLINE - 1)) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for editor in %s",filename);
			errorlog(G.errmsg);
			strcpy(userbuf->editor,"");
		} else {
			strcpy(userbuf->editor,D.editor);
		}
	} else {
		strcpy(userbuf->editor,result);
	}
	if (firstread) {
		strcpy(D.editor,userbuf->editor);
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	tnt(result);
	if ((int)strlen(result) > (MAINLINE - 1)) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for editorname in %s",filename);
			errorlog(G.errmsg);
			strcpy(userbuf->editorname,"");
		} else {
			strcpy(userbuf->editorname,D.editor);
		}
	} else {
		strcpy(userbuf->editorname,result);
	}
	if (firstread) {
		strcpy(D.editorname,userbuf->editorname);
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	tnt(result);
	if (strlen(result) > 20) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for termtype in %s",filename);
			errorlog(G.errmsg);
			strcpy(userbuf->envterm,"dumb");
		} else {
			strcpy(userbuf->envterm,D.envterm);
		}
	} else {
		strcpy(userbuf->envterm,result);
	}
	if (firstread) {
		strcpy(D.envterm,userbuf->envterm);
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if (sscanf(result, " %d ", &userbuf->chatcolour) != 1) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for chatcolour in %s",filename);
			errorlog(G.errmsg);
			userbuf->chatcolour = 10;
		} else {
			userbuf->chatcolour = D.chatcolour;
		}
	}
	if (firstread) {
		D.chatcolour = userbuf->chatcolour;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if (sscanf(result, " %d ", &userbuf->cols) != 1) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for columns in %s",filename);
			errorlog(G.errmsg);
			userbuf->cols = 80;
		} else {
			userbuf->cols = D.cols;
		}
	}
	if (firstread) {
		D.cols = userbuf->cols;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if (sscanf(result, " %d ", &userbuf->rows) != 1) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for rows in %s",filename);
			errorlog(G.errmsg);
			userbuf->rows = 24;
		} else {
			userbuf->rows = D.cols;
		}
	}
	if (firstread) {
		D.rows = userbuf->rows;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if ((sscanf(result, " %d ", &i) != 1) || ((i < 0) || (i > 255))) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for erase in %s",filename);
			errorlog(G.errmsg);
			userbuf->erase = 8;
		} else {
			userbuf->erase = D.erase;
		}
	}
	userbuf->erase = (char)i;
	if (firstread) {
		D.erase = userbuf->erase;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if ((sscanf(result, " %d ", &i) != 1) || ((i < 0) || (i > 255))) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for werase in %s",filename);
			errorlog(G.errmsg);
			userbuf->werase = 23;
		} else {
			userbuf->werase = D.werase;
		}
	}
	userbuf->werase = (char)i;
	if (firstread) {
		D.werase = userbuf->werase;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if ((sscanf(result, " %d ", &i) != 1) || ((i < 0) || (i > 255))) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for kill in %s",filename);
			errorlog(G.errmsg);
			userbuf->kill = 21;
		} else {
			userbuf->kill = D.kill;
		}
	}
	userbuf->kill = (char)i;
	if (firstread) {
		D.kill = userbuf->kill;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if ((sscanf(result, " %d ", &i) != 1) || ((i < 0) || (i > 255))) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for reprint in %s",filename);
			errorlog(G.errmsg);
			userbuf->reprint = 18;
		} else {
			userbuf->reprint = D.reprint;
		}
	}
	userbuf->reprint = (char)i;
	if (firstread) {
		D.reprint = userbuf->reprint;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if (sscanf(result, " %d ", &userbuf->recent) != 1) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for recent in %s",filename);
			errorlog(G.errmsg);
			userbuf->recent = 0;
		} else {
			userbuf->recent = D.recent;
		}
	}
	if (firstread) {
		D.recent = userbuf->recent;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	tnt(result);
	if (strlen(result) > 20) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for readmode in %s",filename);
			errorlog(G.errmsg);
			strcpy(userbuf->readmode,"NUMERIC");
		} else {
			strcpy(userbuf->readmode,D.readmode);
		}
	} else {
		strcpy(userbuf->readmode,result);
	}
	if (firstread) {
		strcpy(D.readmode,userbuf->readmode);
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if ((sscanf(result, " %d ", &userbuf->readown) != 1) || ((userbuf->readown != 0) && (userbuf->readown != 1))) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for readown in %s",filename);
			errorlog(G.errmsg);
			userbuf->readown = 1;
		} else {
			userbuf->readown = D.readown;
		}
	}
	if (firstread) {
		D.readown = userbuf->readown;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if (sscanf(result, " %d ", &userbuf->pausetime) != 1) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for pausetime in %s",filename);
			errorlog(G.errmsg);
			userbuf->pausetime = 0;
		} else {
			userbuf->pausetime = D.pausetime;
		}
	}
	if (firstread) {
		D.pausetime = userbuf->pausetime;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if (sscanf(result, " %d ", &userbuf->timeout) != 1) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for timeout in %s",filename);
			errorlog(G.errmsg);
			userbuf->timeout = 0;
		} else {
			userbuf->timeout = D.timeout;
		}
	}
	if (firstread) {
		D.timeout = userbuf->timeout;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if ((sscanf(result, " %d ", &userbuf->chat) != 1) || ((userbuf->chat != 0) && (userbuf->chat != 1))) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for chat in %s",filename);
			errorlog(G.errmsg);
			userbuf->chat = 0;
		} else {
			userbuf->chat = D.chat;
		}
	}
	if (firstread) {
		D.chat = userbuf->chat;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if (sscanf(result, " %d ", &userbuf->chatsendcolour) != 1) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for chatsendcolour in %s",filename);
			errorlog(G.errmsg);
			userbuf->chatsendcolour = 10;
		} else {
			userbuf->chatsendcolour = D.chatsendcolour;
		}
	}
	if (firstread) {
		D.chatsendcolour = userbuf->chatsendcolour;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	tnt(result);
	if (strlen(result) > 10) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for language in %s",filename);
			errorlog(G.errmsg);
			strcpy(userbuf->language,"txt");
		} else {
			strcpy(userbuf->language,D.language);
		}
	} else {
		strcpy(userbuf->language,result);
	}
	if (firstread) {
		strcpy(D.language,userbuf->language);
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	tnt(result);
	if (strlen(result) < 1) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for languagename in %s",filename);
			errorlog(G.errmsg);
			strcpy(userbuf->languagename,"Default");
		} else {
			strcpy(userbuf->languagename,D.languagename);
		}
	} else {
		strcpy(userbuf->languagename,result);
	}
	if (firstread) {
		strcpy(D.languagename,userbuf->languagename);
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if (sscanf(result, " %d ", &userbuf->spare3) != 1) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for spare3 in %s",filename);
			errorlog(G.errmsg);
			userbuf->spare3 = 0;
		} else {
			userbuf->spare3 = D.spare3;
		}
	}
	if (firstread) {
		D.spare3 = userbuf->spare3;
	}	


	get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
	if (sscanf(result, " %d ", &userbuf->spare4) != 1) {
		if (firstread) {
			sprintf(G.errmsg,"Invalid value for spare4 in %s",filename);
			errorlog(G.errmsg);
			userbuf->spare4 = 0;
		} else {
			userbuf->spare4 = D.spare4;
		}
	}
	if (firstread) {
		D.spare4 = userbuf->spare4;
	}	


	fclose(CFG);
	return 1;
}


int defaults_write (char *user, struct uservars *userbuf) {
	char filename[MAINLINE + 100];
	FILE *CFG;
	int result = 0;
	
	sprintf(filename,"%s/%s/.drealmrc",C.users,user);
	if ((CFG = fopen(filename,"w"))) {
		result = drealmrc_write(CFG,userbuf);
		fclose(CFG);
		return result;
	} else {
		return 0;
	}
}	


int drealmrc_write (FILE *CFG, struct uservars *userbuf) {

	fprintf(CFG, "::%d::\n",userbuf->hotkeys);
	fprintf(CFG, "::%s::\n",userbuf->display);
	fprintf(CFG, "::%s::\n",userbuf->displayname);
	fprintf(CFG, "::%s::\n",userbuf->editor);
	fprintf(CFG, "::%s::\n",userbuf->editorname);
	fprintf(CFG, "::%s::\n",userbuf->envterm);
	fprintf(CFG, "::%d::\n",userbuf->chatcolour);
	fprintf(CFG, "::%d::\n",userbuf->cols);
	fprintf(CFG, "::%d::\n",userbuf->rows);
	fprintf(CFG, "::%d::\n",userbuf->erase);
	fprintf(CFG, "::%d::\n",userbuf->werase);
	fprintf(CFG, "::%d::\n",userbuf->kill);
	fprintf(CFG, "::%d::\n",userbuf->reprint);
	fprintf(CFG, "::%d::\n",userbuf->recent);
	fprintf(CFG, "::%s::\n",userbuf->readmode);
	fprintf(CFG, "::%d::\n",userbuf->readown);
	fprintf(CFG, "::%d::\n",userbuf->pausetime);
	fprintf(CFG, "::%d::\n",userbuf->timeout);
	fprintf(CFG, "::%d::\n",userbuf->chat);
	fprintf(CFG, "::%d::\n",userbuf->chatsendcolour);
	fprintf(CFG, "::%s::\n",userbuf->language);
	fprintf(CFG, "::%s::\n",userbuf->languagename);
	fprintf(CFG, "::%d::\n",userbuf->spare3);
	fprintf(CFG, "::%d::\n",userbuf->spare4);

	return 1;
}

int resetmy_userdefaults (char *dummy) {
/* MENU COMMAND */
	int result = 0;
	
	if ((result = reset_userdefaults(U.id))) {
		update("");	
	}
	return result;
}

int resethis_userdefaults (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);	

	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	result = reset_userdefaults(user);
	if (result) {
		update_force(user);	
	}
	return result;
}




int reset_userdefaults (char *id) {
	char temp[MAINLINE * 2];
	char filename[MAINLINE + 100];

	sprintf(temp,"%s/config.user",C.configdir);
	sprintf(filename,"%s/%s/.drealmrc",C.users,id);
	copy_file(temp,filename,0);
	chmod(filename,0770);
	return 1;
}


void init_ustrings (void) {
	int i = 0;
	
	ustrings[0] = 0;
	for (i=0;i<=USTRINGCOUNT;i++) {
		Ustring[i] = ustrings;
	}
	G.bigyes = 'Y';
	G.littleyes = 'y';
	G.bigno = 'N';
	G.littleno = 'n';
	G.bigquit = 'Q';
	G.littlequit = 'q';
	sprintf(G.yesno,"%c%c%c%c",G.bigyes,G.littleyes,G.bigno,G.littleno);
}

void init_mstrings (void) {
	int i = 0;
	
	mstrings[0] = 0;
	for (i=0;i<=MSTRINGCOUNT;i++) {
		Mstring[i] = mstrings;
	}
}


void whichstrings (void) {
	char filename[MAINLINE + 100]; 
	
	if (whichlangfile("ustrings",filename) && stringsread(filename,USTRINGCOUNT,USTRINGSIZE,'u')) {
		tnt(Ustring[56]);
		tnt(Ustring[58]);
		G.bigyes = toupper(Ustring[56][0]);
		G.littleyes = tolower(Ustring[56][0]);
		G.bigno = toupper(Ustring[58][0]);
		G.littleno = tolower(Ustring[58][0]);
		G.bigquit = toupper(Ustring[459][0]);
		G.littlequit = tolower(Ustring[459][0]);
		sprintf(G.yesno,"%c%c%c%c",G.bigyes,G.littleyes,G.bigno,G.littleno);
	} else {
		printf("Cannot find a valid ustrings language file.\n");
		init_ustrings();
	}

	if (!MSTRINGCOUNT) {
		return;
	}
	if (whichlangfile("mstrings",filename) && stringsread(filename,MSTRINGCOUNT,MSTRINGSIZE,'m')) {
		/*EMPTY*/
	} else {
		printf("Cannot find a valid mstrings language file.\n");
		init_mstrings();
	}
}	


int stringsread (char *filename,int stringcount,int stringsize,char type) {
	FILE *FIL;
	int i;
	char buffer[MAINLINE];
	int firstfree = 0;


	if (!filename[0]) {
		return 0;
	}
	
	if (!stringcount || !stringsize) {
		return 0;
	}
	
	if (!(FIL = fopen(filename,"r"))) {
		return 0;
	}
	
	i = 1;
	while (i <= stringcount) {
		buffer[0] = '#';
		while (buffer[0] == '#') {
			buffer[0] = 0;
			if (!(fgets(buffer,MAINLINE,FIL))) {
				fclose(FIL);
				return 0;
			}
		}
			
		buffer[strlen(buffer) - 1] = 0;
		
		if ((firstfree + strlen(buffer) + 1) < stringsize) {
			if (type == 'u') {
				strcpy(&ustrings[firstfree],buffer);
				Ustring[i] = &ustrings[firstfree];
			} else if (type == 'm') {
				strcpy(&mstrings[firstfree],buffer);
				Mstring[i] = &mstrings[firstfree];
			}
			firstfree += strlen(buffer) + 1;
			i++;
		} else {
			sprintf(buffer,"%s too big for requested STRINGSIZE",filename);
			errorlog(buffer);
			fclose(FIL);
			return 0;
		}
		
	}
	fclose(FIL);
	return 1;
}


int setmy_lang (char *in) {
/* MENU COMMAND */
	int result;

	if ((result = set_lang(U.id))) {
		update("");
		whichstrings();
	}
	flushcom("");
	return result;
}

int sethis_lang (char *in) {
/* MENU COMMAND */
	char user[9];
	int result = 0;
	char userstring[21];
	char *copy = strdup(in);
	
	shiftword(copy,userstring,21);
	free(copy);
	if (!userstring[0]) {
		shiftword(G.comline,userstring,21);
	}
	if (!ask_for_user(userstring,user)) {
		return 0;
	}		
	
	if ((result = set_lang(user))) {
		/*printf("Setting will take effect next time user logs in.\n");*/
		/*printf("%s\n",Ustring[494]);*/
		update_force(user);
	}
	return result;
}

int set_lang (char *user) {
	struct uservars u;
	FILE *FIL;
	char ext[20][MAINLINE];
	char name[20][MAINLINE];
	char temp[MAINLINE];
	char filename[MAINLINE + 100];
	char field[MAINLINE];
	int choice;
	int result = 0;
	int i;
	int totalshift;

	if (defaults_read(user,&u)) {

		sprintf(filename,"%s/config.langs",C.configdir);
		if ((FIL = fopen(filename,"r"))) {

			i = 0;
			totalshift = 0;
			while (i<MAXLANGS) {
	
				get_next_cfgfield (FIL,&totalshift,field,MAINLINE);
				tnt(field);
				if (!field[0]) { break;
				}
				strcpy(ext[i],field);
	
	
				get_next_cfgfield (FIL,&totalshift,field,MAINLINE);
				tnt(field);
				strcpy(name[i],field);

				i++;
			}
		
			ext[i][0] = 0;
			name[i][0] = 0;
			fclose(FIL);	
		} else {
			/*printf("No other languages to choose from.\n");*/
			printf("%s\n",Ustring[493]);
			return 1;
		}
	
		/* CONSTCOND */
		while (1) {
			for (i=0;ext[i][0];i++) {
				if (ext[i][0]) {
					printf("[%d] %s\n",i+1,name[i]);
				}
			}
			/*printf("[%d] quit\n",i+1);*/
			printf("[%d] %s\n\n",i+1,Ustring[63]);
			/*make_prompt("\nWhich language? ");*/
			sprintf(temp,Ustring[393],Ustring[492]);
			make_prompt(temp);

			choice = get_one_num(1,0);
			if ((choice == 0) || (choice == i+1)) {
				return 1;
			}
			choice--; /*because the menu shows all the  numbers up one*/
			if ((choice < i) && (ext[choice][0])) {
				strcpy(u.language,ext[choice]);
				strcpy(u.languagename,name[choice]);
				/*("Language set to %s.\n",name[choice]);*/
				printf(Ustring[387],Ustring[492],name[choice]);
				printf("\n");
				break;
			} else {
				/*printf("'%d' invalid.\n",choice + 1);*/
				sprintf(temp,"%d",choice + 1);
				printf(Ustring[472],temp);
				printf("\n");
			}
		}

		if (defaults_write(user,&u)) {
			result = 1;
		} else {
			/*printf("Unable to write new defaults - abandoned.\n");*/
			printf(Ustring[67],Ustring[344]);
			printf(" - %s\n",Ustring[60]);
		}
	} else {
		/*printf("Unable to read current defaults - abandoned.\n");*/
		printf(Ustring[66],Ustring[344]);
		printf(" - %s\n",Ustring[60]);
	}
	return result;
}





