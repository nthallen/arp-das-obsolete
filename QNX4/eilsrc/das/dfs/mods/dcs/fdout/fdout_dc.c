/*
    User supplied client functions for fdout
    Assumes output to descriptor is non-blocking.
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
extern int fd;

void DC_data(dbr_data_type *dr_data) {
/* called when a DCDATA message received */
int n_bytes;
int s_bytes;
static int msg_ctrl;

n_bytes = dr_data->n_rows * tmi(nbrow);

if ( (s_bytes=write(fd, dr_data->data, n_bytes)) != n_bytes)
	if (s_bytes==-1)
	    switch (errno) {
			case EAGAIN: break;
			default: msg(MSG_EXIT_ABNORM,"error writing to descriptor %d",fd);
	    }
	else if (!msg_ctrl) {
	    msg(MSG_WARN,"descriptor %d output queue full: (%d/%d)",fd, s_bytes, n_bytes);
	    msg_ctrl++;
	}
else if (msg_ctrl) {
    msg(MSG,"satisfactory output to descriptor %d resumed",fd);
    msg_ctrl=0;
}
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
