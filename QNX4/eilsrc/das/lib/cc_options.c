/*
    Command Control registration for DAS programs -- options.
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <process.h>
#include <cmdctrl.h>
#include <msg.h>
#include <cc.h>

extern char *opt_string;

int cc_init_options(int argcc, char *argvv[], unsigned char min_dasc,
		    unsigned char max_dasc, unsigned char min_msg,
		    unsigned char max_msg, quit_type how_to_quit) {
extern char *optarg;
extern int optind, opterr, optopt;
int c;
int action=NOTHING_ON_DEATH;
char cmd_line[MAX_MSG_SIZE];
char ts[MAX_MSG_SIZE];

    /* error handling intialisation if the client code didnt */
    if (!msg_initialised()) msg_init(basename(argvv[0]),0,1,-1,0,1,1);

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
	if (getcmd(cmd_line)==NULL)
	    msg(MSG_EXIT_ABNORM,"Can't get command line args");
	sprintf(ts,"%s %s",argvv[0],cmd_line);
    }
    return (cc_init(min_dasc, max_dasc, min_msg, max_msg, how_to_quit, action, ts));
}
