#include "nortlib.h"
#include "tma.h"
char rcsid_tmaR1_c[] = 
  "$Header$";

/* Check to see if a significant time is up. Also display
   pertinent time values. I do not display next times of 0,
   since tma_time_check is only called prior to switching
   to a new substate, and the new substate always calls
   tma_new_time which will display a new next time, possibly
   zero.
   I initialize tma_runbasetime (and all the partitions) here,
   since here I am guaranteed that itime() returns a valid
   number.
*/
int tma_time_check(unsigned int partition) {
  tma_prtn *p;
  long int now, dt;
  
  if (partition < tma_n_partitions) {
	p = &tma_partitions[partition];
	now = itime();
	if (tma_runbasetime == 0L) {
	  unsigned int i;

	  tma_runbasetime = now;
	  for (i = 0; i < tma_n_partitions; i++)
		tma_partitions[i].basetime = tma_partitions[i].lastcheck = now;
	}
	if (now != p->lastcheck) {
	  tma_time(p, RUNTIME_COL, now - tma_runbasetime);
	  dt = now - p->lastcheck;
	  p->lastcheck = now;
	  if (tma_is_holding) {
		p->basetime += dt;
	  } else {
		tma_time(p, STATETIME_COL, now - p->basetime);
		if (p->nexttime != LONG_MAX) {
		  p->nexttime -= dt;
		  if (p->nexttime <= 0) {
			p->nexttime = 0;
			return(1);
		  }
		  tma_time(p, NEXTTIME_COL, p->nexttime);
		}
	  }
	}
  }
  return(0);
}

/* Display Command at debug(2)
   Send Command if that option is specified
*/
void tma_sendcmd(const char *cmd) {
  ci_sendcmd(cmd, 0);
}

void tma_new_time(unsigned int partn, long int t1, const char *next_cmd) {
  tma_prtn *p;
  
  if (partn < tma_n_partitions) {
	tma_next_cmd( partn, next_cmd );
	p = &tma_partitions[partn];
	if (t1 == 0) p->nexttime = LONG_MAX;
	else p->nexttime = t1 - (p->lastcheck - p->basetime);
	if (p->row >= 0) {
	  tma_time(p, NEXTTIME_COL,
			(p->nexttime==LONG_MAX) ? 0L : p->nexttime);
	}
  }
}
