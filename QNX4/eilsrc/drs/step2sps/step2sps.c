#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include "attribut.h"
#include "snafucmd.h"
#include "snafuerr.h"
#include "snfbatch.h"

void stepin(char *filename, char *ss_name);
void init_attrs(char *fname);
void start_dr_disp(void);
void end_dr_disp(void);
int no_aux_ask = 0;
int not_gmt = 0;
int nmon = 0;
int nxwhich = 0;

#define USE_MSG \
 {\
 fprintf(stderr, "Command is STEP2SPS  [-a] [-g] [-m] [-i<ind var>] filename ssname\n");\
 fprintf(stderr, "  -a    don't ask about auxillary variables if they exist\n");\
 fprintf(stderr, "  -g    don't assume GMT time\n");\
 fprintf(stderr, "  -m    independent variable not monitonically increasing\n");\
 fprintf(stderr, "  -i <independent var 1,2,...>	used for format 2010\n");\
 fprintf(stderr, "  -b[n]filename Specify SNAFU-style batch file\n");\
 fprintf(stderr, "  -k    Die if keyboard input is required\n");\
 exit(-1);\
 }

void main(int argc, char **argv) {
  int i;
  
  init_strvars();
  for (i=1;i<argc;i++) {
    if (argv[i][0]=='-') {
      switch (argv[i][1]) {
      case 'a': no_aux_ask=1; break;
      case 'g': not_gmt=1; break;
      case 'm': nmon=1; break;
      case 'i':
	if (argv[i][2]=='\0') {
	  if (i<argc) {
	    nxwhich=atoi(argv[++i]);
	  } else USE_MSG
	} else nxwhich=atoi(argv[i]+2);
      case 'b':
	{ char *cmd = argv[i]+2;
	  int mode, bret;
	  if ( isdigit(*cmd) ) mode = (*(cmd++)) - '0' + 1;
	  else mode = 2;
	  bret = read_batch( cmd, mode );
	  if ( bret ) {
	    fprintf( stderr, "Batch File Error %d\n", snafu );
	    exit(1);
	  }
	}
	break;
      case 'k':	define_var( "KBDdie", "y" ); break;
      default: USE_MSG
      }
    } else break; /* if */
  } /* for */
  if ( i+2 > argc ) USE_MSG;
  /*  init_attrs(CFG_FILE);
      if (initscr() == ERR) error(-1, "Cannot initscr()"); */
  init_windows();
  /*  cursor_off();
      noecho();
      nodelay(stdscr, TRUE);
      keypad(stdscr, TRUE); */
  start_dr_disp();
  stepin(argv[i], argv[i+1]);
  end_dr_disp();
  end_windows();
  /*  attrset(7);
      clear();
      refresh();
      endwin();
      cursor_on(); */
  exit(0);
}
