/*
    Data client Serial Out main module.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <eillib.h>
#include <das.h>
#include <dbr.h>

/* defines */
#define HDR "sout"
#define OPT_MINE ""

/* global variables */
char *opt_string=OPT_DC_INIT OPT_MSG_INIT OPT_CC_INIT OPT_SERIAL_INIT OPT_MINE;
int n_opens;
int *fds;
char **argvv;
int index;

main( int argc, char **argv) {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */	
int i;
struct stat st;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
    cc_init_options(argc,argv,0,0,0,0,NOTHING_ON_QUIT);

    /* initialisations */
    argvv = argv;

    /* process command line args */
    opterr = 0;
    optind = 0;
    do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
	    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
	    default: break;
	}
    }  while (i!=-1);

    if (optind >= argc) msg(MSG_EXIT_ABNORM,"no files/devices specified");

    index = optind;

    if ( (fds=(int *)malloc( (argc-index)*sizeof(int) )) == -1)
	msg(MSG_EXIT_ABNORM,"Can't allocate %d bytes of memory for descriptors",(argc-index)*sizeof(int));

    for (n_opens=0; index+n_opens < argc; n_opens++)
	if ( (*(fds + n_opens) = open(argv[index+n_opens], O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) ==-1)
	    msg(MSG_EXIT_ABNORM,"Can't open %s",basename(argv[index+n_opens]));
	else
	    if ( (fstat(*(fds + n_opens), &st)) == -1)
		msg(MSG,"Can't get status of %s",basename(argv[index+n_opens]));
	    else if (S_ISCHR(st.st_mode))
		serial_init_options(argc,argv,*(fds + n_opens));

    /* initialise into DRing */
    if (DC_init_options(argc,argv) != 0) 
	msg(MSG_EXIT_ABNORM,"Can't DC initialise");

    /* main loop of command/data transmission around ring */
    DC_operate();

    DONE_MSG;
}
