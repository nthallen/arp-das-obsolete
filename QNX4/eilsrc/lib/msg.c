/*
	msg.c: an informing system for the data aquisition system.
	msg is "msg_hdr: [FATAL: ]formatted given string: errno msg".
	written by Eil.
	ported to QNX 4 by Eil 4/2/92.
*/  

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dev.h>
#include <errno.h>
#include <process.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/qnxterm.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <fcntl.h>
#include <globmsg.h>
#include <memo.h>
#include <symname.h>
#include <msg.h>
#include <sounds.h>

/* IBM compatable display attributes for console functions */
#define IBM_NORMAL 0x07
#define IBM_REVERSE 0x70
#define IBM_BLINK (IBM_NORMAL | 0x80)

#define LEN 300

static int msg_inited;
static int msg_verbose = 1;
static int msg_memo_node = -1;
static FILE *msg_fp = 0;
static int msg_devfd = 0;
static struct _console_ctrl *msg_cc = NULL;
static int msg_row = 0;
static int msg_col = 0;
static int msg_size = 0;
static char msg_hdr[40] = {'\0'};
static unsigned int msg_pass = 0;
static unsigned int msg_warn = 0;
static unsigned int msg_fail = 0;
static int msg_sounds = 1;
static int msg_sys = 1;
static int scrwidth = 0;
static pid_t memo_tid;
static char *msg_buf = 0;
static char *msg_fbuf = 0;
static time_t timevar;

/* returns true if messages already initialised */
int msg_initialised(void) {
	if (msg_inited) return 1;
	return 0; 
}

/* ends messages */
void msg_end() {
	if (msg_fp) fclose(msg_fp);
	if (msg_devfd) close(msg_devfd);
	if (msg_cc) console_close(msg_cc);
	if (msg_buf) free(msg_buf);
	if (msg_fbuf) free(msg_fbuf);
	msg_inited=0;
}

/*  write an informative message, notes in msg.h */
void msg(int fatal, char *format,...) {

va_list ap;
char errline[LEN] = {MEMO_MSG,'\0'};
char *p;
reply_type replymsg;
int msgsize, i;
unsigned attr;

	/* form error string */	
	if (!format && (!errno || !msg_sys) && fatal<MSG_EXIT_ABNORM)
	    return;	
	va_start(ap, format);
	if (msg_hdr)
		if (strlen(msg_hdr)) {
			strncat(errline, msg_hdr, 10);
			strcat(errline, ": ");	
		}
	if ( fatal>=MSG_EXIT_ABNORM )  strcat(errline,"FATAL: ");
	p = errline + strlen(errline);
	vsprintf(p, format, ap);
	p = errline + 1;
	if ( errno && msg_sys )  {
		strcat(p,": ");
		strcat(p,strerror(errno)); 
	}
	va_end(ap);
	msgsize = strlen(p);

	/* log to MEMO */
	if (msg_memo_node >= 0 && memo_tid != getpid() )
	    Send( memo_tid, errline, &replymsg, msgsize+1+sizeof(msg_hdr_type), sizeof(reply_type));

	/* log to file */		
	if (msg_fp)  {
	    time(&timevar);
	    strftime(msg_fbuf,10,"%T",localtime(&timevar));
	    fprintf(msg_fp,"%s: %s\n",msg_fbuf,p);
	    fflush(msg_fp);
	}

	/* to stdout */
	if (msg_verbose)  {
		printf("%s\n",p);
		fflush(stdout);
	}

	if (msg_devfd) {
	    switch (fatal) {
		case MSG_WARN: attr = msg_warn; break;	/* warning */
		case MSG_PASS: attr = msg_pass; break;	/* just a msg */
		case MSG_FAIL: attr = msg_fail; break;  /* error, no exit */
		case MSG_EXIT_NORM: attr = msg_pass; break; /* exit normally */
		default: attr = msg_fail;		/* exit abnormally */
		}

	    if (!msg_cc) {
		for (i=0;i<msg_size;i++)
		    term_type(msg_row,msg_col+i," ",1,msg_pass);
		term_type(msg_row,msg_col,p,(msg_size<=strlen(p))?msg_size:0,attr);
		term_flush();
	    } else {
		console_read(msg_cc,0,2*(msg_row*scrwidth+msg_col),msg_buf,msg_size*2,0,0,0);
		strnset(msg_buf,32,msg_size*2);	
		for (i=0;i<(msg_size*2);i+=2,p++)
		    *(msg_buf+i)=*p;
		for (i=1;i<=(msg_size*2);i+=2)
		    *(msg_buf+i)=(char)attr;
		console_write(msg_cc,0,2*(msg_row*scrwidth+msg_col),msg_buf,msg_size*2,0,0,0);
	    }
	}
	
	errno=0;

	/* fatal action */
	switch (fatal) {
		case MSG_WARN: if (msg_sounds) WARN_NOTE; break;
		case MSG_FAIL: if (msg_sounds) FAIL_NOTE; break;
		case MSG_PASS: break;
		case MSG_EXIT_NORM: exit(0); break;
		default: if (msg_sounds) FAIL_TUNE; exit(1);
	}
}


/* initialise messages options */
void msg_init(char *hdr, char *fn, int verbose, nid_t memo_node, char *oarg, int sounds, int sys) {

    int j;
    char *p;
    char devfn[FILENAME_MAX+1] = {'\0'};
    char name[FILENAME_MAX+1] = {'\0'};

	msg_verbose = verbose;
	msg_sounds = sounds;
	msg_memo_node = -1;
	msg_sys = sys;
	if (hdr && strlen(hdr)) strncpy(msg_hdr,hdr,39);
	if (oarg && strlen(oarg)) {
	    j = strcspn(oarg,",;");
	    p = oarg+j+1;	
	    oarg[j]='\0';
	    strncat(devfn,oarg,FILENAME_MAX-1);
	    j = strcspn(p,",;"); p[j] = '\0';
	    msg_row=atoi(p); p = p+j+1;
	    j = strcspn(p,",;"); p[j] = '\0';
	    msg_col=atoi(p); p = p+j+1;
	    j = strcspn(p,",;"); p[j] = '\0';
	    msg_size=atoi(p); p = p+j+1;
	    j = strcspn(p,",;"); p[j] = '\0';
	    msg_pass = (unsigned)atoh(p); p = p+j+1;
	    j = strcspn(p,",;"); p[j] = '\0';
	    msg_warn = (unsigned)atoh(p); p = p+j+1;
	    j = strcspn(p,",;"); p[j] = '\0';
	    msg_fail = (unsigned)atoh(p);
	}

	if ( memo_node >= 0 ) {
	    for (j=0;j<3;j++)
		if ( (memo_tid = qnx_name_locate(memo_node, LOCAL_SYMNAME(MEMO,name),0,0))==-1) {
		    if (!j) msg(MSG,"Im trying to find %s on node %ld", name, memo_node);
		    sleep(1);
		}
		else break;
	    if (j>0 && j<3) {
		errno=0;
		msg(MSG,"Found %s on node %ld, continuing...",name,memo_node);
	    }
	    else if (j>=3)
		msg(MSG_EXIT_ABNORM,"Can't find %s for messaging on node %ld",name,memo_node);
	}
	msg_memo_node = memo_node;

	if (fn && strlen(fn))
	    if ( !(msg_fp = fopen(fn,"a")))
		msg(MSG_EXIT_ABNORM,"Can't open error file %s",fn);
	    else {
		msg_fbuf = malloc(10);
		time(&timevar);
		strftime(msg_fbuf,10,"%D",localtime(&timevar));
		fprintf(msg_fp,"Log Date: %s\n",msg_fbuf);
		fflush(msg_fp);
	    }
	if (devfn && strlen(devfn))
	    if ( !(msg_devfd = open(devfn,O_RDWR)))
		msg(MSG_EXIT_ABNORM,"Can't open device %s",devfn);
	    else {
		if ( (msg_cc = console_open(msg_devfd,O_RDWR))) {
		    console_size(msg_cc,0,0,0,0,&scrwidth);
		    msg_size = (msg_size <= 0) ? scrwidth-msg_col : msg_size;
		    msg_buf=(char *)malloc(2*msg_size);
		    msg_fail = msg_fail ? msg_fail : IBM_BLINK;
		    msg_warn = msg_warn ? msg_warn : IBM_REVERSE;
		    msg_pass = msg_pass ? msg_pass : IBM_NORMAL;
		} else {
		    tcsetct(msg_devfd, getpid());
		    if (term_load()) {
			msg(MSG_EXIT_ABNORM,"Can't load terminal definition");
			close(msg_devfd);
			msg_devfd = 0;
		    } else {
			scrwidth = term_state.num_cols;
			msg_size = (msg_size <= 0) ? scrwidth-msg_col : msg_size;
			msg_fail = msg_fail ? msg_fail : TERM_BLINK;
			msg_warn = msg_warn ? msg_warn : TERM_INVERSE;
			msg_pass = msg_pass ? msg_pass : TERM_NORMAL;
		    }
		}
	    }
	msg_inited = 1;
}
