/*
    DBR Data Generator QNX messages in.
    Written Aug 1993 by Eil for QNX 4.
*/

/* includes */
#include <stdlib.h>
#include <globmsg.h>
#include <das.h>
#include <dfs.h>
#include <dg.h>
#include <eillib.h>

/* defines */
#define HDR "min"
#define OPT_MINE

/* global variables */
char *opt_string=OPT_DG_INIT OPT_DG_DAC_IN OPT_MSG_INIT OPT_CC_INIT OPT_MINE;
char *buf;

main( int argc, char **argv )  {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */		
int i;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
	cc_init_options(argc,argv, DCT_TM, DCT_TM, 0, 0, FORWARD_QUIT);

    /* process command line args */
    opterr = 0;
    optind = 0;
    do {
		i=getopt(argc,argv,opt_string);
		switch (i) {
		    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		    default : break;
		}
    }  while (i!= -1);

    /* read relevant information */
    if (DG_dac_in(argc,argv) !=0)
		msg(MSG_EXIT_ABNORM,"Can't dac initialise");

	if ((buf=malloc(sizeof(msg_hdr_type)+sizeof(token_type)+dbr_info.max_rows*tmi(nbrow)))==NULL)
		msg(MSG_EXIT_ABNORM,"Can't allocate memory for msg buffer");
	buf[0]=DEATH;

    /* initialise data generator */
    if (DG_init_options(argc,argv) != 0)
		msg(MSG_EXIT_ABNORM,"Can't DG initialise");

    DG_operate();
}
	
