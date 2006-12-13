/*
    DBR Data Generator Serial In.
    Written Sep 1992 by Eil for QNX 4.
*/

/* includes */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <globmsg.h>
#include <das.h>
#include <dbr.h>
#include <nortlib.h>
#include <eillib.h>

/* defines */
#define HDR "sin"
#define OPT_MINE

/* global variables */
char *opt_string=OPT_DG_INIT OPT_DG_DAC_IN OPT_MSG_INIT OPT_SERIAL_INIT OPT_MINE;
int fd; /* descriptor to read from */

main( int argc, char **argv )  {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */		
int i;
struct stat st;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
	cc_init_options(argc,argv, DCT_TM, DCT_TM, 0, 0, FORWARD_QUIT);

    /* initialisations */

    /* process command line args */
    opterr = 0;
    optind = 0;
    do {
		i=getopt(argc,argv,opt_string);
		switch (i) {
		    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		    default : break;
		}
    }  while (i!= -1);

    if (optind >= argc) msg(MSG_EXIT_ABNORM,"no device specified");
    if (argc > optind+1) msg(MSG_EXIT_ABNORM,"only one file/device allowed");

    if ( (fd = open(argv[optind],O_RDONLY)) == -1)
		msg(MSG_EXIT_ABNORM,"Can't open %s",basename(argv[optind]));

    if (!S_ISCHR(st.st_mode))
	    msg(MSG_EXIT_ABNORM,"%s: not a character special file",argv[optind]);

	serial_init_options(argc,argv,fd);
	if (dup2(fd, STDIN_FILENO)==-1) msg(MSG_EXIT_ABNORM,"Can't dup to stdin");
	free(argv[0]);
	argv[0]=malloc(5);
	strcpy(argv[0],"fdin");
	if (execvp("fdin", argv)==-1) msg(MSG_EXIT_ABNORM,"Can't exec fdin");
}
