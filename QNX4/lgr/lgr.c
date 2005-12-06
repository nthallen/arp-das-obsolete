/*
    lgr main module.
    Written by Eil for QNX 4 4/28/92.
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <i86.h>
#include "globmsg.h"
#include "cmdctrl.h"
#include "eillib.h"
#include "das.h"
#include "dbr.h"
#include "dbr_mod.h"

/* defines */
#define HDR "lgr"
#define OPT_MINE "wd:r:N:z:L:F:"

/* global variables */
char *opt_string=OPT_DC_INIT OPT_MSG_INIT OPT_CC_INIT OPT_MINE;
int logging;		/* logging enable switch */
long maxfilesize;	/* maximum allowable log file size */
int fcount;		/* file count */
char rootname[ROOTLEN];
char dirname[FILENAME_MAX];
char name[FILENAME_MAX];
int filesperdir;

main( int argc, char **argv) {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */	
int  i, wflag;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
    cc_init_options(argc,argv,0,0,0,0,NOTHING_ON_QUIT);

    /* initialisations */
    strcpy(rootname,ROOTNAME);
    getcwd(dirname,FILENAME_MAX);
    wflag=0;
    logging = 1;
    fcount = 0;
    filesperdir = FILESPERDIR;
    maxfilesize = FILESIZE;

    /* process command line args */
    opterr = 0;
    optind = 0;
	do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
	    case 'd': strncpy(dirname,optarg,FILENAME_MAX-1);  break;
	    case 'L': fcount=atoi(optarg) + 1; break;
	    case 'F': break;
	    case 'r': strncpy(rootname,optarg,ROOTLEN-1);  break;
	    case 'N': filesperdir=atoi(optarg); break;
	    case 'w': wflag=1; break;
	    case 'z': maxfilesize=atol(optarg);
			if (strpbrk(optarg,"kK")) maxfilesize*=K;
			break;
	    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
	    default: break;
	}
    }  while (i!=-1);


    /* see if files already exist */
    if (wflag) {
	scanf("%*d %d %d %s %s",&fcount,&i,dirname,rootname);
	fcount++;
	if (i) filesperdir=i;
    }

    /* check dirname for existance and access for effective user id */
    if ( (i=open(dirname,O_RDONLY) ) ==-1)
	msg(MSG_EXIT_ABNORM,"error opening %s for r/w",dirname);
    else close(i);

    /* initialise logger into DRing */
    if (DC_init_options(argc,argv) != 0) 
	msg(MSG_EXIT_ABNORM,"Can't initialise into DBR");

    /* check that file size is within limits */
    if (maxfilesize > MAXFILESIZE || maxfilesize < (dbr_info.max_rows*tmi(nbrow) + dbr_info.nrowminf*tmi(nbrow) + sizeof(tstamp_type))) {
	msg(MSG,"Can't have filesize %ld, defaulted to %d",maxfilesize, FILESIZE);
	maxfilesize=FILESIZE;
    }
    /* check files/dir is within limits */
    if (filesperdir > MAXFILESPERDIR || filesperdir <= 0) {
	msg(MSG,"Can't have files/dir %d, defaulted to %d",filesperdir, FILESPERDIR);
	filesperdir=FILESPERDIR;
    }

    /* main loop of command/data transmission around ring */
    DC_operate();
}
