#include <string.h>
#include <errno.h>
#include <unix.h>
#include <sys/stat.h>
#include "nortlib.h"
#include "runnum.h"
#include "da_cache.h"

static FILE *begin_run_log( int runnum ) {
  char fname[PATH_MAX+1];
  FILE *fp;

  if ( rundate == NULL || *rundate == '\0' ) {
	nl_error( 2, "rundate undefined: Cannot write run log" );
	return NULL;
  }
  if ( snprintf( fname, PATH_MAX, "%s/rundate", runlogdir )
		>= PATH_MAX )
	nl_error( 3, "TMLOGDIR too long in begin_run_log" );
  fp = fopen( fname, "w" );
  if ( fp == NULL ) {
	nl_error( 2, "Unable to write to '%s': %s", fname,
	  strerror(errno) );
  } else {
	fprintf( fp, "%s/%03d\n", rundate, runnum+1 );
	if ( fclose(fp) ) {
	  nl_error( 2, "Error closing rundate: %s",
		strerror(errno) );
	}
  }
  if ( snprintf( fname, PATH_MAX, "%s/runs/%03d.log",
			  runlogdir, runnum ) >= PATH_MAX )
	nl_error( 3, "TMLOGDIR too long in begin_run_log[2]" );
  fp = fopen( fname, "w" );
  if ( fp == NULL ) {
	nl_error( 2, "Unable to write to '%s': %s", fname,
	  strerror(errno) );
  }
  return fp;
}

/* Returns non-NULL if successfully opened new run log */
FILE *RunLog_write( char *runtype ) {
  FILE *fp = NULL;

  if ( srvr_runHasBegun() ) {
	nl_error( 2, "Run Begin while Run is Active" );
  } else {
	fp = begin_run_log( next_run_number );
	if ( fp != NULL ) {
	  fprintf( fp, "!Begin Run %03d\n!RunType %s\n",
					next_run_number, runtype );
	}
  }
  return fp;
}

/* Utility function to output a header line */
void RunLog_hdr( FILE *fp, char *hdr, char *val ) {
  if ( fp && val && *val ) {
	fprintf( fp, "!%s %s\n", hdr, val );
  }
}

/* Finishes writing the run log and requests that the server
   signal the start of run to TM via srvr_BeginRun()
*/
#define CD_BUFSZ 512
void RunLog_close( FILE *fp ) {
  if ( fp ) {
	FILE *ifp;
	fprintf( fp, "!Defs\n" );
	/* Now we look for configuration.def. This might be in
	   the current directory, but might also be found in
	   TMBINPATH. When I write those functions, add hook
	   here. */
	ifp = fopen( "configuration.def", "r" );
	if ( ifp != NULL ) {
	  char buf[CD_BUFSZ];
	  fprintf( fp, "-\n" );
	  while ( fgets( buf, CD_BUFSZ, ifp ) ) {
		fprintf( fp, "%s", buf );
	  }
	  fclose(ifp);
	}
	fclose(fp);
	srvr_BeginRun( next_run_number++ );
  }
}

void Run_LinkAlgo( char *link, char *src ) {
  char full[ PATH_MAX+1 ];
  struct stat buf;

  unlink( src );
  if ( *link == '\0' || stricmp( link, "none" ) == 0 )
	return;
  if ( stat( link, &buf ) ) {
	nl_error( 2, "Error identifying algo '%s': %s",
	  link, strerror(errno) );
	return;
  }
  if ( qnx_fullpath( full, link ) == NULL ) {
	nl_error( 2, "Error getting fullpath for '%s': %s",
	  link, strerror(errno) );
	return;
  }
  if ( symlink( full, src ) ) {
	nl_error( 2, "Error creating symlink '%s' -> '%s': %s",
	  src, full, strerror(errno) );
  }
}

/*
=Name RunLog_write(): Open and initialize an HPF run log
=Subject HPF Support Routines
=Subject HPF Command Server Routines
=Name RunLog_hdr(): Output a header line to an HPF run log
=Subject HPF Support Routines
=Subject HPF Command Server Routines
=Name RunLog_close(): Finish an HPF run log and begin the run
=Subject HPF Support Routines
=Subject HPF Command Server Routines
=Name Run_LinkAlgo(): Softlink an HPF run algorithm
=Subject HPF Support Routines
=Subject HPF Command Server Routines
=Synopsis
  #include "runnum.h"
  FILE *RunLog_write( char *runtype ) {
  void RunLog_hdr( FILE *fp, char *hdr, char *val ) {
  void RunLog_close( FILE *fp ) {
  void Run_LinkAlgo( char *link, char *src ) {
=Description
  RunLog_write() attempts to create a new HPF run log,
  writing out the initial "!Begin Run" and "!RunType"
  lines. It will die if rundate has not been successfully
  initialized by =RunNum_init=(). RunLog_write() calls
  =srvr_runHasBegun=() to determine whether a run is currently
  executing, and refuses to create a new run log if so.
  
  RunLog_hdr() is a simple support routine to output
  lines of the form "!<hdr> <value>" to a currently open run log.
  
  RunLog_close() outputs the "!Defs" line to the open run log
  as well as the contents of configuration.def if present,
  closes the file and calls =srvr_BeginRun=() to actually
  start the run.

=Returns
  RunLog_write() return the FILE pointer to the newly create run
  log on success or NULL on failure. None of the other routines
  return values.
=SeeAlso
  =HPF Command Server Routines=, =HPF Support Routines=.
=End

=Name srvr_runHasBegun(): HPF utility function
=Subject HPF Command Server Routines
=Name srvr_BeginRun(): HPF utility function
=Subject HPF Command Server Routines
=Synopsis
  #include "runnum.h"
  int srvr_runHasBegun( void );
  void srvr_BeginRun( unsigned short runnum );
=Description
  Each instrument's command server needs to supply these two
  functions. They are closely related to the functions defined
  in status.h except these don't take arguments. It is the
  responsibility of these functions to call the ones in status.h
  with the appropriate [instrument-specific] arguments.
=Returns
  srvr_runHasBegun() returns non-zero if a run is current
  executing. srvr_BeginRun() returns void.
=SeeAlso
  =HPF Command Server Routines=.
=End
*/
