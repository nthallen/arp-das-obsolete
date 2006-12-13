/*
    User supplied client functions for filter
*/

/* includes */
#include <unistd.h>
#include <errno.h>
#include <sys/kernel.h>
#include <globmsg.h>
#include <eillib.h>
#include <das.h>
#include <dbr.h>

/* globals */

void DC_data(dbr_data_type *dr_data) {
/* called when a DCDATA message received */
}

void DC_tstamp( tstamp_type *tstamp )  {
/* called when a TSTAMP message received */
}

void DC_DASCmd(unsigned char type, unsigned char number) {
/* called when a DASCmd message received */
}

void DC_other(char *message, pid_t tid_that_sent) {
/* called when message received whose header isn't DCDATA, DCDASCmd or TSTAMP */
reply_type r = DAS_UNKN;
Reply(tid_that_sent,&r,sizeof(reply_type));
msg(MSG_WARN,"received unknown msg with header %d from %d",message[0],tid_that_sent);
}
