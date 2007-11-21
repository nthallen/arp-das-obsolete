/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		strmfe -- make file name with new extension
*
* synopsis	strmfe(newname,oldname,ext);
*		char *newname;		new file name
*		char *oldname;		new file name
*		char *ext;		extension
*
* description	This function copies the old name to the new name, deleting
*		any extension.  Then it appends a period to the new name,
*		followed by the specified extension.
*
*		Note that no length check is made.
*
**/
/* $Revision$ $Date$ */

#include <string.h>

void strmfe(newname,oldname,ext)
char *newname;
const char *oldname;
const char *ext;
{
char *p,c;

p = 0;
while(c = *newname++ = *oldname++) switch(c)
        {
        case '.':
        p = newname - 1;
        continue;

/*        case '\\':*/
        case '/':
        p = 0;

        default:
        continue;
        }
if(p == 0) p = newname - 1;
if((ext != 0) && (*ext != '\0'))
        {
        *p++ = '.';
        strcpy(p,ext);
        }
else *p = '\0';
}
