#ifndef _SYMNAME_H_INCLUDED
#define _SYMNAME_H_INCLUDED

#include <company.h>

char *symname( char *company, char *procname, int globalflag,char *env_varname);

#define LOC_OR_GLOB_SYMNAME(P,X) symname(COMPANY,(P),(X),"Experiment")
#define LOCAL_SYMNAME(P) symname(COMPANY,(P),0,"Experiment")
#define GLOBAL_SYMNAME(P) symname(COMPANY,(P),1,"Experiment")

#endif
