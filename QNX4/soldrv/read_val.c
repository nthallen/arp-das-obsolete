/* read_val.c reads valving or set point characters.
   Written March 24, 1987
   Modified July 1991 for QNX.
*/
#include <stdio.h>
#include <ctype.h>
#include "solenoid.h"
#include "dtoa.h"
#include "tokens.h"

int open_char = 'O';
int close_char = '_';
int switch_char = '^';

int get_change_code(int type, int dtoa_num) {
  int c, i;

  for (;;) {
    c = gt_getc();
    if (c == '\n' || c == EOF) return(-1);
    if (c == switch_char) return(MODE_SWITCH_OK);
    if (type == TK_SOLENOID_NAME) {
      if (c == open_char) return(SOL_OPEN);
      if (c == close_char) return(SOL_CLOSE);
    } else if (!isspace(c))
      for (i = dtoas[dtoa_num].n_set_points - 1; i >= 0; i--)
        if (c == dtoas[dtoa_num].set_point_name[i])
          return(i);
  }
}
