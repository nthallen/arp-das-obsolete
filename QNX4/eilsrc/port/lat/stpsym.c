/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name        stpsym - get a symbol from a string
*
* synopsis      p = stpsym(s,sym,symlen);
*             char *p;            points to next char in s
*             char *s;            input string
*             char *sym;          output string
*             int symlen;         sizeof(sym)
*
* description   This function breaks out the next symbol from the
*             input string.  The first character of a symbol must
*             be alphabetic, and the remaining characters must
*             be alphanumeric.  Note that no white space skipping
*             is done at the beginning of the input string.
*
* returns       The function returns a pointer to the next character
*             in the input string.  Also, the output string is the
*             null-terminated symbol.
*
**/
/* $Revision$ $Date$ */

#include <ctype.h>
#include <string.h>

char *stpsym(s,sym,symlen)
const char *s;
char *sym;
int symlen;
{
    char c;
    int i;

    i = 0;
    if(isalpha(c = s[0])) while(i < (symlen-1))
    {
        sym[i] = c;
        if(isalnum(c = s[++i]) == 0) break;
    }
    sym[i] = '\0';
    return((char *)(s+i));
}

