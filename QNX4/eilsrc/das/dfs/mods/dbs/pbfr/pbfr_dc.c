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
#include <gw.h>
#include <dfs_mod.h>
#include <bfr.h>
#include "pbfr_vars.h"

/* function declarations */
void add_ll(llist *);
void del_ll(llist *);
int list_seen_check(void);
void client_check(void);
void resolve_requests(msg_hdr_type hdr);
int send_cmd_or_stamp(llist *l, arr *a, unsigned long lseq);
int send_data(token_type nrows, arr *a);
int compare(pid_t *w, arr *a);
int compare_min_seen(unsigned long *seen, arr *a);

/* statics */
static int high_data;
static int high_other;

/* called when data is received from the dbr */
void DC_data(dbr_data_type *dr_data) {
int d,nrows,circrows,atrow;

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


reply_type init_star_client(pid_t who) {
/* initialise a buffered client */
unsigned char rv = DAS_OK;
struct {
	msg_hdr_type h;
	dbr_data_type d;
} ready = {DCDATA,0,0};
int pr;
arr *a;
  if (n_clients>=MAX_STAR_CLIENTS) rv=DAS_BUSY;
  if (rv==DAS_OK) {
    a=(arr *)lsearch( &who, clients, &n_clients, sizeof(arr), compare );
    a->seq=list ? list->seq : t_data_seq-(t_data_seq%dbr_info.nrowminf);
    a->n_rows=0;
    a->rflag=0;
    a->list_seen=list_seq;
    a->data_lost = 0L;
    a->who = who;
    qsort(clients,n_clients,sizeof(arr),compare);
  }
  return rv;
}

int done_client(pid_t who) {
arr *a;
    a=(arr *)lfind(&who, clients, &n_clients, sizeof(arr), compare );
    if (a==NULL) msg(MSG_WARN,"bow out from unknown client %d",who);
    else {
		if (--n_clients == 0)
			msg(MSG,"all my star clients bowed out");
		a->who=0;
		qsort(clients, MAX_STAR_CLIENTS,sizeof(arr), compare );
		while (list_seen_check());
    }
	return 0;
}


reply_type DC_other(unsigned char *msg_ptr, int who) {
reply_type r=DAS_OK;

switch (*msg_ptr) {
	case DCINIT:
		if ( (r=init_star_client(who)) != DAS_OK) return(r);
		break;
	case DCTOKEN:
		if ( *(msg_ptr+1)==0) {
			done_client(who);
			return(r);
		}
		break;
}
return(GW_dc_other( *msg_ptr==DCINIT ? 0 : who ));
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
	    done_client(ids[i]);
    }
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
	if (l->hdr==DCDASCMD) GW_s_dascmd(a->who,l->u.c.type,l->u.c.val);
	else GW_s_tstamp(a->who,&l->u.s);
	a->seq=l->seq;
	a->list_seen++;
	if (l==list) list_seen_check(l);
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
    GW_s_data(a->who,circrows,bfr+atrow*tmi(nbrow),rows-circrows,bfr);
    a->seq += rows;
    return(1);
}
return 0;
}
