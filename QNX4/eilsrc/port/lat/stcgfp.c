/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stcgfp -- isolate path portion of file name
*
* synopsis	size = stcgfp(path,name);
*		char *path;		path pointer
*		char *name;		name pointer
*
* description	This function scans the file name in reverse order to
*		isolate the path portion, as follows:
*
*		  NAME => "d:abc/def/ghi.jkl"
*		  PATH => "d:abc/def"
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include <lat.h>

stcgfp(path,name)
char *path;
const char *name;
{
const char *p;
int x;

*path = '\0';
if(x = strlen(name)) for(p = name+x; x > 0; x--) switch(*--p)
        {
/*        case ':':*/
        case '/':
/*        case '\\':*/
        stccpy(path,name,x+1);
        return(x);

        default:
        continue;
        }
*path = '\0';
return(0);
}
