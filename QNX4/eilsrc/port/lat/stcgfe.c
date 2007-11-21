/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stcgfe -- get file extension
*
* synopsis	size = stcgfe(ext,name);
*		int size;	size of extension
*		char *ext;	extension string pointer
*		char *name;	file name string pointer
*
* description	This function finds the extension part of the specified
*		file name and returns it as a null-terminated string.
*		The extension is the last part of the file name and begins
*		with a period.
*
* returns	A return value of 0 indicates that no extension was found
*		and that ext is simply a null string.  Otherwise, the
*		return is the same number you would get via strlen(ext).
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include <lat.h>

stcgfe(ext,name)
char *ext;
const char *name;
{
const char *p;
int x;

*ext = '\0';
if(x = strlen(name))
 for(p = name+x; x > 0; x--) switch(*--p)
        {
        case '.':
        return(stccpy(ext,p+1,FESIZE)-1);

        case '/':
/*        case '\\':*/
/*        case ':':*/
        return(0);
        }
return(0);
}

