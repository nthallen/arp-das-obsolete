/* Run Number Handling for kinetics experiments */
#ifndef RUNNUM_H_INCLUDED
#define RUNNUM_H_INCLUDED

#include <stdio.h>

typedef struct {
  char *man, *bulb, *radical, *algo, *det1, *det2, *Flow;
} run_params;

/* *.cmd
  Each instrument's command server needs to supply these two
  functions. They are closely related to the functions defined
  in status.h except these don't take arguments. It is the
  responsibility of these functions to call the ones in status.h
  with the appropriate [instrument-specific] arguments.
*/
int srvr_runHasBegun( void );
void srvr_BeginRun( unsigned short runnum );

/* runnum.c */
extern unsigned short next_run_number;
extern char *rundate, *runlogdir;
void RunNum_init( void );

/* runwrite.c */
FILE *RunLog_write( char *runtype );
void RunLog_hdr( FILE *fp, char *hdr, char *val );
void RunLog_close( FILE *fp );
void Run_LinkAlgo( char *tgt, char *src );

/* runmol.c */
void read_molecule_list( FILE *fp, run_params *p );

/* runcat.c */
void RunLog_cat( int runnum );
void RunLog_recat( void );
#define RN_BUFSIZ 120

/* drinit.c */
void drext_init( int argc, char ** argv );
void dr_printf( char *fmt, ... );

#endif
