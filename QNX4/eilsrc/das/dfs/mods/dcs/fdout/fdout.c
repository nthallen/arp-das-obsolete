/*
    Data client Ring Data to Stdout
*/

#include <stdlib.h>
#include <unistd.h>
#include "eillib.h"
#include "das.h"
#include "dbr.h"

/* defines */
#define HDR "fout"
#define OPT_MINE ""

/* global variables */
char *opt_string=OPT_DC_INIT OPT_MSG_INIT OPT_CC_INIT OPT_SERIAL_INIT OPT_MINE;
int fd;

main( int argc, char **argv) {
  /* getopt variables */
  extern char *optarg;
  extern int optind, opterr, optopt;

  /* local variables */	
  int i;

  /* initialise msg options from command line */
  msg_init_options(HDR,argc,argv);
  BEGIN_MSG;
  cc_init_options(argc,argv,0,0,0,0,NOTHING_ON_QUIT);

  /* initialisations */
  fd = STDOUT_FILENO;

  /* process command line args */
  opterr = 0;
  optind = 0;
  do {
    i=getopt(argc,argv,opt_string);
    switch (i) {
    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
    default: break;
    }
  }  while (i!=-1);

  /* initialise into DRing */
  if (DC_init_options(argc,argv)!=0)
    msg(MSG_EXIT_ABNORM,"Can't DC initialise");

  /* main loop of command/data transmission around ring */
  DC_operate();
}
