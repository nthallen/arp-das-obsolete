#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <process.h> /* for spawnlp() */
#include <unix.h> /* for gethostname() */
#include <unistd.h>
#include "nortlib.h"
#include "oui.h"
#include "dbr.h"
#include "msg.h"
#include "globmsg.h"

static int TM_socket;

void forbidden( int fail, char *where ) {
  if ( fail )
	nl_error( 3, "Error %d %s", errno, where );
}

static void tmwrite( void *buf, int size ) {
  int rv = write( TM_socket, buf, size );
  if (rv == -1)
	nl_error(3, "write returned error %d", errno );
  else if ( rv != size )
	nl_error(3, "write returned %d, expected %d", rv, size );
}

void finish_connection( int argc, char **argv ) {
  struct sockaddr_in InSockAddr;
  struct sockaddr *SockAddrPtr = (struct sockaddr *)&InSockAddr;
  int status;
  struct hostent *host;
  unsigned short Port;
  char *RemoteHost;

  { optind = 0; /* start from the beginning */
	opterr = 0; /* disable default error message */
	while (getopt(argc, argv, opt_string) != -1);
  }

  if ( argc < optind + 2 )
    nl_error( 3, "Insufficient arguments" );
  RemoteHost = argv[optind];
  if ( RemoteHost == 0 || *RemoteHost == '\0' )
	nl_error( 3, "Must specify Hostname" );
  Port = strtoul( argv[optind+1], NULL, 10 );
  if ( Port == 0 )
	nl_error( 3, "Bad port number: %s", argv[optind+1] );
  
  TM_socket = socket( AF_INET, SOCK_STREAM, 0);
  forbidden( TM_socket == -1, "from socket" );

  host = gethostbyname( RemoteHost );
  forbidden( host == 0, "from gethostbyname" );

  InSockAddr.sin_len = 0;
  InSockAddr.sin_family = AF_INET;
  InSockAddr.sin_port = Port;
  InSockAddr.sin_addr.s_addr =
	*(unsigned long *)host->h_addr_list[0];
  { int i; for ( i = 0; i < 8; i++ ) InSockAddr.sin_zero[i] = 0; }
  status = connect( TM_socket, SockAddrPtr, sizeof(InSockAddr) );
  forbidden( status == -1, "from connect" );
  
  if ( fork() ) exit(0);
  /* We really must close our std fds, because the rsh is going
     to shut down... */
  close( 0 ); close( 1 ); close( 2 );
}

void main( int argc, char **argv ) {
  oui_init_options( argc, argv );
  tmwrite( & dbr_info.tm, sizeof(dbr_info.tm) );
  BEGIN_MSG;
  DC_operate();
  DONE_MSG;
}

void DC_data(dbr_data_type *dr_data) {
  msg_hdr_type hdr = DCDATA;
  unsigned short msg_size;

  tmwrite( &hdr, sizeof(hdr) );
  msg_size = dr_data->n_rows * tmi(nbrow) + sizeof(token_type);
  tmwrite( dr_data, msg_size );
}

void DC_tstamp(tstamp_type *tstamp) {
  msg_hdr_type hdr = TSTAMP;

  tmwrite( &hdr, sizeof(hdr) );
  tmwrite( &tstamp, sizeof(tstamp_type) );
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
  tmwrite( &cmdmsg, sizeof(cmdmsg) );

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
