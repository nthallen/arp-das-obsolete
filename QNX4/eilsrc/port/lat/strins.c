/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		strins -- insert a string in front of another
*
* synopsis	strins(to,from);
*		char *from;	source string
*		char *to;	destination string
*
* description	This function inserts the source string in front of the
*		destination, pushing the destination up in memory.
*
**/
/* $Revision$ $Date$ */

#include <string.h>

void strins(to,from)
const char *from;
char *to;
{
int len;

len = strlen(from);
memmove(to+len,to,strlen(to)+1);
memmove(to,from,len);
}

