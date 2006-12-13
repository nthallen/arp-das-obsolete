/*
	Filter for das data flow system.
	Written Aug 1993 by Eil.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <eillib.h>
#include <globmsg.h>
#include <das.h>
#include <dbr.h>

/* defines */
#define HDR "fil"
#define OPT_MINE "tdm"
#define D 0x01
#define T 0x02
#define M 0x04

/* global variables */
char *opt_string=OPT_MSG_INIT OPT_MINE;
char flag = 0xFF;

main( int argc, char **argv) {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */	
int i;
char r;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;

    /* initialisations */

    /* process command line args */
    opterr = 0;
    optind = 0;
    do {
		i=getopt(argc,argv,opt_string);
		switch (i) {
			case 't': flag ^ T; break;
			case 'd': flag ^ D; break;
			case 'm': flag ^ M ;break;			
		    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		    default: break;
		}
    }  while (i!=-1);

    /* initialise as a data flow client */
    if (DC_init_options(argc,argv)!=0)
		msg(MSG_EXIT_ABNORM,"Can't DC initialise");

    /* main loop of command/data transmission around ring */
    DC_operate();    
}
