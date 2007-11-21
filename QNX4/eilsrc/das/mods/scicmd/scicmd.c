/*
	scicmd.c : receives cmds by RS232 and sends to encoder card.
	requires encodcrd.c.
	Ported to QNX 4.01 3/31/92 by Eil.
	Assumes interrupts and options set on com ports.
*/


#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/dev.h>
#include <errno.h>
#include <eillib.h>

/* defines */
#define HDR "cmd"
#define OPT_MINE ""

/* global variables */
char *opt_string=OPT_MSG_INIT OPT_MINE;

/* function declarations */
void send_cmd(unsigned int address, unsigned int data, int nbytes);
void init_8255(void);


main (int argc, char **argv) {

/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;
	
int nports;
FILE **fps;
FILE *f1;
int i,c;
char *d;
char rep[19], *date;
char c1[10], c2[10], c3[10], c4[10];
char *buf;
unsigned char *bufp;
unsigned int addr, dta, nbytes;
time_t t;
	

    /* initialise das options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;

    /* process args */
    opterr = 0;
    optind = 0;
    do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
	    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
	    default: break;
	}
    } while (i!=-1);

    /* process additional command line args (if need) */
    if (optind >= argc)
	msg(MSG_EXIT_ABNORM,"no communication devices specified");

    i=optind;
	
    /* open the com ports */
    if ( (fps=(FILE **)malloc((argc-i)*sizeof(FILE *))) == 0)
	msg(MSG_EXIT_ABNORM,"Can't allocate %d bytes of space",(argc-i)*sizeof(FILE *));
    for (nports=0; i<argc; i++,nports++)
	/* need r+b instead? */
	if ( (*(fps+nports)=fopen(argv[i],"r+"))==0 )
	    msg(MSG_EXIT_ABNORM,"Can't open %s for reading",argv[i]);

    flushall();
	
    if (!(buf=malloc(40*nports)))
	msg(MSG_EXIT_ABNORM,"Can't allocate memory for port buffers");
    if (!(bufp=(unsigned char *)calloc(nports,1)))
	msg(MSG_EXIT_ABNORM,"Can't allocate memory for port buffers index");		
	
    init_8255();
	
    /* read com ports and send to encoder card forever */
    for (i=0,f1=*fps;1;i=(i+1)%nports,f1=*(fps+i)) {

	if (dev_ischars(fileno(f1))) {
		    
	    /* read the char */
	    c=fgetc(f1);
      			
	    /* place in buffer */
	    if (isgraph(c))
	    if (bufp[i]<40) {
		buf[i*40+bufp[i]]=c;
		bufp[i]++;
	    }
	    else bufp[i]=0;
			
	    /* full command received */
	    if (c=='\n' && bufp[i]) {
		c1[0]=c2[0]=c3[0]=c4[0]='\0';
		sscanf(buf+(i*40)," %5s %8s %8s %3s",c1,c2,c3,c4);
		bufp[i]=0;

		/* get time */
		time(&t);
		date=asctime(localtime(&t));
		date[19]='\0';
		d=date+11; errno=0;
		dta=strtol(c1+1,NULL,16);		
		addr=strtol(c2,NULL,8);	
		msg(MSG,"RECEIVED %s %s %s %s\n",c1,c2,c3,c4); 
		/* check S-error */
		if (toupper(c1[0])!='S')
		    sprintf(rep,"S-ERROR %s\n",d);	
		/* check c_error */
		else if (addr>037)
		    sprintf(rep,"C-ERROR %s\n",d);
		/* check 1-error */			
		else if (strncmp(c2,c3,8) || strncmp(c1,c2+3,5) || strncmp(c4,c2,3))
		    sprintf(rep,"1-ERROR %s\n",d);
		else {
		    nbytes=(c4[2]=='K') ? 1 : 2;
		    c4[2]='\0';
		    sprintf(rep,"%s/%s/%s\n",c4,c1+1,d);
		    send_cmd(addr, dta, nbytes);
		}
		errno=0;		
		if (fputs(rep,f1)==EOF) 
		    msg(MSG_WARN,"Error trying to REPLY: %s",rep);
		else if (strstr(rep,"ERROR"))
		    msg(MSG_WARN,"REPLY %s",rep);
		else msg(MSG,"REPLY %s",rep);
	    }
	}
    }
}
