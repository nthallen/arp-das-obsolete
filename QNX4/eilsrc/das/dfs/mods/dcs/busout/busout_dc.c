/*
    User supplied client functions for busout.
*/

/* includes */
#include <stdio.h>
#include <unistd.h>
#include <globmsg.h>
#include <eillib.h>
#include <das.h>
#include <dc.h>

/* globals */
extern int blocking;

void DC_data(dbr_data_type *dr_data) {
/* called when a DCDATA message received */
int i;
i=dr_data->n_rows * tmi(nbrow)+sizeof(token_type);
bus_write(STDOUT_FILENO,blocking ? DCDATA : DATA,(char *)dr_data,i);
}

void DC_tstamp( tstamp_type *tstamp )  {
/* called when a TSTAMP message received */
if (blocking)
	bus_write(STDOUT_FILENO,STAMP,(char *)tstamp,sizeof(tstamp_type));
}

void DC_DASCmd(unsigned char type, unsigned char number) {
/* called when a DASCmd message received */
unsigned char buf[2];
if (blocking) {
	buf[0]=type; buf[1]=number;
	bus_write(STDOUT_FILENO,DASCMD,buf,sizeof(dascmd_type));
}
}

reply_type DC_other(char *message, pid_t tid_that_sent) {
/* called when message received whose header isn't DCDATA, DCDASCmd or TSTAMP */
return DAS_UNKN;
}
