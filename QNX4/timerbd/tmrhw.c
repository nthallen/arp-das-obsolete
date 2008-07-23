/* tmrhw.c interfaces to the hardware of the timer board.
 * $Log$
 * Revision 1.2  1995/06/01  01:16:11  nort
 * *** empty log message ***
 *
 * Revision 1.1  1992/10/26  16:17:31  nort
 * Initial revision
 *
 * Revision 1.1  1992/09/24  20:50:28  nort
 * Initial revision
 *
 */
#include <unistd.h>
#include "msg.h"
#include "timerbd.h"
#include "subbus.h"
#include "tmrdrvr.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static struct ta {
  int ctrl;
  int data;
} tim_adr[N_TMRS] = {
  0x16, 0x10,
  0x16, 0x12,
  0x16, 0x14,
  0x1E, 0x18,
  0x1E, 0x1A,
  0x1E, 0x1C,
  0x26, 0x20,
  0x26, 0x22,
  0x26, 0x24
};

#define tmr_ok(x) ((x) >= 0 && (x) < N_TMRS)

/* returns 0 on success */
int timer_mode(int timer, int mode) {
  int ctrl_word;

  if (tmr_ok(timer)) {
    ctrl_word = ((timer % 3) << 6) | 0x30 | ((mode & 7) << 1);
    if (sbwra(tim_adr[timer].ctrl, ctrl_word)) return(0);
  }
  return(1);
}

/* returns 0 on success */
int timer_count(int timer, unsigned int count) {
  unsigned int addr;

  if (tmr_ok(timer)) {
    addr = tim_adr[timer].data;
    if (sbwra(addr, count) &&
        sbwra(addr, count>>8)) return(0);
  }
  return(1);
}

struct tmrvec tmrvs[N_TMRS];
struct eirvec eirvs[N_EIRS];

void tmr_hw_init(void) {
  int i;
  
  if (timer_mode(0, TMR_REPEAT) != 0 ||
      timer_count(0, (unsigned int)(1048576L/TMR_0_FREQ)) != 0 ||
      picinit() != 0) {
	msg(MSG_EXIT_ABNORM, "Timer Board Initialization Failure");
  }
  tmrvs[0].owner = getpid();
  tmrvs[0].mode = TMR_REPEAT;
  for (i = 1; i < N_TMRS; i++) {
	tmrvs[i].owner = 0;
	tmrvs[i].mode = TMR_NO_MODE;
  }
  for (i = 0; i < N_EIRS; i++) {
	eirvs[i].owner = 0;
	eirvs[i].action = TMR_NOP;
  }
}
