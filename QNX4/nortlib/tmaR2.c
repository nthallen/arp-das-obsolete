#include <stdlib.h>
#include "nortlib.h"
#include "tma.h"

void tma_init_state( int partno, tma_state *cmds, char *name ) {
  tma_prtn *p;

  if ( partno < tma_n_partitions ) {
	/* tma_new_state() sets part basetime, outputs STATETIME
	   Does *not* set RUNTIME or NEXTTIME or next command area
	   basetime is set only if tma_basetime is non-zero (set
	   in tma_process first time after TM starts). lastcheck is 
	   set to basetime. partno, name );
	 */
	tma_new_state( partno, name );
	p = &tma_partitions[partno];
	p->cmds = cmds;
	p->next_cmd = 0;
	p->waiting = 0;
	p->lastcheck = -1;
	if ( p->basetime == 0L )
	  tma_process( 0L );
  }
}

int tma_process( long int now ) {
  int partno;
  tma_prtn *p;
  long int dt, pdt;
  char *cmd;
  
  if (tma_runbasetime == 0L) {
	unsigned int i;

	tma_runbasetime = now;
	for (i = 0; i < tma_n_partitions; i++)
	  tma_partitions[i].basetime = now;
  }
  for ( partno = 0; partno < tma_n_partitions; partno++ ) {
	p = &tma_partitions[ partno ];
	if ( now != p->lastcheck ) {
	  dt = now - p->basetime;
	  if ( p->cmds != 0 && p->next_cmd >= 0 ) {
		for (;;) {
		  pdt = p->cmds[p->next_cmd].dt;
		  cmd = p->cmds[p->next_cmd].cmd;
		  if ( pdt == -1 ) {
			p->next_cmd = -1;
			p->nexttime = 0;
			break;
		  } else if ( pdt > dt ) {
			p->nexttime = pdt;
			if ( ( p->next_str == 0 || *p->next_str == '\0' )
				  && *cmd == '>' )
			  p->next_str = cmd+1;
			break;
		  } else {
			p->next_cmd++;
			switch( *cmd ) {
			  case '>':
				ci_sendcmd(cmd+1, 0);
				p->next_str = "";
				break;
			  case '"':
				p->next_str = cmd;
				break;
			  case '#':
				return atoi( cmd+1 );
			  default:
				nl_error( 1, "Onknown cmd char %c in tma_process", *cmd );
			}
		  }
		}
	  }
	}
  }
  for ( partno = 0; partno < tma_n_partitions; partno++ ) {
	p = &tma_partitions[ partno ];
	if ( p->lastcheck != now ) {
	  p->lastcheck = now;
	  if ( p->row >= 0 ) {
		if ( p->next_str != 0 ) {
		  tma_next_cmd( partno, p->next_str );
		  p->next_str = NULL;
		}
		tma_time( p, RUNTIME_COL, now - tma_runbasetime );
		tma_time( p, STATETIME_COL, now - p->basetime );
		pdt = p->nexttime;
		switch ( pdt ) {
		  case -1: continue;
		  case 0:
			pdt = 0;
			p->nexttime = -1;
			break;
		  default:
			pdt += p->basetime - now;
			break;
		}
		tma_time( p, NEXTTIME_COL, pdt );
	  }
	}
  }
  return 0; /* Nothing left to do */
}
