/*
    Command Control registration for DAS programs.
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <process.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <das_utils.h>
#include <globmsg.h>
#include <cmdctrl.h>
#include <symname.h>
#include <msg.h>

/* returns cmd tid on success */
int cc_init(unsigned char min_dasc, unsigned char max_dasc,
	    unsigned char min_msg, unsigned char max_msg,
	    quit_type how_to_quit, death_type how_to_die,
	    char *ts) {

pid_t cmd_tid;
reply_type replycode;
ccreg_type reg = {CCReg_MSG};
char name[FILENAME_MAX+1];
int i;

    if (!msg_initialised()) msg_init("",0,1,-1,0,1,1);

    if (how_to_quit==NOTHING_ON_QUIT && how_to_die==NOTHING_ON_DEATH
	&& min_dasc==0 && max_dasc==0 && min_msg==0 && max_msg==0)
	    /* don't bother registering */
	    return 0;

    for (i=0;i<3;i++)
	if ( (cmd_tid = qnx_name_locate(getnid(),LOCAL_SYMNAME(CMD_CTRL,name),0,0)) == -1) {
	    if (!i) msg(MSG,"Im trying to find %s on node %d", name, getnid());
	    sleep(1);
	}
	else break;
    if (i>0 && i<3) {
	errno=0;
	msg(MSG,"Found %s on node %d, continuing...",name,getnid());
    }
    else if (i>=3) msg(MSG_EXIT_ABNORM,"Couldn't find %s on node %d",name,getnid());

    reg.min_dasc = min_dasc;
    reg.max_dasc = max_dasc;
    reg.min_msg  = min_msg;
    reg.max_msg  = max_msg;
    reg.how_to_quit = how_to_quit;
    reg.how_to_die =  how_to_die;

    if (how_to_die==TASK_RESTART)
	if (strlen(ts) >(MAX_MSG_SIZE-7))
	    msg(MSG_WARN,"task startup command too big");

    strncpy(reg.task_start,ts,MAX_MSG_SIZE-8);

    if ((Send( cmd_tid, &reg, &replycode, sizeof(reg), sizeof(reply_type) ))==-1)
	msg(MSG_EXIT_ABNORM,"Error sending to cmdctrl");

    if (replycode != DAS_OK)
	msg(MSG_EXIT_ABNORM, "Bad response from cmdctrl");

    return(cmd_tid);
}
