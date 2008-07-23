/* timerbd.c Provides interface to the timerbd.
   Registers local name
   Etc.
   $Log$
 * Revision 1.4  1995/06/01  01:15:30  nort
 * Proper Experiment name expansion
 *
 * Revision 1.3  1993/11/24  15:10:29  nort
 * *** empty log message ***
 *
 * Revision 1.2  1992/10/28  06:57:23  nort
 * Changes to delay quitting until all requests are complete.
 *
 * Revision 1.1  1992/10/26  16:17:31  nort
 * Initial revision
 *
 * Revision 1.1  1992/09/24  20:50:28  nort
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/name.h>
#include <sys/irqinfo.h>
#include <sys/kernel.h>
#include <unistd.h>
#include <conio.h>
#include "msg.h"
#include "globmsg.h"
#include "cc.h"
#include "timerbd.h"
#include "tmrdrvr.h"
#include "nortlib.h"
#include "subbus.h"
#include "oui.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static int irq = TIMERBD_DFLT_IRQ;
static int active_requests = 0;

/* Handles -i n (Specify IRQ) and -x (Kill existing timerbd) */
void tmr_init_options(int argc, char **argv) {
  int c;
  pid_t pid;

  optind=0; /* added by eil 4/2/93 */
  do {
	c=getopt(argc,argv,opt_string);
	switch (c) {
	  case 'i':
		irq = atoi(optarg);
		break;
	  case 'x': /* kill and existing timerbd */
		pid = qnx_name_locate(getnid(),
				nl_make_name(TIMERBD_NAME, 0), 0, NULL);
		if (pid != -1) {
		  unsigned char rqst[3] = { DASCMD, DCT_QUIT, DCV_QUIT };
		  Send(pid, rqst, rqst, sizeof(rqst), 0);
		} else msg(MSG, "No Timer Board Driver Located");
		exit(0);
	  default:
		break;
	}
  } while (c!=-1);
  optind = 0;
  opterr = 1;
}

static void tmr_set_name(void) {
  char *name;
  
  name = nl_make_name(TIMERBD_NAME, 0);
  if (qnx_name_attach(0, name) == -1)
	msg(MSG_EXIT_ABNORM, "Unable to attach name %s", name);
}

static int tmr_iid = -1;

#define MAX_IRQ_104 12
static unsigned short irq104[ MAX_IRQ_104 ] = {
  0, 0, 0, 0x21, 0x22, 0x23, 0x24, 0x25, 0, 0x20, 0x26, 0x27
};

static void irq_init(void) {
  tmr_iid = qnx_hint_attach(irq, tmr_handler, FP_SEG(eirvs));
  if (tmr_iid == -1)
    msg(MSG_EXIT_ABNORM, "Unable to attach IRQ %d", irq);
  nl_error( -2, "Subbus version is %d", subbus_subfunction );
  if ( subbus_subfunction == SB_SYSCON104 ) {
	unsigned short cfg_word;
	
	if ( irq < 0 || irq >= MAX_IRQ_104 || irq104[ irq ] == 0 )
	  msg( 3, "Specified IRQ %d is out of range or invalid", irq );
	cfg_word = ( inpw( 0x312 ) & ~0x3F ) | irq104[ irq ];
	nl_error( -2, "SC104 writing CPA word %04X", cfg_word );
	outpw( 0x312, cfg_word );
  }
}

static void irq_reset(void) {
  qnx_hint_detach(tmr_iid);
  if ( subbus_subfunction == SB_SYSCON104 ) {
	unsigned short cfg_word;
	
	cfg_word = ( inpw( 0x312 ) & ~0x3F );
	outpw( 0x312, cfg_word );
  }
}

/* rqst->eir is guaranteed to fall within the appropriate range,
   and any ownership issues have been resolved.
   program_eir() is responsible for replying.
 */
static void program_eir(pid_t who, struct tmrbdmsg *rqst) {
  eirvs[rqst->eir].owner = who;
  switch (rqst->action) {
	case TMR_PROXY:
	  if (eirvs[rqst->eir].action != TMR_PROXY) {
		active_requests++;
		msg(MSG_DBG(0), "Active requests = %d", active_requests);
	  }
	  eirvs[rqst->eir].action = rqst->action;
	  eirvs[rqst->eir].u.proxy = rqst->u.proxy;
	  EIR_unmask(rqst->eir);
	  break;
	default:
	case TMR_NOP:
	  eirvs[rqst->eir].action = rqst->action;
	  EIR_mask(rqst->eir);
	  break;
	case TMR_QUERY:
	  rqst->action = eirvs[rqst->eir].action;
	  if (rqst->action == TMR_PROXY)
		rqst->u.proxy = eirvs[rqst->eir].u.proxy;
	  break;
  }
  rqst->op_rtn = DAS_OK;
  Reply(who, rqst, sizeof(struct tmrbdmsg));
}

/* At this point, rqst->timer is in the realm of existing timers.
   program_timer is responsible for replying, but in all
   succesful cases it leaves this to program_eir().
 */
static void program_timer(pid_t who, struct tmrbdmsg *rqst) {
  int mode, timer;
  
  timer = rqst->timer;
  mode = rqst->mode;
  if (mode != TMR_ONCE && mode != TMR_REPEAT)
	reply_byte(who, DAS_UNKN);
  else {
	tmrvs[timer].owner = who;
	if (tmrvs[timer].mode != mode) {
	  timer_mode(timer, mode);
	  tmrvs[timer].mode = mode;
	}
	timer_count(timer, rqst->divisor);
	rqst->eir = timer + TMR_0_EIR;
	program_eir(who, rqst);
  }
}

/* rqst->eir is guaranteed in range and owned by who (whom?)
   so the reset is a valid request. reset_eir() must not only
   reset the eir, but the reply is to explain what the programmed
   mode was. In particular, I want the proxy back.
*/
static void reset_eir(pid_t who, struct tmrbdmsg *rqst) {
  EIR_mask(rqst->eir);
  rqst->action = eirvs[rqst->eir].action;
  switch (rqst->action) {
	case TMR_PROXY:
	  rqst->u.proxy = eirvs[rqst->eir].u.proxy;
	  active_requests--;
	  msg(MSG_DBG(0), "Active requests = %d", active_requests);
	  break;
	case TMR_NOP: break;
	default:
	  msg(MSG_WARN, "Unknown action in reset_eir");
	  break;
  }
  eirvs[rqst->eir].owner = 0;
  eirvs[rqst->eir].action = TMR_NOP;
  rqst->op_rtn = DAS_OK; /* added by eil 4/2/93 */
  Reply(who, rqst, sizeof(struct tmrbdmsg));
}

static void main_loop(void) {
  struct tmrbdmsg rqst;
  pid_t who, owner;
  int quitting_time = 0;
  
  while (!quitting_time || active_requests > 0) {
	do who = Receive(0, &rqst, sizeof(rqst)); while (who==-1);
	switch (rqst.op_rtn) {
	  case DASCMD:
		if (rqst.timer == DCT_QUIT && rqst.mode == DCT_QUIT) {
		  reply_byte(who, DAS_OK);
		  quitting_time = 1;
		} else reply_byte(who, DAS_UNKN);
		break;
	  case TMR_SET:
		switch (rqst.timer) {
		  case TMR_ANY:
			for (rqst.timer = 0; rqst.timer < N_TMRS; rqst.timer++)
			  if (tmrvs[rqst.timer].owner == 0) break;
			if (rqst.timer < N_TMRS) program_timer(who, &rqst);
			else reply_byte(who, DAS_BUSY);
			break;
		  default: /* explicit timer number */
			if (rqst.timer < N_TMRS) {
			  owner = tmrvs[rqst.timer].owner;
			  if (owner == 0 || owner == who)
				program_timer(who, &rqst);
			  else reply_byte(who, DAS_BUSY);
			} else reply_byte(who, DAS_UNKN);
			break;
		  case TMR_NONE: /* non-timer EIR */
			if (rqst.eir <= TMR_0_EIR) {
			  if (eirvs[rqst.eir].owner == 0)
				program_eir(who, &rqst);
			  else reply_byte(who, DAS_BUSY);
			} else reply_byte(who, DAS_UNKN);
			break;
		}
		break;
	  case TMR_RESET:
		switch (rqst.timer) {
		  default: /* explicit timer number */
			if (rqst.timer >= N_TMRS)
			  reply_byte(who, DAS_UNKN);
			else if (tmrvs[rqst.timer].owner != who)
			  reply_byte(who, DAS_BUSY); /* should be NOT_OWNER */
			else {
			  tmrvs[rqst.timer].owner = 0;
			  rqst.eir = rqst.timer + TMR_0_EIR;
			  reset_eir(who, &rqst);
			}
			break;
		  case TMR_NONE: /* non-timer EIR: with fall-through */
			if (rqst.eir >= N_EIRS)
			  reply_byte(who, DAS_UNKN);
			else if (eirvs[rqst.eir].owner != who)
			  reply_byte(who, DAS_BUSY);
			else reset_eir(who, &rqst);
			break;
		}
	  default:
		reply_byte(who, DAS_UNKN);
	}
  }
}

/* Ask CC to let us know about quitting time, but don't
   get too upset if no CC is to be found
*/
static void cc_send_quit(void) {
  int nlrsp;
	
  nlrsp = set_response(NLRSP_QUIET);
  if (find_CC(0) != -1)
	cc_init(0, 0, 0, 0,
	  FORWARD_QUIT, NOTHING_ON_DEATH, NULL);
  set_response(nlrsp);
}

void main(int argc, char **argv) {
  oui_init_options( argc, argv );

  cc_send_quit();
  tmr_set_name();
  
  /* Initialize the timer board hardware and data structures */
  tmr_hw_init();

  irq_init();
  BEGIN_MSG;
  main_loop();
  irq_reset();
  
  DONE_MSG;
}
