/*
    User supplied functions for a DG; busin
*/
	
/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <eillib.h>
#include <das.h>
#include <dfs.h>
#include <dg.h>
#include <globmsg.h>

extern char *buf;

int DG_get_data( token_type n_rows ) {
switch (buf[0]) {
	case DCDATA: DG_s_data(buf[1],buf+sizeof(msg_hdr_type)+sizeof(token_type),0,0); break;
	case TSTAMP: DG_s_tstamp(buf+sizeof(msg_hdr_type)); break;
	case DEATH: break;
	default: break;
}
buf[0] = DEATH;
return 1;
}

void DG_DASCmd( unsigned char type, unsigned char num ) {
return;
}

reply_type DG_other(unsigned char *msg_ptr, int sent_tid) {
unsigned char i,j;
/* called when a message is recieved which is not handled by the distributor */
if (buf[0] != DEATH) msg(MSG_EXIT_ABNORM,"Buffer Overflow");
switch (msg_ptr[0]) {
	case DATA:
		memcpy(buf,msg_ptr,buf[1]*tmi(nbrow) + sizeof(msg_hdr_type) + sizeof(token_type));
		buf[0]=DCDATA;
		break;
	case STAMP:
		memcpy(buf,msg_ptr,sizeof(tstamp_type) + sizeof(msg_hdr_type));
		buf[0]=TSTAMP;
		break;
	default:
		msg(MSG_EXIT_ABNORM,"Unknown stream data received");
		return DAS_UNKN;
}
return DAS_OK;
}



