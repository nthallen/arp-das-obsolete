/* 
    Buffer client functions.
    Written 5/25/92 by Eil.
*/

/* includes */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <search.h>
#include "globmsg.h"
#include "das.h"
#include "dc.h"
#include "dg.h"
#include "eillib.h"
#include "dfs_mod.h"
#include "bfr.h"
#include "bfr_vars.h"

/* function declarations */
void add_ll(llist *);
void del_ll(llist *);
int list_seen_check(llist *);
void client_check(void);
unsigned long send_cmd_or_stamp(arr *a, msg_hdr_type h);
int send_data(token_type nrows, arr *a);
int compare(pid_t *w, arr *a);

/* statics */
static int high_data;
static int high_other;

/* called when data is received from the dfs */
void DC_data(dbr_data_type *dr_data) {
  int d,nrows,circrows,atrow;
  if (DG_IS_STAR) DC_init(0,0);

  /* place in buffer, circularly */
  nrows=d=0;
  atrow=putrow;
  circrows=min(bfr_sz_rows-putrow,dr_data->n_rows);

  do {
    memcpy(bfr+atrow*tmi(nbrow),dr_data->data+d,circrows*tmi(nbrow));
    nrows+=circrows;
    t_data_seq += circrows;
    if (t_data_seq >= bfr_sz_rows) {
      data_seq += circrows;
      startrow = (startrow + circrows) % bfr_sz_rows;
    }
    d=circrows*tmi(nbrow);
    circrows=dr_data->n_rows-circrows;
    atrow=0;
  }	while (nrows<dr_data->n_rows);

  /* update variables */
  putrow=(putrow+dr_data->n_rows)%bfr_sz_rows;
}

/* called when a time stamp received from dfs */
void DC_tstamp(tstamp_type *tstamp)  {
  if (DG_IS_STAR) {
    /* add to stamp list */
    llist *t;

    DC_init(0,0);
    /* stamps are assumed on minor frame boundary */
    if (n_clients) {
      if ( !(t=(llist *)malloc(sizeof(llist))))
	msg(MSG_EXIT_ABNORM,"can't allocate memory in DC_tstamp");
      t->u.s=*tstamp;
      t->seq=t_data_seq;
      t->hdr=TSTAMP;
      add_ll(t);
    }
  }
}

/* called when a DASCmd is received from dfs */
void DC_DASCmd(unsigned char type, unsigned char number) {
  if (DG_IS_STAR) {
    llist *c;
    DC_init(0,0);
    if (n_clients) {
      if ( !(c=(llist *)malloc(sizeof(llist))) )
	msg(MSG_EXIT_ABNORM,"Can't allocate memory in DC_DASCmd");
      c->hdr=DCDASCMD;
      c->seq=t_data_seq;
      c->u.c.type=type;
      c->u.c.val=number;
      add_ll(c);
    }
  }
}

reply_type init_star_client(pid_t who) {
  /* initialise a buffered client */
  reply_type rv = DAS_NO_REPLY;
  arr *a=NULL;
  pid_t zero = 0;
  if (n_clients>=MAX_STAR_CLIENTS) rv=DAS_BUSY;
  if (rv==DAS_NO_REPLY) {
    a=FIND_CLIENT(who);
    if (!a) {
      /* add to array */
      a=(arr *)bsearch(&zero,clients,MAX_STAR_CLIENTS,sizeof(arr),compare );
      if (a==NULL) 
	msg(MSG_FATAL,"Programmatic Error in function init_star_client");
    }
    a->seq=list ? list->seq : t_data_seq-(t_data_seq%dbr_info.nrowminf);
    a->list_seen=list_seq;
    a->data_lost = 0L;
    a->who = who;
    n_clients++;
    qsort(clients,MAX_STAR_CLIENTS,sizeof(arr),compare);
  }
  return rv;
}

reply_type DC_other(unsigned char *msg_ptr, int who) {
  reply_type r=DAS_NO_REPLY;
  if (who)
    if (DG_IS_STAR) {
      if (got_quit) DG_init(0,0,0);
      DC_init(0,0);
      switch (*msg_ptr) {
      case DCINIT:
	r=init_star_client(who);
	break;
      case DCTOKEN:
	switch (*msg_ptr+1) {
	case 0: done_client(who); break;
	case UBYTE1_MAX: break;
	default:
	  {
	    arr *a=NULL;
	    a=FIND_CLIENT(who);
	    if (!a) {
	      r=DAS_UNKN;
	      msg(MSG_WARN,"request from unregistered task %d",who);
	    }
	    else send_cmd_or_stamp(a,DCDASCMD);
	  }
	}
	break;
      }
    }
  if (r==DAS_NO_REPLY) DG_process_msg();
  return(r);
}

int done_client(pid_t who) {
  arr *a=NULL;
  a=FIND_CLIENT(who);
  if (!a) msg(MSG_WARN,"bow out from unknown client %d",who);
  else {
    a->who=0;
    n_clients--;
    qsort(clients, MAX_STAR_CLIENTS, sizeof(arr), compare );
    while (list_seen_check(NULL));
  }
  return 0;
}

void add_ll(llist *addee) {
  /* adds addee to end of list */
  if (!(t_list))  t_list=list=addee;
  else {
    t_list->next=addee;
    t_list=addee;
  }
  addee->next=NULL;
  if (++stamps_and_cmds >= CHECK_CLIENTS) client_check();
  if (stamps_and_cmds > high_other) high_other = stamps_and_cmds;
  t_list_seq++;
}

void del_ll(llist *delee) {
  /* deletes delee from top of list */
  if (list==NULL || t_list==NULL) /*assert*/
    msg(MSG_FATAL,"Programmatic Error in function del_ll");
  if (list==t_list)  list=t_list=NULL;
  else list=delee->next;
  list_seq++;
  stamps_and_cmds--;
  free(delee);
}

int list_seen_check(llist *l) {
  int i;
  /* only remove from top of list */
  if (!list) return(0);
  if (l && l!=list) return(0);
  for (i=0;i<n_clients;i++)
    if (clients[i].list_seen<=list_seq) break;
  if (i>=n_clients) {
    int fin=0;
    if (list->hdr==DCDASCMD)
      if (list->u.c.type==DCT_QUIT && list->u.c.val==DCV_QUIT) fin=1;
    del_ll(list);
    if (fin) DC_bow_out();
    return(1);
  }
  return(0);
}

void client_check() {
  int i,num;
  pid_t ids[MAX_STAR_CLIENTS];

  if (!DG_IS_STAR) return;
  if (got_quit==1) {
    msg(MSG,"final data row sequence number: %lu",t_data_seq);
    msg(MSG,"highest buffered row usage: %d",high_data);
    msg(MSG,"final stamp/cmd sequence number: %lu",t_list_seq);
    msg(MSG,"highest number of stamps/cmds held at once: %d",high_other);
  }

  for (i=0,num=0;i<n_clients;i++) {
    if (got_quit==1 && clients[i].data_lost)
      msg(MSG,"star client %d lost %lu total rows of data",
	  clients[i].who,clients[i].data_lost);
    if (kill(clients[i].who,0)==-1)
      /* client crashed badly */
      ids[num++]=clients[i].who;
  }
  for (i=0;i<num;i++) {
    msg(MSG_WARN,"star client %d terminated ungracefully",ids[i]);
    msg(MSG_WARN,"un-registering task %d",ids[i]);
    done_client(ids[i]);
  }
}

/* sorting in descending order so null entries are at the end */
int compare(pid_t *w, arr *a) {
  if (*w==a->who) return(0);
  if (*w>a->who) return(-1);
  return(1);
}

/* returns 0 if sends a cmd or stamp. Otherwise sequence nummber of next */
unsigned long send_cmd_or_stamp(arr *a, msg_hdr_type h) {
  llist *l;
  int i;
  unsigned long lseq;
  if (!a) return 0;
  /* go through list of current stamps and cmds */
  for (i=0, l=list;
       l && (!dbr_info.tm_started || l->seq<(a->seq+dbr_info.nrowminf));
       l=l->next, i++) {
    lseq=list_seq+i;
    if (a->list_seen <= lseq && (l->seq<=a->seq || !dbr_info.tm_started))
      if (h==l->hdr) {
	if (l->hdr==DCDASCMD) {
	  dc_cmd=l;
	  DG_s_dascmd(l->u.c.type,l->u.c.val);
	} else DG_s_tstamp(&l->u.s);
	a->seq=l->seq;
	a->list_seen++;
	list_seen_check(l);
	return(0);
      }
  }
  if (l) return(l->seq);
  else return(ULONG_MAX);
}

int send_data(token_type nrows, arr *a) {
  /* Guarentee that data sent is always at minor frame resolution */
  int rows;
  int atrow;
  int circrows;
  int bfd;
  unsigned long req_seq;
  rows = nrows;
  req_seq = a->seq;

  /* check for loss of data to a client */
  if (req_seq < data_seq) {
    if (a->data_lost == 0L)
      msg(MSG_WARN,"client %d first lost %lu rows",
	  a->who, data_seq - req_seq);
    a->data_lost += (data_seq - req_seq);
    req_seq = a->seq = data_seq;
  }

  /* figure where in data buffer at a minor frame boundary */
  atrow = startrow + (req_seq - data_seq);
  bfd = atrow % dbr_info.nrowminf;
  if (bfd) {
    atrow += (dbr_info.nrowminf - bfd);
    req_seq += (dbr_info.nrowminf - bfd);
  }
  atrow %= bfr_sz_rows;

  /* check if we have any of required data yet */
  if (req_seq < t_data_seq) {
    bfd = t_data_seq - req_seq;
    if (got_quit && bfd < rows) rows = bfd;
    /* adjust request to figure minor frame resolution */
    rows = min(rows-(rows%dbr_info.nrowminf),
	       dbr_info.max_rows-(dbr_info.max_rows%dbr_info.nrowminf));
    if (rows <= 0) rows = dbr_info.nrowminf;
    if (bfd > high_data) high_data = bfd;
    circrows = min(rows, bfr_sz_rows - atrow);
    DG_s_data(circrows,bfr+atrow*tmi(nbrow),rows-circrows,bfr);
    a->seq += rows;
  } else DG_s_data(0,0,0,0);
  return 1;
}
