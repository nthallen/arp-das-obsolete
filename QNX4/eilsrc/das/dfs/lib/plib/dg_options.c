/* Data Acquisition System data flow system generator code - options handler */

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <msg.h>
#include <dfs.h>
#include <dg.h>
#include <ipc.h>

extern char *opt_string;

int DG_init_options(int argcc, char **argvv) {
extern char *optarg;
extern int optind, opterr, optopt;
int c,s,d;
module_type B = 0;

	s = -1;
    d = 0;
    B |= DG;
    
    B |= ipc_init_options(argcc, argvv);
    
    opterr = 0;
    optind = 0;

    do {
	  c=getopt(argcc,argvv,opt_string);
	  switch (c) {
		case 'n': s = atoi(optarg); break;
		case 'j': d = atoi(optarg); break;
		case 'z': DG_rows_requested = (unsigned)atoi(optarg); break;
		case 'x': B |= DSG; break;
		case '?': msg(MSG_EXIT_ABNORM, "Invalid option -%c", optopt);
		default : break;
	  }
	} while (c != -1);
    opterr = 1;
    return(DG_init(s,d,B));
}

