/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		fopene -- Level 2 open with environment search
*
* synopsis	fp = fopene(name,mode,path);
*		FILE *fp;	file pointer
*		char *name;	file name
*		char *mode;	access mode
*		char *path;	return path
*
* description	This function opens a file for Level 2 I/O, just like
*		the "fopen" function.  However, "fopene" makes an extended
*		directory search to find a relative file name, that is,
*		a name that does not begin with a slash, backslash, or
*		drive code.   ^^^^^ for qnx 4, just / ^^^^^
*
*		First it attempts to resolve the name in the current
*		directory.  If that fails, it converts the extension
*		to upper case and attempts to find an environment variable
*		with that name.  The variable is a list of directories
*		that are searched in turn.  If all thoses searches fail,
*		then "fopene" attempts to resolve the name via the PATH
*		variable.
*
* returns	1. A NULL return indicates that the open function failed.
*		Otherwise, a file pointer is returned.
*		
*		2. If the open succeeds and the "path" argument is not
*		a null pointer, the directory path is returned.
*		^^^^^ take care that the mode argument is valid for qnx 4 ^^^^^
*
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <env.h>
#include <lat.h>

FILE *fopene(name,mode,path)
const char *name;
const char *mode;
char *path;
{
    char ext[FESIZE],new[FMSIZE],var[FMSIZE];
    FILE *fp;
    int errsave;
    
    if(path != NULL) *path = '\0';
    if((fp = fopen(name,mode)) != NULL) return(fp);
    if(*name == '/') return(NULL);
    errsave = errno;
    if(stcgfe(ext,name) > 0)
    {
        strupr(ext);
        strcpy(var,ext);
    }
    else strcpy(var,"PATH");

    new[0]='\0';

    do {

	_searchenv(name,var,new);
	if (strcmp(var,"PATH")) strcpy(var,"PATH");
	else break;

	}  while  (*new=='\0');

    if (path && *new != '\0') strcpy(path,new);

    if (strlen(new))
	if((fp = fopen(new,mode)) != NULL) return(fp);

    if( path != NULL ) *path = '\0';
    return(NULL);
}

