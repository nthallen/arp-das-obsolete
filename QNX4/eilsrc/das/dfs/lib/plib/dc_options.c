/* Data Acquisition System data buffered ring client code - options handler */

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dfs.h>
#include <dc.h>
#include <msg.h>
#include <ipc.h>

extern char *opt_string;
extern token_type DC_data_rows;

/*
  Table of Contents:
  int DC_init_options(int argcc, char *argvv[]);
*/

int DC_init_options(int argcc, char *argvv[]) {
/* initialse client parameters from command line args */
extern char *optarg;
extern int optind, opterr, optopt;
module_type B = 0;
nid_t node = 0;
int c;

	B |= DC;
    B |= ipc_init_options(argcc, argvv);

    opterr = 0;
    optind = 0;

	do {
	c=getopt(argcc,argvv,opt_string);
	switch (c) {
		case 'b': B|=DSC; node=(nid_t)atoi(optarg); break;
		case 'i': DC_data_rows=(unsigned)atoi(optarg); break;
		case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		default : break;
	}
    }  while (c!=-1);
    opterr = 1;
    return(DC_init(B, node));
}
