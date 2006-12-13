#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <env.h>
#include <sys/psinfo.h>
#include <unix.h>
#include <unistd.h>

#ifdef LOTS_OF_STUFF
  #include <sys/types.h>
  #include <sys/param.h>
#endif

/* for getpeername() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
extern int h_errno;

#include "nortlib.h"
#include "oui.h"
#include "dbr.h"
#include "msg.h"
#include "globmsg.h"
#include "Inet.h"

#define BANNER "Inetout v1.0\n"

void forbidden( int fail, char *where ) {
  if ( fail )
	nl_error( 3, "Error %d %s", errno, where );
}

/* returns the node number where the name is found */
static pid_t find_name( char *name, nid_t node ) {
  char *fname = nl_make_name( name, node == 0 );
  for (;;) {
	int i;
	for ( i = 0; i < 15; i++ ) {
	  pid_t who;
	  who = qnx_name_locate( node, fname, 0, NULL );
	  if ( who != -1 ) return who;
	  sleep(1);
	}
	tmwritestr( 1, "-Still looking for " );
	tmwritestr( 1, fname );
	tmwritestr( 1, "\n" );
  }
}

nid_t get_pids_nid(pid_t pid) {
  struct _psinfo psdata;
  
  if (qnx_psinfo(0, pid, &psdata, 0, NULL) != pid)
	nl_error(3, "Unable to get process info on pid %d", pid);
  return (psdata.flags & _PPF_VID) ?
	psdata.un.vproc.remote_nid : getnid();
}

static char *report_connection(void) {
  struct sockaddr_in addr;
  int length = sizeof(addr);

  if ( getpeername( 1, (struct sockaddr *)&addr, &length ) == -1 ) {
	tmwritestr( 1, "Error in getpeername()\n" );
	msg(2, "getpeername(): %s", strerror(errno) );
	return NULL;
  } else {
	struct hostent *host;
	
	host = gethostbyaddr( (char *)&addr.sin_addr, sizeof(struct in_addr),
					AF_INET );
	if (host == NULL ) {
	  tmwritestr( 1, "Error in gethostbyaddr()\n" );
	  msg(2, "gethostbyaddr(): %d", h_errno );
	  return NULL;
	} else {
	  tmwritestr( 1, "Connection Established from " );
	  tmwritestr( 1, host->h_name );
	  tmwritestr( 1, "\n" );
	  return host->h_name;
	}
  }
}

void finish_connection( void ) {
  char buf[MYBUFSIZE];
  char *peer;
  pid_t dg, db;
  nid_t node;

  tmreadline( 0, buf, MYBUFSIZE );
  if ( strcmp( buf, INET_REQUEST ) ) {
	tmwritestr( 1, "Unauthorized Access\n!" );
	exit(1);
  }
  tmwritestr( 1, BANNER );
  /* Read the Experiment */
  tmreadline( 0, buf, MYBUFSIZE );
  if ( setenv( "Experiment", buf, 1 ) ) {
	tmwritestr( 1, "Cannot set Experiment\n!" );
	exit(1);
  }
  peer = report_connection();
  tmwritestr( 1, "Looking for DG\n" );
  
  dg = find_name( DG_NAME, 0 );
  node = get_pids_nid( dg );
  tmwritestr( 1, "Found DG, registering with memo\n" );
  sprintf( buf, "%d,memo", node );
  msg_init( "Iout", "", 0, buf, "", 0, 0, 0 );
  msg(0, "Connection Established from %s", peer );
  tmwritestr( 1, "Looking for bfr\n" );
  db = find_name( DB_NAME, node );
  tmwritestr( 1, "Proceeding with Initialization\n*" );
  DC_data_rows = 1;
  DC_init( DBC, node );
}

void main( int argc, char **argv ) {
  signal( SIGPIPE, SIG_IGN );
  oui_init_options( argc, argv );
  if ( tmwrite( 1, & dbr_info, sizeof(dbr_info) ))
	DC_bow_out();
  BEGIN_MSG;
  DC_operate();
  DONE_MSG;
}

void DC_data(dbr_data_type *dr_data) {
  msg_hdr_type hdr = DCDATA;
  unsigned short msg_size;

  msg_size = dr_data->n_rows * tmi(nbrow) + sizeof(token_type);
  if ( tmwrite( 1, &hdr, sizeof(hdr) ) ||
	   tmwrite( 1, dr_data, msg_size ) )
    DC_bow_out();
}

void DC_tstamp(tstamp_type *tstamp) {
  msg_hdr_type hdr = TSTAMP;

  msg( 0, "Received TSTAMP: %u = %ld", tstamp->mfc_num,
		tstamp->secs );
  if ( tmwrite( 1, &hdr, sizeof(hdr) ) ||
	   tmwrite( 1, tstamp, sizeof(tstamp_type) ) )
    DC_bow_out();
}

#define MKDCTV(x,y) ((x<<8)|y)
#define DCTV(x,y) MKDCTV(DCT_##x,DCV_##y)

void DC_DASCmd(unsigned char type, unsigned char val) {
  struct {
	msg_hdr_type hdr;
	dascmd_type cmd;
  } cmdmsg;

  cmdmsg.hdr = DCDASCMD;
  cmdmsg.cmd.type = type;
  cmdmsg.cmd.val = val;
  if ( tmwrite( 1, &cmdmsg, sizeof(cmdmsg) ) )
	DC_bow_out();

  switch (MKDCTV(type, val)) {
	case DCTV(TM,TM_START):
	  break;
	case DCTV(TM,TM_END):
	  break;
	case DCTV(QUIT,QUIT):
	  break;
	default:
	  break;
  }
}

/* Handle other commands */
void DC_other(unsigned char *msg_ptr, int sent_tid) {
  reply_byte( sent_tid, DAS_UNKN );
}
