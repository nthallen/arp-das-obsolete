/* Submain.c Subbus main program
 * Provides C-level functionality for resident subbus library under QNX4
 * $Log$
 * Revision 1.2  1995/04/26  18:02:18  nort
 * *** empty log message ***
 *
 * Revision 1.1  1992/06/18  15:59:26  nort
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <i86.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/seginfo.h>
#include "subbus.h"
#include "sb_internal.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

#ifdef __USAGE
%C	
	Installs subbus resident library.
%C	-q
	requests an already resident subbus library to terminate.
#endif

typedef struct {
  unsigned char id;
  unsigned int count;
} trec;
#define MAXTRECS 10
static trec trecs[MAXTRECS];
static unsigned int tps = 1, ndfltd = 0, dfltd = 0, ntrecs = 0;
static unsigned short oldseg;

void ltick_check(unsigned char id, unsigned int secs) {
  int i, j;
  
  secs = tps;
  for (i = 0; i < ntrecs; i++)
	if (trecs[i].id == id) break;
  if (i < ntrecs) {
	if (trecs[i].count == 0) {
	  if (--ndfltd == 0) {
		lltick_sic();
		write_novram(SIC_TICKFAIL_ADDR, 0);
	  } else if (id == dfltd) {
		for (j = 0; j < ntrecs; j++)
		  if (j != i && trecs[j].count == 0) {
			write_novram(SIC_TICKFAIL_ADDR, dfltd = trecs[j].id);
			break;
		  }
		if (j == ntrecs) {
		  ndfltd = 0;
		  write_novram(SIC_TICKFAIL_ADDR, 0);
		}
	  }
	}
	if (secs == 0) {
	  if (i < --ntrecs)
		trecs[i] = trecs[ntrecs];
	} else trecs[i].count = secs;
  } else if (ntrecs < MAXTRECS) {
	trecs[ntrecs].id = id;
	trecs[ntrecs++].count = secs;
  } /* else complain about not enough room */
}

void ltick_sic(void) {
  int i;
  
  for (i = 0; i < ntrecs; i++) {
	if (trecs[i].count != 0) {
	  if (--trecs[i].count == 0)
		ndfltd++;
	}
  }
  if (ndfltd == 0)
	lltick_sic();
}


union sb_msg {
  unsigned char type;
  struct sb_tps tps;
  struct sb_tchk tchk;
  struct sb_ena_nmi enmi;
  struct sb_reboot rbt;
  struct nvram_dir_rqst nvrr;
};

void operation(void) {
  union sb_msg msg;
  unsigned short newseg;
  pid_t who;
  
  for (;;) {
	do who = Receive(0, &msg, sizeof(msg)); while (who == -1);
	switch (msg.type) {
	  case SBMSG_LOAD:
		newseg = qnx_segment_put(who, oldseg, 0);
		if (newseg == -1) {
		  fprintf(stderr, "Error putting segment\n");
		  exit(1);
		}
		sbfs.read_subbus = MK_FP(newseg, FP_OFF(sbfs.read_subbus));
		sbfs.writeack = MK_FP(newseg, FP_OFF(sbfs.writeack));
		sbfs.read_ack = MK_FP(newseg, FP_OFF(sbfs.read_ack));
		sbfs.set_cmdenbl = MK_FP(newseg, FP_OFF(sbfs.set_cmdenbl));
		sbfs.read_novram = MK_FP(newseg, FP_OFF(sbfs.read_novram));
		sbfs.write_novram = MK_FP(newseg, FP_OFF(sbfs.write_novram));
		sbfs.read_switches = MK_FP(newseg, FP_OFF(sbfs.read_switches));
		sbfs.set_failure = MK_FP(newseg, FP_OFF(sbfs.set_failure));
		sbfs.read_rstcnt = MK_FP(newseg, FP_OFF(sbfs.read_rstcnt));
		sbfs.read_pwrcnt = MK_FP(newseg, FP_OFF(sbfs.read_pwrcnt));
		sbfs.read_failure = MK_FP(newseg, FP_OFF(sbfs.read_failure));
		sbfs.cmdstrobe = MK_FP(newseg, FP_OFF(sbfs.cmdstrobe));
		Reply(who, &sbfs, sizeof(sbfs));
		continue; /* i.e. don't reply after switch */
	  case SBMSG_ENA_NMI:  lenable_nmi(); break;
	  case SBMSG_DIS_NMI:  ldisable_nmi(); break;
	  case SBMSG_TICK:     ltick_sic(); break;
	  case SBMSG_DIS_TICK: ldisarm_sic(); break;
	  case SBMSG_TICK_CHK: ltick_check(msg.tchk.id, msg.tchk.secs); break;
	  case SBMSG_SET_TPS:  if (msg.tps.tps) tps = msg.tps.tps; break;
	  case SBMSG_REBOOT:   lreboot(msg.rbt.critstat); break;
	  case SBMSG_NVR_INIT: nvram_init(); break;
	  case SBMSG_GET_NAME:
		Reply(who, subbus_name, strlen(subbus_name)+1);
		continue; /* i.e. Don't do another reply */
	  case SBMSG_NVRR:
		nvram_dir(who, msg.nvrr.ID, msg.nvrr.size);
		continue; /* i.e. Don't do another reply */
	  case SBMSG_QUIT:
		Reply(who, NULL, 0); return;
	}
	Reply(who, NULL, 0);
  }
}

#define COMPANY "huarp"
#define LOCAL_SYMNAME(x) COMPANY "/" x

int main(int argc, char **argv) {
  int name_id, c;
  pid_t who;
  
  optind = 0;
  opterr = 0;
  do {
	c = getopt(argc, argv, "q");
	switch (c) {
	  case 'q':
		who = qnx_name_locate(getnid(), LOCAL_SYMNAME("subbus"), 0, NULL);
		if (who == -1) fprintf(stderr, "No subbus resident\n");
		else {
		  unsigned char msg;

		  msg = SBMSG_QUIT;
		  if (Send(who, &msg, NULL, sizeof(msg), 0) == -1)
			fprintf(stderr, "Error sending to subbus\n");
		}
		return(0);
	}
  } while (c != -1);
  
  /* Check basic assumptions about code segment */
  oldseg = FP_SEG(sbfs.read_subbus);
  assert(oldseg == FP_SEG(sbfs.writeack));
  assert(oldseg == FP_SEG(sbfs.read_ack));
  assert(oldseg == FP_SEG(sbfs.set_cmdenbl));
  assert(oldseg == FP_SEG(sbfs.read_novram));
  assert(oldseg == FP_SEG(sbfs.write_novram));
  assert(oldseg == FP_SEG(sbfs.read_switches));
  assert(oldseg == FP_SEG(sbfs.set_failure));
  assert(oldseg == FP_SEG(sbfs.read_rstcnt));
  assert(oldseg == FP_SEG(sbfs.read_pwrcnt));
  assert(oldseg == FP_SEG(sbfs.read_failure));
  
  /* Attach the name */
  name_id = qnx_name_attach(0, LOCAL_SYMNAME("subbus"));
  if (name_id == -1) {
	fprintf(stderr, "Subbus already resident\n");
	return(1);
  }
  
  /* Initialize the subbus */
  init_subbus();
  
  /* Wait for commands */
  operation();
  
  /* Terminate gracefully */
  cleanup();
  qnx_name_detach(0, name_id);
  return(0);
}
