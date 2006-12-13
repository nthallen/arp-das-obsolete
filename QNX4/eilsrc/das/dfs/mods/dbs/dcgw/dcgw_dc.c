/* 
*/

/* includes */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <globmsg.h>
#include <das.h>
#include <eillib.h>
#include <gw.h>
#include <dfs_mod.h>

extern dfs_msg_type *bfr;

/* called when data is received from the dfs */
void DC_data(dbr_data_type *dr_data) {
/* put in buffer */
if (bfr->msg_type != DEATH) msg(MSG_EXIT_ABNORM,"buffer overflow");
memcpy(&bfr->u.drd,dr_data,dr_data->n_rows*tmi(nbrow)+sizeof(token_type));
bfr->msg_type = DCDATA;
}

/* called when a time stamp received from dfs */
void DC_tstamp( tstamp_type *tstamp )  {
if (bfr->msg_type != DEATH) msg(MSG_EXIT_ABNORM,"buffer overflow");	
memcpy(&bfr->u.tst,tstamp,sizeof(tstamp_type));
bfr->msg_type = TSTAMP;
}

/* called when a DASCmd is received from dbr */
void DC_DASCmd(unsigned char type, unsigned char number) {
if (bfr->msg_type != DEATH) msg(MSG_EXIT_ABNORM,"buffer overflow");	
bfr->u.dasc.type = type;
bfr->u.dasc.val = number;
bfr->msg_type = DCDASCMD;
}

reply_type DC_other(unsigned char *msg_ptr, int who) {
return(GW_dc_other(who));
}

