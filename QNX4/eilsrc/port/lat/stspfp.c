/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name          stspfp - parse file pattern
*
* synopsis      error = stspfp(p,n);
*               int error           -1 if error
*               char *p                   pattern string
*               int n[16]           node index array
*
* description   This function parses a file name pattern, which consists
*               of node names separated by _SLASH.  Each slash is
*               replaced by a null byte, and the beginning index of that
*               node is placed in the index array.  For example, the
*               pattern "/abc/de/f" has three nodes, and their indexes
*               are 1 for abc, 5 for de, and 8 for f.  Note that the
*               leading slash, if present, is skipped.
*
* returns       error = 0 if successful
*                     = -1 if too many nodes or other error
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include <ctype.h>

extern char _SLASH;

stspfp(p,n)
char *p;
int *n;
{
int nx,px,c;

px = 0;
if(*p == _SLASH) px++;
n[0] = px;
for(nx = 1;(c = p[px]) != '\0';px++)
{
if(c == _SLASH)
      {
      p[px] = '\0';
      if(p[px+1] == '\0') return(-1);
      n[nx++] = px+1;
      if(nx == 16) return(-1);
      continue;
      }
if(iscntrl(c)) return(-1);
}
n[nx] = -1;
return(0);
}
