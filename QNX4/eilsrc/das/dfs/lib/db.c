/*
	buffer library functions.
	Written By Eil.
	Ported to QNX 4 by Eil 4/25/92.
*/


/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <sys/name.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <malloc.h>
#include <sys/sendmx.h>
#include <dac.h>
#include <globmsg.h>
#include <das_utils.h>
#include <dbr_utils.h>

/* globals */
int db_id;

/* function declarations */
static int init_client_2(int who);
static void db_forward(pid_t who, unsigned char msg_type, unsigned short seq,
		       unsigned char n_rows, void *other, unsigned char n_rows1, void *other1);

int DB_init() {
/* intialises the buf task as the buf task */	
char name[FILENAME_MAX+1];

/* initialise error handling if the user didn't */
if (!msg_initialised()) msg_init(DB_NAME,0,1,0,0,1);
  
/* attach name */
if ((db_id=qnx_name_attach(getnid(),LOCAL_SYMNAME(DB_NAME,name)))==-1)
    msg(MSG_EXIT_ABNORM,"Can't attach name %s",name);  

num_clients=0;

return 0;	
}

void DB_s_data(int who, unsigned short seq, unsigned short n_rows,
       unsigned char *data, unsigned short n_rows1, unsigned char *data1) {

    /* sends the data back to the bclient */
    if (!n_rows) return;
    db_forward(who,DCDATA,seq,n_rows,data,n_rows1, data1);
}


void DB_s_tstamp(int who, unsigned short seq, tstamp_type *tstamp) {
    /* sends the stamp back to the bclient */	
    db_forward(who, TSTAMP, seq, 0, tstamp, 0,0);
}


void DB_s_dascmd(int who, unsigned short seq, unsigned char type,
		 unsigned char val) {
dascmd_type dasc;

    dasc.type=type;
    dasc.val=val;
    /* sends the cmd back to the bclient */	
    db_forward(who, DCDASCMD, seq, 0, &dasc,0,0);
}

/* some signals other than DC signals are special for DB */
int DB_operate(int who, unsigned char *msg_ptr) {
unsigned char r = DAS_UNKN;
struct db_msg_type {
    msg_header_type hdr;
    db_request_type req;
} *rq;

rq=(struct db_msg_type *)msg_ptr;

  switch (rq->hdr.msg_type) {          	
    case REQUEST:
	/* check for null request */
	if (!rq->req.min_rows && !rq->req.max_rows) num_clients--;
	else DB_get_data(who, rq->hdr.seq, rq->req.min_rows, rq->req.max_rows);
    break;
    case DCINIT:
	init_client_2(who);
    break;
    default: msg(MSG_WARN,"unknown msg received from %d with type %d",who,rq->hdr.msg_type);
	Reply(who, &r, 1);
  }
  return 0;
}


static int init_client_2(int who) {
/* initialise a buffered client */
unsigned char rv = DAS_OK;
struct _mxfer_entry mlist[2];
int next_tid;

  next_tid = dbr_info.next_tid;
  dbr_info.next_tid=getpid();
  _setmx(&mlist[0],&rv, 1);
  _setmx(&mlist[1],&dbr_info,sizeof(dbr_info));
  if (!Replymx(who, 2, mlist))
    num_clients++;
  dbr_info.next_tid=next_tid;

  return 0;
}

static void db_forward(pid_t who, unsigned char msg_type, unsigned short seq,
		       unsigned char n_rows, void *other, unsigned char n_rows1, void *other1) {
static struct _mxfer_entry slist[5];
unsigned char tmp;
int scount;

  _setmx(&slist[0],&msg_type,1);
  _setmx(&slist[1],&seq, 2);
  scount=3;
  switch(msg_type) {
    case DCDATA:
	assert(n_rows);
	tmp=n_rows;
	if (other1 && n_rows1) tmp+=n_rows1;
	_setmx(&slist[2],&tmp,1);
	_setmx(&slist[3],other,n_rows*tmi(nbrow));
	scount=4;
	if (other1 && n_rows1) {
	_setmx(&slist[4],other1,n_rows1*tmi(nbrow));
	    scount=5;
	}
	break;
    case TSTAMP:
	_setmx(&slist[2],other,sizeof(tstamp_type));	
	break;
    case DCDASCMD:
	_setmx(&slist[2],other,sizeof(dascmd_type));	
	break;
    default: msg(MSG_EXIT_ABNORM,"in db_forward, type %d",msg_type);
  }

  if (Replymx(who, scount, slist)==-1)
    msg(MSG_WARN,"Can't reply to request of task %d",who);
    /* send a break to that task */

}
