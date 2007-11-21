/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stcl_d -- convert long to string
*
* synopsis	length = stcl_d(out,in);
*		int length;	output length
*		char *out;	destination string
*		long in;	value to be formatted
*
* description	This function is called by the various formatted print
*		routines whenever an integer format descriptor is
*		encountered.
*
* returns	actual output length
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include <lat.h>

stcl_d(out,in)
char *out;
long in;
{
char *p;

p = out;
if(in < 0)
        {
        *p++ = '-';
        in = -in;
        }
return(stcul_d(p,in) + (p - out));
}
