/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		strbpl - build a list of string pointers
*
* synopsis	n = strbpl(s,max,t);
*		char *s[];		pointer array
*		int n;			number of pointers
*		int max;		maximum number of pointers
*		char *t;		null-terminated strings
*
* description	The strbpl function scans through a set of null-terminated
*		strings and builds a list of pointers.  That is, the s
*		array will contain a pointer for each string in the t array.
*		Also, s will be terminated with a null pointer, and the
*		function returns the number of pointers (not including the
*		null pointer).  If s is not large enough, strbpl returns -1.
*
*		See also the dirfnl and strsrt functions.
*
**/
/* $Revision$ $Date$ */

#include <string.h>

strbpl(s,max,t)
char **s;
const char *t;
int max;
{
const char **ls = s;
int i;

if(--max < 0) return(-1);
for(i=0; (i < max) && (*t != '\0'); i++)
        {
        ls[i] = t;
        while(*t++);
        }
if(i >= max) return(-1);
ls[i] = NULL;
return(i);
}

