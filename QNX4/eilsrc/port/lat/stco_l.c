/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stco_l - convert octal to long integer
*
* synopsis	count = stco_l(p,r);
*		int count;	number of characters scanned
*		char *p;	input string
*		long *r;		output integer
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
#include <ctype.h>

stco_l(p,r)
char *p;
long *r;
{
long i;
int j,s;

i = 0;
j = s = 0;
if(*p == '-') j = s = 1;
else if(*p == '+') j = 1;
while((p[j] >= '0') && (p[j] < '8')) i = (i << 3) + (p[j++] - '0');
if(s) i = -i;
*r = i;
return(j);
}
