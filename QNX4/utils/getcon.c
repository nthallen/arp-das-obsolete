/* getcon /dev/con n
   Writes out the names of n consoles if available (or none)
   forks and exits, holding the consoles open
   getcon -q sends message to forked process to quit, releasing
   the consoles
*/
#include <unistd.h>
#include <fcntl.h>
#include <sys/console.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <string.h>
#include <stdlib.h>
#include "nortlib.h"
#include "oui.h"
#include "getcon.h"


static char *sstr = 0;
static int sstrlen = 0;
static int sstrpos = 0;

void save_string( char *dev, int num ) {
  char buf[80];
  int newlen;

  sprintf( buf, "%s%d", dev, num );
  newlen = strlen( buf );
  if ( sstrpos + newlen + 2 > sstrlen ) {
	if ( sstrlen == 0 ) sstrlen = 80;
	else sstrlen *= 2;
	sstr = realloc( sstr, sstrlen );
	if ( sstr == 0 ) nl_error( 4, "Memory allocation" );
  }
  if ( sstrpos > 0 ) sstr[ sstrpos++ ] = ' ';
  strcpy( sstr+sstrpos, buf );
  sstrpos += newlen;
  sstr[ sstrpos ] = '\0';
}

void print_string( void ) {
  if ( sstr != 0 )
	puts( sstr );
}

int Request_Quit = 0;
char *dev_name;

void con_server( void ) {
  fclose( stdin ); fclose( stdout ); fclose( stderr );
  Receive( 0, NULL, 0 );
}

int con_fork( void ) {
  pid_t pid;
  
  pid = fork();
  switch ( pid ) {
	case 0:  /* We're the child */
	  con_server();
	  return 0;
	case -1: /* error */
	  nl_error( 4, "Forking" );
	default: /* We're the parent */
	  printf( "gcpid=%d;", pid );
	  return 1;
  }
}

/* Acquires count consoles. Returns non-zero on success */
int get_console( int argc, char **argv ) {
  int fd, rv;

  if ( optind >= argc )
	return con_fork();
  fd = open( dev_name, O_RDWR );
  if ( fd == -1 ) {
	nl_error( 2, "Unable to open console for %s", argv[ optind ] );
	return 0;
  } else {
	struct _console_ctrl *cc;
	  
	cc = console_open( fd, O_RDWR );
	if ( cc == 0 ) nl_error( 2, "%s is not a console", dev_name );
	else {
	  char *var;
	  int console;
	  
	  var = argv[ optind++ ];
	  console = cc->console;
	  console_close( cc );
	  rv = get_console( argc, argv );
	  close( fd );
	  if ( rv ) printf( "%s=%s%d;", var, dev_name, console );
	  return rv;
	}
  }
  return 0;
}

void main( int argc, char **argv ) {
  oui_init_options( argc, argv );
  if ( Request_Quit ) {
	Send( Request_Quit, 0, 0, 0, 0 );
  } else if ( optind+1 >= argc )
	nl_error( 3, "Invalid arguments!" );
  else {
	dev_name = argv[ optind++ ];
	if ( get_console( argc, argv ) )
	  putchar( '\n' );
  }
  exit( 0 );
}
