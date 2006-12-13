/* 
    Buffer Generator functions.
*/

/* includes */
#include <sys/types.h>
#include <search.h>
#include <globmsg.h>
#include <eillib.h>
#include <gw.h>
#include <bfr.h>
#include "pbfr_vars.h"

void DG_DASCmd(unsigned char type, unsigned char val) {
	if (type==DCT_QUIT && val==DCV_QUIT) {
		DC_init(0,0);
		DC_operate();
	}
return;
}

reply_type DG_other(unsigned char *msg_ptr, pid_t who) {
return(GW_dg_other(who,msg_ptr));
}

/* Called when data buffer receives an order for data from its clients. */
int DG_get_data(token_type n_rows) {
llist *l;
arr *a;
int i;

/* find registered client */
if (n_clients && !(a=bsearch(&dbr_info.next_tid, clients, n_clients, sizeof(arr), compare ))) {
    msg(MSG_WARN,"request from unregistered task %d",dbr_info.next_tid);
    return(0);
}

/* traverse list to find any imminent stamps or cmds */
for (i=0, l=list, a->n_rows=n_rows;
     l && l->seq < (a->seq+dbr_info.nrowminf); l=l->next, i++)
	    if ( send_cmd_or_stamp(l,a,list_seq+i) ) return(1);

if (send_data( l ? min(l->seq - a->seq, n_rows) : n_rows, a)) return(1);

/* if request can't be fullfilled, raise flag */
a->rflag = 1;
requests++;

return(0);
}




