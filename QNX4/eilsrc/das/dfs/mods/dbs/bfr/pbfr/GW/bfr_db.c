/*
    Buffer application functions.
    Written 5/26/92 by Eil.
*/

/* includes */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include "globmsg.h"
#include "das.h"
#include "eillib.h"
#include "dfs.h"
#include "dfs_mod.h"
#include "bfr.h"
#include "bfr_vars.h"

/* function declarations */
extern int compare(pid_t *w, arr *a);
extern int send_cmd_or_stamp(llist *l, arr *a, unsigned long lseq);
extern int send_data(token_type nrows, arr *a);

/* Called when data buffer receives an order for data from its clients. */
int DB_get_data(pid_t who, token_type n_rows) {
llist *l;
arr *a;
int i;

/* find registered client */
if (n_clients && !(a=bsearch(&who, clients, n_clients, sizeof(arr), compare ))) {
    msg(MSG_WARN,"request from unregistered task %d",who);
    return(0);
}

/* traverse list to find any imminent stamps or cmds */
for (i=0, l=list, a->n_rows=n_rows;
     l && l->seq < (a->seq+dbr_info.nrowminf); l=l->next, i++)
	    if ( send_cmd_or_stamp(l,a,list_seq+i) ) return(1);

if (send_data( l ? min(l->seq - a->seq, n_rows) : n_rows, a)) return(1);

/* if request can't be fullfilled, raise flag */
a->rflag = 1;
requests++;

return(0);
}
