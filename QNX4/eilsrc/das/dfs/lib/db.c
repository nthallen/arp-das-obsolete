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
		       unsigned char n_rows, void *other, unsigned int size);

int DB_init() {
/* intialises the buf task as the buf task */	
char name[FILENAME_MAX+1];

/* initialise error handling if the user didn't */
if (!msg_initialised()) msg_init(DB_NAME,0,1,0,0,1);
  
/* attach name */
if ((db_id=qnx_name_attach(getnid(),LOCAL_SYMNAME(DB_NAME,name)))==-1)
    msg(MSG_EXIT_ABNORM,"Can't attach name %s",name);  

return 0;	
}

void DB_s_data(int who, unsigned short seq, unsigned short n_rows,
       unsigned char *data, unsigned short n_rows1, unsigned char *data1) {
unsigned char *d;
    /* sends the data back to the bclient */
    if (!n_rows) return;
    if (data1 && n_rows1) {
	if ( (d=malloc((n_rows+n_rows1)*tmi(nbrow)))==NULL)
	    msg(MSG_EXIT_ABNORM,"can't allocate memory in DB_s_data");
	memcpy(d,data,n_rows*tmi(nbrow));
	memcpy(d+(n_rows*tmi(nbrow)),data1,n_rows1);
	db_forward(who,DCDATA,seq,n_rows+n_rows1,d,(n_rows+n_rows1)*tmi(nbrow));
	free(d);
    }
    else db_forward(who,DCDATA,seq,n_rows,data,n_rows*tmi(nbrow));
}


void DB_s_tstamp(int who, unsigned short seq, tstamp_type *tstamp) {
    /* sends the stamp back to the bclient */	
    db_forward(who, TSTAMP, seq, 0, tstamp, sizeof(tstamp_type));	
}


void DB_s_dascmd(int who, unsigned short seq, unsigned char type,
		 unsigned char val) {
dascmd_type dasc;

    dasc.type=type;
    dasc.val=val;
    /* sends the cmd back to the bclient */	
    db_forward(who, DCDASCMD, seq, 0, &dasc, sizeof(dascmd_type));	
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
	if (!rq->req.min_rows && !rq->req.max_rows) dbr_info.num_clients--;
	DB_get_data(who, rq->hdr.seq, rq->req.min_rows, rq->req.max_rows);
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
    dbr_info.num_clients++;
  dbr_info.next_tid=next_tid;
  return 0;
}

static void db_forward(pid_t who, unsigned char msg_type, unsigned short seq,
		       unsigned char n_rows, void *other, unsigned int size) {
static struct _mxfer_entry slist[4];
int scount;

  _setmx(&slist[0],&msg_type,1);
  _setmx(&slist[1],&seq, 2);
  scount=2;
  if (n_rows > 0) {
    _setmx(&slist[2],&n_rows,1);
    scount++;
  }
  if (size > 0) {
    _setmx(&slist[scount],other,size);
    scount++;
  }
  if (Replymx(who, scount, slist)==-1)
    msg(MSG_WARN,"Can't reply to request of task %d",who);
    /* send a break to that task */

}
