/* Data Acquisition System data flow system generator code - options handler */

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <msg.h>
#include <dfs.h>
#include <dg.h>

extern char *opt_string;

int DG_init_options(int argcc, char **argvv) {
extern char *optarg;
extern int optind, opterr, optopt;
int c,s,d;
module_type B;
char N[_POSIX_NAME_MAX];

#ifdef __QNX__
    B = DRG;
#else
	B = DBG;
#endif
    s = d = 0;
    opterr = 0;
    optind = 0;
    strcpy(N,DG_NAME);
    if (dbr_info.mod_type != 0) strcpy(N,DB_NAME);

    do {
	  c=getopt(argcc,argvv,opt_string);
	  switch (c) {
		case 'n': s = atoi(optarg); break;
#ifdef __QNX__
		case 'N': strncpy(N,optarg,_POSIX_NAME_MAX-1); break;
#endif
		case 'B': B = DBG; break;
		case 'j': d = atoi(optarg); break;
		case '?': msg(MSG_EXIT_ABNORM, "Invalid option -%c", optopt);
		default : break;
	  }
	} while (c != -1);
    opterr = 1;
    return(DG_init(s,d,B,N));
}

