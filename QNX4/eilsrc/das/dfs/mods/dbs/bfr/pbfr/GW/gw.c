/*
    bfr main module.
    Written by Eil for QNX 4 4/28/92.
*/

#include <stdlib.h>
#include "das.h"
#include "dc.h"
#include "dg.h"
#include "eillib.h"
#include "dfs_mod.h"

/* defines */
#define HDR "gw"
#define OPT_MINE ""

/* **************** */
/* global variables */
/* **************** */
char *buf;
int bufrows;

/* command line option string */
char *opt_string=OPT_DC_INIT OPT_DG_INIT OPT_MSG_INIT OPT_CC_INIT OPT_MINE;

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


  if (DG_init_options(argc,argv) != 0)
    msg(MSG_EXIT_ABNORM,"Can't initialise as a Data Generator");

  if (DC_init_options(argc,argv) != 0) 
    msg(MSG_EXIT_ABNORM,"Can't initialise as a Data Client");

  bufrows = dbr_info.max_rows - (dbr_info.max_rows % dbr_info.nrowminf);
  buf = malloc(bufrows);

  /* main loop of a client */
  DC_operate();
}
