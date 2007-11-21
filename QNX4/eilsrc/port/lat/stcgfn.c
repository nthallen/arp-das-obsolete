/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stcgfn -- isolate node portion of file name
*
* synopsis	size = stcgfn(node,name);
*		char *node;		node pointer
*		char *name;		name pointer
*
* description	This function scans the file name in reverse order to
*		isolate the node portion, as follows:
*
*		  NAME => "d:abc/def/ghi.jkl"
*		  NODE => "ghi.jkl"
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include <lat.h>

stcgfn(node,name)
char *node;
const char *name;
{
const char *p;
int x;

*node = '\0';
if(x = strlen(name)) for(p = name+x; x > 0; x--) switch(*--p)
        {
        case '/':
/*        case '\\':*/
/*        case ':':*/
        return(stccpy(node,p+1,FNSIZE)-1);

        default:
        continue;
        }
return((stccpy(node,name,FNSIZE)-1));
}
