/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stci_d - convert integer to ASCII string
*
* synopsis	length = stci_d(out,in,outlen);
*		int length;	output string length (excluding null)
*		char out[];	output string
*		int in;		input value
*
* description	This function converts an integer into a string of ASCII
*		characters terminated with a null character.  If the
*		integer is negative, the output string is preceded by
*		a '-'.  Leading zeroes are not copied to the output string.
*
* returns	length = number of characters in output string, not
*		including the null character
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include <lat.h>

stci_d(out,in)
char *out;
int in;
{
int i;

i = 0;
*out = '\0';
if(in<0)
        {
        *out = '-';
        in = -in;
        i = 1;
        }
return(i+stcu_d(&out[i],in));
}
