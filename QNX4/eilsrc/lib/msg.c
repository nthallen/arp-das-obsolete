/*
	msg.c: an informing system.
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
#include <reply.h>
#include <memo.h>
#include <symname.h>
#include <msg.h>
#include <get_priv.h>
#include <beeps.h>
#include <sounds.h>

/* IBM compatable display attributes for console functions */
#define IBM_NORMAL 0x07
#define IBM_REVERSE 0x70
#define IBM_BLINK (IBM_NORMAL | 0x80)

#define LEN 200

static int msg_inited;
static int msg_verbose = 1;
static int msg_fd = -1;
static int msg_devfd = -1;
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
static char *msg_dbuf = 0;
static time_t timevar;

/* returns true if messages already initialised */
int msg_initialised(void) {
	if (msg_inited) return 1;
	return 0; 
}

/* ends messages */
void msg_end() {
	if (msg_fd != -1) close(msg_fd);
	if (msg_devfd != -1) close(msg_devfd);
	if (msg_cc) console_close(msg_cc);
	if (msg_buf) free(msg_buf);
	if (msg_fbuf) free(msg_fbuf);
	if (msg_dbuf) free(msg_dbuf);
	msg_inited=0;
}

/*  write an informative message, notes in msg.h */
void msg(int fatal, char *format,...) {

va_list ap;
char errline[LEN] = {MEMO_HDR,'\0'};
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

	switch(fatal) {
	    case MSG_PASS: break;
	    case MSG_EXIT_NORM: break;
	    case MSG_WARN: strcat(errline,WARN_STR); break;
	    case MSG_FAIL: strcat(errline,FAIL_STR); break;
	    case MSG_EXIT_ABNORM:
	    default: strcat(errline,FATAL_STR); break;
	}

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
	if (memo_tid != getpid() )
	    Send( memo_tid, errline, &replymsg, msgsize+2, sizeof(reply_type));

	/* log to file */		
	if (msg_fd != -1)  {
	    time(&timevar);
	    strftime(msg_dbuf,10,"%T",localtime(&timevar));
	    sprintf(msg_fbuf,"%s: %s\n",msg_dbuf,p);
	    write(msg_fd,msg_fbuf,strlen(msg_fbuf));
	}

	/* to stdout */
	if (msg_verbose)  {
		printf("%s\n",p);
		fflush(stdout);
	}

	if (msg_devfd != -1) {
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

	/* for now, sound server later */

	if (msg_sounds)
	    switch (get_priv()) {
		case PRIV_SPECIAL:
		    switch (fatal) {
			case MSG_WARN: WARN_NOTE; break;
			case MSG_FAIL: FAIL_NOTE; break;
			case MSG_PASS: break;
			case MSG_EXIT_NORM: break;
			default: FAIL_TUNE; break;
		    }
		default:
		    switch (fatal) {
			case MSG_WARN: WARN_BEEPS; break;
			case MSG_FAIL: FAIL_BEEPS; break;
			case MSG_PASS: break;
			case MSG_EXIT_NORM: break;
			default: FAIL_BEEPS; WARN_BEEPS; break;
		    }
	    }
    switch (fatal) {
	case MSG_WARN: break;
	case MSG_FAIL: break;
	case MSG_PASS: break;
	case MSG_EXIT_NORM: exit(0); break;
	default: exit(1); break;
    }
}

/* initialise messages options */
void msg_init(char *hdr, char *fn, int verbose, char *targ, char *oarg,
	      int sounds, int sys) {

    int j;
    nid_t nd;
    char *p;
    char devfn[80] = {'\0'};
    char name[NAME_MAX] = {'\0'};
    char memo_name[NAME_MAX] = MEMO;

	msg_verbose = verbose;
	msg_sounds = sounds;
	msg_sys = sys;
	if (hdr && strlen(hdr)) strncpy(msg_hdr,hdr,39);
	if (oarg && strlen(oarg)) {
	    j = strcspn(oarg,",;");
	    p = oarg+j+1;	
	    oarg[j]='\0';
	    strncat(devfn,oarg,79);
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
	memo_tid = getpid();
	if (targ && strlen(targ)) {
	    j = strcspn(targ,",;");
	    p = targ+j+1;	
	    targ[j]='\0';
	    nd = atoi(targ);
	    j = strcspn(p,",;"); p[j] = '\0';
	    if (strlen(p)) strncpy(memo_name,p,NAME_MAX-1);
	    if ( (memo_tid = qnx_name_locate(nd, LOCAL_SYMNAME(memo_name,name),0,0))==-1)
		msg(MSG_EXIT_ABNORM,"Can't find %s for messaging on node %ld",name,nd);
	}

	if (fn && strlen(fn))
	    if ( (msg_fd = open(fn,O_WRONLY | O_CREAT | O_APPEND | O_FSYNCH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1)
		msg(MSG_EXIT_ABNORM,"Can't open error file %s",fn);
	    else {
		if (errno == ENOENT) errno = 0;
		msg_fbuf = malloc(LEN+10);
		msg_dbuf = malloc(10);
		time(&timevar);
		strftime(msg_dbuf,10,"%D",localtime(&timevar));
		sprintf(msg_fbuf,"Log Date: %s\n",msg_dbuf);
		write(msg_fd,msg_fbuf,strlen(msg_fbuf));
	    }
	if (devfn && strlen(devfn))
	    if ( (msg_devfd = open(devfn,O_RDWR)) == -1)
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
			msg_devfd = -1;
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
