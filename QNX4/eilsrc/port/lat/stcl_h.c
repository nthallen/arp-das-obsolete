/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stcl_h -- convert unsigned long to hexadecimal string
*		stci_h -- convert unsigned int to hexadecimal string
*
* synopsis	length = stcl_h(out,in);
*		length = stci_h(out,ins);
*
*		int length;	actual output length
*		char *out;	output buffer pointer
*		long in;	input value
*		int ins;	input value
*
* description	These functions convert integers to hexadecimal strings.
*		The input values are considered to be unsigned.
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include <lat.h>

stcl_h(out,in)
char *out;
long in;
{
int i;
char buff[9];
static const char hex[17] = "0123456789abcdef";

buff[8] = '\0';
i = 8;
do 	{
        buff[--i] = hex[in&15];
        in >>= 4;
        in &= 0xfffffff;
        }
while (in != 0);

strcpy(out,&buff[i]);
return(8-i);
}

stci_h(out,in)
char *out;
int in;
{
long l;

l = (long)in & 0x0000ffffL;
return(stcl_h(out,l));
}
