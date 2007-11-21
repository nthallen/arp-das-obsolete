/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		strmfp -- make file name from directory path
*
* synopsis	strmfp(name,dir,file);
*		char *dir;	directory name
*		char *file;	file name
*
* description	This function makes a file name by appending the
*		specified file name to the specified directory path.
*		A node separator (slash) is inserted if necessary.
*
**/
/* $Revision$ $Date$ */

#include <string.h>

void strmfp(name,dir,file)
char *name;
const char *dir;
const char *file;
{
int i;

strcpy(name,dir);
i = strlen(name);
if(i) switch(name[i-1])
        {
/*        case ':':*/
        case '/':
/*        case '\\':*/
        break;

        default:
        name[i++] = '/';
        }
strcpy(&name[i],file);
return;
}
