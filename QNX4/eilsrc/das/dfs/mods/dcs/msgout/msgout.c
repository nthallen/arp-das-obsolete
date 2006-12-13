/*
    Data client Ring Data/Stamps/Cmds to qnx msgs.
*/

#include <stdlib.h>
#include <unistd.h>
#include <eillib.h>
#include <das.h>
#include <dc.h>

/* defines */
#define HDR "mout"
#define OPT_MINE "m:"

/* global variables */
char *opt_string=OPT_DC_INIT OPT_MSG_INIT OPT_CC_INIT OPT_MINE;
pid_t p;

main( int argc, char **argv) {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */	
int i;
nid_t node;

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
			case 'm': node=atoi(optarg); break;		    
		    default: break;
		}
    }  while (i!=-1);

	if ((p=qnx_name_locate(node, GLOBAL_SYMNAME(DG_NAME), 0, 0)) == -1)
	    msg(MSG_EXIT_ABNORM,"Can't find symbolic name for %s on node %d",DG_NAME, node);

    /* initialise as client */
    if (DC_init_options(argc,argv)!=0)
		msg(MSG_EXIT_ABNORM,"Can't DC initialise");

    /* main loop of command/data transmission around ring */
    DC_operate();
}
