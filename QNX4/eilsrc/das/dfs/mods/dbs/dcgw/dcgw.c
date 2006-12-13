/*
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <malloc.h>
#include <limits.h>
#include <globmsg.h>
#include <cmdctrl.h>
#include <das.h>
#include <eillib.h>
#include <gw.h>
#include <dfs_mod.h>

/* defines */
#define HDR "dcgw"
#define OPT_MINE ""

/* **************** */
/* global variables */
/* **************** */
dfs_msg_type *bfr;

/* command line option string */
char *opt_string=OPT_DC_INIT OPT_MSG_INIT OPT_CC_INIT OPT_MINE;

main( int argc, char **argv) {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */
int  i;

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
		    default : break;
		}
    }  while (i!=-1);

	GW_init_options(argc, argv);

    if ( !(bfr = (dfs_msg_type *)malloc(sizeof(dfs_msg_type)+dbr_info.max_rows*tmi(nbrow))))
    	msg(MSG_EXIT_ABNORM,"Can't allocate space");
    bfr->msg_type = DEATH;

    /* main loop of a client */
    GW_operate();
}
