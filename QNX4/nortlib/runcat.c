#include "nortlib.h"
#include "runnum.h"
#include <limits.h>
#include <unix.h>

#define CBUFSZ 512
/* Read from the indexed run log file up to EOF or
   a line beginning with '-'. That line will be ignored.
   The lines after that will be output after the extraction
   header gets written to allow for sensible column aliasing.
*/
static FILE *fp;

void RunLog_cat( int runnum ) {
  char fname[PATH_MAX+1];

  if ( fp != NULL ) {
	nl_error( 1, "RunLog_cat fp still open." );
	fclose(fp);
	fp = NULL;
  }  
  if ( snprintf( fname, PATH_MAX, "%s/runs/%03d.log",
				runlogdir, runnum ) >= PATH_MAX )
	nl_error( 3, "TMLOGDIR too long in RunLog_cat" );
  fp = fopen( fname, "r" );
  if ( fp == NULL ) {
	nl_error( 2, "Unable to open run log '%s'", fname );
  } else {
	RunLog_recat();
  }
}

void RunLog_recat( void ) {
  char buf[CBUFSZ];
  int atbol = 1;
  
  if ( fp == NULL ) return;
  while ( fgets( buf, CBUFSZ, fp ) ) {
	int n;

	if ( atbol && buf[0] == '-' ) return;
	dr_printf( "%s", buf );
	n = strlen(buf);
	atbol = ( buf[n-1] == '\n' );
  }
  fclose( fp );
  fp = NULL;
}

/*
=Name RunLog_cat(): Copy Beginning of HPF Run Log to Output
=Subject HPF Support Routines
=Subject HPF datarecvext
=Name RunLog_recat(): Copy More of HPF Run Log to Output
=Subject HPF Support Routines
=Subject HPF datarecvext

=Synopsis
  #include "runnum.h"
  void RunLog_cat( int runnum );
  void RunLog_recat( void );
=Description
  RunLog_cat() opens the run log for the specified HPF run
  number, then calls RunLog_recat() to send some or all of
  the contents to the output. The run log is found in the
  "runs" subdirectory of the current directory or the
  directory specified by the TMLOGDIR environment variable,
  which is set by the extract script.
  
  RunLog_recat() requires that RunLog_cat() be called first
  to open a specific run log. It will then read lines from the
  log, outputting each via =dr_printf=() until it reaches EOF or
  a line starting with '-'.
  
=Returns
  Nothing.
=SeeAlso
  =HPF datarecvext= functions, =HPF Support Routines=.
=End
*/
