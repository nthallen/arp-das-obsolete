/* indexer.c Indexer driver
 * $Log$
 * Revision 1.9  1994/02/14  18:43:28  nort
 * Removed Proxy definitions now to be handled by cmd server
 * Added IX_PRESET_POS command to allow external calibrations
 *
 * Revision 1.8  1994/02/14  18:16:05  nort
 * Added -P option to disable pot calibration
 *
 * Revision 1.7  1993/11/17  18:55:28  nort
 * Made status bytes into words to support scanning of STV.
 * Additional bit doesn't show up in TM, which expects just
 * a byte, but scanning works anyway.
 *
 * Revision 1.6  1993/11/17  17:52:00  nort
 * Adjusted adjustable gate walk code
 *
 * Revision 1.5  1992/11/16  06:07:23  nort
 * Removed unused cdef
 *
 * Revision 1.4  1992/11/16  06:06:23  nort
 * Slight Mod allowing user setting of gate width
 *
 * Revision 1.3  1992/11/16  06:01:39  nort
 * Added some adjgate manipulations
 *
 * Revision 1.2  1992/11/16  05:56:31  nort
 * Update
 *
 * Revision 1.1  1992/10/16  18:24:31  nort
 * Initial revision
 *
 * Revision 1.5  1992/09/24  20:36:23  nort
 * With command queueing.
 *
 * Revision 1.4  1992/09/24  13:12:04  nort
 * Running, but without command queueing
 *
 * Revision 1.3  1992/09/03  16:10:39  nort
 * Removed the test msg's
 *
 * Revision 1.2  1992/09/03  16:09:58  nort
 * Added a bunch of test msg's.
 *
 * Revision 1.1  1992/09/02  16:41:20  nort
 * Initial revision
 *
 */
#include <stdlib.h>
#include <sys/kernel.h>
#include <unistd.h> /* for getopt() */
#include "globmsg.h"
#include "nortlib.h"
#include "indexer.h"
#include "ixdrv.h"
#include "timerbd.h"
#include "cc.h"
#include "subbus.h"
#include "msg.h"
#include "collect.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

char *opt_string = OPT_MSG_INIT OPT_CC_INIT "P:" ;
int (*nl_error)(unsigned int level, char *s, ...) = msg;

unsigned short tm_status_byte;
static unsigned short scan_status;

/* cal_set defines which pots to calibrate to
   0 = hox bellows calibration
   1 = no calibrations
 */
static int cal_set = 0;

chandef channel[N_CHANNELS] = {
  0xA00,  8, BLW_SCAN_BIT,            0,            0,
  0xA08,  9, ETN_SCAN_BIT, ETN_CHOP_BIT, ETN_SUPP_BIT,
  0xA10, 10, ATN_SCAN_BIT, ATN_CHOP_BIT,            0,
  0xA20, 11, PTV_SCAN_BIT,            0,            0,
  0xA28, 12, STV_SCAN_BIT,            0,            0,
  0xA30, 13,            0,            0,            0
};

drvstat drive[N_CHANNELS];

#ifndef NO_ADJGATE_STUFF
static unsigned char bit_rev(unsigned char n) {
  int i;
  unsigned char o = 0;

  for (i = 0; i < 8; i++) {
    o <<= 1;
	o |= n & 1;
	n >>= 1;
  }
  return(o);
}

static void gate_update(void) {
  static unsigned char delay = 0;
  unsigned short rdel;

  rdel = (bit_rev(delay++) << 8);
  /* sbwr(0x64E, rdel | sbb(0x64E)); */ /* SX1AG & SX2AG */
  sbwr(0x65E, rdel | sbb(0x65E)); /* SOCAG & LABAG */
}
#endif

static void execute_cmd(unsigned int drvno) {
  step_t curpos, addr;
  step_t steps_each, steps_to_write;
  ixcmdl *im;
  drvstat *drv;
  chandef *cdef;

  drv = &drive[drvno];
  im = drv->first;
  cdef = &channel[drvno];

  addr = cdef->base_addr;

  /* Translate ONLINE and OFFLINE commands to TO commands */
  if (im->c.dir_scan == IX_ONLINE) {
	im->c.dir_scan = IX_TO;
	im->c.steps = drv->online;
	if (drv->state & CST_ON_ALT) {
	  im->c.steps += drv->online_delta;
	  tm_status_byte |= (cdef->supp_bit | cdef->on_bit);
	} else tm_status_byte = (tm_status_byte & ~cdef->supp_bit)
							 | cdef->on_bit;
	drv->state ^= CST_ON_ALT;
  } else if (im->c.dir_scan == IX_OFFLINE) {
	#ifndef NO_ADJGATE_STUFF
	  if (drvno == IX_ETALON) gate_update();
	#endif
	im->c.dir_scan = IX_TO;
	im->c.steps = drv->offline;
	tm_status_byte = (tm_status_byte & ~cdef->on_bit)
					 | cdef->supp_bit;
  } else tm_status_byte &= ~(cdef->on_bit | cdef->supp_bit);
  
  /* Translate destination to direction */
  if (im->c.dir_scan & IX_TO) {
	/* Don't need to stop(?) so we can get a clear reading */
	/* sbwr(addr, ~0); */
	curpos = sbw(addr+2);

	im->c.dir_scan &= ~IX_DIR;
	if (curpos < im->c.steps) {
	  im->c.dir_scan |= IX_OUT;
	  im->c.steps -= curpos;
	} else {
	  im->c.dir_scan |= IX_IN; /* nop! */
	  im->c.steps = curpos - im->c.steps;
	}
  }
  
  steps_to_write = steps_each =
	(im->c.dir_scan & IX_SCAN) ? im->c.dsteps : im->c.steps;
  
  /* Translate direction into an address and check hysteresis */
  if ((im->c.dir_scan & IX_DIR) == IX_IN) {
	if (im->c.drive & IX_USE_HYSTERESIS) {
	  steps_to_write += ~sbw((addr & ~0x1F) | 0x18);
	  addr += 6;
	} else addr += 4;
  }
  
  if (im->c.dir_scan & IX_SCAN) {
	drv->scan_addr = addr;
	drv->scan_amount = steps_each;
	drv->w_amount = steps_to_write;
	drv->to_go = im->c.steps;
	if (scan_status == 0)
	  Col_set_proxy(INDEXER_PROXY_ID, IX_SCAN_PROXY);
	scan_status |= cdef->scan_bit;
  } else sbwr(addr, ~steps_to_write);
}

/* Called when interrupt proxy for the specified drive is received
   We can advance to the next command if we aren't scanning this
   drive.
 */
static void end_of_drive(int drvno) {
  int scan_bit;
  drvstat *drv;
  ixcmdl *nc;
  
  scan_bit = channel[drvno].scan_bit;
  drv = &drive[drvno];
  if (!(scan_status & scan_bit) && drv->first != NULL) {
	nc = drv->first->next;
	free(drv->first);
	drv->first = nc;
	if (nc != NULL) execute_cmd(drvno);
  }
}

static void end_scan(int drvno) {
  int scan_bit;
  
  scan_bit = channel[drvno].scan_bit;
  if (scan_status & scan_bit) {
	tm_status_byte &= ~scan_bit;
	scan_status &= ~scan_bit;
	if (scan_status == 0)
	  Col_reset_proxy(INDEXER_PROXY_ID);
	end_of_drive(drvno);
  }
}

static void scan_proxy(void) {
  int drvno, scan_bit;
  drvstat *drv;
  
  for (drvno = 0; drvno < N_CHANNELS; drvno++) {
	drv = &drive[drvno];
	scan_bit = channel[drvno].scan_bit;
	if (scan_status & scan_bit) {
	  if (!(tm_status_byte & scan_bit))
		tm_status_byte |= scan_bit;
	  else if (drv->to_go == 0) end_scan(drvno);
	  else {
		if (drv->to_go < drv->scan_amount) {
		  drv->w_amount -= drv->scan_amount - drv->to_go;
		  drv->scan_amount = drv->to_go;
		}
		sbwr(drv->scan_addr, ~drv->w_amount);
		drv->to_go -= drv->scan_amount;
	  }
	}
  }
}

/* Queues a drive or scan command */
static unsigned char drive_scan(idxr_msg *im) {
  unsigned int drvno;
  ixcmdl *nc;
  drvstat *drv;
  
  drvno = im->c.drive & ~IX_USE_HYSTERESIS;
  if (drvno >= N_CHANNELS) {
	msg(MSG_WARN, "Invalid Drive Number");
	return(DAS_UNKN);
  }
  
  /* Create a command record */
  nc = malloc(sizeof(ixcmdl));
  if (nc == NULL) {
	msg(MSG_FAIL, "Out of memory in drive_scan");
	return(DAS_BUSY);
  }
  nc->next = NULL;
  nc->c = im->c;
  
  drv = &drive[drvno];
  if (drv->first == NULL) {
	drv->first = drv->last = nc;
	execute_cmd(drvno);
  } else drv->last = drv->last->next = nc;
  return(DAS_OK);
}

static void full_stop(int drvno) {
  chandef *cdef;
  drvstat *drv;
  ixcmdl *nc;
  
  cdef = &channel[drvno];
  sbwr(cdef->base_addr, ~0); /* drive out 0 to stop */
  drv = &drive[drvno];
  while (drv->first != NULL) {
	nc = drv->first->next;
	free(drv->first);
	drv->first = nc;
  }
  end_scan(drvno);
}

static unsigned char indexer_cmd(idxr_msg *im) {
  int drvno = im->c.drive & ~IX_USE_HYSTERESIS;
  drvstat *drv;
  
  if (drvno < N_CHANNELS) {
	drv = &drive[drvno];
	switch (im->c.dir_scan) {
	  case IX_STOP:
		full_stop(drvno);
		return(DAS_OK);
	  case IX_ONLINE:
	  case IX_OFFLINE:
		return(drive_scan(im));
	  case IX_MOVE_ONLINE_OUT:
		drv->online += drv->online_delta;
		drv->offline += drv->online_delta;
		return(DAS_OK);
	  case IX_MOVE_ONLINE_IN:
		drv->online -= drv->online_delta;
		drv->offline -= drv->online_delta;
		return(DAS_OK);
	  case IX_SET_ONLINE:
		drv->offline += im->c.steps - drv->online;
		drv->online = im->c.steps;
		return(DAS_OK);
	  case IX_SET_ON_DELTA:
		drv->online_delta = im->c.steps;
		return(DAS_OK);
	  case IX_SET_OFF_DELTA:
		drv->offline = drv->online + im->c.steps;
		return(DAS_OK);
	  case IX_SET_NO_LOOPS:
		if (im->c.steps) tm_status_byte |= NO_LOOPS_BIT;
		else tm_status_byte &= ~NO_LOOPS_BIT;
		return(DAS_OK);
	  case IX_PRESET_POS:
		if (im->c.drive < N_CHANNELS) {
		  sbwr(channel[im->c.drive].base_addr+2, im->c.steps);
		  return(DAS_OK);
		} else
		  msg(MSG_WARN, "Invalid Drive Number %d for preset", im->c.drive);
		break;
	  case IX_DEFINE_BITS:
		if (im->c.drive < N_CHANNELS) {
		  unsigned short bit = 1, mask;
		  
		  mask = im->c.steps;
		  while (bit != 0 && (mask & bit) == 0) bit <<= 1;
		  channel[im->c.drive].scan_bit = bit;
		  for (bit <<= 1; bit != 0 && (mask & bit) == 0; bit <<= 1);
		  channel[im->c.drive].on_bit = bit;
		  for (bit <<= 1; bit != 0 && (mask & bit) == 0; bit <<= 1);
		  channel[im->c.drive].supp_bit = bit;
		  return(DAS_OK);
		} else
		  msg(MSG_WARN, "Invalid Drive Number %d for defbits", im->c.drive);
		break;
	  default:
		if (im->c.dir_scan < 8)
		  return(drive_scan(im));
		break;
	}
  }
  return(DAS_UNKN);
}

void main(int argc, char **argv) {
  idxr_msg im;
  pid_t sent_pid;
  int i, c;
  
  msg_init_options("IXR", argc, argv);
  if (load_subbus() == 0)
	msg(MSG_EXIT_ABNORM, "Indexer Requires Subbus");

  /* Register with CmdCtrl to get commands and quit notification */
  cc_init_options(argc, argv, 0, 0, INDEXER_MSG, INDEXER_MSG, FORWARD_QUIT);

  /* Handle our own options */
  optind = 0; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt(argc, argv, opt_string)) != -1) {
	switch (c) {
	  case 'P':
		cal_set = atoi(optarg);
		break;
	  case '?':
		nl_error(3, "Unrecognized Option -%c", optopt);
	}
  }

  /* Initialize the Drives:
	  Zero all down counters by driving out zero steps.
	  Initialize up/downs to zero or pot calibration
	  Initialize drive structure
	  Register for EIR proxies
  */
  for (i = 0; i < N_CHANNELS; i++) {
	sbwr(channel[i].base_addr, ~0);
	sbwr(channel[i].base_addr+2, 0);
	drive[i].first = drive[i].last = NULL;
	drive[i].online = drive[i].online_delta
	  = drive[i].offline = 0;
	drive[i].state = 0;
	EIR_proxy(channel[i].EIR, IX_CHAN_0_PROXY+i);
  }

  if (cal_set == 0) {  
	/* Bellows is initialized by reading BAPos (C06) and applying the
	   calibration BlwPs = 471*BAPos - 38703;
	   Calibration derived from 920928.3
	*/
	sbwr(channel[IX_BELLOWS].base_addr+2, sbb(0xC06) * 471 - 38703);
  }

  /* Register with Collection for reporting */
  set_response(1);

  { char *p;

	p = malloc(20 * sizeof(ixcmdl));
	Col_set_pointer(INDEXER_FLAG_ID, &tm_status_byte, 0);
	free(p);
  }

  set_response(3);
  
  BEGIN_MSG;
  
  for (;;) {
	sent_pid = Receive(0, &im, sizeof(im));
	if (sent_pid == -1) msg(MSG_WARN, "Error Receiving");
	else {
	  if (im.msgcode == DASCMD && im.c.dir_scan == DCT_QUIT
		  && im.c.drive == DCV_QUIT) {
		reply_byte(sent_pid, DAS_OK);
		break;
	  } else if (im.msgcode == INDEXER_MSG)
		reply_byte(sent_pid, indexer_cmd(&im));
	  else if (im.msgcode == IX_SCAN_PROXY)
		scan_proxy();
	  else if (im.msgcode >= IX_CHAN_0_PROXY)
		end_of_drive(im.msgcode - IX_CHAN_0_PROXY);
	  else reply_byte(sent_pid, DAS_UNKN);
	}
  }
  for (i = 0; i < N_CHANNELS; i++)
	EIR_reset(channel[i].EIR);
  
  DONE_MSG;
}
