#include "nortlib.h"
#include "runnum.h"
#include <ctype.h>
#include <string.h>


/* read_molecule_list( fp, rp )
   Reads the seven fields of molecule.list, which are
   manifold bulb size molecule balance balance-chi tracer tracer-chi
   and compares them to the parameters specified in *rp.
   If the manifold matches rp->man and rp->bulb matches
   either bulb or molecule, we have a match
*/

#define LBUFSZ 120
#define MAX_FIELDS 8
void read_molecule_list( FILE *fp, run_params *rp ) {
  FILE *mfp = fopen( "molecule.list", "r" );
  char lbuf[LBUFSZ];
  char *fields[ MAX_FIELDS ];
  int line_number = 0;

  if ( mfp == NULL ) {
	nl_error( 2, "Unable to read molecule.list" );
	return;
  }
  while ( fgets( lbuf, LBUFSZ, mfp ) != NULL ) {
	int fldno = 0;
	char *p = lbuf;
	line_number++;
	while ( fldno < MAX_FIELDS ) {
	  char c;
	  while ( isspace(*p) ) p++;
	  if ( *p == '\0' || *p == '#' ) break;
	  fields[fldno++] = p;
	  while ( *p && ( ! isspace(*p) ) && *p != '#' ) p++;
	  c = *p;
	  *p = '\0';
	  if ( c == '\0' || c == '#' ) break;
	  p++;
	}
	/* for ( i = 0; i < fldno; i++ )
	  nl_error( 0, "mlist:%d field %d = '%s'",
		  line_number, i, fields[i] ); */
	if ( fldno == 0 ) continue;
	else if ( fldno < 6 || fldno == 7 )
	  nl_error( 1, "molecule.list:%d Incomplete line, ignored",
				  line_number );
	if ( ! stricmp( fields[0], rp->man ) &&
		 ( ! stricmp( fields[1], rp->bulb ) ||
		   ! stricmp( fields[3], rp->bulb ) ) ) {
	  /* We have a match! */
	  fclose( mfp );
	  fprintf( fp,
		"!Manifold %s\n"
		"!Volume %s\n"
		"!Molecule %s\n"
		"!Balance %s\n"
		"!Balance-Chi %s\n",
		fields[0], fields[2], fields[3], fields[4], fields[5] );
	  if ( fldno == 8 )
		fprintf( fp,
		  "!Tracer %s\n"
		  "!Tracer-Chi %s\n",
		  fields[6], fields[7] );
	  return;
	}
  }
  fclose( mfp );
  nl_error( 2, "molecule.list: No match for '%s', '%s'",
    rp->man, rp->bulb );
}

/*
=Name read_molecule_list(): Reads molecule.list file
=Subject HPF Support Routines
=Synopsis
  #include "runnum.h"
  void read_molecule_list( FILE *fp, run_params *rp );
=Description
  read_molecule_list() is called by an HPF command server
  to extract useful information about the current run from
  the file "molecule.list". The invocation is found in
  /usr/local/lib/src/hpfrun.cmd.
  
  If syntax errors are found or if no line matches the
  specified run_params, errors or warnings will be issued,
  but they will not interfere with the further processing of the
  run.
  
=Returns
  Nothing.
=SeeAlso
  =HPF Support Routines=.
=End
*/
