/* 
    Buffer Generator functions.
*/

/* includes */
#include <sys/types.h>
#include <search.h>
#include "globmsg.h"
#include "eillib.h"
#include "dg.h"
#include "dc.h"
#include "bfr.h"
#include "bfr_vars.h"

void DG_DASCmd(unsigned char type, unsigned char number) {
  /* i am about to forward a cmd to dbr_info.next_tid */
  if (DG_IS_STAR) {
    llist *c;
    arr *a=NULL;

    /* commands are assumed on minor frame boundary */
    if (type==DCT_QUIT && number==DCV_QUIT) got_quit++;
    if (got_quit==1) client_check();

    a=FIND_CLIENT(dbr_info.next_tid);
    if (!a) msg(MSG_EXIT_ABNORM,"Programmatic Error in DG_DASCmd");

    if (dc_cmd==NULL) {
      if (n_clients>1) {
	if ( !(c=(llist *)malloc(sizeof(llist))) )
	  msg(MSG_EXIT_ABNORM,"Can't allocate memory in DG_DASCmd");
	c->hdr=DCDASCMD;
	c->seq=t_data_seq;
	c->u.c.type=type;
	c->u.c.val=number;
	add_ll(c);
      }
      if (t_data_seq > a->seq) {
	if (a->data_lost == 0L)
	  msg(MSG_WARN,"client %d first lost %lu rows",
	      a->who, t_data_seq - a->seq);
	a->data_lost += (t_data_seq - a->seq);
      }
      a->seq=t_data_seq;
      a->list_seen=t_list_seq;
      if (c==list) list_seen_check();
    } else {
      a->seq=dc_cmd->seq;
      a->list_seen++;
      if (dc_cmd==list) list_seen_check();
    }

    dc_cmd = NULL;
    if (type==DCT_QUIT && number==DCV_QUIT) {
      done_client(dbr_info.next_tid);
      if (n_clients<1) DC_bow_out();
    }
  }
}

reply_type DG_other(unsigned char *msg_ptr, pid_t who) {
  return(DAS_UNKN);
}

/* Called when data buffer receives an order for data from its clients. */
int DG_get_data(token_type n_rows) {
  if (DG_IS_STAR) {
    arr *a=NULL;
    unsigned long nseq;
    a=FIND_CLIENT(dbr_info.next_tid);
    if (!a) {
      msg(MSG_WARN,"request from unregistered task %d",dbr_info.next_tid);
      return(0);
    }
    if (!(nseq=send_cmd_or_stamp(a,TSTAMP))) goto out;
    send_data( nseq ? min(nseq - a->seq, n_rows) : n_rows, a);
  } else {
    /* not a star dg */
    clients[0].who = dbr_info.next_tid;
    send_data(n_rows, &clients[0]);
  }
 out:
  return 1;
}




