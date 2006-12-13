/* 
    Buffer Generator functions.
*/

/* includes */
#include <sys/types.h>
#include <search.h>
#include "globmsg.h"
#include "eillib.h"
#include "gw.h"
#include "bfr.h"
#include "bfr_vars.h"

void GW_dg_DASCmd(unsigned char type, unsigned char val) {
  GW_dc_DASCmd(type,val);
  if (DG_IS_STAR) {
    /* defeat the dg.c pass on */
    nt=dbr_info.next_tid;
    dbr_info.next_tid=0;
  }
}

reply_type GW_dg_other(unsigned char *msg_ptr, pid_t who) {
  return(DAS_UNKN);
}

/* Called when data buffer receives an order for data from its clients. */
int GW_dg_get_data(token_type n_rows) {
  arr *a;

  if (DG_IS_STAR) {
    llist *l;
    int i;
    if (!dbr_info.next_tid && nt) {
      dbr_info.next_tid=nt;
      nt=0;
    }

    /* find registered client */
    if (n_clients && !(a=bsearch(&dbr_info.next_tid, clients, n_clients,
				 sizeof(arr), compare ))) {
      msg(MSG_WARN,"request from unregistered task %d",dbr_info.next_tid);
      return(0);
    }

    /* traverse list to find any imminent stamps or cmds */
    for (i=0, l=list;
	 l && l->seq < (a->seq+dbr_info.nrowminf); l=l->next, i++)
      if ( send_cmd_or_stamp(l,a,list_seq+i) ) return(1);

    send_data( l ? min(l->seq - a->seq, n_rows) : n_rows, a);
  } else {
    /* not a star dg */
    clients[0].who = dbr_info.next_tid;
    send_data(n_rows, &clients[0]);
  }
  return 1;
}




