/*
    constructs symbolic names.
*/

#include <string.h>
#include <symname.h>

char *symname( char *company, char *procname, int globalflag, char *dst) {

if (!dst) return(NULL);
if (globalflag) strcpy(dst,"/");
else strcpy(dst,"");
strcat(dst,company);
strcat(dst,"/");
strcat(dst,procname);
return(dst);
}
