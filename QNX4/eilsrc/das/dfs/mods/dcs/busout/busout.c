/*
    Data client Ring Data/Stamps/Cmds to Stdout
*/

#include <stdlib.h>
#include <unistd.h>
#include <eillib.h>
#include <das.h>
#include <dfs.h>
#include <dc.h>

/* defines */
#define HDR "bout"
#define OPT_MINE "m"

/* global variables */
char *opt_string=OPT_DC_INIT OPT_MSG_INIT OPT_CC_INIT OPT_MINE;
int blocking=1;

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
			case 'm': blocking=0; break;
		    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		    default: break;
		}
    }  while (i!=-1);

    /* initialise as a client */
    if (DC_init_options(argc,argv)!=0)
		msg(MSG_EXIT_ABNORM,"Can't DC initialise");

	if (bus_write(STDOUT_FILENO,DCINIT,(char *)&dbr_info,sizeof(dbr_info_type))!=sizeof(dbr_info_type)+sizeof(msg_hdr_type))
		msg(MSG_EXIT_ABNORM,"Can't write all of dbr_info");
	if (!blocking) bus_nonblock(STDOUT_FILENO);

    /* main loop of command/data transmission around ring */
    DC_operate();
}
