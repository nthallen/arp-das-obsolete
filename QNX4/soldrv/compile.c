/* compile.c takes the information in solenoids and modes and compiles it
   into a numerical code.
   Written March 25, 1987
   Modified April 10, 1987 for proper optimization.
   Modified July 1991 for QNX.
   July 1991: dosn't compile multiple SOL_STROBES and SOL_DTOA commands.
*/
#include <stdio.h>
#include <assert.h>
#include "tokens.h"
#include "modes.h"
#include "solenoid.h"
#include "codes.h"
#include "dtoa.h"
#include "solfmt.h"


#define MODE_CODE_SIZE 10000
char mode_code[MODE_CODE_SIZE];
int mci = 0;    /* index into mode_code */
int verbose = 0;

void new_mode_code(int x) {
  if (mci == MODE_CODE_SIZE) error("Mode Code Table exceeded\n");
  mode_code[mci++] = x;
}

void compile(void) {
  int i, time, cycle_index;
  change *ch;

  if (verbose) describe();
  for (i = 0; i < MAX_MODES; i++) {
    if (modes[i].init == NULL && modes[i].first == NULL &&
        modes[i].next_mode < 0) {
      modes[i].index = -1;
      continue;
    }
    modes[i].index = mci;
    ch = modes[i].init;
    for (; ch != NULL; ch = ch->next)
      if (ch->type == TK_SOLENOID_NAME) {
        new_mode_code(SOL_STROBES);
        if (ch->state == SOL_OPEN)
          new_mode_code(solenoids[ch->t_index].open_cmd);
        else new_mode_code(solenoids[ch->t_index].close_cmd);
      } else {
        new_mode_code(SOL_DTOA);
        new_mode_code(dtoas[ch->t_index].set_point_index[ch->state]);
      }
    ch = modes[i].first;
    if (ch != NULL) {
      new_mode_code(SOL_SET_TIME);
      new_mode_code(modes[i].count & 0xFF);
      new_mode_code((modes[i].count >> 8) & 0xFF);
      cycle_index = mci;
      time = 0;
      while (ch != NULL) {
        comp_waits((ch->time - time) * modes[i].iters);
        time = ch->time;
        if (ch->state == MODE_SWITCH_OK) {
          new_mode_code(SOL_MSWOK);
          ch = ch->next;
        }
        if (ch != NULL) for (; ch != NULL && ch->time == time; ch = ch->next)
          if (ch->type == TK_SOLENOID_NAME) {
            assert(ch->state == SOL_OPEN || ch->state == SOL_CLOSE);
            new_mode_code(SOL_STROBES);
            if (ch->state == SOL_OPEN)
              new_mode_code(solenoids[ch->t_index].open_cmd);
            else new_mode_code(solenoids[ch->t_index].close_cmd);
          } else {
            assert(ch->state != MODE_SWITCH_OK);
            new_mode_code(SOL_DTOA);
            new_mode_code(dtoas[ch->t_index].set_point_index[ch->state]);
          }
      }
      comp_waits((modes[i].length - time) * modes[i].iters);
      time = modes[i].length;
      if (modes[i].next_mode >= 0) {
        new_mode_code(SOL_SELECT);
        new_mode_code(modes[i].next_mode);
      } else {
        new_mode_code(SOL_GOTO);
        new_mode_code(cycle_index & 0xFF);
        new_mode_code((cycle_index >> 8) & 0xFF);
      }
    }
    new_mode_code(SOL_END_MODE);
  }
}

void comp_waits(int j) {
  for (; j > 255; j -= 255) {
    new_mode_code(SOL_WAITS);
    new_mode_code(255);
  }
  if (j == 1) new_mode_code(SOL_WAIT);
  else if (j > 1) {
    new_mode_code(SOL_WAITS);
    new_mode_code(j);
  }
}
