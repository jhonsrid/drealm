
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
/* ANSI headers */
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Non-ANSI headers */
#if defined(SVR42)
#  define _POSIX_SOURCE
#endif
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/stat.h>
#if defined(SVR42)
#  include <libgen.h>
#else
#  include <regex.h>
#endif

/* Local headers */
#include "drealm.h"
#include "mainfuncs.h"
#include "drealmgen.h"
#include "inputfuncs.h"
#include "configfuncs.h"
#include "setupfuncs.h"
#include "genfuncs.h"
#include "display.h"
#include "getvalf.h"
#include "sendmail.h"
#include "sendmess.h"

#if defined(READ_COMMANDS)
#  include "readfuncs.h"
#endif

#include "filefuncs.h"


/* ARGSUSED0 */
int cleardir (char *dummy) {
/* MENU COMMAND */
/* CHECKED */
/* for clearing out ALL directory information */
	G.topdir[0] = 0;
	G.taildir[0] = 0;
	G.dir[0] = 0;
	G.backdir[0] = 0;
	G.backtop[0] = 0;
	return 1;
}

/* ARGSUSED0 */
int pushpfu (char *dummy) {
/* MENU COMMAND */
/* Relies on pre-determined arbitrary lengths */
	char temp[MAINLINE + 100];

	sprintf(temp,"%s/%s",C.privatefiles,U.id);
	if (strcmp(G.topdir,temp)) {
		strcpy(G.backdir,G.dir);
		strcpy(G.backtop,G.topdir);
		strcpy(G.backtail,G.taildir);
		sprintf(G.dir,"%s/%s",C.privatefiles,U.id);   
		sprintf(G.topdir,"%s/%s",C.privatefiles,U.id);
		G.taildir[0] = 0;
	}
	return 1;
}

/* ARGSUSED0 */
int poppfu (char *dummy) {
/* MENU COMMAND */
	char temp[MAINLINE + 100];

	sprintf(temp,"%s/%s",C.privatefiles,U.id);
	if (!strcmp(G.topdir,temp)) {
		strcpy(G.dir,G.backdir);
		strcpy(G.topdir,G.backtop);
		strcpy(G.taildir,G.backtail);
		G.backdir[0] = 0;
		G.backtop[0] = 0;
		G.backtail[0] = 0;
	}
	return 1;
}

int is_dir_elig (char mode, char *params) {
	DIR *DIRT;
	char *copy = strdup(params);
		
	tnt(copy);
	if ((!copy[0]) || (copy[0] != '/')) {
		strcpy(G.errmsg,"Directory incorrectly specified.");
		errorlog(G.errmsg);
		if (mode == 'v') {
			/*printf("Directory not available.\n");*/
			printf("%s\n",Ustring[245]);
		}
		free(copy);
		return 0;
	}

	DIRT = opendir(copy);
	if (!DIRT) {
		if (mode == 'v') {
			/*printf("Directory not available.\n");*/
			printf("%s\n",Ustring[245]);
		}
		free(copy);
		return 0;
	}
	closedir(DIRT);
	free(copy);
	return 1;
}

int startdir (char *params) {
/* MENU COMMAND */
/* params in from menu only */
/* sets the top directory for CURRENT dirpointer */

	char *copy = strdup(params);

	tnt(copy);
	if (!copy[0]) {
		cleardir("");
	}
	if ((strlen(G.dir) + strlen(copy)) > (size_t)(MAINLINE)) {
		/*printf("Path too long - chdir abandoned.\n");*/
		printf("%s - %s\n",Ustring[246],Ustring[60]);
		free(copy);
		return 0;
	}

	if (is_dir_elig('v',copy)) {
		strcpy(G.topdir,copy);
		strcpy(G.dir,copy);
		G.taildir[0] = 0;
		free(copy);
		return 1;
	} else {
		free(copy);
		return 0;
	}
}

int nestdir (char *in) {
/* MENU COMMAND */
/* allows users to choose from among the children */
	char temp[MAINLINE + 80];
	struct valid_files *vf;
	char *copy;
	
	if (!is_dir_elig('v',G.dir)) {
		return 0;
	}

	copy = strdup(in);
	shiftword(copy,temp,MAINLINE);
	free(copy);
	if (!temp[0]) {
		shiftword(G.comline,temp,MAINLINE);
	}

	/*vf = get_valid_dirs('v',1,"directory",G.dir,temp,C.filesensitive);*/
	vf = get_valid_dirs('v',1,Ustring[247],G.dir,temp,C.filesensitive);
	if (!vf->files[0]) {
		free(vf->input);
		free(vf->files);
		free(vf);
		return 0;
	}

	if ((strlen(G.dir) + strlen(vf->files)) > (size_t)(MAINLINE)) {
		/*printf("Path too long - chdir abandoned.\n");*/
		printf("%s - %s\n",Ustring[246],Ustring[60]);
		free(vf->input);
		free(vf->files);
		free(vf);
		return 0;
	}

	sprintf(temp,"%s/%s",G.dir,vf->files);

	if (!is_dir_elig('v',temp)) {
		/*printf("Unable to access %s - chdir abandoned.\n",temp);*/
		printf("%s\n",Ustring[245]);
		free(vf->input);
		free(vf->files);
		free(vf);
		return 0;
	}
	strcpy(G.dir,temp);
	if (G.taildir[0]) {
		strcpy(temp,G.taildir);
		sprintf(G.taildir,"%s/%s",temp,vf->files);
	} else {
		sprintf(G.taildir,"%s",vf->files);
	}
	free(vf->input);
	free(vf->files);
	free(vf);
	return 1;
}

/* ARGSUSED0 */
int parentdir (char *dummy) {
/* MENU COMMAND */
/* May only go back as far as the beginning of the startdir */
	char *temp;

	if (!strcmp(G.dir,G.topdir)) {
		/*printf("Parent directory not available - chdir abandoned.\n");*/
		printf("%s\n",Ustring[245]);
		return 0;
	}

	temp = dir_name(G.dir,'/');
	if (temp[0] && is_dir_elig('v',temp)) {
		strcpy(G.dir,temp);
		temp = dir_name(G.taildir,'/');
		strcpy(G.taildir,temp);
	} else {
		/*printf("Parent directory not available - chdir abandoned.\n");*/
		printf("%s\n",Ustring[245]);
		free(temp);
		return 0;
	}
	free(temp);
	return 1;
}

int list_dir(char *in) {
/* MENU COMMAND */
/* Lists only G.dir, defaults to short */
	char list_type[2];
	char command[MAINLINE + 100];
	char *copy;
	int result;

	if (!is_dir_elig('v',G.dir)) {
		return 0;
	}

	copy = strdup(in);	
	shiftword(copy,list_type,2);
	free(copy);

	if (!list_type[0]) {
		shiftword(G.comline,list_type,2);
	}

	switch (list_type[0]) {
		case 'l':
			sprintf(command,"ls -lLp %s > view",G.dir);
			break;
		case 'x':
			sprintf(command,"ls -xLp %s > view",G.dir);
			break;
		case 'c':
			sprintf(command,"ls -CLp %s > view",G.dir);
			break;
		default:
			sprintf(command,"ls -Lp %s > view",G.dir);
			break;
	}
	
	result = dsystem(command);
	if (result) {
		/*printf("Directory is empty.\n");*/
		printf(Ustring[313],Ustring[247]);
		printf("\n");
		return 0;
	}
	return display("view");
}

#if 0
int take_files (char *in) {
	struct valid_files *vf;

	if (!is_dir_elig('v',G.dir)) {
		return 0;
	}

	free(G.files);
	
	tnt(in);	
	if (in[0]) {
		/*vf = get_valid_files('v',0,"files",G.dir,in,1);*/
		vf = get_valid_files('v',0,Ustring[266],G.dir,in,1);
	} else {
		vf = get_valid_files('v',0,Ustring[266],G.dir,G.comline,1);
		flushcom("");
	}

	G.files = vf->files;
	free(vf->input);
	free(vf);
}
#endif


int file_to_user (char *in) {
/* MENU COMMAND */
/* will take recipient from menuline if required, if so the syntax is */
/* ftu recipient file_list */
	struct valid_files *vfa; /* the files being transferred */
	struct valid_files *vfb; /* the receiving directory */
	char topd[MAINLINE + 100]; /* parentdir of dir where it's going */
	char childdir[MAINLINE + 100];  /* the receiving directory input checking */
	char src_file[MAINLINE * 2];
	char dest_file[MAINLINE * 2];
	char temp[MAINLINE * 2];
	char tempa[MAINLINE + 100];
	char flags[UFLAGMAX + 2];
	int  from_pfu;
	struct stat statbuf;
	char *copy;

	if (!is_dir_elig('v',G.dir)) {
		return 0;
	}
	strcpy(topd,C.privatefiles);

	copy = strdup(in);
	sprintf(temp,"%s/%s",C.privatefiles,U.id);
	from_pfu = !strcmp(G.dir,temp); /* yes or no, is it from your pfu? */
	shiftword(copy,childdir,MAINLINE);
	strshift(copy,temp,2*MAINLINE,Ustring[501]);
	free(copy);

	tnt(temp);
	if (!temp[0]) {
		strshift(G.comline,temp,2*MAINLINE,Ustring[501]);
		tnt(temp);
	}

	
	/*vfa = get_valid_files('v',0,"files",G.dir,temp,1);*/
	vfa = get_valid_files('v',0,Ustring[266],G.dir,temp,1);
	if (!vfa->files[0]) {
		free(vfa->input);
		free(vfa->files);
		free(vfa);
		return 0;
	}

	if (!childdir[0]) {
		shiftword(G.comline,childdir,MAINLINE);
	}

	/*vfb = get_valid_dirs('v',1,"user",topd,childdir,0);*/
	vfb = get_valid_dirs('v',1,Ustring[196],topd,childdir,0);
	if (!vfb->files[0]) {
		free(vfb->input);
		free(vfb->files); /*recipient*/
		free(vfb);
		free(vfa->input);
		free(vfa->files); /*fromfiles*/
		free(vfa);

		return 0;
	}

	/*now we have something to do and somewhere to put it BUT */

	if (level_read(vfb->files) < C.pvtfileslevel) {
		/*printf("%s does not have private file facilities.\n",vfb->files);*/
		printf(Ustring[250],vfb->files,Ustring[251]);
		printf("\n");
		free(vfb->input);
		free(vfb->files); /*recipient*/
		free(vfb);
		free(vfa->input);
		free(vfa->files); /*fromfiles*/
		free(vfa);
		return 0;
	}
	if (flags_read(vfb->files,flags) && !comp_flags(C.pvtfilesmask,flags)) {
		/*printf("%s does not have private file facilities.\n",vfb->files);*/
		printf(Ustring[250],vfb->files,Ustring[251]);
		printf("\n");
		free(vfb->input);
		free(vfb->files); /*recipient*/
		free(vfb);
		free(vfa->input);
		free(vfa->files); /*fromfiles*/
		free(vfa);
		return 0;
	}
	while (vfa->files[0]) {
		shiftword(vfa->files,temp,MAINLINE);
		sprintf(src_file,"%s/%s",G.dir,temp);
		sprintf(dest_file,"%s/%s/%s",topd,vfb->files,temp);
		if (stat(dest_file,&statbuf)) {
			sprintf(tempa,"cp -p %s %s",src_file,dest_file);
			dsystem(tempa);
			chmod(dest_file,0660);
			if ((C.canchown) && (!strcmp(C.privatefiles,C.homedirs))) {
				sprintf(tempa,"chown %s %s",vfb->files,dest_file);
				dsystem(tempa);
			}
			if (from_pfu) {
				/*sprintf(tempa,"\nDelete original copy of %s? Y/n ",temp);*/
				sprintf(tempa,Ustring[252],temp);
				if (yes_no(tempa)) {
					remove(src_file);
				}
			}
		} else {
			/*printf("%s already has a file called %s. Not posted.\n",vfb->files,temp);*/
			printf(Ustring[253],vfb->files,temp);
			printf(" - %s\n",Ustring[60]);
		}
	}
	free(vfb->input);
	free(vfb->files); /*recipient*/
	free(vfb);
	free(vfa->input);
	free(vfa->files); /*fromfiles*/
	free(vfa);
	return 1;
}

int file_to_public (char *in) {
/* MENU COMMAND */
/* Will take receiving area from 2nd param on menuline if required */
/* Can come from pfu or other public area */
/* Can obviously use this to send things to users as well, but it won't check levels then */
/* In params should be the parent directory which holds the directories between
   which the users can choose between to send their files to
 */
	struct valid_files *vfa; /*files being sent*/
	struct valid_files *vfb; /*recipient*/
	char src_file[MAINLINE * 2];
	char dest_file[MAINLINE * 2];
	char temp[MAINLINE * 2];
	char tempa[MAINLINE + 100];
	char file[MAINLINE * 2];
	char topd[MAINLINE + 100];
	char childdir[MAINLINE + 100];
	int  from_pfu;
	struct stat statbuf;
	char *copy;

	if (!is_dir_elig('v',G.dir)) {
		return 0;
	}

	copy = strdup(in);
	sprintf(temp,"%s/%s",C.privatefiles,U.id);
	from_pfu = !strcmp(G.dir,temp);
	shiftword(copy,topd,MAINLINE);
	if (!is_dir_elig('v',topd)) {
		free(copy);
		return 0;
	}
	shiftword(copy,childdir,MAINLINE);
	strshift(copy,temp,2*MAINLINE,Ustring[501]);

	tnt(temp);
	if (!temp[0]) {
		strshift(G.comline,temp,2*MAINLINE,Ustring[501]);
		tnt(temp);
	}
	free(copy);

	/*vfa = get_valid_files('v',0,"files",G.dir,temp,1);*/
	vfa = get_valid_files('v',0,Ustring[266],G.dir,temp,1);
	if (!vfa->files[0]) {
		free(vfa->input);
		free(vfa->files);
		free(vfa);
		return 0;
	}

	if (!childdir[0]) {
		shiftword(G.comline,childdir,MAINLINE);
	}

	/*vfb = get_valid_dirs('v',1,"directory",topd,childdir,0);*/
	vfb = get_valid_dirs('v',1,Ustring[247],topd,childdir,0);
	if (!vfb->files[0]) {
		free(vfb->input);
		free(vfb->files);
		free(vfb);
		free(vfa->input);
		free(vfa->files);
		free(vfa);
		return 0;
	}

	while (vfa->files[0]) {
		shiftword(vfa->files,file,MAINLINE);
		sprintf(src_file,"%s/%s",G.dir,file);
		sprintf(dest_file,"%s/%s/%s",topd,vfb->files,file);
		if (stat(dest_file,&statbuf)) {
			sprintf(temp,"cp -p %s %s",src_file,dest_file);
			dsystem(temp);
			chmod(dest_file,C.filemode);
			if ((C.canchown) && (!(stat(src_file,&statbuf)))) {
#if !defined(LINUX)
				sprintf(temp,"chown %ld %s",statbuf.st_uid,dest_file);
#else
				sprintf(temp,"chown %d %s",statbuf.st_uid,dest_file);
#endif
				dsystem(temp);
			}

			sprintf(temp,"%s/%s",topd,vfb->files); /* Used twice - please leave */
			if (from_pfu) {
				sprintf(tempa,Ustring[252],file);/* ask whether to delete original*/
				if (yes_no(tempa)) {
					remove(src_file);
				}
			} else {
				copy_desc(G.dir,temp,file);
			}
			describe_file(temp,file);
		} else {
			/*printf("%s already has a file called %s. Not posted.\n",vfb->files,file);*/
			printf(Ustring[253],vfb->files,file);
			printf(" - %s\n",Ustring[60]);
			free(vfb->input);
			free(vfb->files);
			free(vfb);
			free(vfa->input);
			free(vfa->files);
			free(vfa);
			return 0;
		}
	}
	free(vfb->input);
	free(vfb->files);
	free(vfb);
	free(vfa->input);
	free(vfa->files);
	free(vfa);
	return 1;
}
	
int copy_desc (char *fromparent, char *toparent, char *file) {
	DIR *DIRT;
	char temp[MAINLINE * 2];
	char tempa[MAINLINE * 2];

	sprintf(temp,"%s/filedescs",toparent);
	if (!(DIRT = opendir(temp))) {
		if (mkdir(temp,0770)) {
			/*printf("Unable to copy description.\n");*/
			printf(Ustring[254],Ustring[199]);
			printf("\n");
			return 0;
		}
	} else {
		closedir(DIRT);
	}
	sprintf(temp,"%s/filedescs/%s",fromparent,file);
	sprintf(tempa,"%s/filedescs/%s",toparent,file);
	return copy_file(temp,tempa,0);
}

int do_describefile(char *in) {
/* MENU COMMAND */
/*
 *  takes single filename which must be in current G.dir
 *  takes from comline or menu
 */
	char file[MAINLINE + 100];
	struct valid_files *vf;
	char *copy;

	if (!is_dir_elig('v',G.dir)) {
		return 0;
	}

	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		/*vf = get_valid_files('v',1,"file",G.dir,copy,1);*/
		vf = get_valid_files('v',1,Ustring[249],G.dir,copy,1);
	} else {
		shiftword(G.comline,file,MAINLINE);
		vf = get_valid_files('v',1,Ustring[249],G.dir,file,1);
	}
	
	strcpy(file,vf->files);
	free(vf->input);
	free(vf->files);
	free(vf);
	free(copy);
	if (!file[0]) {
		return 0;
	}

	return describe_file(G.dir,file);
}

int describe_file (char *dir,char *file) {
	DIR *DIRT;
	char temp[MAINLINE * 2];

	sprintf(temp,"%s/filedescs",dir);
	if (!(DIRT = opendir(temp))) {
		if (mkdir(temp,0770)) {
			/*printf("Unable to write description.\n");*/
			printf(Ustring[67],Ustring[199]);
			printf("\n");
			return 0;
		}
	} else {
		closedir(DIRT);
	}

	/*printf("\n\nEditing DESCRIPTION of %s\n",file);*/
	sprintf(temp,Ustring[256],file);
	printf("\n\n");
	printf(Ustring[255],temp);
	printf("\n");
	sprintf(temp,"%s/filedescs/%s",dir,file);
	return (edit_special(temp));
}

int file_del (char *in) {
/* MENU COMMAND */
	struct valid_files *vf;
	int result = 0;
	int from_pfu = 0;
	int confirm = 0;
	char response[2];
	char *copy;
	char filename[MAINLINE + MAINLINE];
	char onefile[MAINLINE + 100];
	char temp[MAINLINE];

	if (!is_dir_elig('v',G.dir)) {
		return 0;
	}

	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		/*vf = get_valid_files('v',0,"files",G.dir,copy,1);*/
		vf = get_valid_files('v',0,Ustring[266],G.dir,copy,1);
	} else {
		vf = get_valid_files('v',0,Ustring[266],G.dir,G.comline,1);
		flushcom("");
	}
	
	sprintf(filename,"%s/%s",C.privatefiles,U.id);
	from_pfu = !strcmp(G.dir,filename); /* yes or no, is it from your pfu? */

	if (vf->files[0]) {
		/*make_prompt("Confirm for each one? Y/n/q ");*/
		sprintf(temp,"%s %c/%c/%c",Ustring[458],G.bigyes,G.littleno,G.littlequit);
		make_prompt(temp);
		get_one_lc_char(response);
		if ((!response[0] || response[0] == G.littleyes)) {
			confirm = 1;
		} else if (response[0] == G.littleno) {
			confirm = 0;
		} else if (response[0] == G.littlequit) {
			free(vf->input);
			free(vf->files);
			free(vf);
			free(copy);
			return result;
		} else {
			confirm = 1;
		}
	}

	while (vf->files[0]) {
		shiftword(vf->files,onefile,MAINLINE);
		if (confirm) {
			printf("\n");
			/*sprintf(temp,"Delete %s? y/N/q ",onefile);*/
			sprintf(filename,Ustring[108],onefile);
			sprintf(temp,"%s %c/%c/%c",filename,G.littleyes,G.bigno,G.littlequit);
			make_prompt(temp);
			get_one_lc_char(response);
			if (response[0] == G.littlequit) {
				break;
			} else if (response[0] != G.littleyes) {
				continue;
			}
		}
		sprintf(filename,"%s/%s",G.dir,onefile);
		remove(filename);
		if (!from_pfu) {
			sprintf(filename,"%s/filedescs/%s",G.dir,onefile);
			remove(filename);
		}
		/*printf("%s now removed.\n",onefile);*/
		printf(Ustring[257],onefile);
		printf("\n");
		result = 1;
	}
	free(vf->input);
	free(vf->files);
	free(vf);
	free(copy);
	return result;
}

int file_rename (char *in) {
/* MENU COMMAND */
	char temp[MAINLINE + 100];
	char tempa[MAINLINE];
	char old[MAINLINE + 100];
	char new[MAINLINE + 100];
	char oldname[MAINLINE + 100];
	char newname[MAINLINE + 100];
	struct valid_files *vf;
	struct stat statbuf;
	int from_pfu;
	char *copy;

	if (!is_dir_elig('v',G.dir)) {
		return 0;
	}

	sprintf(oldname,"%s/%s",C.privatefiles,U.id);
	from_pfu = !strcmp(G.dir,oldname); /* yes or no, is it from your pfu? */

	copy = strdup(in);
	shiftword(copy,old,MAINLINE);
	if (!old[0]) {
		shiftword(G.comline,old,MAINLINE);
	}
		
	shiftword(copy,new,MAINLINE);
	if (!new[0]) {
		shiftword(G.comline,new,MAINLINE);
	}
		
	if (!Dstrcmp(new,Ustring[501])) {
		shiftword(copy,new,MAINLINE);
		if (!new[0]) {
			shiftword(G.comline,new,MAINLINE);
		}
	}
	free(copy);

	/*vf = get_valid_files('v',1,"file",G.dir,old,1);*/
	vf = get_valid_files('v',1,Ustring[249],G.dir,old,1);
	strcpy(old,vf->files);
	free(vf->input);
	free(vf->files);
	free(vf);

	if (!old[0]) {
		return 0;
	}

	sprintf(tempa,Ustring[400],Ustring[249]);/*Enter new filename*/
	new_get_one_param('v',tempa,new,temp,C.maxfilename + 1);

	if (!temp[0]) {
		return 0;
	}
	get_one_file(new,C.maxfilename + 1,temp);
	if (strcmp(new,temp)) {
		printf(Ustring[398],temp);
		printf("\n");
		return 0;
	}

	sprintf(oldname,"%s/%s",G.dir,old);
	sprintf(newname,"%s/%s",G.dir,new);
	if (!stat(newname,&statbuf)) {
		/*printf("There already is a %s.  %s not renamed.\n",new,old);*/
		printf(Ustring[253],Ustring[247],new);
		printf("- %s\n",Ustring[60]);
		return 0;
	}
	rename(oldname,newname);

	sprintf(oldname,"%s/filedescs",G.dir);
	if ((!from_pfu) && is_dir_elig('q',oldname)) {
		sprintf(oldname,"%s/filedescs/%s",G.dir,old);
		sprintf(newname,"%s/filedescs/%s",G.dir,new);
		rename(oldname,newname);
	}
	/*printf("%s renamed to %s.\n",old,new);*/
	printf(Ustring[259],old,new);
	printf("\n");
	return 1;
}

/* ARGSUSED0 */
int catalogue (char *dummy) {
/* MENU COMMAND */
/* Now to list files with descs */
	FILE *FIL;
	char name[MAINLINE];
	char filename[MAINLINE + 100];
	struct valid_files *vf;
	struct passwd *pw;
	struct stat sb;
	char *date;
	int linecount = -1;
	int k;

	if (!is_dir_elig('v',G.dir)) {
		return 0;
	}

	/*vf = get_valid_files('q',0,"files",G.dir,"*",1);*/
	vf = get_valid_files('q',0,Ustring[266],G.dir,"*",1);

	if (!vf->files[0]) {
		/*printf("Directory is empty.\n");*/
		printf(Ustring[313],Ustring[247]);
		printf("\n");
		free(vf->files);
		free(vf->input);
		free(vf);
		return 0;
	}
	
	printf("\n");
	while (vf->files[0]) {
		shiftword(vf->files,name,51);
		sprintf(filename,"%s/%s",G.dir,name);
		if ((!stat(filename,&sb)) && !S_ISDIR(sb.st_mode)) {
			date = drealmtime(sb.st_mtime);
			if (C.canchown && (pw = getpwuid(sb.st_uid)) ) {
				printf("%-20s %10lld %s %s %s\n",
					name, sb.st_size, date, Ustring[260], pw->pw_name);
			} else {
				printf("%-20s %10lld %s\n",name,sb.st_size,date);
			}
			free(date);

			if (linecount > LINES-5) {
				if (!do_continue("")) {
					free(vf->files);
					free(vf->input);
					free(vf);
					return 1;
				}
				linecount = 0;
			}
			sprintf(filename,"%s/filedescs/%s",G.dir,name);
			linecount++;
			if (!stat(filename,&sb) && (sb.st_size > 0)) {
				if ((FIL = fopen(filename,"r"))) {
					while (fgets(filename,80,FIL)) {
						k = strlen(filename) - 1;
						if (filename[k] == '\n') {
							printf("%s",filename);
						} else {
							printf("%s\n",filename);
						}
						linecount++;
						if (linecount > LINES-5) {
							if (!do_continue("")) {
								return 1;
							}
							linecount = 0;
						}
					}
					fclose(FIL);
				}
			} else {
				/*printf("No Description.\n");*/
				printf("%s\n",Ustring[261]);
				linecount++;
			}
			printf("----------------------------------------------------------------------\n");
			linecount++;
		}
	}
	free(vf->files);
	free(vf->input);
	free(vf);
	return 1;
}


int oktosend (char *wholefilename) {
	struct stat statbuf;
	char *filename;
	char temp[MAINLINE + 100];

	if (stat(wholefilename,&statbuf)) {
		return 1;
	}

	filename = base_name(wholefilename,'/');
	if (G.uid == statbuf.st_uid) {
		/*sprintf(temp,"%s already exists.  Overwrite?",filename);*/
		sprintf(temp,Ustring[263],filename);
		strcat(temp," ");
		strcat(temp,Ustring[262]);
		if (no_yes(temp)) {
			remove(wholefilename);
			free(filename);
			return 1;
		}
	} else {
		/*printf("%s already exists.\n",filename);*/
		printf(Ustring[263],filename);
		printf(" - %s\n",Ustring[60]);
	}
	free(filename);
	return 0;

}


int dir_create (char *in) {
/* MENU COMMAND */
/* CHECKED */
/* Parent directory MUST be stated on menuline, new dir is optional param */
	char *date;
	int result;
	FILE *LOG;
	char up_dir[MAINLINE + 100];  /* Watch this if max menufield length changes */
	char newdir[51]; 
	char filename[MAINLINE + 100];
	char temp[MAINLINE + 100];
	char *copy = strdup(in);
	char tempa[MAINLINE];

	shiftword(copy,up_dir,MAINLINE);
	if (up_dir[0] != '/') {
		/*printf("Unable to create directory - abandoned.\n");*/
		printf(Ustring[68],Ustring[247]);
		printf(" - %s\n",Ustring[60]);
		strcpy(G.errmsg,"Parent directory path not specified.");
		errorlog(G.errmsg);
		free(copy);
		return 0;
	}


	shiftword(copy,temp,MAINLINE);
	if (!temp[0]) {
		shiftword(G.comline,temp,MAINLINE);
	}		
	free(copy);
	
	/*new_get_one_param('v',"Enter directory name (without path): ",temp,filename,C.maxfilename + 1);*/
	sprintf(tempa,Ustring[399],Ustring[247]);
	new_get_one_param('v',tempa,temp,filename,C.maxfilename + 1);

	if (!filename[0]) {
		return 0;
	}

	get_one_file(newdir,C.maxfilename + 1,filename);

	if (strcmp(newdir,filename)) {
		/*printf("'%s' invalid.\n",filename);*/
		printf(Ustring[398],filename);
		printf("\n");
		return 0;
	}

	sprintf(filename,"%s/%s",up_dir,newdir);

	if (dirmake(filename)) {
		/*printf("Directory %s created.\n",filename);*/
		printf(Ustring[265],filename);
		printf("\n");

		date = shorttime(time(0));
		sprintf(filename,"%s/newdirlog",C.datadir);
		if ( (LOG=fopen(filename,"a")) ) {
			fprintf(LOG,"%s %s created %s\n",date,U.id,newdir);
			fclose(LOG);
		}
		free(date);
		printf("\n");
	} else {
		sprintf(temp,"%s was unable to create %s\n",U.id,filename);
		errorlog(temp);
		/*printf("Unable to create directory - abandoned.\n");*/
		printf(Ustring[68],Ustring[247]);
		printf("- %s\n",Ustring[60]);
		result = 0;
	}
	result = 1;
	return result;
}


int dirmake (char *newdir) {
/* CHECKED - Nothing to go wrong! */
	if (mkdir(newdir,0770)) {
		return 0;
	} else {
		return 1;
	}
}

int dir_remove (char *in) {
/* MENU COMMAND */
/* Parent directory MUST be stated on menuline, new dir is optional param */
	struct valid_files *vf;
	int still_files;
	FILE *LOG;
	char up_dir[MAINLINE + 100];
	char newdir[51];
	char filename[MAINLINE + 100];
	char temp[MAINLINE + 100];
	char *date;
	char *copy = strdup(in);

	shiftword(copy,up_dir,MAINLINE);
	if (up_dir[0] != '/') {
		/*printf("Unable to find directory - abandoned.\n");*/
		printf(Ustring[65],Ustring[247]);
		printf("- %s\n",Ustring[60]);
		strcpy(filename,"Parent directory path not specified.");
		errorlog(filename);
		free(copy);
		return 0;
	}


	shiftword(copy,temp,MAINLINE);
	if (!temp[0]) {
		shiftword(G.comline,temp,MAINLINE);
	}		
	free(copy);

	/*vf = get_valid_dirs('v',1,"directory",up_dir,temp,C.filesensitive);*/
	vf = get_valid_dirs('v',1,Ustring[247],up_dir,temp,C.filesensitive);
	strcpy(newdir,vf->files);
	free(vf->input);
	free(vf->files);
	free(vf);

	if (!newdir[0]) {
		return 0;
	}

	sprintf(filename,"%s/%s",up_dir,newdir);
	/*vf = get_valid_files('q',1,"file",filename,"*",0);*/
	vf = get_valid_files('q',1,Ustring[249],filename,"*",0);
	still_files = 0;
	if (vf->files[0]) {
		still_files = 1;
	}
	free(vf->files);
	free(vf->input);
	free(vf);
	if (still_files) {
		/*printf("There are files remaining which must be moved or deleted.\n");*/
		printf("%s\n",Ustring[267]);
		return 0;			
	}

	/*printf("Delete newdir?");*/
	sprintf(temp,Ustring[108],newdir);
	if (!no_yes(temp)) {
		return 0;			
	}

	if (dirrm(filename)) {
		date = shorttime(time(0));
		sprintf(temp,"%s/newdirlog",C.datadir);
		if ( (LOG=fopen(filename,"a")) ) {
			fprintf(LOG,"%s %s removed %s\n",date,U.id,newdir);
			fclose(LOG);
		}
		free(date);
		printf("\n");
		/*printf("Directory %s removed.\n",newdir);*/
		printf(Ustring[257],newdir);
		printf("\n");
	} else {
		sprintf(temp,"%s was unable to remove %s\n",U.id,filename);
		errorlog(temp);
		/*printf("Unable to remove directory - abandoned.\n");*/
		printf(Ustring[268],newdir);
		printf(" - %s\n",Ustring[60]);
		return 0;
	}
	return 1;
}


int dirrm (char *newdir) {
	char command[MAINLINE * 2];

	if ((int)strlen(newdir) > (MAINLINE + 50)) {
		/*printf("Directory name too long - abandoned.\n");*/
		printf("%s - %s\n",Ustring[246],Ustring[60]);
		return 0;
	}
	sprintf(command,"rm -rf %s",newdir);
	return !dsystem(command);
}


int up_file (char *in) {
/* MENU COMMAND */
	char filename[MAINLINE + 100];
	char *params;
	char *changed;
	char *copy;

	if (!is_dir_elig('v',G.dir)) {
		return 0;
	}

	sprintf(filename,"%s/%s",C.privatefiles,U.id);
	if (!strcmp(G.dir,filename) && (U.level < C.pvtfileslevel)) {
		/*printf("You do not have private file facilities.\n");*/
		printf(Ustring[250],U.id,Ustring[251]);
		printf("\n");
		return 0;
	}

	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		changed = uploading(G.dir,copy);
	} else {
		changed = uploading(G.dir,G.comline);
		flushcom("");
	}
	
	params = dir_name(G.dir,'/');

	if (strcmp(params,C.privatefiles) && strcmp(params,C.homedirs)) {
		while (changed[0]) {
			shiftword(changed,filename,MAINLINE);
			describe_file(G.dir,filename);
		}
	}
	free(params);
	free(changed);
	free(copy);
	return 1;
}


char *uploading (char *dir, char *params) {
	char filename[MAINLINE + 100];
	char origname[MAINLINE + 100];
	char command[MAINLINE + MAINLINE + 100];
	char string[MAINLINE];
	DIR *DIRT;
	FILE *LOG;
	struct stat statbuf;
	struct dirent *dent;
	time_t starttime = time(0);
	int i = 0;
	int choice;
	char F[8][6][MAINLINE];
	char temp[1024];
	char changed[1024];
	char src_file[MAINLINE * 2];
	char dest_file[MAINLINE * 2];
	int ind;
	char indstring[4];

	cfgfiles_read(F);

	temp[0] = 0;
	changed[0] = 0;
	
	/* CONSTCOND */
	while (1) {
		for (i=0;F[i][0][0];i++) {
			if (F[i][2][0]) {
				printf("[%d] %s\n",i+1,F[i][0]);
			}
		}
		/*printf("[%d] quit\n",i+1);*/
		printf("[%d] %s\n\n",i+1,Ustring[63]);
		/*make_prompt("\nProtocol: ");*/
		make_prompt(Ustring[269]);

		choice = get_one_num(1,0);
		if ((choice == 0) || (choice == i+1)) {
			return strdup("");
		}
		choice--; /*because the menu shows all the  numbers up one*/
		if ((choice < i) && (F[choice][2][0])) {
			break;
		} else {
			/*printf("'%d' invalid.\n",choice + 1);*/
			sprintf(temp,"%d",choice + 1);
			printf(Ustring[472],temp);
			printf("\n");
		}

	}

	chdir(G.home);
	if (!strcmp(F[choice][4],"1")) {
		printf("\n%s\n\n",F[choice][3]);
		external_term();
		sprintf(command,"%s",F[choice][2]);
		usystem(command);
		internal_term();
	} else {
		if (!params[0]) {
			/*make_prompt("Please state filename: ");*/
			sprintf(filename,Ustring[399],Ustring[249]);
			make_prompt(filename);
		}
		get_one_file(filename,C.maxfilename + 1,params);
		if ((filename[0]) && (oktosend(filename))) {
			printf("\n%s\n\n",F[choice][3]);
			external_term();
			sprintf(command,"%s %s",F[choice][2],filename);
			usystem(command);
			internal_term();
		}
	}

	
	if ((DIRT = opendir(G.home))) {
		while ((dent = readdir(DIRT))) {
			if (dent->d_name[0] == '.') {
				continue;
			}
			stat(dent->d_name,&statbuf);
			if ((statbuf.st_ctime > starttime) && (statbuf.st_uid == G.uid)) {
				chmod(dent->d_name,0770);
				strcpy(string,dent->d_name);
				if (((int)strlen(temp) + (int)strlen(string)) < 1020) {
					strcat(temp,string);
					strcat(temp," ");
				}
			}
		}
		closedir(DIRT);
	}

	while (temp[0]) {
		shiftword(temp,filename,MAINLINE);
		strcpy(origname,filename);
		if (strcmp(dir,G.home)) {
			sprintf(src_file,"%s/%s",G.home,filename);
			sprintf(dest_file,"%s/%s",dir,filename);
			ind = 0;
			while (!stat(dest_file,&statbuf)) {
				ind++;
				if (ind > 999) {
					/*printf("Unable to process %s",origname);*/
					printf(Ustring[67],origname);
					printf(" - %s\n",Ustring[60]);
					remove(src_file);
					break;
				}
				sprintf(indstring,"%d",ind);
				if ((strlen(indstring) + strlen(origname)) <= (size_t)C.maxfilename) {
					sprintf(filename,"%s%s",origname,indstring);
				} else {
					strcpy(&filename[C.maxfilename - strlen(indstring)],indstring);
				}
				sprintf(dest_file,"%s/%s",dir,filename);

			}
			if (ind > 999) {
				continue;
			}
			if (strcmp(origname,filename)) {
				/*printf("%s already existed, name changed to %s.\n",origname,filename);*/
				printf(Ustring[263],origname);
				printf(Ustring[259],"",filename);
				printf("\n");
			}

			sprintf(command,"mv %s %s",src_file,dest_file);
			dsystem(command);
			if ((C.canchown) && (!(stat(src_file,&statbuf)))) {
#if !defined(LINUX)
				sprintf(temp,"chown %ld %s",statbuf.st_uid,dest_file);
#else
				sprintf(temp,"chown %d %s",statbuf.st_uid,dest_file);
#endif
				dsystem(temp);
			}
		}
		if (C.uploadreport) {
			char *date = shorttime(time(0));

			sprintf(src_file,"%s/uploadlog",C.datadir);
			if ( (LOG=fopen(filename,"a")) ) {
				fprintf(LOG,"%s %s uploaded %s\n",date,U.id,dest_file);
				fclose(LOG);
			}
			free(date);
		}		
		
		if (((int)strlen(changed) + (int)strlen(filename)) < 1020) {
			strcat(changed,filename);
			strcat(changed," ");
		}
	
		
	} 
	return strdup(changed);
}


int down_file_bf (char *in) {
/* MENU COMMAND */
	return down_file(1,in);
}

int down_file_ubf (char *in) {
/* MENU COMMAND */
	return down_file(0,in);
}

int down_file (int bf, char *in) {
	char filename[MAINLINE + 100];
	char temp[MAINLINE + 100];
	char onefile[MAINLINE + 100];
	struct valid_files *vf;
	char *copy;
	int result = 0;
	
	if (!is_dir_elig('v',G.dir)) {
		flushcom("");
		return 0;
	}

	copy = strdup(in);
	tnt(copy);
	if (copy[0]) {
		/*vf = get_valid_files('v',0,"files",G.dir,copy,1);*/
		vf = get_valid_files('v',0,Ustring[266],G.dir,copy,1);
	} else {
		/*vf = get_valid_files('v',0,"files",G.dir,G.comline,1);*/
		vf = get_valid_files('v',0,Ustring[266],G.dir,G.comline,1);
		flushcom("");
	}	
	free(copy);

	if (!vf->files[0]) {
		free(vf->input);
		free(vf->files);
		free(vf);
		return 0;
	}

	result = downloading(bf,G.dir,vf->files);

	sprintf(onefile,"%s/%s",C.privatefiles,U.id);
	if (!strcmp(onefile,G.dir)) {
		while (vf->files[0]) {
			shiftword(vf->files,onefile,MAINLINE);
			/*sprintf(temp,"\nDelete original copy of %s? Y/n ",onefile);*/
			sprintf(temp,Ustring[252],onefile);
			if (yes_no(temp)) {
				sprintf(filename,"%s/%s",G.dir,onefile);
				remove(filename);
			}
		}
	}
	free(vf->input);
	free(vf->files);
	free(vf);
	return result;
}

int down_special_bf (char *in) {
/* MENU COMMAND */
	return downloading(1,"/",in);
}

int down_special_ubf (char *in) {
/* MENU COMMAND */
	return downloading(0,G.home,in);
}


int downloading (int bf, char *dir, char *files) {
/*
 * peter - Thu Feb 29 21:51:27 GMT 1996
 * This is called by down_file and down_special.
 * For down_file, the list of files is relative to G.dir and comes from the
 * menu, comline or the user.  "dir" is G.dir.
 * For down_special, the list of files is not relative.  "dir" is "/" for buffered,
 * purely to get the system "cp" command to work correctly.  "dir" is G.home for
 * unbuffered.
 * During a buffered download, the user sits in ~/.dl.  Otherwise in "dir".
 */
	struct valid_files *vf;
	int i = 0;
	int choice;
	char filename[MAINLINE + 100];
	char command[MAINLINE + MAINLINE + 100];
	char temp[MAINLINE];
	char F[8][6][MAINLINE];
	char *copy = strdup(files);
	char *copyfiles;

	cfgfiles_read(F);
	/* CONSTCOND */
	while (1) {
		for (i=0;F[i][0][0];i++) {
			if (F[i][1][0]) {
				printf("[%d] %s\n",i+1,F[i][0]);
			}
		}
		/*printf("[%d] quit\n",i+1);*/
		printf("[%d] %s\n\n",i+1,Ustring[63]);
		/*make_prompt("\nProtocol: ");*/
		make_prompt(Ustring[269]);

		choice = get_one_num(1,0);
		if ((choice == 0) || (choice == i+1)) {
			free(copy);
			return 0;
		}
		choice--; /*because the menu shows all the  numbers up one*/
		if ((choice < i) && (F[choice][1][0])) {
			break;
		} else {
			/*printf("'%d' invalid.\n",choice + 1);*/
			sprintf(temp,"%d",choice + 1);
			printf(Ustring[472],temp);
			printf("\n");
		}
	}


	if (bf) {
		chdir(G.home);
		mkdir(".dl",0770);
		copyfiles = strdup(copy);
		while(copyfiles[0]) {
			shiftword(copyfiles,temp,MAINLINE);
			sprintf(command,"cp %s/%s .dl/",dir,temp);
			dsystem(command);
		}
		free(copyfiles);
		chdir(".dl");

		vf = get_valid_files('q',0,"",".","*",1);
		if (vf->files[0]) {
			copy = strdup(vf->files);
		} else {
			copy[0] = 0;
		}
		free(vf->input);
		free(vf->files);
		free(vf);
		
	} else {
		chdir(dir);
	}


/*
 * peter - Thu Feb 29 21:51:27 GMT 1996
 * For buffered download, "copy" should contain a list of basenames (i.e.
 * without paths) at this point.
 */


	if (!strcmp(F[choice][4],"1")) {
		printf("\n%s\n\n",F[choice][3]);
		external_term();
		sprintf(command,"%s %s",F[choice][1],copy);
		usystem(command);
		internal_term();
	} else {

		while (copy[0]) {
			shiftword(copy,filename,MAINLINE);
			/*printf("About to start sending %s.\n",filename);*/
			printf(Ustring[271],filename);
			printf("\n");
			press_enter("");
			printf("\n%s\n\n",F[choice][3]);
			external_term();
			sprintf(command,"%s %s",F[choice][1],filename);
			usystem(command);
			internal_term();
		}
	}
	chdir(G.home);
	
	if (bf) {
		sprintf(command,"rm -rf .dl");
		dsystem(command);
	}
	free(copy);
	return 1;
}


void cfgfiles_read (char F[][6][MAINLINE]) {
	char filename[MAINLINE + 50];
	FILE *CFG;

	F[0][0][0] = 0;
	sprintf(filename,"%s/config.files",C.configdir);
	if ((CFG = fopen(filename,"r"))) {

		if (cfgfiles_parse(CFG,F)) {
			fclose(CFG);
		} else {
			fclose(CFG);
			errorlog("Something wrong in reading config.files");
			F[0][0][0] = 0;
		}
	} else {
		/*printf("Cannot open %s\n",filename);*/
		printf(Ustring[64],filename);
		printf("\n");
	}
}

int cfgfiles_parse (FILE *CFG, char F[][6][MAINLINE]) {
	int i = 0;
	char result[MAINLINE];
	int totalshift = 0;

	while (i<MAXPROTO) {

		get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
		tnt(result);
		if (!result[0]) {
			break;
		}
		strcpy(F[i][0],result);

		get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
		tnt(result);
		if (!result[0]) {
			break;
		}
		strcpy(F[i][1],result);

		get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
		tnt(result);
		if (!result[0]) {
			break;
		}
		strcpy(F[i][2],result);

		get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
		tnt(result);
		strcpy(F[i][3],result);

		get_next_cfgfield (CFG,&totalshift,result,MAINLINE);
		tnt(result);
		strcpy(F[i][4],result);

		if (!is_num(F[i][5])) {
			strcpy(F[i][5],"0");
		}
		i++;
	}
	F[i][0][0] = 0;
	return 1;
}




int search_filedescs (char *in) {
/* MENU COMMAND */
	int result = 0;
	char *copy = strdup(in);

	result = filesearch('d','r',copy);
	free(copy);
	return result;
}

int search_filenames (char *in) {
/* MENU COMMAND */
	int result = 0;
	char *copy = strdup(in);

	result = filesearch('n','r',copy);
	free(copy);
	return result;
}

int filesearch (char type, char recurse, char *copy) {
	char string[MAINLINE];
	char pattern[MAINLINE];
	char restdir[MAINLINE + 100];
	char topdir[MAINLINE + 100];
	int tempflag = 0;
	int stopped = 0;
	int linecount = 0;
	int nr_found = 0;

	tnt(copy);

	shiftword(copy,topdir,MAINLINE + 50);
	if (!topdir[0]) {
		strcpy(topdir,G.dir);
	}
	if (!is_dir_elig('v',topdir)) {
		return 0;
	}
	
	if (copy[0]) {
		if (copy[0] == '"') {
			strshift(copy,pattern,MAINLINE,"\"");
			strshift(copy,pattern,MAINLINE,"\"");
		} else {
			shiftword(copy,pattern,MAINLINE);
		}
	} else {
		if (G.comline[0] == '"') {
			strshift(G.comline,pattern,MAINLINE,"\"");
			strshift(G.comline,pattern,MAINLINE,"\"");
		} else {
			shiftword(G.comline,pattern,MAINLINE);
		}
	}
	
	flushcom("");

	if (!pattern[0]) {
		/* CONSTCOND */
		while (1) {
			if (type == 'd') {
				/*printf("Please type search pattern between double quotes (\"\") or ? for help.\n");*/
				printf("%s (%s)\n",Ustring[194],Ustring[200]);
			} else {
				/*printf("Please type filename using wildcards if required or ? for help.\n");*/
				printf("%s (%s)\n",Ustring[273],Ustring[200]);
			}				
			make_prompt(Ustring[119]);
			get_one_line(string);
			tnt(string);
			if (!string[0]) {
				return 0;
			}

			if (!strcmp(string,"?")) {
				if (type == 'd') {
					do_fds_help();
				} else if (type == 'n') {
					do_fns_help();
				}
			continue;
			}

			if ((string[0] == '"') && (string[strlen(string) - 1] == '"')) {
				strshift(string,pattern,21,"\"");
				strshift(string,pattern,MAINLINE,"\"");
			} else {
				shiftword(string,pattern,MAINLINE);
			}
			if (!pattern[0]) {
				return 0;
			}
			/*
			 * for filenames, put wildcards on
			 */
			if (type == 'n') {
				sprintf(string,"*%s*",pattern);
				strcpy(pattern,string);
			}
			break;
		}
		/* OK we have a topdir and a pattern */

	}
	restdir[0] = 0;
	/*printf("\nCtrl-C to abandon.\n");*/
	printf("\n%s\n",Ustring[274]);
	intr_on();
	/*printf("Searching...\n");*/
	printf("%s\n",Ustring[275]);

	recurse_dirs (recurse,&linecount,&stopped,&nr_found,type,topdir,restdir,pattern);
	tempflag = G.intflag;	
	intr_off();
		if (tempflag) {
		return 0;
	} else if (nr_found) {
		return 1;
	} else {
		if (type == 'n') {
			/*printf("No filenames were found to match your pattern.\n");*/
			printf("%s\n",Ustring[123]);
		} else if (type == 'd') {
			/*printf("No file descriptions contained your pattern.\n");*/
			printf("%s\n",Ustring[123]);
		}
		return 0;
	}	
}


/*===============================================================*/
void recurse_dirs (char recurse, int *linecount,int *stopped, int *nr_found, char type, char *topdir, char *restdir, char *pattern) {
	char tempdir[MAINLINE + 100];
	char subdir[MAINLINE + 100];
	struct valid_files *vd;

	if (type == 'd') {
		search_descs (recurse,stopped,nr_found,linecount,topdir,restdir,pattern);
	} else if (type == 'n') {
		search_names (recurse,stopped,nr_found,linecount,topdir,restdir,pattern);
	}
	if ((*stopped) || (G.intflag)) {
		return;
	}

	sprintf(tempdir,"%s/%s",topdir,restdir);

	vd = get_valid_dirs('q',0,"",tempdir,"*",0);
	while(vd->files[0] && !*stopped) {
		if (G.intflag) {
			return;
		}
		shiftword(vd->files,subdir,MAINLINE + 50);
		if (!strcmp(subdir,"filedescs")) {
			continue;
		}
		if ((strlen(topdir) + strlen(restdir) + strlen(subdir)) > (size_t)(MAINLINE)) {
			/*printf("Path too long - search abandoned.\n");*/
			printf("%s - %s\n",Ustring[246],Ustring[60]);
			*stopped = 1;
			return;
		}
		sprintf(tempdir,"%s%s/",restdir,subdir);
		recurse_dirs(recurse,linecount,stopped,nr_found,type,topdir,tempdir,pattern);
	}		
}


/*===============================================================*/

void search_names (char recurse, int *stopped,int *nr_found,int *linecount, char *topdir, char *restdir, char *pattern) {
	struct valid_files *vf;
	char tempdir[MAINLINE + 100];
	char filename[51];

	sprintf(tempdir,"%s/%s",topdir,restdir);
	

	if (!is_dir_elig('q',tempdir)) {
		return;
	}

	vf = get_valid_files('q',0,"",tempdir,pattern,0);
	while (vf->files[0] && !*stopped) {
		if (G.intflag) {
			return;
		}		
		shiftword(vf->files,filename,51);

		printf("%s%s\n",restdir,filename);
		*nr_found += 1;
		*linecount+=1;
		if (*linecount > LINES-5) {
			if (!do_continue("")) {
				*stopped = 1;
				break;
			}
			*linecount = 0;
		}
	}
}

void search_descs (char recurse,int *stopped,int *nr_found,int *linecount, char *topdir, char *restdir, char *pattern) {
	char descdir[MAINLINE + 200];
	FILE *FIL;
	DIR  *DIRT;
	struct dirent *d;
	char filename[MAINLINE + 100];
	char line[MAINLINE + 1];
	char caseline[MAINLINE + 1];
	int i = 0;
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


/* HERE BEGINS THE SEARCH */

	sprintf(descdir,"%s/%s/filedescs",topdir,restdir);

	if (!is_dir_elig('q',descdir)) {
		return;
	}


	if (re) {
		if ((DIRT = opendir(descdir))) {
			/* LINTED */
			while ((d = readdir(DIRT)) && !*stopped) {
				if (G.intflag)  {
					return;
				}
				if (d->d_name[0] == '.') {
					continue;
				}
				sprintf(filename,"%s/%s",descdir,d->d_name);
				if ((FIL = fopen(filename,"r"))) {
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
							if (recurse != 'r') {
								printf("%-14.14s: %-.60s\n",d->d_name,caseline);
								*linecount += 1;
							} else {
								printf("%s%s\n",restdir,d->d_name);
								printf("\t%s\n",caseline);
								*linecount += 2;
							}
							*nr_found += 1;
							if (*linecount > LINES-5) {
								if (!do_continue("")) {
									*stopped = 1;
									break;
								}
								*linecount = 0;
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
			return;
		}
#  if defined(SVR42)
		free(re);
#  else
		regfree(&preg);
#  endif
		return;
	} else {
		/*printf("Regex error.\n");*/
		printf("%s Regex\n",Ustring[276]);
		return;
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
					*nr_found += 1;
					*linecount += 1;
					if (*linecount > LINES-5) {
						if (!do_continue("")) {
							*stopped = 1;
							break;
						}
						*linecount = 0;
					}
				}
			}
		}
	} else {
		printf("%s\n",Ustring[60]);
	}
	return;
#endif
}

void do_fds_help (void) {
	display_lang("fds");
}


void do_fns_help (void) {
	display_lang("fns");
}

