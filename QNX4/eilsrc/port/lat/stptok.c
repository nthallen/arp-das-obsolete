/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name        stptok - get a token from a string
*
* synopsis      p = stptok(s,tok,toklen,brk);
*             char *p;            points to next char in s
*             char *s;            input string
*             char *tok;          output string
*             int toklen;         sizeof(tok)
*             char *brk;          break string
*
* description   This function breaks out the next token from the
*             input string.  The token consists of all characters
*             in s up to but not including the first character that
*             is in the break string.  Note that no white space
*             skipping is done at the beginning of the input string.
*
* returns       The function returns a pointer to the next character
*             in the input string.  Also, the output string is the
*             null-terminated token.
*
**/
/* $Revision$ $Date$ */

#include <string.h>

char *stptok(s,tok,toklen,brk)
const char *s;
char *tok;
const char *brk;
int toklen;
{
int i,j;

for(i = 0;i < (toklen-1);i++)
        {
        if(s[i] == '\0') break;
        for(j = 0;brk[j] != '\0';j++) if(s[i] == brk[j]) break;
        if(brk[j] != '\0') break;
        tok[i] = s[i];
        }
tok[i] = '\0';
return((char *)(s+i));
}

