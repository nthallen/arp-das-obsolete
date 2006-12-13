/*
    bfr main module.
    Written by Eil for QNX 4 4/28/92.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <string.h>
#include <malloc.h>
#include <limits.h>
#include <env.h>
#include "globmsg.h"
#include "das.h"
#include "dc.h"
#include "dg.h"
#include "eillib.h"
#include "dfs_mod.h"
#include "bfr.h"
#include "bfr_vars.h"

/* defines */
#define HDR "bfr"
#define OPT_MINE "z:E:"

/* **************** */
/* global variables */
/* **************** */

/* command line option string */
char *opt_string=OPT_DC_INIT OPT_DG_INIT OPT_MSG_INIT OPT_CC_INIT OPT_MINE;

/* data structures */
arr clients[MAX_STAR_CLIENTS];	/* clients array */
llist *list;			/* list of current commands and stamps */
llist *t_list;			/* tail of list of commands stamps */

/* buffer status vars: all integers */
int bfr_sz_bytes;		/* buffer size in bytes */
int bfr_sz_rows;		/* buffer size in rows */
int startrow;			/* row of oldest data */
int putrow;			/* row of where to put incoming data */
int stamps_and_cmds;		/* number of stamps and cmds outstanding */
int got_quit;			/* whether received DASCmd QUIT */
unsigned int n_clients;		/* number of clients */

unsigned char *bfr;		/* THE buffer */

/* sequence numbers: data rows and stamps/cmds: assume rollover wont occur */
unsigned long data_seq;		/* the starting data sequence number */
unsigned long t_data_seq;	/* next data sequence number to be assigned */
unsigned long list_seq;		/* the starting stamp/cmd sequence number */
unsigned long t_list_seq;	/* next stamp/cmd seq number for assignment */

llist *dc_cmd;

main( int argc, char **argv) {
  /* getopt variables */
  extern char *optarg;
  extern int optind, opterr, optopt;

  /* local variables */
  int  i;
  char envstr[50]={'\0'};
  char *e;

  assert(MAX_BFR_SIZE <= SHRT_MAX);

  /* initialise msg options from command line */
  msg_init_options(HDR,argc,argv);
  BEGIN_MSG;
  cc_init_options(argc,argv,0,0,0,0,NOTHING_ON_QUIT);

  /* initialisations */
  bfr_sz_bytes=DEF_BFR_SIZE;
  data_seq=t_data_seq=0;
  list_seq=t_list_seq=0;
  list=t_list=0;
  n_clients=0;
  stamps_and_cmds=0;
  startrow=putrow=0;
  got_quit=0;
  dc_cmd=NULL;

  /* process command line args */
  opterr = 0;
  optind = 0;
  do {
    i=getopt(argc,argv,opt_string);
    switch (i) {
    case 'z': bfr_sz_bytes=atoi(optarg);
      if (strpbrk(optarg,"kK")) bfr_sz_bytes*=K;
      break;
    case 'E': strncpy(envstr,optarg,49); break;
    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
      default : break;
    }
  }  while (i!=-1);


  if (DG_init_options(argc,argv) != 0)
    msg(MSG_EXIT_ABNORM,"Can't initialise as a Data Generator");

  if (strlen(envstr)) {
    if ((e=strchr(envstr,'='))!=NULL) {
      *e='\0';
      e=e+1;
      if (!strlen(e)) e=NULL;
    }
    if (setenv(envstr,e,1)==-1) msg(MSG_FATAL,"Can't change environment");
  }

  if (DC_init_options(argc,argv) != 0) 
    msg(MSG_EXIT_ABNORM,"Can't initialise as a Data Client");

  /* check buffer size */
  if (bfr_sz_bytes < (dbr_info.max_rows*tmi(nbrow)+tmi(nbminf)) ||
      bfr_sz_bytes > MAX_BFR_SIZE) {
    msg(MSG_WARN,"can't have buffer size %d, defaulted to %d",
	bfr_sz_bytes,DEF_BFR_SIZE);
    bfr_sz_bytes=DEF_BFR_SIZE;
  }

  bfr_sz_bytes-=(bfr_sz_bytes%tmi(nbminf));
  bfr_sz_rows=bfr_sz_bytes/tmi(nbrow);

  msg(MSG,"buffer size: %d rows (%d bytes)",bfr_sz_rows, bfr_sz_bytes);
    
  /* create buffer */
  if (!(bfr=malloc((size_t)bfr_sz_bytes)))
    msg(MSG_EXIT_ABNORM,"can't allocate %d bytes for buffer",bfr_sz_bytes);

  /* main loop of a client */
  DC_operate();

  /* handle remaining clients */
  if (breaksignal) DC_DASCmd(DCT_QUIT, DCV_QUIT);
  DC_init(0,0);
}
