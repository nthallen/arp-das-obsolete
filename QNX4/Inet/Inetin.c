#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <netinet/in.h>
#include <process.h> /* for spawnlp() */
#include <unix.h> /* for gethostname() */
#include <unistd.h> /* for fork() */
#include <conio.h>
#include "nortlib.h"
#include "oui.h"
#include "dbr.h"
#include "msg.h"
#include "globmsg.h"

#define INET_HDR_BYTE 243 /* totally arbitrary */

static int TM_socket;

void forbidden( int fail, char *where ) {
  if ( fail )
	nl_error( 3, "Error %d %s", errno, where );
}

void Inet_parent( pid_t parent_pid, pid_t child_pid ) {
  struct {
	msg_hdr_type Inet_hdr;
	msg_hdr_type hdr;
	union {
	  dbr_data_type data;
	  tstamp_type tstamp;
	  dascmd_type cmd;
	} u;
  } *Inet_msg;
  int done = 0, nb;
  struct sockaddr_in InSockAddr;
  struct sockaddr *SockAddrPtr = (struct sockaddr *)&InSockAddr;
  struct sockaddr SockAddr;
  int sock, status;
  unsigned short Port;
  char Hostname[MAXHOSTNAMELEN], PortStr[8];
  struct hostent *host;
  /* char *RemoteHost; */
  /* char *Experiment; */
  
  Inet_msg = malloc(MAX_BUF_SIZE+1);
  if (Inet_msg == 0 )
	msg( 3, "Unable to allocate buffer" );
  Inet_msg->Inet_hdr = INET_HDR_BYTE;
  
  sock = socket( AF_INET, SOCK_STREAM, 0);
  forbidden( sock == -1, "from socket" );

  InSockAddr.sin_len = 0;
  InSockAddr.sin_family = AF_INET;
  InSockAddr.sin_port = 0;
  InSockAddr.sin_addr.s_addr = INADDR_ANY;
  { int i; for ( i = 0; i < 8; i++ ) InSockAddr.sin_zero[i] = 0; }
  status = bind( sock, SockAddrPtr, sizeof(InSockAddr) );
  forbidden( status != 0, "binding to socket" );
  { int len;
	len = sizeof(InSockAddr);
	status = getsockname( sock, SockAddrPtr, &len );
	forbidden( status != 0, "in getsockname" );
  }
  Port = InSockAddr.sin_port;
  forbidden( Port == 0, "Bad Port Number from getsockname" );

  status = gethostname( Hostname, MAXHOSTNAMELEN );
  forbidden( status == -1, "in gethostname" );
  host = gethostbyname( Hostname );
  forbidden( host == 0, "from gethostbyname" );

  sprintf( PortStr, "%u", Port );
  status = listen( sock, 1 );
  forbidden( status == -1, "from listen()" );

  { FILE *fp;
	fp = fopen( "Inet.ports", "a" );
	forbidden( fp == 0, "Opening Inet.ports" );
	fprintf( fp, "TM %s %u\n", host->h_name, Port );
	fclose( fp );
  }
  /* Let original parent know it's OK to quit */
  Send( parent_pid, NULL, NULL, 0, 0 );
  TM_socket = 0;
  while ( TM_socket <= 0 ) {
	int AddrLen = sizeof(SockAddr);
	TM_socket = accept( sock, &SockAddr, &AddrLen );
  }
  /* First get dbr_info and send to child */
  nb = read( TM_socket, & dbr_info.tm, sizeof(dbr_info.tm) );
  if (nb != sizeof(dbr_info.tm))
	nl_error( 2, "Read returned %d, expected %d", nb,
				  sizeof(dbr_info.tm));
  Send( child_pid, &dbr_info.tm, NULL, sizeof(dbr_info.tm), 0 );

  while (!done) {
	token_type n_rows;
	int msg_size;
	unsigned char rv;

	read( TM_socket, &Inet_msg->hdr, sizeof(Inet_msg->hdr) );
	switch ( Inet_msg->hdr ) {
	  case DCDATA:
		read( TM_socket, &n_rows, sizeof(token_type) );
		Inet_msg->u.data.n_rows = n_rows;
		read( TM_socket, &Inet_msg->u.data.data, n_rows * tmi(nbrow) );
		msg_size = sizeof(token_type) + n_rows * tmi(nbrow);
		break;
	  case DCDASCMD:
		Inet_msg->hdr = DASCMD; /* Change it for DG */
		read( TM_socket, &Inet_msg->u.cmd, sizeof(dascmd_type) );
		msg_size = sizeof(dascmd_type);
		msg_size += sizeof(Inet_msg->hdr);
		if ( Send( child_pid, &Inet_msg->hdr, &rv, msg_size, sizeof(rv))
			  == -1 && errno == ESRCH )
		  return;
		if ( Inet_msg->u.cmd.type == DCT_QUIT ) done = 1;
		continue;
	  case TSTAMP:
		read( TM_socket, &Inet_msg->u.tstamp, sizeof(tstamp_type) );
		msg_size = sizeof(tstamp_type);
		break;
	  default:
		msg( 3, "Parent: Unexpected type code %d", Inet_msg->hdr );
	}
	msg_size += 2 * sizeof(msg_hdr_type);
	if ( Send( child_pid, Inet_msg, &rv, msg_size, sizeof(rv) )
		  == -1 && errno == ESRCH )
	  return;
  }
  wait( NULL );
}

void initiate_connection( void ) {
  pid_t child_pid, parent_pid, gparent_pid;

/*  Experiment = getenv( "Experiment" );
  if ( Experiment == 0 || *Experiment == '\0' )
	Experiment = "none";
  RemoteHost = getenv( "RemoteHost" );
  if ( RemoteHost == 0 || *RemoteHost == '\0' )
	RemoteHost = "localhost"; */

  gparent_pid = getpid();
  child_pid = fork();
  if ( child_pid ) {
	Receive( child_pid, NULL, 0 );
	Reply( child_pid, NULL, 0 );
	exit(0);
  }
  parent_pid = getpid();
  child_pid = fork();
  /* probably safe to leave 0,1,2 open */
  if ( child_pid != 0 ) {
	Inet_parent( gparent_pid, child_pid );
	nl_error( 0, "Parent Terminating" );
	exit(0);
  } else {
	pid_t who;
	who = Receive( parent_pid, &dbr_info.tm, sizeof(dbr_info.tm) );
	if ( who !=parent_pid )
	  nl_error( 3, "Unexpected Receive error" );
	Reply( who, NULL, 0 );
  }
}

void main( int argc, char **argv ) {
  oui_init_options( argc, argv );
  BEGIN_MSG;
  DG_operate();
  DONE_MSG;
}

int DG_DASCmd(unsigned char type, unsigned char val) {
  return(0);
}

/* Called from DG_operate when data is required. From here, we
can call DG_s_data, DG_s_tstamp and/or DG_s_dascmd */
static int Inet_rows = 0, Inet_row;
static pid_t Inet_pid;
static tstamp_type Inet_tstamp;
static unsigned char *Inet_data = NULL;

int DG_get_data(unsigned int n_rows) {
  switch (Inet_rows ) {
	case 0: return 0; /* Don't reply */
	case -1:
	  DG_s_tstamp( &Inet_tstamp );
	  Inet_rows = 0;
	  break;
	default:
	  if ( n_rows > Inet_rows ) n_rows = Inet_rows;
	  DG_s_data( n_rows, &Inet_data[Inet_row*tmi(nbrow)], 0, NULL );
	  Inet_rows -= n_rows;
	  if ( Inet_rows > 0 ) return 0;
	  Inet_row += n_rows;
	  break;
  }
  reply_byte( Inet_pid, DAS_OK );
  return 0;
}

int DG_other(unsigned char *msg_ptr, int sent_tid) {
  struct Inet_msg_st {
	msg_hdr_type Inet_hdr;
	msg_hdr_type hdr;
	union {
	  dbr_data_type data;
	  tstamp_type tstamp;
	} u;
  } *Inet_msg;

  Inet_msg = (struct Inet_msg_st *) msg_ptr;
  if ( Inet_msg->Inet_hdr == INET_HDR_BYTE ) {
	if ( Inet_rows != 0 )
	  msg( 4, "Inet_rows should be zero! %d", Inet_rows );
	Inet_pid = sent_tid;
	switch (Inet_msg->hdr) {
	  case TSTAMP:
		Inet_rows = -1;
		Inet_tstamp = Inet_msg->u.tstamp;
		break;
	  case DCDATA:
		Inet_rows = Inet_msg->u.data.n_rows;
		if ( Inet_data == 0 ) {
		  Inet_data = malloc( MAX_BUF_SIZE );
		  if ( Inet_data == 0 )
			msg( 3, "Unable to allocate buffer" );
		}
		if (Inet_rows > tmi(nbrow))
		  msg( 3, "Too many rows received! %d", Inet_rows );
		memcpy( Inet_data, Inet_msg->u.data.data,
		  Inet_rows * tmi(nbrow) );
		Inet_row = 0;
		break;
	  default:
		msg( 4, "Unexpected subtype %u", Inet_msg->hdr );
	}
  } else reply_byte( sent_tid, DAS_UNKN );
  return 0;
}
