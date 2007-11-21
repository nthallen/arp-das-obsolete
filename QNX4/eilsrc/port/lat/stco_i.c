/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stco_i - convert octal to integer
*
* synopsis	count = stco_i(p,r);
*		int count;	number of characters scanned
*		char *p;	input string
*		int *r;		output integer
*
* description	This function performs an anchored scan of the input
*		string to convert a octal value into an integer.
*		The scan terminates when a non-octal character is
*		hit.  Valid octal characters are 0 to 7.  Also,
*		the first character can be + or -.
*
* returns	count = 0 if input string does not begin with an octal digit
*		      = number of characters scanned
*
**/
/* $Revision$ $Date$ */

#include <string.h>

stco_i(p,r)
char *p;
int *r;
{
int i,j,s;

i = 0;
j = s = 0;
if(*p == '-') j = s = 1;
else if(*p == '+') j = 1;
while((p[j] >= '0') && (p[j] < '8')) i = (i << 3) + (p[j++] - '0');
if(s) i = -i;
*r = i;
return(j);
}
