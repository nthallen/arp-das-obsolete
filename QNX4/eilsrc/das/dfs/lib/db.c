/*
	buffer library functions.
	Written By Eil.
	Ported to QNX 4 by Eil 4/25/92.
*/


/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <sys/name.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <sys/psinfo.h>
#include <sys/sendmx.h>
#include <globmsg.h>
#include <das_utils.h>
#include <dbr_utils.h>

/* function declarations */
static void db_forward(pid_t who, msg_hdr_type msg_type,
       token_type n_rows, void *other, token_type n_rows1, void *other1);

int DB_init() {
/* intialises the buf task as the buf task */	
char name[FILENAME_MAX+1];

/* initialise error handling if the user didn't */
if (!msg_initialised()) msg_init(DB_NAME,0,1,-1,0,1,1);
  
/* attach name */
if (qnx_name_attach(getnid(),LOCAL_SYMNAME(DB_NAME,name))==-1)
    msg(MSG_EXIT_ABNORM,"Can't attach name %s",name);  

if ( (qnx_pflags(~0,_PPF_PRIORITY_REC,0,0)) == -1)
    msg(MSG_EXIT_ABNORM,"Can't set receiving priority order");

return 0;	
}

void DB_s_data(pid_t who, token_type n_rows,
       unsigned char *data, token_type n_rows1, unsigned char *data1) {
    /* sends the data back to the bclient */
    if (!n_rows) return;
    db_forward(who,DCDATA,n_rows,data,n_rows1, data1);
}


void DB_s_tstamp(pid_t who, tstamp_type *tstamp) {
    /* sends the stamp back to the bclient */	
    db_forward(who, TSTAMP, 0, tstamp, 0,0);
}

void DB_s_dascmd(pid_t who, unsigned char type, unsigned char val) {
dascmd_type dasc;
    dasc.type=type;
    dasc.val=val;
    /* sends the cmd back to the bclient */	
    db_forward(who, DCDASCMD, 0, &dasc,0,0);
}

void DB_s_init_client(pid_t who, unsigned char r, token_type t) {
pid_t next_tid;
    next_tid = dbr_info.next_tid;
    dbr_info.next_tid = getpid();
    db_forward(who,r,t,&dbr_info,0,0);
    dbr_info.next_tid = next_tid;
}


static void db_forward(pid_t who, msg_hdr_type msg_type,
       token_type n_rows, void *other, token_type n_rows1, void *other1) {
static struct _mxfer_entry slist[4];
token_type tmp;
int scount;

  _setmx(&slist[0],&msg_type,sizeof(msg_hdr_type));
  scount=2;
  switch(msg_type) {
    case DCDATA:
	assert(n_rows);
	tmp=n_rows;
	if (other1 && n_rows1) tmp+=n_rows1;
	_setmx(&slist[1],&tmp,sizeof(token_type));
	_setmx(&slist[2],other,n_rows*tmi(nbrow));
	scount=3;
	if (other1 && n_rows1) {
	_setmx(&slist[3],other1,n_rows1*tmi(nbrow));
	    scount=4;
	}
	break;
    case TSTAMP:
	_setmx(&slist[1],other,sizeof(tstamp_type));	
	break;
    case DCDASCMD:
	_setmx(&slist[1],other,sizeof(dascmd_type));	
	break;
    /* init clients */
    case DAS_OK:
    case DAS_UNKN:
    case DAS_BUSY:
	_setmx(&slist[1],&n_rows,sizeof(token_type));
	assert(other!=NULL);
	_setmx(&slist[2],other,sizeof(dbr_info_type));
	scount = 3;
	break;
    default: msg(MSG_EXIT_ABNORM,"in db_forward, bad type %ud",msg_type);
  }

  if (Replymx(who, scount, slist)==-1)
    msg(MSG_WARN,"Can't reply to request/init of task %d",who);
}
