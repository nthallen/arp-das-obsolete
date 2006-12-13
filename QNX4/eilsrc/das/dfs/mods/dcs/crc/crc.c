/*
*/

#include <stdlib.h>
#include <unistd.h>
#include <eillib.h>
#include <das.h>
#include <dfs.h>
#include <dc.h>

/* defines */
#define HDR "crc"
#define OPT_MINE ""

/* global variables */
char *opt_string=OPT_DC_INIT OPT_MSG_INIT OPT_CC_INIT OPT_MINE;

main( int argc, char **argv) {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */	
int i,j;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
    cc_init_options(argc,argv,0,0,0,0,NOTHING_ON_QUIT);

    /* initialisations */

    /* process command line args */
    opterr = 0;
    optind = 0;
    do {
		i=getopt(argc,argv,opt_string);
		switch (i) {
		    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		    default: break;
		}
    }  while (i!=-1);

    /* initialise as a client */
    if (DC_init_options(argc,argv)!=0)
		msg(MSG_EXIT_ABNORM,"Can't DC initialise");

    /* main loop of command/data transmission around ring */
    DC_operate();
}
