/* compile.c takes the information in solenoids and modes and compiles it
   into a numerical code.
   Written March 25, 1987
   Modified April 10, 1987 for proper optimization.
   Modified JUly 1991 for QNX.
*/
#include <stdio.h>
#include <assert.h>
#include "tokens.h"
#include "modes.h"
#include "solenoid.h"
#include "dtoa.h"
#include "solfmt.h"


extern int verbose;

/* optimize tries to eliminate switching commands that are redundant.  For
   example, it moves switching commands that are at the beginning of a
   cycle into the initialization if the state of that solenoid is the
   same at the end of the cycle.  e.g.  A: OOOO  or even A: O__O.  In
   both cases, there shouldn't be an open command at the beginning of
   the cycle except the first time the mode is invoked.
*/
void optimize(int mn) {
  change *ch, *och;
  long count;
  int ti;

  if (modes[mn].init == NULL && modes[mn].first == NULL) return;
  if (modes[mn].next_mode < 0) {
    for (ch = modes[mn].first; ch != NULL && ch->time == 0;) {
      if ((ch->state != MODE_SWITCH_OK) &&
          (((ch->type == TK_SOLENOID_NAME) &&
            (solenoids[ch->t_index].first_state ==
                solenoids[ch->t_index].last_state)) ||
           ((ch->type == TK_DTOA_NAME) &&
         (dtoas[ch->t_index].first_state == dtoas[ch->t_index].last_state)))) {
        if (verbose)
          if (ch->type == TK_SOLENOID_NAME)
            printf("Mode %d, Solenoid %s requires cycle optimization\n",
                mn, solenoids[ch->t_index].name);
          else printf("Mode %d, DtoA %s requires cycle optimization\n",
                mn, dtoas[ch->t_index].name);
        if (ch == modes[mn].first) {
          och = modes[mn].first = ch->next;
          ch->next = modes[mn].init;
          modes[mn].init = ch;
          ch = och;
        } else {
          och->next = ch->next;
          ch->next = modes[mn].init;
          modes[mn].init = ch;
          ch = och->next;
        }
      } else { och = ch; ch = ch->next; }
    }
  }
  if (modes[mn].first != NULL) {
    count = (((long)16384) * modes[mn].res_num)/modes[mn].res_den;
    if (count > 0x10000l) {
      for (ti = count/0x10000l + 1; ti < 1000; ti++)
        if (count % ti == 0) break;
      if (ti == 1000) error("Solfmt - Can't deal with long resolution\n");
      modes[mn].iters = ti;
      count /= ti;
      if (verbose)
        printf("   Mode %d resolution requires %d iterations\n", mn, ti);
    } else modes[mn].iters = 1;
    if ((((long)16384) * modes[mn].res_num) % modes[mn].res_den != 0) {
      fprintf(stderr,
        "Solfmt - Warning: Mode %d resolution is irrational mod 16384\n", mn);
      fprintf(stderr, "    Actual resolution will be %ld/16384\n", count);
    }
    modes[mn].count = (unsigned int)count;
  }
}

void describe(void) {
  int i, j, k;
  change *ch;

  printf("Solenoids defined are:\n");
  for (i = 0; i < n_solenoids; i++)
    printf("  %s    open: %d  close: %d  status:  %d\n",
           solenoids[i].name, solenoids[i].open_cmd,
           solenoids[i].close_cmd, solenoids[i].status_bit);
  printf("DtoAs defined are:\n");
  for (i = 0; i < n_dtoas; i++) {
    printf("  %s\n", dtoas[i].name);
    for (j = 0; j < dtoas[i].n_set_points; j++) {
      k = dtoas[i].set_point_index[j];
      printf("    %c    addr: %x  value: %x\n",
           dtoas[i].set_point_name[j],
           set_points[k].address, set_points[k].value);
    }
  }
  for (i = 0; i < MAX_MODES; i++) {
    if (modes[i].init == NULL && modes[i].first == NULL &&
        modes[i].next_mode < 0) {
      modes[i].index = -1;
      continue;
    }
    printf("\nMode %d is defined with length %d at resolution %d/%d\n",
            i, modes[i].length, modes[i].res_num, modes[i].res_den);
    ch = modes[i].init;
    if (ch != NULL) {
      printf("\n  Initializations:\n");
      for (; ch != NULL; ch = ch->next) {
        if (ch->type == TK_SOLENOID_NAME) {
          assert(ch->state == SOL_OPEN || ch->state == SOL_CLOSE);
          printf("    Solenoid %s ", solenoids[ch->t_index].name);
          if (ch->state == SOL_OPEN) printf("opened\n");
          else printf("closed\n");
        } else {
          assert(ch->state >= 0);
          assert(ch->state < dtoas[ch->t_index].n_set_points);
          printf("    DtoA %s to set point %c\n", dtoas[ch->t_index].name,
                dtoas[ch->t_index].set_point_name[ch->state]);
        }
      }
      printf("\n");
    }
    ch = modes[i].first;
    if (ch != NULL) {
      if (modes[i].next_mode < 0) printf("  Cycle:\n");
      else printf("  Commands:\n");
      for (; ch != NULL; ch = ch->next) {
        if (ch->state == MODE_SWITCH_OK) printf("    Mode switch OK");
        else if (ch->type == TK_SOLENOID_NAME) {
          printf("    Solenoid %s ", solenoids[ch->t_index].name);
          if (ch->state == SOL_OPEN) printf("opened");
          else printf("closed");
        } else {
          printf("    DtoA %s to set point %c", dtoas[ch->t_index].name,
                dtoas[ch->t_index].set_point_name[ch->state]);
        }
        printf(" at t=%d\n", ch->time);
      }
    }
    if (modes[i].next_mode >= 0)
      printf("    Select mode %d at t=%d\n", modes[i].next_mode,
                 modes[i].length);
    if (modes[i].first == NULL && modes[i].next_mode < 0)
      printf("  This mode has no cycle.\n");
    printf("\n");
  }
}
