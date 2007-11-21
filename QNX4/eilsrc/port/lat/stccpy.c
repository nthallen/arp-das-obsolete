/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stccpy - move one string to another
*
* synopsis	actual = stccpy(to,from,length);
*		int actual;         actual number of characters moved
*		char *to;           destination string
*		char *from;         source string
*		int length;         sizeof(to)
*
* description	This function moves the null-terminated source string
*		to the destination string.  If the source is too long,
*		its rightmost characters are not moved.  The destination
*		string is always null-terminated.
*
* returns	actual = the actual number of characters moved, including
*			 the null terminator
*
**/
/* $Revision$ $Date$ */

#include <string.h>

stccpy(to,from,length)
char *to;
const char *from;
int length;
{
int i;

for(i = 0;i < (length-1);i++) if((to[i] = from[i]) == '\0') break;
if (i == (length-1)) to[i] = '\0';
return(++i);
}
