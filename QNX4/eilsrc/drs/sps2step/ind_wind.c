
/* DR Windows
   Written by MBM,  Dec. 10, 1987
   Made ANSI on or after October 17, 1989
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <curses.h>
#include <assert.h>
#include "scurses.h"
#include "snfbatch.h"
#include "attribut.h"
#include "snafuerr.h"
#include "snafucmd.h"
#include "snf_dr.h"


#define HEADER_ROW      0

static swindow sdrwin, cewin;

void start_dr_disp(void) {
  newwina(LINES-2, COLS, 0, 0, &sdrwin);
  cewin.w = newwin(4, COLS, LINES/2, 0);
  cewin.h = wactivate(cewin.w);
  wattrset(cewin.w, ATTR_CMDERR);
  refresh_dr_disp();
  sdoupdate(0);
}

void refresh_dr_disp(void) {
  wattrset(sdrwin.w, ATTR_DRBACK);
  wclear(sdrwin.w);
  wattrset(sdrwin.w, ATTR_DRBACK);
  swrefresh(sdrwin.h);
  sdoupdate(0);
}

void end_dr_disp(void) {
  delwina(sdrwin.h);
  delwina(cewin.h);
}

/*+
<sort> cmderr


cmderr(char *cntrl, ...);

cmderr outputs the specified information on the command prompt line.  If
snafu is operating in any mode except blind batch mode, it will request a
response from the keyboard.  In the default query batch mode, the prompt
will request [t/c/i]? which stands for Terminate, Continue or Interact,
and refers simply to the batch mode, not to the current snafu operation.
A response of 't' will end the batch mode and close the batch file, taking
all further input from the keyboard.  A response of 'c' will proceed as if
nothing had happened.  A response of 'i' will suspend batch mode operation,
giving the user an opportunity to correct whatever the error was and then
resume batch operation by typing ^B.
-*/
int cerrout(char *etext, char *options) {
  int c;

  wclear(cewin.w);
  mvwprintw(cewin.w, 1, 1, "%s\n  %s", etext, options);
  box(cewin.w, VERT_DOUBLE, HORIZ_DOUBLE);
  swrefresh(cewin.h);
  sdoupdate(0);
  c = negetch();
  wnorefresh(cewin.h);
  sdoupdate(1);
  return(c);
}

void vcmderr(char *cntrl, va_list ap) {
  char buf[101];
  int i;

  vsprintf(buf, cntrl, ap);
  while (input_mode == IM_BATCH_3) quit_batch();
  switch (input_mode) {
    case IM_BATCH_0:
      quit_batch();
      suspend_batch();
    default:
    case IM_INTERACTIVE:
      cerrout(buf, "Hit any key to continue.");
      break;
    case IM_BATCH_1:
      i = cerrout(buf, "Response? [i/c/t] ");
      switch (i) {
        case 'c':       /* Continue */
        case 'C':
          break;
        case 't':       /* 't' for terminate batch */
        case 'T':
          quit_batch();
          suspend_batch();
          break;
        default:        /* Interact for awhile */
          suspend_batch();
          break;
      }
      break;
    case IM_BATCH_2:
      n_prompt(buf);
      break;
  }
}

void cmderr(char *cntrl, ...) {
  va_list ap;

  va_start(ap, cntrl);
  vcmderr(cntrl, ap);
  va_end(ap);
}

int error(int code, char *cntrl, ...) {
  va_list ap;

  va_start(ap, cntrl);
  vcmderr(cntrl, ap);
  va_end(ap);
  return(snafu = code);
}

/* Idle will perform doupdate() if necessary */
void idle(void) {
  if (supdate()) cursor_on();
}

int q_prompt(char *cntrl,...) {
  va_list ap;
  char buf[160];
  int c;

  va_start(ap, cntrl);
  vsprintf(buf, cntrl, ap);
  va_end(ap);
  wclear(cewin.w);
  mvwprintw(cewin.w, 1, 1, "%s", buf);
  box(cewin.w, VERT_DOUBLE, HORIZ_DOUBLE);
  swrefresh(cewin.h);
  sdoupdate(0);
  c = negetch();
  wnorefresh(cewin.h);
  sdoupdate(1);
  return(c);
}
