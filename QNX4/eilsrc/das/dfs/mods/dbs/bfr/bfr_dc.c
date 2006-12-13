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
#include <sys/kernel.h>
#include <sys/types.h>
#include <sys/psinfo.h>
#include <search.h>
#include <assert.h>
#include <globmsg.h>
#include <das.h>
#include <eillib.h>
#include <dbr.h>
#include <dbr_mod.h>
#include <bfr.h>
#include "bfr_vars.h"

/* function declarations */
void add_ll(llist *);
void del_ll(llist *);
int list_seen_check(void);
void client_check(void);
static int init_star_client(pid_t who);
void resolve_requests(msg_hdr_type hdr);
int send_cmd_or_stamp(llist *l, arr *a, unsigned long lseq);
int send_data(token_type nrows, arr *a);
int compare(pid_t *w, arr *a);
int compare_min_seen(unsigned long *seen, arr *a);

/* statics */
static int high_data;
static int high_other;
static db_msg_type *rq;

/* called when data is received from the dbr */
void DC_data(dbr_data_type *dr_data) {
int d,nrows,circrows,atrow;

    if (dbr_info.mod_type == DBC)
	while ( (rq_who = Creceive(0, &rq_buf, sizeof(rq_buf))) != -1)
	    DC_other( (unsigned char *)&rq_buf, rq_who);

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
    resolve_requests(DCDATA);
}

/* called when a time stamp received from dbr */
void DC_tstamp( tstamp_type *tstamp )  {
/* add to stamp list */
llist *t;

    /* stamps are assumed on minor frame boundary */

    if (dbr_info.mod_type == DBC)
	while ( (rq_who = Creceive(0, &rq_buf, sizeof(rq_buf))) != -1)
	    DC_other( (unsigned char *)&rq_buf, rq_who);

    if (n_clients) {
	if ( !(t=(llist *)malloc(sizeof(llist))))
	    msg(MSG_EXIT_ABNORM,"can't allocate memory in DC_tstamp");
	t->u.s=*tstamp;
	t->seq=t_data_seq;
	t->hdr=TSTAMP;
	add_ll(t);
	resolve_requests(TSTAMP);
    }
}

/* called when a DASCmd is received from dbr */
void DC_DASCmd(unsigned char type, unsigned char number) {
llist *c;

    /* commands are assumed on minor frame boundary */

    if (dbr_info.mod_type == DBC)
	while ( (rq_who = Creceive(0, &rq_buf, sizeof(rq_buf))) != -1)
	    DC_other( (unsigned char *)&rq_buf, rq_who);

    if (type==DCT_QUIT && number==DCV_QUIT) got_quit++;
    if (got_quit==1) client_check();

    /* add to cmd list */
    if (n_clients) {
	if ( !(c=(llist *)malloc(sizeof(llist))) )
	    msg(MSG_EXIT_ABNORM,"can't allocate memory in DC_DASCmd");
	c->hdr=DCDASCMD;
	c->seq=t_data_seq;
	c->u.c.type=type;
	c->u.c.val=number;
	add_ll(c);
	resolve_requests(DCDASCMD);
    }
}

void DC_other(unsigned char *msg_ptr, pid_t who) {
unsigned char r = DAS_UNKN;

  rq=(struct db_msg_type *)msg_ptr;
  switch (rq->hdr) {
    case DCTOKEN:
	/* check for null request = bow_out */
	if ( !rq->n_rows) {
	    if (un_register(who)) r=DAS_OK;
	    Reply(who,&r,sizeof(reply_type));
	    if (got_quit && !n_clients) DB_exitfunction();
	} else DB_get_data(who, rq->n_rows);
    break;
    case DCINIT: init_star_client(who);  break;
    default:
	msg(MSG_WARN,"unknown msg received from %d with type %d",who,rq->hdr);
	if ( (Reply(who, &r, 1)) == -1 )
	    msg(MSG_WARN,"error replying UNKNOWN to task %d",who);
	break;
  }
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
    assert( list && t_list );
    if (list==t_list)  list=t_list=NULL;
    else list=delee->next;
    list_seq++;
    stamps_and_cmds--;
    free(delee);
}

int list_seen_check() {
if (list)
    if (list_seen >= list_seq && lfind(&list_seen, clients,
	&n_clients, sizeof(arr), compare_min_seen )==NULL ) {
    list_seen++;
    del_ll(list);
    return(1);
}
return(0);
}

int un_register(pid_t who) {
arr *a;
    a=bsearch(&who, clients, n_clients, sizeof(arr), compare );
    if (a==NULL) return(0);
    else { 
	n_clients--;
	msg(MSG,"task %d bowed out from star, (center node %d)",who, getnid());
	a->who=0;
	qsort(clients, MAX_STAR_CLIENTS,sizeof(arr), compare );
	while (list_seen_check());
    }
    return 1;
}

void client_check() {
int i,num;
struct _psinfo2 info;
pid_t ids[MAX_STAR_CLIENTS];
static int did_msg;

    if (got_quit) {
	msg(MSG,"final data row sequence number: %lu",t_data_seq);
	msg(MSG,"highest buffered row usage: %d",high_data);
	msg(MSG,"final stamp/cmd sequence number: %lu",t_list_seq+1);
	msg(MSG,"highest number of stamps/cmds held at once: %d",high_other+1);
    }

    for (i=0,num=0;i<n_clients;i++) {
	if (got_quit && clients[i].data_lost)
	    msg(MSG,"star client %d lost %lu total rows of data",clients[i].who,clients[i].data_lost);
	qnx_getids(clients[i].who, &info);
	if (info.pid!=clients[i].who)
	    /* client crashed badly */
	    ids[num++]=clients[i].who;
    }
    if (stamps_and_cmds >= CHECK_CLIENTS && !num) {
	if (!did_msg++)
	    msg(MSG_WARN,"lots of stamps/commands being held");
    }
    else for (i=0;i<num;i++) {
	    msg(MSG_WARN,"recognised that star client %d crashed",ids[i]);
	    msg(MSG_WARN,"un-registering task %d",ids[i]);
	    un_register(ids[i]);
    }
}

static int init_star_client(pid_t who) {
/* initialise a buffered client */
unsigned char rv = DAS_OK;
arr *a;
  if (n_clients>=MAX_STAR_CLIENTS) rv=DAS_BUSY;
  DB_s_init_client(who,rv);
  if (rv==DAS_OK) {
    a=(arr *)lsearch( &who, clients, &n_clients, sizeof(arr), compare );
    a->seq=list ? list->seq : t_data_seq-(t_data_seq%dbr_info.nrowminf);
    a->n_rows=0;
    a->rflag=0;
    a->list_seen=list_seq;
    a->data_lost = 0L;
    qsort(clients,n_clients,sizeof(arr),compare);
    msg(MSG,"task %d is my star client, (center node %d)",who, getnid());
  }
  return 0;
}
    
/* sorting in descending order so null entries are at the end */
int compare(pid_t *w, arr *a) {
    if (*w==a->who) return(0);
    if (*w>a->who) return(-1);
    return(1);
}

int compare_min_seen(unsigned long *seen, arr *a) {
    if (*seen==a->list_seen) return(0);
    return(1);
}

void resolve_requests(msg_hdr_type hdr) {
int i;
    /* resolve outstanding requests */
    for (i=0; i<n_clients && requests; i++)
	if (clients[i].rflag)
	    switch (hdr) {
		case TSTAMP:
		case DCDASCMD:
		    if (send_cmd_or_stamp(t_list,&clients[i],t_list_seq-1)) {
			clients[i].rflag=0;
			requests--;
			break;
		    }
		case DCDATA:
		    /* send data */
 		    if (send_data(hdr!=DCDATA ? t_list->seq-clients[i].seq : clients[i].n_rows, &clients[i])) {
			clients[i].rflag=0;
			requests--;
		    }
	    }
}

int send_cmd_or_stamp(llist *l, arr *a, unsigned long lseq) {
    if ( a->list_seen <= lseq && (l->seq==a->seq ||
	(l->seq > a->seq && (t_data_seq - a->seq) < dbr_info.nrowminf))) {
	if (l->hdr==DCDASCMD) DB_s_dascmd(a->who,l->u.c.type,l->u.c.val);
	else DB_s_tstamp(a->who,&l->u.s);
	a->seq=l->seq;
	a->list_seen++;
	if (l==list) list_seen_check();
	return(1);
    }
    return(0);
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

if (!rows) return(0);

/* check for loss of data to a client */
if (req_seq < data_seq) {
    if (a->data_lost == 0L)
	msg(MSG_WARN,"star client %d first lost %lu rows",a->who, data_seq - req_seq);
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
if (req_seq >= t_data_seq) return(0);

bfd = t_data_seq - req_seq;
if (got_quit && bfd < rows) rows = bfd;

/* adjust request to figure minor frame resolution */
rows = min(rows-(rows%dbr_info.nrowminf),
	   dbr_info.max_rows-(dbr_info.max_rows%dbr_info.nrowminf));
if (rows <= 0) rows = dbr_info.nrowminf;

/* if can do */
if ( bfd >= rows ) {
    if (bfd > high_data) high_data = bfd;
    circrows = min(rows, bfr_sz_rows - atrow);
    DB_s_data(a->who,circrows,bfr+atrow*tmi(nbrow),rows-circrows,bfr);
    a->seq += rows;
    return(1);
}
return 0;
}
