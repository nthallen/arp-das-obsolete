/*
    DBR Data Generator Serial in main common module.
    Written Sep 1992 by Eil for QNX 4.
*/

/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "globmsg.h"
#include "das.h"
#include "dbr.h"
#include "dbr_mod.h"
#include "eillib.h"

/* defines */
#define HDR "demo: dg"
#define OPT_MINE ""

/* global variables */
char *opt_string=OPT_DG_INIT OPT_DG_DAC_IN OPT_MSG_INIT OPT_MINE;

main( int argc, char **argv )  {
  /* getopt variables */
  extern char *optarg;
  extern int optind, opterr, optopt;

  /* local variables */		
  int i;

  /* initialise msg options from command line */
  msg_init_options(HDR,argc,argv);
  BEGIN_MSG;

  /* initialisations */

  /* process command line args */
  opterr = 0;
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

  /* initialise data generator */
  if (DG_init_options(argc,argv) != 0)
    msg(MSG_EXIT_ABNORM,"Can't DG initialise");

  DG_operate();
    
  DONE_MSG;
}
