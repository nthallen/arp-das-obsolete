/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		iabs - return integer absolute value
*
* synopsis	ai = iabs(i)
*		int ai;
*		int i;
*
* description	This function returns the absolute value of its integer
*		operand.
*
**/
/* $Revision$ $Date$ */

#include <stdlib.h>

iabs(i)
int i;
{
return( (i >= 0) ? i : -i);
}
