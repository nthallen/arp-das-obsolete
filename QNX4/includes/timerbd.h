/* timerbd.h
   Include file for the timer board administrator or
   applications wishing to address it.
 * $Log$
 * Revision 1.2  1992/09/24  20:23:10  nort
 * Running Well
 *
 * Revision 1.1  1992/09/21  16:06:35  nort
 * Initial revision
 *
 */
#ifndef _TIMERBD_H_INCLUDED
#define _TIMERBD_H_INCLUDED

#include <sys/types.h>

#define TIMERBD_NAME "timerbd"
#define TMR_0_FREQ 16384
#define TIMERBD_DFLT_IRQ 9

/* Timer 0 doesn't have an EIR, but if it did...
   For timer n > 0, the corresponding EIR is n + TMR_0_EIR.
 */
#define TMR_0_EIR 15

/* The message type for communication to/from timerbd */
struct tmrbdmsg {
  unsigned char op_rtn;
  unsigned char timer;
  unsigned char mode;
  unsigned short divisor;
  unsigned char eir;
  unsigned char action;
  union {
	pid_t proxy;
  } u;
};

/* op_rtn indicates the basic operation. For now the operations
   are TMR_SET and TMR_RESET, but I can imagine TMR_QUERY or
   something like that. On TMR_RESET, the reply contains the
   definition of the action before reset. This is useful
   for retrieving the proxy pid so it can be detached.
   On return op_rtn will convey error information.
*/
#define TMR_SET 255
#define TMR_RESET 254

/* timer takes on values 0-8 for specific timers or TMR_ANY or TMR_NONE
   Requesting TMR_ANY without defining an EIR is a syntax error.
 */
#define TMR_ANY 255
#define TMR_NONE 254

/* mode and divisor are significant only if timer != TMR_NONE.
   mode can be either TMR_ONCE or TMR_REPEAT. divisor assumes
   intimate knowledge of the hardware.
*/
#define TMR_ONCE 0
#define TMR_REPEAT 3
#define TMR_NO_MODE 255

/* eir is significant only if timer == TMR_NONE, otherwise the eir
   is derived from the timer selection. eir is in [0-5] or [8-23].
   action specifies what to do when the interrupt occurs.
   Currently there are two possibilities.
   TMR_NOP says do nothing and is used when programming hardware
   timing functions.
   TMR_PROXY requests that the proxy indicated in u.proxy be
   triggered on interrupt.
   Other actions will be defined as needed.
*/
#define TMR_NOP 0
#define TMR_PROXY 1
#define TMR_QUERY 2

/* The following are library functions for applications */
pid_t find_Tmr(void); /* find_tmr.c */
int send_Tmr(struct tmrbdmsg *rqst); /* send_tmr.c */

/* Programs the indicated timer (0-8) to the indicated mode/divisor.
   If not previously assigned to caller, assigned to caller, and
   hence removed from the available timer list. Returns
     DAS_OK
	 DAS_UNKN if timer# is out of range.
	 DAS_BUSY if timer is already owned by someone else
   Does not change the EIR handling (TMR_QUERY).
 */
int Tmr_set(int timer, int mode, unsigned short divisor); /* tmr_set.c */

/* Creates a proxy and also programs a timer. Timer is set to TMR_ANY.
   Returns the number of the timer or -1 on error.
 */
int Tmr_proxy(int mode, unsigned short divisor, unsigned char msg);

/* Resets the specified timer (0-8) and associated EIR, deleting
   proxy if necessary.
 */
int Tmr_reset(int timer);

/* Creates a proxy and defines the specified EIR to return the proxy.
   EIR is unmasked.
 */
int EIR_proxy(int EIR, unsigned char msg);

/* Resets the specified EIR, deleting proxy if necessary. EIR is masked. */
int EIR_reset(int EIR);
#endif
