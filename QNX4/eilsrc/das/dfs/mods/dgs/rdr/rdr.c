/*
    DBR Data Generator Reader main common module.
    Written Aug 1991 by EIl.
    Ported to QNX 4 5/5/92 by Eil.
*/

/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <limits.h>
#include <signal.h>
#include <fcntl.h>
#include <globmsg.h>
#include <eillib.h>
#include <das.h>
#include <dbr.h>
#include <dbr_mod.h>

/* defines */
#define HDR "rdr"
#define OPT_MINE "d:r:t:q:a:Q"

/* global variables */
char *opt_string=OPT_DG_INIT OPT_DG_DAC_IN OPT_MSG_INIT OPT_BREAK_INIT OPT_MINE;
char rootname[ROOTLEN +1];
char dirname[FILENAME_MAX+1];
char fromtime[20];
char totime[20];
time_t starttime;	/* start time of logged files */
time_t endtime;		/* end time of logged files */
int setfromfile;
int settofile;
long setfrompos, settopos;
long setnexttimepos;
tstamp_type set_stamp;
int fcount;
int startfile;
int filesperdir;
char *mfbuffer=0;
char *rowbuffer=0;
unsigned int sleeper=0;
int quitter=0;

/* function declarations */
extern void set_time(char *m, time_t t, int *f, long *p,  long *n,
		     tstamp_type *s, char *h);


main( int argc, char **argv )  {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */		
char command[200];
int i;
long t;
tstamp_type T;
FILE *f;
unsigned int mfc;
char fullname[FILENAME_MAX];

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
    break_init_options(argc,argv);

    /* initialisations */
    strcpy(rootname,ROOTNAME);
    if (getcwd(dirname,FILENAME_MAX+1)==0)
	msg(MSG_EXIT_ABNORM,"Can't get cwd");

    /* process command line args */
    opterr = 0;
    optind = 0;
    do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
	    case 'd': strncpy(dirname,optarg,FILENAME_MAX-1);break;
	    case 'r': strncpy(rootname,optarg,ROOTLEN-1);break;
	    case 't': strncpy(fromtime,optarg,19);break;
	    case 'q': strncpy(totime,optarg,19);break;
	    case 'a': sleeper = atoi(optarg); break;
	    case 'Q': quitter = 1; break;
	    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
	    default : break;
	}
    }  while (i!= -1);

    /* check dirname for existance and access for effective user id */
    if ( (i=open(dirname,O_RDONLY) ) ==-1)
	msg(MSG_EXIT_ABNORM,"error opening %s for r/w",dirname);
    else close(i);

    /* read relevant information */
    if (DG_dac_in(argc,argv) !=0)
	msg(MSG_EXIT_ABNORM,"Can't dac initialise");

    /* initialise data generator */
    if (DG_init_options(argc,argv) != 0)
	msg(MSG_EXIT_ABNORM,"Can't DG initialise");

    /* see if files exist and count them */
    fullname[0]='\0';
    _searchenv(FILECOUNT_PROG,"PATH",fullname);
    sprintf(command,"%s -d %s -r %s",fullname,dirname,rootname);
    if ( !strlen(fullname) || !(f=popen(command,"r")) )
	msg(MSG_EXIT_ABNORM,"Can't open pipe to %s",FILECOUNT_PROG);
    else {
	fscanf(f,"%d %d %d %*d %*s %*s", &fcount, &i, &startfile);
	pclose(f);
    }
    if (i) filesperdir=i;

    if (gettimestamp( getfilename(fullname,dirname,rootname,startfile,filesperdir), 1, &set_stamp) <=0 )
	msg(MSG_EXIT_ABNORM,"Can't read first timestamp from %s",basename(fullname));
    if ( findmf( set_stamp.secs, fcount, startfile, dirname, rootname, filesperdir, 0,0,0, &mfc ) == -1 )
	msg(MSG_EXIT_ABNORM,"Can't find first minor frame");
    starttime = MFC_TIME(set_stamp,mfc);
    if (gettimestamp( getfilename(fullname,dirname,rootname,startfile+fcount-1,filesperdir), 0, &set_stamp) <=0 )
	msg(MSG_EXIT_ABNORM,"Can't read last timestamp from %s", basename(fullname));
    if ( findmf( LONG_MAX, fcount, startfile, dirname, rootname, filesperdir, 0,0,0, &mfc ) == -1 )
	msg(MSG_EXIT_ABNORM,"Can't find last minor frame");
    endtime = MFC_TIME(set_stamp,mfc);

    tzset();
    set_time(fromtime,starttime,&setfromfile,&setfrompos,&setnexttimepos,&set_stamp,"Start");
    set_time(totime,endtime,&settofile,&settopos,0,0,"End");

    if ( !(mfbuffer=malloc(dbr_info.max_rows * tmi(nbrow) + tmi(nbminf))))
	msg(MSG_EXIT_ABNORM,"Can't allocate minor frame buffer");

    if ( !(rowbuffer=malloc(tmi(nbminf))))
	msg(MSG_EXIT_ABNORM,"Can't allocate row buffer");

    DG_operate();
    
    DONE_MSG;
}
