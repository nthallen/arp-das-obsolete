#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <unix.h>
#include "nortlib.h"
#include "runnum.h"

unsigned short next_run_number = 0;
char *rundate, *runlogdir;

void RunNum_init( void ) {
  char buf[RN_BUFSIZ];
  char fname[PATH_MAX+1];
  FILE *fp;
  
  runlogdir = getenv( "TMLOGDIR" );
  if ( runlogdir == NULL ) runlogdir = ".";
  if ( snprintf( fname, PATH_MAX, "%s/rundate", runlogdir )
		>= PATH_MAX )
	nl_error( 3, "TMLOGDIR too long!" );
  fp = fopen( fname, "r" );
  if ( fp == NULL )
	nl_error( 2, "Unable to open '%s'", fname );
  else {
	char *p, *w = NULL;
	int nb = fread( buf, 1, RN_BUFSIZ-1, fp );
	if ( nb >= RN_BUFSIZ-1 ) {
	  nl_error( 2, "rundate file too long" );
	  return;
	}
	buf[nb] = '\0';
	for ( p = buf; *p != '\0' && *p != '\n'; ) {
	  if ( *p != '/' ) {
		nl_error( 2,
		  "rundate syntax: '%s', Expected '/', but saw '%c'",
		  buf, *p );
		return;
	  }
	  w = p++; /* Points to the last '/' */
	  if ( ! isalnum(*p) ) {
		nl_error( 2,
		  "rundate syntax: Expected alnum after '/' but saw '%c'",
		  *p );
		return;
	  }
	  while ( isalnum(*p) || *p == '.' ) p++;
	}
	if ( w == NULL ) {
	  nl_error( 2,
		"rundate was apparently empty" );
	  return;
	}
	*w++ = '\0';
	rundate = nl_strdup( buf );
	while (isdigit(*w)) {
	  next_run_number = next_run_number * 10 + *w++ - '0';
	}
	if ( *w != '\n' && *w != '\0' ) {
	  nl_error( 2,
		"rundate syntax: Invalid char '%c' in run number",
		*w );
	}
  }
}

/*
=Name rundate: String defining dataRecv's date path
=Subject HPF Support Routines
=Name next_run_number: HPF Run Number
=Subject HPF Support Routines
=Name runlogdir: Location of Log Files
=Subject HPF Support Routines
=Name RunNum_init(): Initialize rundate, runlogdir, next_run_number
=Subject HPF Support Routines
=Synopsis
  unsigned short next_run_number;
  char *rundate, *runlogdir;
  void RunNum_init( void );
=Description
  RunNum_init() performs a number of initializations required by
  both HPF command servers and datarecvext programs.
  
  It initializes runlogdir by referencing the TMLOGDIR
  environment variable. It then opens the rundate file
  and extracts the rundate and the next_run_number
  values.
  
  Specification of a TMLOGDIR value that causes the fullpath
  to exceed PATH_MAX is a fatal error. All other errors are
  reported but are not fatal and may result in rundate
  being left as NULL. =drext_init=() checks for this
  condition and will subsequently issue a fatal error, so
  =HPF datarecvext= programs can assume rundate is defined.
  =RunLog_write=() will refuse to write a new run log if
  rundate has not been initialized, which will effectively
  prevent runs from starting.

=Returns
  Nothing.
=SeeAlso
  =HPF datarecvext= functions, =HPF Support Routines=.
=End
*/
