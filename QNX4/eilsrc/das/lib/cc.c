/*
    Command Control registration for DAS programs.
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <process.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <das_utils.h>
#include <globmsg.h>
#include <cmdctrl.h>

int cc_init_options(int argcc, char *argvv[], unsigned char min_dasc,
		    unsigned char max_dasc, unsigned char min_msg,
		    unsigned char max_msg, quit_type how_to_quit) {
extern char *optarg;
extern int optind, opterr, optopt;
int c;
int action=NOTHING_ON_DEATH;
char cmd_line[MAX_MSG_SIZE];
char full[MAX_MSG_SIZE];
char ts[MAX_MSG_SIZE];

    /* error handling intialisation if the client code didnt */
    if (!msg_initialised())
	msg_init(argvv[0],0,1,-1,0,1,1);

    opterr = 0;
    optind = 0;

	do {
	c=getopt(argcc,argvv,opt_string);
	switch (c) {
		case 'R': action=REBOOT; break;
		case 'D': action=DAS_RESTART; break;
		case 'T': action=TASK_RESTART; break;
		case 'S': action=SHUTDOWN; break;
		case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		default : break;
	}
    }  while (c!=-1);
    optind = 0;
    opterr = 1;

    if (action==TASK_RESTART) {
	if (qnx_fullpath(full,argvv[0])==NULL)
	    msg(MSG_EXIT_ABNORM,"Can't get fullpath of %s",basename(argvv[0]));
	if (getcmd(cmd_line)==NULL)
	    msg(MSG_EXIT_ABNORM,"Can't get command line args");
	sprintf(ts,"%s %s",full,cmd_line);
    }

    return (cc_init(min_dasc, max_dasc, min_msg, max_msg, how_to_quit, action, ts));

}

int cc_init(unsigned char min_dasc, unsigned char max_dasc,
	    unsigned char min_msg, unsigned char max_msg,
	    quit_type how_to_quit, death_type how_to_die,
	    char *ts) {

pid_t cmd_tid;
reply_type replycode;
ccreg_type reg = {CCReg_MSG};
char name[FILENAME_MAX+1];
int i;

    if (how_to_quit==NOTHING_ON_QUIT && how_to_die==NOTHING_ON_DEATH
	&& min_dasc==0 && max_dasc==0 && min_msg==0 && max_msg==0)
	    /* don't bother registering */
	    return 0;

    for (i=0;i<3;i++)
	if ( (cmd_tid = qnx_name_locate(getnid(),LOCAL_SYMNAME(CMD_CTRL,name),0,0)) == -1) {
	    if (!i) msg(MSG_WARN,"Im trying to find %s on node %d", name, getnid());
	    sleep(1);
	}
	else break;
    if (i>0 && i<3) msg(MSG,"Found %s on node %d, continuing...",name,getnid());
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
