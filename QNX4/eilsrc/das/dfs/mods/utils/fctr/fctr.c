#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <dbr_mod.h>
/*
    Counts files in the format <root><d><d><d><d> where <d> is a
    digit. The number of files, number of files per directory, minimum file,
    directory name and root name are written to STDOUT.
*/

main (int argc, char **argv) {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

int c;
struct stat st;
DIR *dirup,*dirdown;
struct dirent *direntup, *direntdown;
char pat[FILENAME_MAX+1];
char pth[FILENAME_MAX+1];	
char pth1[FILENAME_MAX+1];	
int count,maxfdcount,i;
char root[ROOTLEN +1];
char dir[FILENAME_MAX+1];
int fsize, maxfsize;
int prevdir;
int curdir;
int curstart;
int minstart;
int out;

    /* initialisations */
    strcpy(root,ROOTNAME);
    getcwd(dir,FILENAME_MAX+1);
    out = 0;

    while ( (c=getopt(argc,argv,"d:r:EO") ) != -1 )
	    switch (c) {
		case 'd': strncpy(dir,optarg,FILENAME_MAX-1);  break;
		case 'r': strncpy(root,optarg,ROOTLEN-1);  break;
		case 'E': out = 2; break;
		case 'O': out = 1; break;
		default : break;
	    }

	dirup = dirdown = NULL;

	if ( !(dirup=opendir(dir) ))
	    fprintf(stderr,"fcnt: can't open dir %s\n",dir);

	sprintf(pat,"%s[0-9][0-9][0-9][0-9]",root);	

	fsize=0;
	maxfsize=0;
	maxfdcount=0;
	count=0;
	prevdir=0;
	curdir=0;
	curstart=0;
	minstart=INT_MAX;

	for (;;) {

	    if (dirdown) closedir(dirdown);
	    if ( !(direntup=readdir(dirup)) )  break;
	    strcpy(pth,dir);
	    if (dir[strlen(dir)-1]!='/') strcat(pth,"/");
	    strcat(pth,direntup->d_name);

	    /* check if its a dir and opendir it */
	    if ( !fnmatch(pat,direntup->d_name,0) &&
		(dirdown=opendir(pth))!=NULL ) {
		curdir=atoi(direntup->d_name + strlen(direntup->d_name) - 4);
		maxfdcount=abs(curdir-prevdir);
		strcat(pth,"/");
		for (;;) {
		    if ( !(direntdown=readdir(dirdown)) ) break;
		    if ( !fnmatch(pat,direntdown->d_name,0) ) {
			strcpy(pth1,pth);
			strcat(pth1,direntdown->d_name);
			if ( (stat(pth1, &st)) !=0)
			    fprintf(stderr,"fcnt: Can't get status of %s\n",direntdown->d_name);
			else  if (S_ISREG(st.st_mode)) {
			    curstart=atoi(direntdown->d_name + strlen(direntdown->d_name) - 4);
			    if (curstart<minstart) minstart=curstart;
			    fsize=st.st_size;
			    count++;
			    if (fsize>maxfsize) maxfsize=fsize;
			}
		    }
		}
	    }
	    prevdir=curdir;
	}

	if (minstart==INT_MAX) minstart=0;
	if (dirup) closedir(dirup);
	if (count<2) maxfsize=0;
	switch (out) {
	    case 0:
		printf("%d %d %d %d %s %s",count,maxfdcount,minstart,maxfsize,dir,root);
		break;
	    case 1:
		printf("-t %d -n %d -f %d -z %d -d %s -r %s",count,maxfdcount,minstart,maxfsize,dir,root);
		break;
	    case 2:
		printf("Number of files: %d\n"
		    "Number of files/directory: %d\n"
			"Number of first file: %d\n"
			    "File Size: %d\n"
				"Directory: %s\n"
				    "Root Name: %s\n",
			count,maxfdcount,minstart,maxfsize,dir,root);
		break;
	}

}
