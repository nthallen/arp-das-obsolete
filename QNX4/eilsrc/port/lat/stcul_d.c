/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stcul_d -- convert unsigned long to string
*
* synopsis	length = stcul_d(out, in);
*		int length;	output length
*		char *out;	destination string
*		long in;	value to be formatted
*
* description	This function is called by the various formatted print
*		routines whenever an unsigned format descriptor is
*		encountered.
*
* returns	actual output length
*
**/
/* $Revision$ $Date$ */

#include <string.h>

stcul_d(out,in)
char *out;
unsigned long in;
{
char buff[12];
int i;

i = 11;
buff[11] = '\0';
do 	{
        buff[--i] = '0' + (in % 10);
        in /= 10;
        }
while (in != 0);
strcpy(out,&buff[i]);
return(11-i);
}
