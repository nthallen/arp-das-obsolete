/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name          stcpm - pattern match (unanchored)
*
* synopsis      length = stcpm(s,p,q);
*               int length;         length of match
*               char *s;            string being scanned
*               char *p;            pattern string
*               char **q;           points to matched string if successful
*
* description   This function scans the specified string to find the
*               first substring that matches the specified pattern.
*
* returns       length = 0 if no match
*                    = length of matching substring
*               *q = pointer to substring if successful
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include <lat.h>

stcpm(s,p,q)
const char *s,*p;
char **q;
{
int sx,ret;

for(sx=0;((ret=stcpma(&s[sx],p)) == 0) && (s[sx]!='\0');sx++)
	;
*q = (ret == 0)? NULL :(char *)&s[sx];
return(ret);
}

