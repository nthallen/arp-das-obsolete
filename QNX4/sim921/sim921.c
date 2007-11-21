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
#include <termios.h>
#include <signal.h>
#include "ssp.h"
#include "rtgapi.h"
#include "nortlib.h"
#include "oui.h"

#define SERTEMP_BUFSIZE 80
#define SIM900_RESP "Stanford_Research_Systems,SIM900,"
#define SIM921_RESP "Stanford_Research_Systems,SIM921,"


int fileno;
char *ofile;
FILE *logfile;
int done = 0;
void handler( int sig ) { done = 1; }

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
		case 'f':
		  ofile = optarg;
		  break;
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
  if ( ofile == 0 ) {
	logfile = stdout;
  } else {
	logfile = fopen( ofile, "a" );
	if ( logfile == 0 )
	  nl_error( 3, "Unable to append to logfile '%s'", ofile );
  }
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

char *chomp( char *s ) {
  int n = strlen(s) - 1;
  while ( n >= 0 && ( s[n] == '\r' || s[n] == '\n' ) ) {
	s[n--] = '\0';
  }
  return s;
}

int main( int argc, char **argv ) {
  rtg_t *rtg;
  oui_init_options( argc, argv );
  rtg = rtg_init( "SIM921" );
  signal( SIGINT, handler );
  while (!done) {
	int up = 0;
	int fail_cnt = 0;
	char *t;
	while ( !done && up < 2 ) { 
	  tcsendbreak( fileno, 0 );
	  t = query_port( "*IDN?\n" );
	  if ( t == 0 ) {
		if ( up == 0 ) {
		  nl_error( 1, "No response from SIM900... Polling" );
		  up = 1;
		}
	  } else if ( strncmp( t, SIM900_RESP,
				strlen(SIM900_RESP) ) == 0 ) {
		nl_error( 0, "Ident: %s", chomp(t) );
		up = 2;
	  } else {
		nl_error( 1, "Invalid response SIM900: %s", chomp(t) );
	  }
	}
	t = query_port( "CONN 1,\"EXIT\"\n*IDN?\n" );
	if ( t == 0 ) {
	  nl_error( 1, "No response from SIM921" );
	} else if ( strncmp( t, SIM921_RESP,
			  strlen(SIM921_RESP) ) == 0 ) {
	  nl_error( 0, "Ident: %s", chomp(t) );
	  up = 3;
	} else {
	  nl_error( 1, "Invalid response SIM921: %s", chomp(t) );
	}
	while ( !done && up == 3 && fail_cnt < 5 ) {
	  long itm = time(0);
	  double tm = (double) itm;
	  char *v = query_port( "TVAL?\n" );
	  if ( v == 0 ) {
		if ( fail_cnt++ == 0 ) {
		  nl_error( 1, "Lost contact with SIM921" );
		  rtg_report( rtg, tm, non_number );
		  fprintf( logfile, "%ld NaN\n", itm );
		}
	  } else {
		double fv = strtod(v,0);
		if ( fail_cnt ) {
		  nl_error( 1, "Contact restored" );
		  fail_cnt = 0;
		}
		rtg_report( rtg, tm, fv );
		fprintf( logfile, "%ld %.3lf\n", itm, fv );
		fflush( logfile );
	  }
	  sleep(2);
	}
  }
  { long itm = time(0);
    double tm = (double) itm;
    rtg_report( rtg, tm, non_number );
    fprintf( logfile, "%ld NaN\n", itm );
    fclose(logfile);
  }
  nl_error( 0, "Terminating" );
  return 0;
}
