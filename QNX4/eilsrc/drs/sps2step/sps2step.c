#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include "attribut.h"
#include "snafucmd.h"

void stepout(char *ss_name, char *file_name);
void init_attrs(char *fname);
void start_dr_disp(void);
void end_dr_disp(void);

void main(int argc, char **argv) {

  if (argc != 3) {
    fprintf(stderr, "Command is SPS2STEP ssname  stepname\n");
    exit(-1);
  }

  init_windows();
  init_strvars();
  start_dr_disp();
  stepout(argv[1], argv[2]);
  end_dr_disp();

  end_windows();
}
