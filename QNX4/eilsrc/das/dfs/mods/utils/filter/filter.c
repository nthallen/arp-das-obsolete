/*
	Stream filter for das stream data
	Reads from stdin, writes to stdout.
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
#define HDR "filt"
#define OPT_MINE "tdm"

/* global variables */
char *opt_string=OPT_MSG_INIT OPT_MINE;

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
			case 't': break;
			case 'd': break;
			case 'm': break;			
		    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		    default: break;
		}
    }  while (i!=-1);

	while (stream_rd(buf, stdin, tmi(nbrow)) {
		switch (buf[0]) {
			case DATA: i=sizeof(msg_hdr_type)+sizeof(token_type)+buf[1]*tmi(nbrow); break;
			case STAMP: i=sizeof(tstamp_type)+sizeof(msg_hdr_type); break;
			case DASCMD: i=sizeof(msg_hdr_type)+sizeof(dascmd_type); break;
		}
		while (Send(p,buf,&r,i,sizeof(reply_type))==-1)
			if(errno!=EINTR) msg(MSG_EXIT_ABNORM,"Can't send to task %d on node %d",p,node);
	}
}
