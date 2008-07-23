#include <sys/kernel.h>
#include <sys/proxy.h>
#include <unix.h>
#include "nortlib.h"
#include "oui.h"
#include "navout.h"
#include "navutil.h"
#include "da_cache.h"

static char navdata[] =
 "\001G 230:17:06:01 N 9.98915 W 84.22153 82.17  -0.8789"
 "  -0.6262  1.22   78.79  1.1  234.2 -0.021 -0.009 0.015"
 "  0.0   -0.1  -0.3  0.28   919.6   N 9.98915 W 84.22153"
 " 344.648  904.297  -2.026 -0.02  +NAN   949.2   0.000 0.00"
 "   1.1  237.3 81.03  70.15  0.940  4.998  4.998  4.998"
 "  1.120  2.116  4.707  4.998  4.998  4.998  4.998"
 "  3.579  \r\n";

#define SIGN_FMT ' ', '-'
#define PRESSURE_FMT 8, 3, SIGN_FMT, "mbar"
#define LAT_FMT 9, 5, 'N', 'S', "degrees"
#define LONG_FMT 10, 5, 'E', 'W', "degrees"

typedef struct {
  char *name;
  unsigned short addr;
  int offset, width, precision;
  char pos, neg;
  char *units;
} var_def;

var_def vars[] = {
  "sPress", 0x1000, 162, PRESSURE_FMT,
  "tPress", 0x1002, 171, PRESSURE_FMT,
  "sTemp",  0x1004, 194, 6, 2, SIGN_FMT, "C",
  "pTAS",   0x1006, 215, 6, 2, SIGN_FMT, "m/s",
  "iLat",   0x1008,  16, LAT_FMT,
  "iLong",  0x100A,  26, LONG_FMT,
  "gLat",   0x100C, 141, LAT_FMT,
  "gLong",  0x100E, 151, LONG_FMT,
  "gAlt",   0x1010, 133, 7, 1, SIGN_FMT, "m",
  "tHead",  0x1012,  37, 6, 2, SIGN_FMT, "degrees",
  "Pitch",  0x1014,  44, 8, 4, SIGN_FMT, "degrees",
  "Roll",   0x1016,  53, 8, 4, SIGN_FMT, "degrees",
  "sElev",  0x1018, 233, 6, 2, SIGN_FMT, "degrees",
  "sAzim",  0x101A, 240, 6, 2, SIGN_FMT, "degrees"
};
#define N_VARS (sizeof(vars)/sizeof(var_def))

#define NAV_TIME 3

static FILE *serial_port;
static pid_t timer_proxy;
static timer_t timer;
static void init_timer( void );
static void init_cache( void );
static void collect_data( void );

int main( int argc, char **argv ) {
  pid_t who;
  
  oui_init_options( argc, argv );
  init_cache();
  init_timer();
  while ( who = Receive( 0, NULL, 0 ) ) {
	if ( who == timer_proxy ) {
	  collect_data();
	  fprintf( serial_port, "%s", navdata );
	}
  }
  return 0;
}

void init_serial_port( char *path ) {
  if ( strcmp( path, "-" ) == 0 )
	serial_port = stdout;
  else {
	serial_port = fopen( path, "w" );
	if ( serial_port == 0 )
	  nl_error( 3, "Unable to open serial port at '%s'", path );
  }
}

static void init_timer( void ) {
  struct sigevent ev;
  struct itimerspec tval;
  long secs = 1;

  if ( timer <= 0 ) {
	timer_proxy = qnx_proxy_attach( 0, NULL, 0, -1 );
	if ( timer_proxy == -1 )
	  nl_error( 3, "Error getting proxy" );
	ev.sigev_signo = -timer_proxy;
	timer = timer_create( CLOCK_REALTIME, &ev );
	if ( timer == -1 )
	  nl_error( 3, "Error in timer_create" );
  }
  tval.it_value.tv_sec = tval.it_interval.tv_sec = secs;
  tval.it_value.tv_nsec = tval.it_interval.tv_nsec = 0;
  if ( timer_settime( timer, TIMER_ADDREL, &tval, NULL ) == -1 )
	nl_error( 3, "Error in timer_settime" );
}

static void collect_data( void ) {
  int i;
  
  /* Update Time */
  { time_t now;
	struct tm *stm;
	char daybuf[14];
	
    now = time(NULL);
	stm = gmtime(&now);
	if ( snprintf( daybuf, 14, "%03d:%02d:%02d:%02d",
			stm->tm_yday+1, stm->tm_hour, stm->tm_min,
			stm->tm_sec ) != 12 )
	  nl_error( 4, "snprintf for date screwed up" );
	strncpy( &navdata[NAV_TIME], daybuf, 12 );
  }
  for ( i = 0; i < N_VARS; i++ ) {
	char *string;
	long value = cache_lread( vars[i].addr );
	string = nav_ascii( value, vars[i].width, vars[i].precision,
				vars[i].pos, vars[i].neg );
	strncpy( navdata + vars[i].offset, string, vars[i].width );
  }
}

static void init_cache( void ) {
  int i;
  for ( i = 0; i < N_VARS; i++ ) {
	long value;
	value = ascii_nav( navdata + vars[i].offset );
	cache_lwrite( vars[i].addr, value );
  }
}
