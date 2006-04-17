#include <signal.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/proxy.h>
#include <errno.h>
#include "nortlib.h"
#include "oui.h"
#include "parent.h"
#include "globmsg.h"

int quit_if_childless = 0;
int request_quit = 0;
char *pid_string = NULL;
int wait_time = 0;
int register_name = 1;

static int check_children = 1;
static pid_t wait_proxy = -1;
static timer_t timer;

void ChildHandler( int sig_number ) {
  sig_number = sig_number;
  check_children = 1;
}

void setup_timer( void ) {
  struct sigevent evp;
  struct itimerspec value;

  wait_proxy = qnx_proxy_attach( 0, NULL, 0, -1 );
  if ( wait_proxy == -1 ) nl_error( 4, "Unable to attach proxy" );
  evp.sigev_signo = -wait_proxy;
  timer = timer_create( CLOCK_REALTIME, &evp );
  if ( timer == -1 ) nl_error( 4, "Unable to create timer" );
  value.it_value.tv_sec = wait_time;
  value.it_value.tv_nsec = 0;
  value.it_interval.tv_sec = value.it_interval.tv_nsec = 0;
  if ( timer_settime( timer, 0, &value, NULL ) != 0 )
	nl_error( 4, "Error setting timer" );
}

int timer_proxy( void ) {
  timer_delete( timer );
  qnx_proxy_detach( wait_proxy );
  wait_proxy = -1;
  nl_error( 0, "Timed Out" );
  return 1;
}

void main( int argc, char **argv ) {
  int name_id;
  int status;
  int have_children = 1;

  oui_init_options( argc, argv );
  
  if ( request_quit ) {
	pid_t pid;
	unsigned char rv;
	
	pid = qnx_name_locate( 0, nl_make_name( "parent", 1 ), 0, 0 );
	if ( pid == -1 ) nl_error( 2, "Unable to locate another parent" );
	else {
	  status = 'qu';
	  if ( Send( pid, &status, &rv,
		  sizeof( status ), 1 ) == 0 && rv == DAS_OK )
		nl_error( 0, "Quit request acknowledged" );
	  else nl_error( 2, "Quit request failed" );
	}
	exit( 0 );
  }

  /* register the name */
  if ( register_name ) {
	name_id = qnx_name_attach(0, nl_make_name("parent", 1));
	if (name_id == -1)
	  nl_error(3, "Unable to attach global name");
  }

  if ( pid_string != 0 ) {
	char *s, *t;
	pid_t pid;

	for ( s = pid_string ; ; s = t) {
	  pid = (pid_t) strtoul( s, &t, 0 );
	  if ( s == t ) break;
	  if ( kill( pid, SIGINT ) == -1 )
		switch ( errno ) {
		  case EPERM:
			nl_error( 2, "Insufficient permission to kill pid %d", pid );
			break;
		  case ESRCH:
			break;
		  default:
			nl_error( 2, "Error killing pid %d", pid );
			break;
		}
	}
	nl_error( 0, "Issuing SIGINTs" );
  }

  if ( wait_time != 0 ) setup_timer();

  /* Set up the signal handler (not earlier or we might screw
     up the library calls */
  signal( SIGCHLD, ChildHandler );

  while ( have_children || ! quit_if_childless ) {
	pid_t pid;

	if ( ! check_children ) {
	  pid = Receive( 0, &status, sizeof( status ) );
	  if ( pid != -1 ) {
		if ( pid == wait_proxy ) {
		  if ( timer_proxy() )
			break;
		} else if ( status == 'qu' ) {
		  reply_byte( pid, DAS_OK );
		  nl_error( 0, "Received quIT command" );
		  break;
		} else if ( status == 'pf' ) {
		  /* pick_file quit request: Don't do it if we can't find
		     pick_file's name */
		  pid_t pf_pid;

		  pf_pid = qnx_name_locate( 0,
			nl_make_name( "pick_file", 1 ), 0, 0 );
		  if ( pf_pid == -1 ) {
			reply_byte( pid, DAS_BUSY );
		  } else {
			reply_byte( pid, DAS_OK );
			nl_error( 0, "Honoring pick_file quit request" );
			break;
		  }
		} else reply_byte( pid, DAS_UNKN );
	  }
	}
	check_children = 0;
	pid = waitpid( -1, &status, WNOHANG );
	switch ( pid ) {
	  case 0:
		/* We get this if no children have died
		   but we still have some */
		break;
	  case -1:
		switch ( errno ) {
		  case ECHILD:
			have_children = 0;
			nl_error( 0, "No more children" );
			break;
		  case EINTR:
			check_children = 1;
			break;
		  default:
			nl_error( 4, "Unexpected error from waitpid(): %s",
			  strerror( errno ) );
		}
		break;
	  default:
		nl_error( 0, "Process %d terminated", pid );
		check_children = 1;
		break;
	}
  }
  if ( register_name )
	qnx_name_detach(0, name_id);
  
  for ( ; optind < argc; optind++ ) {
	FILE *fp;
	
	fp = fopen( argv[optind], "a" );
	if ( fp != 0 ) fputc( '\f', fp );
	fclose( fp );
  }
}
