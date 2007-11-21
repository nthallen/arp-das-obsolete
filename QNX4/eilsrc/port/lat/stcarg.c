/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name          stcarg - get an argument
*
* synopsis      length = stcarg(s,b);
*               int length;         number of bytes in argument
*               char *s;            text string pointer
*               char *b;            break string pointer
*
* description   This function scans the text string until one of the
*               break characters is reached or until the text string
*               ends (as indicated by a null character).  While
*               scanning, the function skips over partial strings
*               enclosed in single or double quotes.  Also, the
*               backslash is used as an escape character.
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include <lat.h>

stcarg(s,b)
const char *s,*b;
{
const char *sx;
int q;

sx = s;
q = 0;
while(*s != '\0')
{
if(q == 0) if((*s == '\'') || (*s == '\"'))
      {
      q = *s++;
      continue;
      }
if(*s == '\\')
      {
      if(*++s != '\0') s++;
      continue;
      }
if(q)
      {
      if(*s++ == q) q = 0;
      continue;
      }
if(stpchr(b,*s) != 0) break;
s++;
}
return(q = s-sx);
}

