/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stpbrk - find break character in a string
*		strpbrk - same as stpbrk
*
* synopsis	p = stpbrk(s,b);
*		p = strpbrk(s,b);
*		char *p;            points to element of b in s
*		char *s;            points to string being scanned
*		char *b;            points to break character string
*
* description	This function scans the specified string to find the first
*		occurrence of a character from the break string b.  If the
*		terminator byte for s is hit first, a NULL pointer is
*		returned.
*
* returns	p = NULL if no element of b found in s
*		  = pointer to first element of b in s (from left)
*
**/
/* $Revision$ $Date$ */

#include <string.h>

char *stpbrk(s,b)
const char *s;
const char *b;
{
const char *bt;

for(;*s != '\0';s++)
for(bt = b;*bt != '\0';bt++) if(*bt == *s) return((char *)s);
return(NULL);
}

char *strpbrk(s,b)
const char *s,*b;
{
return(stpbrk(s,b));
}


