/*
    User supplied functions for a DG; for qnx msgs in.
*/
	
/* includes */
#include <string.h>
#include <stdlib.h>
#include <sys/kernel.h>
#include <eillib.h>
#include <dfs.h>
#include <dg.h>
#include <globmsg.h>

/* defines */

/* global variables */
extern char *buf;

int DG_get_data( token_type n_rows ) {
switch (buf[0]) {
	case DCDATA: DG_s_data(buf[1], buf+sizeof(msg_hdr_type)+sizeof(token_type), 0, 0); break;
	case TSTAMP: DG_s_tstamp((tstamp_type *)(buf+1)); break;
	default: break;
}
buf[0]=DEATH;
return 1;
}

void DG_DASCmd( unsigned char type, unsigned char num ) {
return;
}

reply_type DG_other(unsigned char *msg_ptr, pid_t sent_tid) {
/* called when a message is recieved which is not handled by the distributor */
reply_type r = DAS_OK;
if (buf[0]!=DEATH) msg(MSG_EXIT_ABNORM,"Buffer Overflow");
switch (msg_ptr[0]) {
	case DATA:
		memcpy(buf,msg_ptr,buf[1]*tmi(nbrow)+sizeof(msg_hdr_type)+sizeof(token_type));
		buf[0]=DCDATA;
		break;
	case STAMP:
		memcpy(buf,msg_ptr,sizeof(tstamp_type)+sizeof(msg_hdr_type));
		buf[0]=TSTAMP;
		break;
	default: r = DAS_UNKN;
		msg(MSG_WARN,"received UNKNOWN msg from process %d",sent_tid);
}
return(r);
}



