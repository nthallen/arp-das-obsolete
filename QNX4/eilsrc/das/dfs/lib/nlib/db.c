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
#include <errno.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <sys/name.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <sys/psinfo.h>
#include <sys/sendmx.h>
#include <globmsg.h>
#include <eillib.h>
#include <dbr.h>

/* must be called after DC_exitfunction, thus DB_init called before DC_init */
void DB_exitfunction(void) {
/* Trigger myself a QUIT */
static int once;
struct {
    msg_hdr_type m;
    dascmd_type d;
} qtrig = {DCDASCMD,DCT_QUIT,DCV_QUIT};
struct {
    msg_hdr_type m;
    token_type t;
} ttrig = {DCTOKEN,0};
static pid_t pr;
pid_t pr1;
	dbr_info.mod_type = DRC;
	if (!pr) pr=qnx_proxy_attach(0,(char *)&qtrig,sizeof(qtrig),-1);
    while (Trigger(pr)==-1 && errno==EINTR);
    if (!once) {
    	once++;
	    DC_init(abs(DBC) + abs(DRC) + 1, 0);
	    DC_operate();
		pr1=qnx_proxy_attach(0,(char *)&ttrig,sizeof(ttrig),-1);
	    while (Trigger(pr1)==-1 && errno==EINTR);	    
   	    DC_init(abs(DBC) + abs(DRC) + 1, 0);
	    DC_operate();    
	    msg(MSG,"task %d: DB operations completed",getpid());
	}
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
	assert(other!=NULL);
	_setmx(&slist[1],other,sizeof(dbr_info_type));
	scount = 2;
	break;
	/* the following would be a programmatic error */
    default: msg(MSG_EXIT_ABNORM,"can't forward msgs with type %d to buffered clients",msg_type);
	return;
  }

  while ( Replymx(who, scount, slist)==-1)
    switch (errno) {
	case EINTR: continue;
	default: msg(MSG_WARN,"Can't reply to request/init of task %d",who);
	    return;
    }
}

int DB_init() {
/* intialises the buf task as the buf task */	

/* attach name */
if (qnx_name_attach(getnid(),LOCAL_SYMNAME(DB_NAME))==-1)
    msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s on node %d",DB_NAME, getnid());

if ( (qnx_pflags(~0,_PPF_PRIORITY_REC,0,0)) == -1)
    msg(MSG_EXIT_ABNORM,"Can't set receiving priority order");

    if (atexit(DB_exitfunction))
	msg(MSG_EXIT_ABNORM,"Can't register DB exit function");

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

void DB_s_init_client(pid_t who, unsigned char r) {
pid_t next_tid;
unsigned char mt;

    assert(r==DAS_OK || r==DAS_UNKN || r==DAS_BUSY);
    mt = dbr_info.mod_type;
    next_tid = dbr_info.next_tid;
    dbr_info.next_tid = getpid();
    dbr_info.mod_type = DBC;
    db_forward(who,r,0,&dbr_info,0,0);
    dbr_info.next_tid = next_tid;
    dbr_info.mod_type = mt;
}
