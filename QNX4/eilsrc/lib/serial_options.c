/*
    Configure serial ports by command line option.
*/
#include <stdlib.h>
#include <process.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <sys/qnx_glob.h>
#include <das_utils.h>

extern char *opt_string;

/* returns 0 on success */
int serial_init_options(int argcc, char *argvv[], int fd) {
extern char *optarg;
extern int optind, opterr, optopt;
int c;
int leave_opts_as_is;
char sttyopts[80];
char *vec[70];

    /* error handling intialisation if the client code didnt */
    if (!msg_initialised()) msg_init(basename(argvv[0]),0,1,-1,0,1,1);

    leave_opts_as_is = 0;
    opterr = 0;
    optind = 0;

	do {
	c=getopt(argcc,argvv,opt_string);
	switch (c) {
		case 't':
		    if (strlen(optarg))
			strncat(sttyopts,optarg,74);
		    else leave_opts_as_is = 1;
		    break;
		case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		default : break;
	}
    }  while (c!=-1);
    optind = 0;
    opterr = 1;

    if (!leave_opts_as_is) {
	if (serial_init(fd)) msg(MSG_EXIT_ABNORM,"Can't initialise descriptor %d to default",fd);
	if ( strlen(sttyopts) > 5 ) {
	    if (vector(sttyopts,vec,69))
		msg(MSG_EXIT_ABNORM,"Can't build argument vector to spawn stty");
	    /* set stdin from fd */
	    qnx_spawn_options.iov[0] = fd;
	    if (spawnvp(P_WAIT,vec[0],vec))
		msg(MSG_EXIT_ABNORM,"Can't set terminal control attributes: %s",sttyopts);
	}
    }

    qnx_spawn_options.iov[0] = 0xff;
    return 0;
}
