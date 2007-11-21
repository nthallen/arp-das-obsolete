/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name 		strsrt - sort a list of string pointers
*
* synopsis 	strsrt(s,n);
*		char *s[];		pointer array
*		int n;			number of pointers
*
* description	The strsrt function sorts an array of pointers to strings
*		in ascending alphnumeric order.  See also strbpl.
*
**/
/* $Revision$ $Date$ */

#include <string.h>

void strsrt(s,n)
char **s;
int n;
{
int i,j;
char seq, *p, *q;

if(n < 2) return;
for(i=n-1; i>0; i--)
        {
        seq = 0;
        for(j=0; j<i; j++) if(strcmp(p=s[j],q=s[j+1]) > 0)
                {
                s[j+1] = p;
                s[j] = q;
                seq = 1;		
                }
        if(seq == 0) break;
        }
}
