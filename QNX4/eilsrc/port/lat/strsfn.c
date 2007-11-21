/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		strsfn -- split a file name
*
* synopsis	strsfn(file,drive,dir,name,ext);
*		char *file;	file name pointer
*		char *drive;	drive code pointer
*		char *dir;	directory path pointer
*		char *name;	node name pointer
*		char *ext;	extension pointer
*
* description	This function splits a file name into its component parts.
*		If any of the drive, dir, name, or ext pointers is NULL, that
*		part is discarded.  The colon is left on the drive code, but
*		the terminating colon is removed from the directory path,
*		and the period is removed from the front of the extension.
*
**/

#include <stdio.h>
#include <string.h>

void strsfn(file,drive,dir,name,ext)
const char *file;
char *drive;
char *dir;
char *name;
char *ext;
{
int i,x,y;
char b[64];

strncpy(b,file,63);
b[63] = '\0';
i = strlen(b);
if(drive != NULL) *drive = '\0';
if(dir != NULL) *dir = '\0';
if(name != NULL) *name = '\0';
if(ext != NULL) *ext = '\0';

for(x = i; x-- > 0; )
        {
        switch(b[x])
                {
                case '.':
                if(ext != NULL) strcpy(ext,&b[x+1]);
                b[x] = '\0';
                break;

/*                case ':':*/
                case '/':
/*                case '\\':*/
                x = i;
                break;

                default:
                continue;
                }
        break;
        }

while(x-- > 0)
        {
        switch(b[x])
                {
/*                case ':':*/
                case '/':
/*                case '\\':*/
                if(name != NULL) strcpy(name,&b[x+1]);
                b[++x] = '\0';
                break;

                default:
                continue;
                }
        break;
        }
if(x < 0)
        {
        if(name != NULL) strcpy(name,b);
        return;
        }

y = x - 1;
while(x-- > 0) switch(b[x])
        {
/*        case ':':
        if(dir != NULL)
                {
                if(((y - x) > 1) && ((b[y] == '\\') || (b[y] == '/')))
                         b[y] = '\0';
                strcpy(dir,&b[x+1]);
                }
        b[x+1] = '\0';
        if(drive != NULL) strcpy(drive,b);
	    return;
*/
        default:
        continue;
        }
if(dir != NULL)
        {
        if((y > 0) &&  (b[y] == '/')) b[y] = '\0';
        strcpy(dir,b);
        }
}
