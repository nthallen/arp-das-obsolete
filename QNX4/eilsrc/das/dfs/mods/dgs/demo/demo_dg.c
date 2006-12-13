/*
    User supplied functions for a DG.
*/
	
/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "das.h"
#include "dbr.h"
#include "dbr_mod.h"
#include "globmsg.h"
#include "eillib.h"

/* global variables */

int DG_get_data( token_type n_rows ) {
return 1;
}


int DG_other(unsigned char *msg_ptr, pid_t sent_tid) {
/* called when a message is recieved which is not handled by the distributor */
return 1;
}


int DG_DASCmd( unsigned char type, unsigned char num ) {
/* called when DG recieves a DASCmd */	
return 1;
}
