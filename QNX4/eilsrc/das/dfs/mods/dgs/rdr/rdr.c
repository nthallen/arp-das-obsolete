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
#define OPT_MINE "F:L:d:r:g:z:QwN:"

/* global variables */
char *opt_string=OPT_DG_INIT OPT_DG_DAC_IN OPT_MSG_INIT OPT_MINE;
char rootname[ROOTLEN];
char dirname[80];
char fromtime[20] = {'\0'};
char totime[20] = {'\0'};
time_t starttime;	/* start time of logged files */
time_t endtime;		/* end time of logged files */
int setfromfile;
int settofile;
long setfrompos, settopos;
long setnexttimepos;
tstamp_type set_stamp;
int startfile;
int endfile;
int filesperdir;
char *mfbuffer=0;
char *rowbuffer=0;
int quitter=0;

/* function declarations */
extern void set_time(char *m, time_t t, int *f, long *p,  long *n,
		     tstamp_type *s, char *h);


main( int argc, char **argv )  {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */		
char command[160];
int i;
long t;
tstamp_type hold_stamp;
FILE *f;
unsigned short mfc;
char fullname[80];
int wflag;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;

    /* initialisations */
    wflag=0;
    startfile = endfile = -1;
    filesperdir=-1;
    strcpy(rootname,ROOTNAME);
    if (getcwd(dirname,80)==0) msg(MSG_EXIT_ABNORM,"Can't get cwd");

    /* process command line args */
    opterr = 0;
    optind = 0;
    do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
	    case 'F': startfile = atoi(optarg); break;
	    case 'L': endfile = atoi(optarg); break;
	    case 'd': strncpy(dirname,optarg,79);break;
	    case 'r': strncpy(rootname,optarg,ROOTLEN-1);break;
	    case 'g': strncpy(fromtime,optarg,19);break;
	    case 'z': strncpy(totime,optarg,19);break;
	    case 'Q': quitter = 1; break;
	    case 'w': wflag = 1; break;
	    case 'N': filesperdir = atoi(optarg); break;
	    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
	    default : break;
	}
    }  while (i!= -1);

    /* see if files already exist */
    if (wflag)
	scanf("%d %d %d %s %s",&startfile,&endfile,&filesperdir,dirname,rootname);

    /* check dirname for existance and access for effective user id */
    if ( (i=open(dirname,O_RDONLY) ) ==-1)
	msg(MSG_EXIT_ABNORM,"Can't open %s for r/w",dirname);
    else close(i);

    /* read relevant information */
    if (DG_dac_in(argc,argv) !=0)
	msg(MSG_EXIT_ABNORM,"Can't dac initialise");

    /* initialise data generator */
    if (DG_init_options(argc,argv) != 0)
	msg(MSG_EXIT_ABNORM,"Can't DG initialise");

    /* see if files exist and count them */
    if (!wflag && (startfile==-1 || endfile==-1 || filesperdir==-1 )) {
	fullname[0]='\0';
	_searchenv(FILECOUNT_PROG,"PATH",fullname);
	sprintf(command,"%s -d %s -r %s",fullname,dirname,rootname);
	if ( !strlen(fullname) || (f=popen(command,"r")) == 0 )
	    msg(MSG_EXIT_ABNORM,"Can't open pipe to %s",FILECOUNT_PROG);
	else {
	    if (startfile==-1) fscanf(f,"%d",&startfile);
	    else fscanf(f,"%*d");
	    if (endfile==-1) fscanf(f,"%d",&endfile);
	    else fscanf(f,"%*d");
	    if (filesperdir==-1) fscanf(f,"%d %*s %*s",&filesperdir);
	    else fscanf(f,"%*d %*s %*s");
	    pclose(f);
	}
    }
    if (startfile == -1) msg(MSG_EXIT_ABNORM,"No files to read");
    if (endfile == -1) endfile = startfile;

    if (startfile > endfile)
	msg(MSG_EXIT_ABNORM,"Start File Limit %d is after End File Limit %d",startfile,endfile);

    tzset();

    while (gettimestamp( getfilename(fullname,dirname,rootname,startfile,filesperdir, 0), 1, &set_stamp) <=0 ) {
	msg(MSG_WARN,"Can't read 1st timestamp from %s: file skipped",fullname);
	if (++startfile > endfile) msg(MSG_EXIT_ABNORM,"All Files Corrrupted");
    }
    while ( (setfromfile=findmf( set_stamp.secs, startfile, startfile, dirname, rootname, filesperdir, &setfrompos, &setnexttimepos, &set_stamp, &mfc )) == -1 ) {
	msg(MSG_WARN,"Log File %d : Can't find starting point: file skipped",startfile);
	if (++startfile > endfile) msg(MSG_EXIT_ABNORM,"All Files Corrrupted");
    }
    starttime = MFC_TIME(set_stamp,mfc);
    msg(MSG,"Limit Start time: Log File %d: %.24s %s",startfile,ctime(&starttime),tzname[0]);

    while (gettimestamp( getfilename(fullname,dirname,rootname,endfile,filesperdir, 0), 0, &hold_stamp) <=0 ) {
	msg(MSG_WARN,"Can't read last timestamp from %s: file skipped", fullname);
	if (--endfile < startfile) msg(MSG_EXIT_ABNORM,"All Files Corrupted");
    }
    while ( (settofile=findmf(LONG_MAX, endfile, endfile, dirname, rootname, filesperdir, &settopos, 0, &hold_stamp, &mfc )) == -1 ) {
	msg(MSG_WARN,"Log File %d: Can't find ending point: file skipped",endfile);
	if (--endfile < startfile) msg(MSG_EXIT_ABNORM,"All Files Corrupted");
    }
    endtime = MFC_TIME(hold_stamp,mfc);
    msg(MSG,"Limit End time: Log File %d: %.24s %s",endfile,ctime(&endtime),tzname[0]);

    if (strlen(fromtime))
	set_time(fromtime,starttime,&setfromfile,&setfrompos,&setnexttimepos,&set_stamp,"Start");
    if (strlen(totime))
	set_time(totime,endtime,&settofile,&settopos,0,&hold_stamp,"End");

    if ( !(mfbuffer=malloc(dbr_info.max_rows * tmi(nbrow) + tmi(nbminf))))
	msg(MSG_EXIT_ABNORM,"Can't allocate minor frame buffer");

    if ( !(rowbuffer=malloc(tmi(nbminf))))
	msg(MSG_EXIT_ABNORM,"Can't allocate row buffer");

    DG_operate();
}
