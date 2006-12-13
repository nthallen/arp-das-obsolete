/*
    Data client Serial Out main module.
*/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <eillib.h>
#include <das.h>
#include <dfs.h>
#include <dc.h>

/* defines */
#define HDR "sout"
#define OPT_MINE "E"

/* global variables */
char *opt_string=OPT_DC_INIT OPT_MSG_INIT OPT_CC_INIT OPT_SERIAL_INIT OPT_MINE;

main( int argc, char **argv) {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */	
int i, fd;
struct stat st;
char ex[80]="busout";

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;

    /* process command line args */
    opterr = 0;
    optind = 0;
    do {
		i=getopt(argc,argv,opt_string);
		switch (i) {
			case 'E': strncpy(ex,optarg,79); break;
		    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		    default: break;
		}
    }  while (i!=-1);

    if (optind >= argc) msg(MSG_EXIT_ABNORM,"no device specified");
    if (argc > optind+1) msg(MSG_EXIT_ABNORM,"only one file/device allowed");

	if ((fd = open(argv[optind], O_WRONLY | O_APPEND | O_NONBLOCK))==-1)
	    msg(MSG_EXIT_ABNORM,"Can't open %s",basename(argv[optind]));

    if ( (fstat(fd, &st)) == -1)
		msg(MSG_EXIT_ABNORM,"Can't get status of %s",basename(argv[optind]));
    if (!S_ISCHR(st.st_mode))
	    msg(MSG_EXIT_ABNORM,"%s: not a character special file",argv[optind]);

	serial_init_options(argc,argv,fd);
	if (dup2(fd, STDOUT_FILENO)==-1)
		msg(MSG_EXIT_ABNORM,"Can't dup to stdout");
	if (strlen(ex)) {
		free(argv[0]);
		argv[0]=malloc(strlen(ex)+1);
		strcpy(argv[0],ex);
		if (execvp(ex, argv)==-1)
			msg(MSG_EXIT_ABNORM,"Can't exec %s",ex);
	}
}
