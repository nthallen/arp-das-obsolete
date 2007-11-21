/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		strmfn -- make a file name
*
* synopsis	strmfn(file,drive,dir,name,ext);
*		char *file;	file name pointer
*		char *drive;	drive code pointer
*		char *dir;	directory path pointer
*		char *name;	node name pointer
*		char *ext;	extension pointer
*
* description	This function makes a complete file name from its components.
*		If any of the drive, dir, name, or ext pointers is NULL, that
*		component is not used.  If the drive code does not end with
*		a colon, one is supplied.  Similarly, if the directory path
*		string does not end with a slash, backslash, or colon, a
*		SLASH is supplied.  A period is always placed before the
*		extension, if that string is not null.
*
**/

#include <stdio.h>
#include <string.h>
#include <lat.h>

extern char _SLASH;

void strmfn(file,drive,dir,name,ext)
char *file;
const char *drive;
const char *dir;
const char *name;
const char *ext;
{
char *p;
char *q;

p = file;

/* if(drive != NULL)
        {
        p = strcpy(p,drive);
        p += strlen(p);
        if(p != file) if(p[-1] != ':')
                {
                *p++ = ':';
                *p = '\0';
                }
        }
*/

if(dir != NULL)
        {
        q = strcpy(p,dir);
        q += strlen(p);        
        if(q != p) switch(q[-1])
                {
/*                case ':':*/
                case '/':
/*                case '\\':*/
                break;

                default:
                *q++ = '/';
                *q = '\0';
                }
        p = q;
        }


if(name != NULL) {
		p = strcpy(p,name);
        p += strlen(p);
}		

if(ext != NULL)
        {
        *p++ = '.';
        strcpy(p,ext);
        }
}

