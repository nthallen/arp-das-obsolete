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

void main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Command is STEP2SPS  [-a] [-g] [-m] filename  ssname\n");
    fprintf(stderr, "  -a    don't ask about auxillary variables if they exist\n");
    fprintf(stderr, "  -g    don't assume GMT time\n");
    fprintf(stderr, "  -m    independent variable not monitonically increasing\n");    
    exit(-1);
  }
/*  init_attrs(CFG_FILE);
  if (initscr() == ERR) error(-1, "Cannot initscr()"); */
  init_windows();
/*  cursor_off();
  noecho();
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE); */
  start_dr_disp();
  if (argc > 3) {
	if (!strcmp(argv[1],"-a")) no_aux_ask = 1;
	else if (!strcmp(argv[1],"-g")) not_gmt = 1;
	else if (!strcmp(argv[1],"-m")) nmon = 1;  			
  }
  if (argc > 4) {
	if (!strcmp(argv[2],"-a")) no_aux_ask = 1;
	else if (!strcmp(argv[2],"-g")) not_gmt = 1;
	else if (!strcmp(argv[2],"-m")) nmon = 1;  		  		
  }
  if (argc > 5) {
	if (!strcmp(argv[3],"-a")) no_aux_ask = 1;
	else if (!strcmp(argv[3],"-g")) not_gmt = 1;
	else if (!strcmp(argv[3],"-m")) nmon = 1;  		  		
  }  
  stepin(argv[argc-2], argv[argc-1]);
  end_dr_disp();
  end_windows();
/*  attrset(7);
  clear();
  refresh();
  endwin();
  cursor_on(); */
}
