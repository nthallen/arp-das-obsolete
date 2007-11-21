/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stcd_l - convert decimal to long integer
*
* synopsis	count = stcd_l(p,r);
*		int count;	number of characters scanned
*		char *p;	input string
*		long *r;	output integer
*
* description	This function performs an anchored scan of the input
*		string to convert a decimal value into an integer.
*		The scan terminates when a non-decimal character is
*		hit.  Valid decimal characters are 0 to 9.  Also,
*		the first character can be + or -.
*
* returns	count = 0 if input string does not begin with a decimal digit
*		      = number of characters scanned
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include <ctype.h>

stcd_l(p,r)
const char *p;
long *r;
{
long i;
int j,s;

i = 0;
j = s = 0;
if(*p == '-') j = s = 1;
else if(*p == '+') j = 1;
while(isdigit(p[j])) i = i*10 + (p[j++]-'0');
if(s) i = -i;
*r = i;
return(j);
}
