/* sertemp.c
  Utility to collect data from Stanford Research instrument
  and report it in various ways.
    1. Append to log file
	2. Send to RTG
	3. Write to dacache
  
  Use msg.oui for standard messages
  Write output to stdout
  Options:
    msg options
	Must specify a serial port device
*/
#include <stdlib.h>
#include <fcntl.h>
#include <sys/dev.h>
#include <unistd.h>
#include <assert.h>
#include "ssp.h"
#include "rtgapi.h"
#include "nortlib.h"
#include "oui.h"

#define SERTEMP_BUFSIZE 80

int fileno;

void open_port( char *port ) {
  if ( fileno != 0 )
	nl_error( 3, "Too many input arguments" );
  fileno = open( port, O_RDWR );
  if ( fileno < 0 )
	nl_error( 3, "Unable to open port: '%s'", port );
}

void sim921_init( int argc, char **argv ) {
  int optltr;

  optind = 0;
  opterr = 0;
  while ((optltr = getopt(argc, argv, opt_string)) != -1) {
	switch (optltr) {
		case '?':
		  nl_error(3, "Unrecognized Option -%c", optopt);
		default:
		  break;
	}
  }
  for (; optind < argc; optind++) {
	optarg = argv[optind];
	open_port(optarg);
  }
  if ( fileno <= 0 )
	nl_error( 3, "No port opened" );
}

char *query_port( char *cmd ) {
  static char buf[SERTEMP_BUFSIZE+1];
  int nbytes, rbytes;
  
  assert(cmd!=0);
  nbytes = strlen(cmd);
  if ( nbytes ) {
	rbytes = write( fileno, cmd, nbytes);
	if ( rbytes != nbytes )
	  nl_error( 1, "nbytes=%d rbytes=%d", nbytes, rbytes );
  }
  rbytes = dev_read( fileno, buf, SERTEMP_BUFSIZE, SERTEMP_BUFSIZE,
          1, 10, 0, 0 );
  if ( rbytes < 0 )
	nl_error( 1, "Received error %d on dev_read", errno );
  assert(rbytes <= SERTEMP_BUFSIZE);
  if ( rbytes > 0 ) {
	buf[rbytes] = '\0';
	return buf;
  } else return 0;
}

int main( int argc, char **argv ) {
  rtg_t *rtg;
  int up = 1;
  oui_init_options( argc, argv );
  rtg = rtg_init( "SIM921" );
  { char *t = query_port( "*IDN?\n" );
	if ( t == 0 ) {
	  nl_error( 1, "No response from SIM921... Polling" );
	  while ( t == 0 )
		t = query_port( "*IDN?\n" );
	}
	nl_error( 0, "Ident: %s", t );
  }
  while ( 1 ) {
	double itm = time();
	double tm = (double) itm;
	char *v = query_port( "TVAL?\n" );
	if ( v == 0 ) {
	  if ( up ) {
		nl_error( 1, "Lost contact with SIM921" );
		up = 0;
		rtg_report( rtg, tm, non_number );
		printf( "%ld NaN\n", itm );
	  }
	} else {
	  double fv = strtod(v,0);
	  if ( !up ) {
		nl_error( 1, "Contact restored" );
		up = 1;
	  }
	  rtg_report( rtg, tm, fv );
	  printf( "%ld %.1lf\n", tm, fv );
	}
	sleep(2);
  }
  return 0;
}
