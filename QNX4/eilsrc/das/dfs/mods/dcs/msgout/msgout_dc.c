/*
    User supplied client functions for msgout
*/

/* includes */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <globmsg.h>
#include <eillib.h>
#include <das.h>
#include <dc.h>

/* globals */
extern pid_t p;

void DC_data(dbr_data_type *dr_data) {
/* called when a DCDATA message received */
struct _mxfer_entry mx[3];
msg_hdr_type h = DCDATA;
_setmx(&mx[0],&h,sizeof(msg_hdr_type));
_setmx(&mx[1],&dr_data->n_rows,sizeof(token_type));
_setmx(&mx[2],&dr_data->data,dr_data->n_rows*tmi(nbrow));
while (Sendmx(p,3,1,&mx,&mx)==-1)
	if (errno==EINTR) continue;
	else msg(MSG_EXIT_ABNORM,"Can't send to task %d",p);
}

void DC_tstamp( tstamp_type *tstamp )  {
/* called when a TSTAMP message received */
struct _mxfer_entry mx[2];
msg_hdr_type h = TSTAMP;
_setmx(&mx[0],&h,sizeof(msg_hdr_type));
_setmx(&mx[1],tstamp,sizeof(tstamp_type));
while (Sendmx(p,2,1,&mx,&mx)==-1)
	if (errno==EINTR) continue;
	else msg(MSG_EXIT_ABNORM,"Can't send to task %d",p);
}

void DC_DASCmd(unsigned char type, unsigned char number) {
/* called when a DASCmd message received */
struct _mxfer_entry mx[3];
msg_hdr_type h = DCDASCMD;
_setmx(&mx[0],&h,sizeof(msg_hdr_type));
_setmx(&mx[1],&type,1);
_setmx(&mx[2],&number,1);
while (Sendmx(p,3,1,&mx,&mx)==-1)
	if (errno==EINTR) continue;
	else msg(MSG_EXIT_ABNORM,"Can't send to task %d",p);
}

reply_type DC_other(char *message, pid_t tid_that_sent) {
/* called when message received whose header isn't DCDATA, DCDASCmd or TSTAMP */
return DAS_UNKN;
}
