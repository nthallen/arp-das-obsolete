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
#include <errno.h>
#include "nortlib.h"
#include "oui.h"
#include "getcon.h"
#include "company.h"

static char *dev_name;
typedef struct {
  char *varname;
  char *devname;
  int fd;
} condef;
static condef *cons;
static int n_cons, open_cons = 0;

#define RBUFSIZ 80
void con_server( pid_t ppid ) {
  int nmid;
  pid_t who;
  
  fclose( stdin ); fclose( stdout ); fclose( stderr );
  nmid = qnx_name_attach( getnid(), getcon_server_name(ppid) );
  if (nmid == -1) nl_error( 3, "Unable to attach name" );
  for (;;) {
	char buf[RBUFSIZ];
	who = Receive( 0, buf, sizeof(buf) );
	if ( who > 0 ) {
	  int i;
	  Reply( who, NULL, 0 );
	  buf[RBUFSIZ-1] = '\0';
	  if ( buf[0] == 'Q' ) return;
	  for ( i = 0; i < n_cons; i++ ) {
		if ( strcmp( cons[i].devname, buf ) == 0 &&
			 cons[i].fd >= 0 ) {
		  close(cons[i].fd);
		  cons[i].fd = -1;
		  if ( --open_cons <= 0 ) return;
		}
	  }
	} else if ( errno != EINTR ) {
	  nl_error( 4, "Unexpected Receive error: %d", errno );
	}
  }
}

int con_fork( pid_t ppid ) {
  pid_t pid;
  
  pid = fork();
  switch ( pid ) {
	case 0:  /* We're the child */
	  con_server( ppid );
	  return 0;
	case -1: /* error */
	  nl_error( 4, "Forking" );
	default: /* We're the parent */
	  return 1;
  }
}

/* Acquires a console and saves the definition in the
   cons array. Returns non-zero on success.
 */
int get_console( int ix, char *name ) {
  int fd;

  fd = open( dev_name, O_RDONLY );
  if ( fd == -1 ) {
	nl_error( 2, "Unable to open console for %s", name );
	return 0;
  } else {
	struct _console_ctrl *cc;
	  
	cc = console_open( fd, O_RDWR );
	if ( cc == 0 ) nl_error( 2, "%s is not a console", dev_name );
	else {
	  int console;
	  char buf[80];
	  
	  console = cc->console;
	  console_close( cc );
	  sprintf( buf, "%s%d", dev_name, console );
	  cons[ix].varname = name;
	  cons[ix].devname = nl_strdup( buf );
	  cons[ix].fd = fd;
	  open_cons++;
	  return 1;
	}
  }
  return 0;
}

void main( int argc, char **argv ) {
  oui_init_options( argc, argv );
  if ( optind+1 >= argc )
	nl_error( 3, "Invalid arguments!" );
  else {
	int i;
	dev_name = argv[ optind++ ];
	n_cons = argc - optind;
	cons = new_memory( n_cons * sizeof(condef));
	for ( i = optind; i < argc; i++ ) {
	  if ( ! get_console( i-optind, argv[i] ) )
		exit(1);
	}
	if ( con_fork(getppid()) ) {
	  /* We're the parent: Output the consoles */
	  for ( i = 0; i < n_cons; i++ ) {
		printf( "%s=%s;", cons[i].varname, cons[i].devname );
		close( cons[i].fd );
	  }
	  printf( "\n" );
	}
  }
  exit( 0 );
}

void getcon_init_options(int argc, char **argv) {
  int optltr;
  int released = 0;

  optind = 0;
  opterr = 0;
  while ((optltr = getopt(argc, argv, opt_string)) != -1) {
	switch (optltr) {
	  case 'q':
		if ( ! getcon_release( "Q" ) )
		  nl_error( 3, "Unable to locate resident getcon" );
		exit(0);
	  case 'r':
		if ( ! getcon_release( optarg ) )
		  nl_error( 3, "Unable to locate resident getcon" );
		released = 1;
		break;
	  case '?':
		nl_error(3, "Unrecognized Option -%c", optopt);
	  default:
		break;
	}
  }
  if ( released ) exit(0);
}
