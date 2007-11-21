/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stcu_d - convert unsigned integer to ASCII string
*
* synopsis	length = stcu_d(out,in);
*		int length;	output string length (excluding null)
*		char out[];	output string
*		unsigned in;	input value
*
* description	This function converts an unsigned integer into a string
*		of ASCII characters terminated with a null character.
*		Leading zeroes are not copied to the output string, and
*		if the input value is 0, only a single '0' character
*		is produced.
*
* returns	length = number of characters in output string, not
*		including the null character
*
**/
/* $Revision$ $Date$ */

#include <string.h>

#define C_UI 11		/* # of chars in unsigned integer conversion */

stcu_d(out,in)
char *out;
unsigned in;
{
int i,j;
char work[C_UI];

*out = '\0';
i = C_UI;
do {
   work[--i] = '0' + in%10;
   in /= 10;
   }
while (in != 0);
for (j=0; i<C_UI; j++)
   out[j] = work[i++];
out[j] = '\0';
return(j);
}
