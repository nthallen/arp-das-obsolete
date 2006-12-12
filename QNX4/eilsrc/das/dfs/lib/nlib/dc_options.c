/* Data Acquisition System data buffered ring client code - options handler */

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dbr.h>
#include <msg.h>

extern char *opt_string;
extern token_type DC_data_rows;

/*
  Table of Contents:
  int DC_init_options(int argcc, char *argv[]);
*/

int DC_init_options(int argcc, char *argvv[]) {
/* initialse client parameters from command line args */
extern char *optarg;
extern int optind, opterr, optopt;
int ring = DRC, opt_x = 0;
nid_t node = 0;
int c;

    opterr = 0;
    optind = 0;

    do {
	c=getopt(argcc,argvv,opt_string);
	switch (c) {
		case 'b': ring=DBC; node=(nid_t)atoi(optarg); break;
		case 'i': DC_data_rows=(unsigned)atoi(optarg); break;
		case 'x': opt_x = 1; break;
		case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		default : break;
	}
    } while (c!=-1);
    opterr = 1;
    if ( ring == DBC && opt_x ) ring = DBCP;
    return(DC_init(ring, node));
}
