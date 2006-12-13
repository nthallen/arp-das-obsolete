/*
    User supplied client functions.
    Assumes mf's are whole and guarenteed.
    Assumes the DG always sends appropriate timestamps.
*/

/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <globmsg.h>
#include <das.h>
#include <dbr.h>
#include <eillib.h>

/* defines */

/* global variables */

void DC_data(dbr_data_type *dr_data) {
/* called when a DCDATA message received */
static int m;
m++;
if (m==10) DC_bow_out();
}

void DC_tstamp( tstamp_type *tstamp )  {
/* called when a TSTAMP message received */
}

void DC_DASCmd(unsigned char type, unsigned char number) {
/* called when a DASCmd message received */
}

void DC_other(char *msg, pid_t tid_that_sent) {
/* called when message received whose header isn't DCDATA, DCDASCmd or TSTAMP */
}
