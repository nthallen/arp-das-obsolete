#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include "attribut.h"
#include "snafucmd.h"

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
 exit(-1);\
 }

void main(int argc, char **argv) {
  int i;
  if (argc<3) USE_MSG;
  for (i=1;i<argc;i++)
    if (argv[i][0]=='-')
      switch (argv[i][1]) {
      case 'a': no_aux_ask=1; break;
      case 'g': not_gmt=1; break;
      case 'm': nmon=1; break;
      case 'i':
	if (argv[i][2]=='\0')
	  if (i<argc)
	    nxwhich=atoi(argv[i+1]);
	  else USE_MSG
	    else nxwhich=atoi(argv[i]+2);
      }
  /*  init_attrs(CFG_FILE);
      if (initscr() == ERR) error(-1, "Cannot initscr()"); */
  init_windows();
  /*  cursor_off();
      noecho();
      nodelay(stdscr, TRUE);
      keypad(stdscr, TRUE); */
  start_dr_disp();
  stepin(argv[argc-2], argv[argc-1]);
  end_dr_disp();
  end_windows();
  /*  attrset(7);
      clear();
      refresh();
      endwin();
      cursor_on(); */
}
