#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <netinet/in.h>

#include "nortlib.h"
#include "oui.h"
#include "runnum.h"

static FILE *Host_fp, *File_fp;

static void forbidden( int fail, char *where ) {
  if ( fail )
	nl_error( 3, "%s %s", strerror(errno), where );
}

void dr_printf( char *fmt, ... ) {
  va_list arg;
  
  va_start( arg, fmt );
  /* do the printing using fmt, arg */
  if ( Host_fp &&
	   vfprintf( Host_fp, fmt, arg ) < 0 ) {
	nl_error( 3, "Error writing to remote host: %s",
	  strerror(errno) );
  }
  if ( File_fp &&
		vfprintf( File_fp, fmt, arg ) < 0 ) {
	nl_error( 3, "Error writing to log file: %s",
	  strerror(errno) );
  }
  va_end( arg );
}

static void dr_host_init( char *RemHost ) {
  struct servent *service;
  struct hostent *host;
  struct sockaddr_in server;
  int status, DR_socket;
  unsigned short Port;
  
  service = getservbyname( "datarecv", "tcp" );
  if ( service == NULL )
	nl_error( 3, "'datarecv' service not defined in /etc/services" );
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
  
  DR_socket = socket( AF_INET, SOCK_STREAM, 0);
  forbidden( DR_socket == -1, "from socket" );

  server.sin_len = 0;
  server.sin_family = AF_INET;
  server.sin_port = Port;
  memcpy( &server.sin_addr, host->h_addr, host->h_length );
  status = connect( DR_socket, (struct sockaddr *)&server, sizeof(server) );
  forbidden( status == -1, "from connect" );
  Host_fp = fdopen( DR_socket, "w" );
  forbidden( Host_fp == NULL, "from fdopen" );
}

void drext_init( int argc, char **argv ) {
  int c;

  if ( rundate == NULL || *rundate == '\0' )
	nl_error( 3, "No rundate" );

  optind = 0; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt( argc, argv, opt_string )) != -1) {
	switch ( c ) {
	  case 'F':
		File_fp = fopen( optarg, "a" );
		if ( File_fp == NULL )
		  nl_error( 3, "Unable to write to log file %s: %s",
			optarg, strerror(errno) );
		break;
	  case 'H':
		dr_host_init( optarg );
		break;
	  case '?':
		nl_error(3, "Unrecognized Option -%c", optopt);
	  default:
		/* Another init routine can handle other options */
		break;
	}
  }
  dr_printf( "!Path %s\n", rundate );
}
/*
=Name drext_init(): datarecvext initialization function.
=Subject HPF datarecvext
=Name dr_printf(): datarecvext output function
=Subject HPF datarecvext
=Synopsis
  #include "runnum.h"
  void drext_init( int argc, char ** argv );
  void dr_printf( char *fmt, ... );
=Description
  drext_init() is the init_options function for datarecvext
  programs. It is called from drext.oui. It is responsible for
  opening a specified log file and/or a socket connection to a
  remote host where dataRecv is presumably waiting. It will look
  for the rundate file in the TMLOGDIR directory and issue a
  fatal error if it is not located.
  
  dr_printf() is a printf() function which will output to
  whatever streams drext_init() has set up.
=Returns
  Neither function returns a value.
=SeeAlso
  =datarecvext= functions, =HPF Support Routines=.
=End
*/
