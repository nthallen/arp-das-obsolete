#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <eillib.h>

/* returns full filename of log file given components, makes data subdirectory */
char *getfilename(char *buffer, char *dir, char *root, int fcount, int fperd, int makedir) {
	
char temp[FILENAME_MAX];
char format[30];
int subdir=0;

	if (fperd) subdir=(fcount/fperd)*fperd;
	if (subdir<10000) strcpy(format,"%s/%s%04d");
	else strcpy(format,"%s/%s%d");
	sprintf(temp,format,dir,root,subdir);
	if (access(temp,F_OK) && makedir) {
		errno=0;
		if (mkdir(temp,S_IRWXU | S_IRWXG | S_IRWXO))
			msg(MSG_EXIT_ABNORM,"Can't create dir %s",temp);
	}
	if (fcount<10000) strcpy(format,"%s/%s%04d");
	else strcpy(format,"%s/%s%d");
	sprintf(buffer,format,temp,root,fcount);
	return(buffer);
}
