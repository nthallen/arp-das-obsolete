/* Data Acquisition System data buffered ring generator code - options handler */

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <msg.h>
#include <dbr.h>

extern char *opt_string;

int DG_init_options(int argcc, char **argvv) {
extern char *optarg;
extern int optind, opterr, optopt;
char filename[FILENAME_MAX] = {'\0'};
int c,s;

    s = 0;
    opterr = 0;
    optind = 0;

    do {
	  c=getopt(argcc,argvv,opt_string);
	  switch (c) {
		case 'n': s = atoi(optarg); break;
		case '?':
		  msg(MSG_EXIT_ABNORM, "Invalid option -%c", optopt);
		default : break;
	  }
	} while (c != -1);
    opterr = 1;
    return(DG_init(s));
}

