/* Data Acquisition System data flow client code - options handler */

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <dfs.h>
#include <dc.h>
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
nid_t node = 0;
char aname[_POSIX_NAME_MAX+1]={'\0'};
int c;
int B=0;

    opterr = 0;
    optind = 0;

	do {
	c=getopt(argcc,argvv,opt_string);
	switch (c) {
		case 'b': if (!strlen(aname)) strcpy(aname,DB_NAME); node=(nid_t)atoi(optarg); break;
		case 'B': B=1; break;
		case 'N': strncpy(aname,optarg,_POSIX_NAME_MAX); break;
		case 'i': DC_data_rows=(unsigned)atoi(optarg); break;
		case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		default : break;
	}
    }  while (c!=-1);
    opterr = 1;
    if (!B && !strlen(aname)) strcpy(aname,DG_NAME);
    return(DC_init(node,aname));
}
