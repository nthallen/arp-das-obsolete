/**
*
**
* name		argopt - get options from arg list
*
* synopsis	ds = argopt(argc,argv,opts,argn,opt);
*		char *ds;		data string pointer
*		int argc;		arg count
*		char *argv[];		arg pointer list
*		char *opts;		options expecting data
*		char *opt;		option character (changed)
*		int *argn;		next arg number (used/changed)
*
* description	This function examines an argument list to find the next
*		option argument, which is simply an arg string that begins
*		with a dash (-) and appears before all non-option arguments.
*		If the first character of argv[*argn] is a dash, then the
*		next character is returned in opt unless it is a null or
*		another dash.  These latter two cases signify the end of the
*		options and the beginning of other types of arguments.
*
*		If an option character has been discovered, the function then
*		scans the opts string.  If the option character is in that
*		string, then the function looks for an associated data string.
*		This can be found in two places.  If the option was typed with
*		no space between the dash, the option character, and the data
*		string, then the data string begins at argv[*argn][2].
*		If there was a space between the option character and the data
*		string, the data string begins at argv[*argn+1], unless
*		*argn equals argc.
*
* returns	If no option was found, the function returns 0.  Otherwise,
*		it returns a pointer to either a null byte or to the
*		associated data string.
*
**/
/* $Revision$ $Date$ */

#include <stdlib.h>

char *argopt(argc,argv,opts,argn,opt)
int argc,*argn;
const char **argv;
const char *opts;
char *opt;
{
const char *p;
const char *q;

if(*argn >= argc) return(NULL);
p = argv[*argn];
if(*p != '-') return(NULL);
*argn += 1;
if((p[1] == '\0') || (p[1] == '-')) return(NULL);
*opt = p[1];
for(q=opts; *q; q++) if(*opt == *q)
        {
        if(p[2] || (*argn >= argc)) return((char *)(&p[2]));
        q = argv[*argn];
        if(*q == '-') return((char *)(&p[2]));
        *argn += 1;
        return((char *)q);
        }
return((char *)(&p[2]));
}

