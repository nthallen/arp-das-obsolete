/* Data Acquisition System data flow system generator code - options handler */

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"
#include "dfs.h"
#include "dg.h"

extern char *opt_string;

int DG_init_options(int argcc, char **argvv) {
  extern char *optarg;
  extern int optind, opterr, optopt;
  topology_type top=RING;
  int c,s,d;

  s = -1;
  d = 0;
    
  opterr = 0;
  optind = 0;

  do {
    c=getopt(argcc,argvv,opt_string);
    switch (c) {
    case 'n': s = atoi(optarg); break;
    case 'j': d = atoi(optarg); break; /* keep this now,change to reg */
/*    case 'z': DG_rows_requested = (unsigned)atoi(optarg); break; */
    case 'B': top = STAR; break;
    case 'U': top = BUS; break;
    case '?': msg(MSG_EXIT_ABNORM, "Invalid option -%c", optopt);
      default : break;
    }
  } while (c != -1);
  opterr = 1;
  return(DG_init(s,d,top));
}
