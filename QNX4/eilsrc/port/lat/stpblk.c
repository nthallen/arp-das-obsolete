/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name        stpblk - skip blanks
*
* synopsis      q = *stpblk(p);
*             char *q;      re-positioned string pointer
*             char *p;      string pointer;
*
* description   This function advances the string pointer past white
*             space characters.
*
**/
/* $Revision$ $Date$ */

#include <ctype.h>
#include <string.h>

char *stpblk(p)
const char *p;
{
while(isspace(*p)) p++;
return((char *)p);
}
