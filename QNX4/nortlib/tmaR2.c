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
	p->lastcheck = p->basetime - 1;
	if ( p->basetime == 0L && p->cmds != 0 ) {
	  /* Issue start-up commands (T==0, ">") */
	  for (;;) {
		cmds = &p->cmds[p->next_cmd];
		if ( cmds->dt != 0 || cmds->cmd[0] != '>' )
		  break;
		p->next_cmd++;
		ci_sendcmd(cmds->cmd+1, 0);
	  }
	}
  }
}

/* tma_process returns 0 when no more action is required for any
   partition at the present time (now) */
int tma_process( long int now ) {
  int partno, success, state_case;
  tma_prtn *p;
  long int dt, pdt, timeout, lastcheck;
  char *cmd;
  
  if (tma_runbasetime == 0L) {
	unsigned int i;

	tma_runbasetime = now;
	for (i = 0; i < tma_n_partitions; i++) {
	  tma_partitions[i].basetime = now;
	  tma_partitions[i].lastcheck = now-1;
	}
  }
  for ( partno = 0; partno < tma_n_partitions; partno++ ) {
	p = &tma_partitions[ partno ];
	if ( now != p->lastcheck ) {
	  lastcheck = p->lastcheck;
	  if ( lastcheck < p->basetime )
		lastcheck = p->basetime;
	  if ( p->waiting > 0 ) {
		dt = now - lastcheck;
		if ( dt >= p->waiting ) {
		  p->waiting = 0;
		  p->next_cmd++;
		}
	  }
	  if ( p->waiting == 0 && tma_is_holding == 0 ) {
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
				case '?':
				  if ( sscanf( cmd+1, "%d,%ld,%d",
					&success, &timeout, &state_case ) != 3 )
					nl_error( 4, "Error reading hold command\n" );
				  if ( timeout > 0 )
					timeout += pdt + p->basetime - lastcheck;
				  p->waiting = timeout;
				  p->next_cmd--;
				  return state_case;
				default:
				  nl_error( 1, "Onknown cmd char %c in tma_process", *cmd );
			  }
			}
		  }
		}
	  }
	}
  }
  for ( partno = 0; partno < tma_n_partitions; partno++ ) {
	p = &tma_partitions[ partno ];
	lastcheck = p->lastcheck;
	if ( lastcheck < p->basetime )
	  lastcheck = p->basetime;
	dt = now - lastcheck;
	if ( dt != 0 ) {
	  if ( p->waiting != 0 || tma_is_holding != 0 ) {
		p->basetime += dt;
		if ( p->waiting > dt ) {
		  p->waiting -= dt;
		} else if ( p->waiting > 0 )
		  p->waiting = 0;
	  }
	}
	if ( now != p->lastcheck ) {
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

void tma_succeed( int partno, int statecase ) {
  tma_prtn *p;
  int success, state_case;
  long int timeout;
  char *cmd;

  if ( partno < 0 || partno >= tma_n_partitions )
	nl_error( 4, "Invalid partno in tma_succeed" );
  p = &tma_partitions[ partno ];
  if ( p->cmds != 0 && p->next_cmd >= 0 ) {
	cmd = p->cmds[p->next_cmd].cmd;
	if ( cmd != 0 && *cmd == '?' ) {
	  if ( sscanf( cmd+1, "%d,%ld,%d",
				&success, &timeout, &state_case ) != 3 )
		nl_error( 4, "Error re-reading hold command\n" );
	  if ( state_case != statecase )
		nl_error( 1, "Different statecase active in tma_succeed" );
	  else {
		p->waiting = 0;
		p->next_cmd += success;
	  }
	  return;
	}
  }
  nl_error( 1, "Hold not active in tma_succeed" );
}
