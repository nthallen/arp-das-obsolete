#ifndef _SYMNAME_H_INCLUDED
#define _SYMNAME_H_INCLUDED

#include <company.h>

char *symname( char *company, char *procname, int globalflag, char *dst);

#define LOC_OR_GLOB_SYMNAME(P,D,X) symname(COMPANY,(P),(X),(D))
#define LOCAL_SYMNAME(P,D) symname(COMPANY,(P),0,(D))
#define GLOBAL_SYMNAME(P,D) symname(COMPANY,(P),1,(D))

#endif
