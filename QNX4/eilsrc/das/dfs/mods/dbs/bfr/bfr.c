/*
    bfr main module.
    Written by Eil for QNX 4 4/28/92.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/types.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <globmsg.h>
#include <cmdctrl.h>
#include <das_utils.h>
#include <dbr_utils.h>
#include <mod_utils.h>
#include "bfr.h"

/* defines */
#define HDR "bfr"
#define OPT_MINE "z:"

/* global variables */
char *opt_string=OPT_DC_INIT OPT_MSG_INIT OPT_BREAK_INIT OPT_CC_INIT OPT_MINE;
arr clients[MAX_STAR_CLIENTS];	/* clients array */
llist *list;			/* list of current commands and stamps */
llist *t_list;			/* tail of list of commands stamps */
unsigned int bfr_sz_bytes;	/* buffer size in bytes */
int bfr_sz_rows;		/* buffer size in rows */
int startrow;			/* row of oldest data */
int putrow;			/* row of where to put incoming data */
unsigned int stamps_and_cmds;	/* number of stamps and cmds outstanding */
unsigned char *bfr;		/* THE buffer */
unsigned long data_seq;		/* the starting data sequence number */
unsigned long t_data_seq;	/* next data sequence number to be assigned */
unsigned long list_seq;		/* the starting stamp/cmd sequence number */
unsigned long t_list_seq;	/* next stamp/cmd sequence number for assignment */
unsigned char requests;		/* number of outstanding requests */
unsigned long list_seen;	/* all clients seen stamps/cmds up to but not including this number */
unsigned int n_clients;		/* number of clients */
unsigned char got_quit;		/* whether received DASCmd QUIT */

/* function declarations */


main( int argc, char **argv) {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */
int  i;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
    break_init_options(argc,argv);
    cc_init_options(argc,argv,0,0,0,0,NOTHING_ON_QUIT);

    /* initialisations */
    bfr_sz_bytes=DEF_BFR_SIZE;
    data_seq=t_data_seq=0;
    list_seq=t_list_seq=0;
    list=t_list=0;
    list_seen=0;
    n_clients=0;
    requests=0;
    stamps_and_cmds=0;
    startrow=putrow=0;
    got_quit=0;
    DB_init();

    /* process command line args */
    opterr = 0;
    do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
	    case 'z': bfr_sz_bytes=atol(optarg);
		  if (strpbrk(optarg,"kK")) bfr_sz_bytes*=K;
		  break;
	    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
	    default : break;
	}
    }  while (i!=-1);

    /* initialise into DBR */
    if (DC_init_options(argc,argv) != 0) 
	msg(MSG_EXIT_ABNORM,"Can't initialise into DBR");

    /* check buffer size */
    if (bfr_sz_bytes < (dbr_info.max_rows*tmi(nbrow)+tmi(nbminf)) ||
			bfr_sz_bytes > MAX_BFR_SIZE) {
	msg(MSG_WARN,"can't have buffer size %ld, defaulted to %ld",bfr_sz_bytes,DEF_BFR_SIZE);
	bfr_sz_bytes=DEF_BFR_SIZE;
    }

    bfr_sz_bytes-=(bfr_sz_bytes%tmi(nbminf));
    bfr_sz_rows=bfr_sz_bytes/tmi(nbrow);
    
    /* create buffer */
    if (!(bfr=malloc((size_t)bfr_sz_bytes)))
	msg(MSG_EXIT_ABNORM,"can't allocate %ld bytes for buffer",bfr_sz_bytes);

    /* main loop of command/data transmission around ring */
    DC_operate();

    /* wait for my star clients to finish */
    if (n_clients) DC_operate();

    DONE_MSG;
}
