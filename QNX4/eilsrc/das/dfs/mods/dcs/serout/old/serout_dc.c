/*
    User supplied client functions for Serial Out.
    Assumes mf's are whole and guarenteed.
*/

/* includes */
#include <unistd.h>
#include <errno.h>
#include <sys/kernel.h>
#include <globmsg.h>
#include <eillib.h>
#include <das.h>
#include <dbr.h>

/* defines */

/* global variables */
extern int n_opens;
extern int *fds;
extern char **argvv;
extern int index;

void DC_data(dbr_data_type *dr_data) {
/* called when a DCDATA message received */
int i;
int n_bytes;
int s_bytes;
static int msg_ctrl;

n_bytes = dr_data->n_rows * tmi(nbrow);

for (i=0; i<n_opens; i++)
    if ( (s_bytes=write( *(fds + i), dr_data->data, n_bytes)) != n_bytes)
	if (s_bytes==-1)
	    switch (errno) {
		case EAGAIN: break;
		default: msg(MSG_WARN,"error writing to %s",basename(argvv[index+i]));
	    }
	else if (!msg_ctrl) {
	    msg(MSG_WARN,"%s output queue full, receiver not fast enough maybe: (%d/%d)",basename(argvv[index+i]), s_bytes, n_bytes);
	    msg_ctrl++;
	}
    else 
	if (msg_ctrl) {
	    msg(MSG,"satisfactory serial out resumed");
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
