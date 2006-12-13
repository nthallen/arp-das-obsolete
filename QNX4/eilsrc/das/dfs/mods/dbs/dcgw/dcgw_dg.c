/* 
*/

/* includes */
#include <sys/types.h>
#include <globmsg.h>
#include <eillib.h>
#include <gw.h>

extern dfs_msg_type *bfr;

void DG_DASCmd(unsigned char type, unsigned char val) {
return;
}

reply_type DG_other(unsigned char *msg_ptr, pid_t who) {
return(GW_dg_other(who,msg_ptr));
}

/* Called when data buffer receives an order for data from its clients. */
int DG_get_data(token_type n_rows) {
	switch (bfr->msg_type) {
		case DCDATA: DG_s_data(bfr->u.drd.n_rows,bfr->u.drd.data,0,0); break;
		case TSTAMP: DG_s_tstamp(&bfr->u.tst); break;
		case DCDASCMD: DG_s_dascmd(bfr->u.dasc.type, bfr->u.dasc.val); break;
	}
bfr->msg_type = DEATH;
return(0);
}




