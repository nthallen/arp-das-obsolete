#include <stdlib.h>
#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#ifndef __QNXNTO__
  #include <sys/kernel.h>
#endif
#include <netinet/in.h>
#include <unix.h> /* for errno.h, wait, etc. */
#include <unistd.h> /* for fork() */
#include <ctype.h>
#include <string.h>
#include "nortlib.h"
#include "oui.h"
#include "dbr.h"
#include "msg.h"
#include "globmsg.h"
#include "Inet.h"

dbr_info_type dbr_info;

int ask_memo_to_quit = 1;
sigjmp_buf jmpbuf;
void handler( int sig_no ) {
  siglongjmp( jmpbuf, 1 );
}

void forbidden( int fail, char *where ) {
  if ( fail )
	nl_error( 3, "%s %s", strerror(errno), where );
}

int open_socket( char *RemHost ) {
  struct servent *service;
  struct hostent *host;
  struct sockaddr_in server;
  int status, TM_socket;
  unsigned short Port;
  
  service = getservbyname( "tm", "tcp" );
  if ( service == NULL )
	nl_error( 3, "'tm' service not defined in /etc/services" );
  Port = service->s_port;

  host = gethostbyname( RemHost );
  if ( host == NULL ) {
	char *errtxt;
	extern int h_errno;
	switch ( h_errno ) {
	  case HOST_NOT_FOUND: errtxt = "Host Not Found"; break;
	  case TRY_AGAIN: errtxt = "Try Again"; break;
	  case NO_RECOVERY: errtxt = "No Recovery"; break;
	  case NO_DATA: errtxt = "No Data"; break;
	  default: errtxt = "Unknown"; break;
	}
	nl_error( 3, "\"%s\" error from gethostbyname", errtxt );
  }
  
  TM_socket = socket( AF_INET, SOCK_STREAM, 0);
  forbidden( TM_socket == -1, "from socket" );

  server.sin_len = 0;
  server.sin_family = AF_INET;
  server.sin_port = Port;
  memcpy( &server.sin_addr, host->h_addr, host->h_length );
  status = connect( TM_socket, (struct sockaddr *)&server, sizeof(server) );
  forbidden( status == -1, "from connect" );
  return TM_socket;
}


/* This is where the protocol is defined
  The client (Inetin) will start with a standard request line
  The server (Inetout) will reply with a line of text introducing itself
  The client (Inetin) will then write a line containing the
  Experiment name.
  The server will then write zero or more lines of text
  indicating the status of its startup procedure followed by
  a single '*' when it is ready to begin transmission.
  If any line begins with '!', it indicates that the server
  is going to quit and the client should also without further
  negotiation. Any line that begins with a '-' can be output
  to the screen, but will be omitted from the log. This is
  used for keep-alive messages "Still waiting..."
*/
void negotiate( int socket, char *Exp ) {
  char buf[MYBUFSIZE];
  
  tmwritestr( socket, INET_REQUEST "\n" );
  tmreadline( socket, buf, MYBUFSIZE );
  msg( 0, "%s", buf );
  /* printf( "Inetout: %s\n", buf ); */
  tmwritestr( socket, Exp );
  tmwritestr( socket, "\n" );
  for (;;) {
	if ( tmread( socket, buf, 1 ) ) exit(1);
	if ( buf[0] == '*' ) break;
	else if ( buf[0] == '!' )
	  msg( 3, "Server broke off negotiation" );
	tmreadline( socket, buf+1, MYBUFSIZE-1 );
	msg( 0, "Inetout: %s", buf );
  }
}

void pipe_write( unsigned char *buf, size_t nbytes ) {
  while ( nbytes > 0 ) {
	int rv = write( STDOUT_FILENO, buf, nbytes );
	nbytes -= rv;
	buf += rv;
  }
}

void Inet_parent( char *Exp, char *RemHost ) {
  struct {
	msg_hdr_type Inet_hdr;
	msg_hdr_type hdr;
	union {
	  dbr_data_type data;
	  tstamp_type tstamp;
	  dascmd_type cmd;
	} u;
  } __attribute__((packed)) *Inet_msg;
  int TM_socket;
  int done = 0;

  Inet_msg = malloc(MAX_BUF_SIZE+1);
  if (Inet_msg == 0 )
	msg( 3, "Unable to allocate buffer" );
  Inet_msg->Inet_hdr = INET_HDR_BYTE;
  
  TM_socket = open_socket( RemHost );
  negotiate( TM_socket, Exp );

  /* Now get dbr_info and send to child */
  if ( tmread( TM_socket, & dbr_info, sizeof(dbr_info) ) )
	exit(1);
  #ifdef __QNXNTO__
    pipe_write( (char *)(&dbr_info), sizeof(dbr_info) );
  #else
	if ( Send( child_pid, &dbr_info, NULL, sizeof(dbr_info), 0 )
		== -1 )
	nl_error( 3, "Parent: Send to child failed: %d", errno );
  #endif

  while (!done) {
	token_type n_rows;
	int msg_size;
	unsigned char rv;

	if ( tmread( TM_socket, &Inet_msg->hdr, sizeof(Inet_msg->hdr) ) )
	  break;
	switch ( Inet_msg->hdr ) {
	  case DCDATA:
		if ( tmread( TM_socket, &n_rows, sizeof(token_type) ) ||
			 tmread( TM_socket, &Inet_msg->u.data.data,
				n_rows * tmi(nbrow) ) ) {
		  done = 1;
		  continue;
		}
		Inet_msg->u.data.n_rows = n_rows;
		msg_size = sizeof(token_type) + n_rows * tmi(nbrow);
		break;
	  case DCDASCMD:
		/* Inet_msg->hdr = DASCMD; */ /* Change it for DG */
		if ( tmread( TM_socket, &Inet_msg->u.cmd,
					sizeof(dascmd_type) ) ) {
		  done = 1; continue;
		}
		msg_size = sizeof(dascmd_type);
		msg_size += sizeof(Inet_msg->hdr);
		#ifdef __QNXNTO__
		  pipe_write( (char *)Inet_msg, msg_size + sizeof(msg_hdr_type));
		#else
		  if ( Send( child_pid, &Inet_msg->hdr, &rv, msg_size, sizeof(rv))
				== -1 && errno == ESRCH )
			return;
		#endif
		if ( Inet_msg->u.cmd.type == DCT_QUIT ) done = 1;
		continue;
	  case TSTAMP:
		if ( tmread( TM_socket, &Inet_msg->u.tstamp,
					  sizeof(tstamp_type) ) ) {
		  done = 1;
		  continue;
		}
		msg_size = sizeof(tstamp_type);
		break;
	  default:
		msg( 3, "Parent: Unexpected type code %d", Inet_msg->hdr );
	}
	msg_size += 2 * sizeof(msg_hdr_type);
	#ifdef __QNXNTO__
	  pipe_write( (char *)Inet_msg, msg_size );
	#else
	  if ( Send( child_pid, Inet_msg, &rv, msg_size, sizeof(rv) )
			== -1 && errno == ESRCH )
		return;
	#endif
  }
  wait( NULL );
}

void initiate_connection( void ) {
  char *RemoteHost, *Experiment;

  Experiment = getenv( "RemEx" );
  if ( Experiment == 0 || *Experiment == '\0' )
	msg( 3, "Experiment undefined" );
  { int i;
	for ( i = 0; Experiment[i] != '\0'; i++ ) {
	  if ( !isalnum(Experiment[i]) )
		msg( 3, "Illegal character in Experiment" );
	}
	if ( i >= MYBUFSIZE )
	  msg( 3, "Experiment definition too long" );
  }
  RemoteHost = getenv( "RemoteHost" );
  if ( RemoteHost == 0 || *RemoteHost == '\0' )
	msg( 3, "RemoteHost undefined" );

  if ( sigsetjmp( jmpbuf, 1 ) == 0 ) {
	signal( SIGINT, handler );
	Inet_parent( Experiment, RemoteHost );
  }
  nl_error( 0, "Parent Terminating" );
  exit(0);
}

int main( int argc, char **argv ) {
  if ( sigsetjmp( jmpbuf, 1 ) == 0 ) {
	oui_init_options( argc, argv );
	/* BEGIN_MSG; */
	/* DG_operate(); */
  }
  msg( 0, "Child Terminating" );
  return 0;
}

#ifndef __QNXNTO__

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
	  Inet_row += n_rows;
	  if ( Inet_rows > 0 ) return 0;
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
		if (Inet_rows * tmi(nbrow) > MAX_BUF_SIZE )
		  msg( 3, "Too many rows received! %d", Inet_rows );
		Readmsg( sent_tid, 3, Inet_data, Inet_rows*tmi(nbrow) );
		Inet_row = 0;
		break;
	  default:
		msg( 4, "Unexpected subtype %u", Inet_msg->hdr );
	}
  } else reply_byte( sent_tid, DAS_UNKN );
  return 0;
}

#endif

