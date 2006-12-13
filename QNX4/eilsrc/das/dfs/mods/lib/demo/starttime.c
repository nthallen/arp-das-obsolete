#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dbr_utils.h>
#include <das_utils.h>
#include <sys/name.h>
#include <sys/kernel.h>
#include <globmsg.h>

struct {
    msg_hdr_type h1;
    msg_hdr_type h2;
    char t[18];
} s = {TIME_SET,TIME_START,'\0'},

main (int arc, char **argv) {
int ds_tid;
char name[20];
char r;
/* find dg */

if ( (ds_tid=qnx_name_locate(getnid(), LOCAL_SYMNAME(DG_NAME,name), 0, 0)) == -1 )
	puts("cant find dg");

strcpy(s.t,argv[2]);

if (!strcmp(argv[1],"start")) s.h2=TIME_START;
else s.h2=TIME_END;

/* send a TM_START to it */
if (Send(ds_tid, &s, &r, sizeof(s), 1)==-1)
    puts("cant send");

}
