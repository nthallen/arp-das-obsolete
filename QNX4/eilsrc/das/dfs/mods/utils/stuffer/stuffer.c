/*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/kernel.h>
#include <eillib.h>
#include <das.h>
#include <dbr.h>

/* defines */
#define HDR "stuff"
#define OPT_MINE "b:"

/* global variables */
char *opt_string=OPT_MSG_INIT OPT_MINE;

main( int argc, char **argv) {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */	
int i;
char r;
pid_t p;
nid_t node;
char *buf;

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
			case 'b': node=atoi(optarg); break;
		    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		    default: break;
		}
    }  while (i!=-1);

	if ((p=qnx_name_locate(node, COMPANY "/" DG_NAME, 0, 0)) == -1)
	    msg(MSG_EXIT_ABNORM,"Can't find name %s on node %d",COMPANY "/" DG_NAME, node);    

	do {
	    stream_rd(buf, stdin, tmi(nbrow));
		switch (buf[0]) {
			case DATA:
						i=sizeof(msg_hdr_type)+sizeof(token_type)+buf[1]*tmi(nbrow);
						break;
			case STAMP:
						i=sizeof(tstamp_type)+sizeof(msg_hdr_type);
						break;
			case DASCMD:
						i=sizeof(msg_hdr_type)+sizeof(dascmd_type);
						break;
		}
		while (Send(p,buf,&r,i,sizeof(reply_type))==-1)
			if(errno!=EINTR) msg(MSG_EXIT_ABNORM,"Can't send to task %d on node %d",p,node);
	}
	while (buf[0] !=DASCMD && buf[1] != DCV_QUIT && buf[2] != DCT_QUIT);
}
