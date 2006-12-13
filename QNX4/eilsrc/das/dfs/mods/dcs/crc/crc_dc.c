/*
*/

/* includes */
#include <stdio.h>
#include <unistd.h>
#include <globmsg.h>
#include <eillib.h>
#include <das.h>
#include <dc.h>

/* globals */

void DC_data(dbr_data_type *dr_data) {
/* called when a DCDATA message received */
/* put into buffer, add a 2 byte crc onto end */
}

void DC_tstamp( tstamp_type *tstamp )  {
/* called when a TSTAMP message received */
/* put into buffer, add a 2 byte crc onto end */
}

void DC_DASCmd(unsigned char type, unsigned char number) {
/* called when a DASCmd message received */
/* put into buffer, add a 2 byte crc onto end */
}

reply_type DC_other(char *message, pid_t tid_that_sent) {
/* called when message received whose header isn't DCDATA, DCDASCmd or TSTAMP */
return DAS_UNKN;
}
