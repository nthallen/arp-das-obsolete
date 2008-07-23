/* tmrdrvr.h Contains definitions internal to the timerbd driver.
 * $Log$
 * Revision 1.1  1992/10/26  16:17:31  nort
 * Initial revision
 *
 * Revision 1.1  1992/09/24  20:50:28  nort
 * Initial revision
 *
 */
#ifndef TMRDRVR_H_INCLUDED
#define TMRDRVR_H_INCLUDED
#include <sys/types.h>

void tmr_hw_init(void); /* tmrhw.c */
int timer_mode(int timer, int mode); /* tmrhw.c */
int timer_count(int timer, unsigned int count); /* tmrhw.c */
int picinit(void); /* picinit.c */
void EIR_mask(int vector); /* handler.c */
void EIR_unmask(int vector); /* handler.c */
pid_t far tmr_handler(void); /* handler.c */
void tmr_init_options(int argc, char **argv); /* timerbd.c */

/* These are the subbus addresses for the external 8259s */
#define icw1 0
#define icw2 2
#define icw3 2
#define icw4 2
#define ocw1 2
#define ocw2 0
#define ocw3 0
#define epic1 4
#define epic2 8
#define epic3 0xC
#define TMR_INTA 0x2C
#define TMR_ISR_ADDR(vector) (4+((vector&0xF8)>>1))

/* What do we need to know about timers, EIRs */
struct tmrvec {
  pid_t owner;
  unsigned char mode;
};

struct eirvec {
  pid_t owner;			/* who owns it */
  unsigned char action;	/* What action to take */
  union {
	pid_t proxy;		/* TMR_PROXY: trigger this one */
  } u;
};

#define N_TMRS 9
#define N_EIRS 24
extern struct tmrvec tmrvs[N_TMRS]; /* tmr_hw.c */
extern struct eirvec eirvs[N_EIRS]; /* tmr_hw.c */
#endif
