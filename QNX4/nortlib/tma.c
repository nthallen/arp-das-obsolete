/* tma.c Defines TMA support services
 * $Log$
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "nortlib.h"
#include "nl_cons.h"
#include "tmctime.h"
#ifdef __WATCOMC__
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
#endif

struct prtn {
  long int basetime; /* Time current state began */
  long int nexttime; /* Seconds until next event */
  long int lastcheck; /* Time of last check */
  int console;
  int row;
};
static struct prtn *partitions = NULL;
static unsigned int n_partitions = 0;
static long int runbasetime = 0L;
static int really_send_commands = 1;
int tma_is_holding = 0;

/*
>Command Text Goes Here
Initialize        Holding   Next: 00:00:00  State: 00:00:00  Run: 00:00:00
SNAME             HOLDING   NEXT  NEXTTIME  STATE  STATETIME RUN  RUNTIME
*/
#define HOLDING_OFFSET ((80+18)*2)
#define STATE_OFFSET ((80+42)*2)
#define RUN_OFFSET ((80+59)*2)
#define SNAME_OFFSET (80*2)
#define NEXTTIME_OFFSET ((80+34)*2)
#define STATETIME_OFFSET ((80+51)*2)
#define RUNTIME_OFFSET ((80+66)*2)
#define HOLDING_ATTR 0x70
#define TMA_ATTR 0x70
#define CMD_ATTR 7

static void tma_time(struct prtn *p, unsigned int offset, long int t) {
  char ts[9];
  int hh, h;
  
  if (p->row >= 0) {
	ts[8] = '\0';
	hh = t % 60;
	h = hh % 10;
	ts[7] = h+'0';
	ts[6] = hh/10 + '0';
	ts[5] = ':';
	t /= 60;
	hh = t % 60;
	h = hh % 10;
	ts[4] = h+'0';
	ts[3] = hh/10 + '0';
	ts[2] = ':';
	t /= 60;
	hh = t % 100;
	h = hh % 10;
	ts[1] = h+'0';
	ts[0] = hh/10 + '0';
	nlcon_display(p->console, p->row + offset, ts, TMA_ATTR);
  }
}

static void update_holding(struct prtn *p) {
  if (p->row >= 0)
	nlcon_display(p->console, p->row + HOLDING_OFFSET,
	  tma_is_holding ? "Holding" : "       ", HOLDING_ATTR);
}

void tma_hold(int hold) {
  int i, update;

  update = 0;
  if (hold) {
	if (! tma_is_holding) {
	  tma_is_holding = 1;
	  update = 1;
	}
  } else {
	if (tma_is_holding) {
	  tma_is_holding = 0;
	  update = 1;
	}
  }
  if (update)
	for (i = 0; i < n_partitions; i++) update_holding(&partitions[i]);
}

void tma_new_state(unsigned int partition, const char *name) {
  struct prtn *p;

  if (partition < n_partitions) {
	nl_error(-2, "Entering State %s", name);
	p = &partitions[partition];
	p->basetime = itime();
	p->lastcheck = p->basetime;
	p->nexttime = 0;
	if (runbasetime == 0) runbasetime = p->basetime;
	if (p->row >= 0) {
	  nlcon_display(p->console, p->row + STATE_OFFSET,
		"  State: ", TMA_ATTR);
	  nlcon_display(p->console, p->row + RUN_OFFSET,
		"  Run: ", TMA_ATTR);
	  nlcon_display(p->console, p->row + SNAME_OFFSET,
		"                            Next: ", TMA_ATTR);
	  nlcon_display(p->console, p->row + SNAME_OFFSET,
		name, TMA_ATTR);
	  update_holding(p);
	  tma_time(p, STATETIME_OFFSET, 0);
	}
  }
}

/* specify the next significant time for a partition */
void tma_new_time(unsigned int partn, long int t1, const char *next_cmd) {
  struct prtn *p;
  const char *txt;
  int len;
  
  if (partn < n_partitions) {
	p = &partitions[partn];
	p->nexttime = t1;
	if (p->row >= 0) {
	  nlcon_display(p->console, p->row, ">", TMA_ATTR);
	  nlcon_display(p->console, p->row+2,
		"                                        "
		"                                       ", CMD_ATTR);
	  len = strlen(next_cmd);
	  if (len > 79) txt = next_cmd + (len - 79);
	  else txt = next_cmd;
	  nlcon_display(p->console, p->row+2, txt, CMD_ATTR);
	  tma_time(p, NEXTTIME_OFFSET, t1);
	}
  }
}

/* Check to see if a significant time is up. Also display
   pertinent time values. I do not display next times of 0,
   since tma_time_check is only called prior to switching
   to a new substate, and the new substate always calls
   tma_new_time which will display a new next time, possibly
   zero.
*/
int tma_time_check(unsigned int partition) {
  struct prtn *p;
  long int now, dt;
  
  if (partition < n_partitions) {
	p = &partitions[partition];
	now = itime();
	if (now != p->lastcheck) {
	  tma_time(p, RUNTIME_OFFSET, now - runbasetime);
	  dt = now - p->lastcheck;
	  if (tma_is_holding) {
		p->basetime += dt;
	  } else {
		p->nexttime -= dt;
		if (p->nexttime <= 0) {
		  p->nexttime = 0;
		  return(1);
		}
		tma_time(p, STATETIME_OFFSET, now - p->basetime);
		tma_time(p, NEXTTIME_OFFSET, p->nexttime);
	  }
	}
  }
  return(0);
}

/* Display Command at debug(2)
   Send Command if that option is specified
*/
void tma_sendcmd(const char *cmd) {
  int len;
  
  len = strlen(cmd);
  if (len > 0 && cmd[len-1] == '\n') len--;
  nl_error(-3, "%*.*s", len, len, cmd);
  if (really_send_commands)
	ci_sendcmd(cmd, 0);
}

/* calls Con_init_options() and
   cic_options()
*/
void tma_init_options(const char *hdr, int nparts, int argc, char **argv) {
  int c, con_index, part_index;

  Con_init_options(argc, argv);
  cic_options(argc, argv, hdr);
  n_partitions = nparts;
  partitions = malloc(sizeof(struct prtn) * n_partitions);
  if (partitions == NULL)
	nl_error(3, "No memory for partitions table");
  for (c = 0; c < n_partitions; c++)
	partitions[c].row = -1;

  optind = 0; /* start from the beginning */
  opterr = 0; /* disable default error message */
  con_index = 0;
  part_index = 0;
  while ((c = getopt(argc, argv, opt_string)) != -1) {
	switch (c) {
	  case 'a': /* move on to the next console */
		con_index++;
		break;
	  case 'r':
		if (part_index < n_partitions) {
		  partitions[part_index].row = atoi(optarg) * 80 * 2;
		  partitions[part_index].console = con_index++;
		  part_index++;
		}
		break;
	  case 'p':
		really_send_commands = 0;
		break;
	  case '?':
		nl_error(3, "Unrecognized Option -%c", optopt);
	}
  }
}
