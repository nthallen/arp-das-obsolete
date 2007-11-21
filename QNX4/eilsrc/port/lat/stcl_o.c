/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stcl_o -- convert unsigned long to octal string
*		stci_o -- convert unsigned int to octal string
*
* synopsis	length = stcl_o(out,in);
*		length = stci_o(out,ins);
*
*		int length;	actual output length
*		char *out;	output buffer pointer
*		long in;	input value
*		int ins;	input value
*
* description	These functions convert integers into octal strings.  The
*		input values are treated as unsigned.
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include <lat.h>

stcl_o(out,in)
char *out;
long in;
{
char buff[12];
int i;

i = 11;
buff[11] = '\0';

do	{
        buff[--i] = '0' + (in & 7);
        in >>= 3;
        in &= 0x1fffffff;
        }
while (in != 0);

strcpy(out,&buff[i]);
return(11-i);
}

stci_o(out,in)
char *out;
int in;
{
return(stcl_o(out,(unsigned long)in));
}
