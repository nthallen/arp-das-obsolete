#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dbr_utils.h>
#include <das_utils.h>
#include <sys/name.h>
#include <sys/kernel.h>
#include <globmsg.h>

struct {
    char i;
    dascmd_type d;
} s = {DASCMD,DCT_TM,DCV_TM_START};

main () {
int ds_tid;
char name[20];
char r;
/* find dg */

if ( (ds_tid=qnx_name_locate(getnid(), LOCAL_SYMNAME(DG_NAME,name), 0, 0)) == -1 )
	puts("cant find dg");

/* send a TM_START to it */
if (Send(ds_tid, &s, &r, sizeof(s), 1)==-1)
    puts("cant send");

}
