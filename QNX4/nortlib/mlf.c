/* config string is of the form
  \w+/(\d\d+/(\d\d/(\d\d\.dat)?)?)?
  Or perhaps more precisely:
  <base>/
  <base>/dd+/
  <base>/dd+/dd/
  <base>/dd+/dd/dd.dat

  Todo:
	Separate out a type to hold a file as an n-tuple
	mlf_ntup_t *mlfn;
	mlf_ntup_t *mlf_convert_fname( mlf_def_t *mlf, const char *fname );
	mlf_set_ntup( mlf_def_t *mlf, mlf_ntup_t *mlfn );
	mlf_compare( mlf_def_t *mlf, mlf_ntup_t *mlfn );
*/
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>
#include "mlf.h"
#include "nortlib.h"

static char *mlf_strtok( char *buf, char *delset, char *delp ) {
  static char *bufp;
  int n;
  char *rbuf;
  
  if ( buf != NULL ) bufp = buf;
  n = strcspn( bufp, delset );
  *delp = bufp[n];
  bufp[n] = '\0';
  rbuf = bufp;
  bufp += n;
  if ( *delp ) bufp++;
  return rbuf;
}

/* Parses fname into an n-tuple.
  If fname == NULL or is empty, a valid zeroed n-tuple is
  returned. The n_tuple has n_levels+1 elements. The 0-th
  points to the BASE directory.
*/
mlf_ntup_t *mlf_convert_fname( mlf_def_t *mlf, const char *fbase, const char *fname ) {
  mlf_ntup_t *mlfn;
  char *cfg;
  char *num, del;
  const char *s;
  int level;

  mlfn = new_memory( sizeof( mlf_ntup_t ) );
  mlfn->ntup = new_memory( (mlf->n_levels+1) * sizeof(int) );
  for ( level = 0; level <= mlf->n_levels; level++ )
	mlfn->ntup[level] = 0;
  mlfn->mlf = mlf;
  mlfn->base = fbase;
  mlfn->suffix = NULL;
  if ( fname == NULL || *fname == '\0' ) return mlfn;

  cfg = nl_strdup( fname );
  mlfn->base = mlf_strtok( cfg, "/", &del );
  for ( s = mlfn->base; *s; s++ )
	if ( ! isalnum(*s) && *s != '_' )
	  nl_error( 3,
		"mlf_convert_fname: illegal char '%c' in base '%s'",
		*s, mlfn->base );

  for ( level = 0; level <= mlf->n_levels; level++ ) {
	num = mlf_strtok( NULL, level < mlf->n_levels ? "/" : "/.",	&del );
	if ( num != NULL && *num != '\0' ) {
	  char *end;

	  if ( del == '/' && level == mlf->n_levels )
		nl_error( 3,
		  "Too many directory levels specified", level );
	  mlfn->ntup[level] = strtoul( num, &end, 10 );
	  if ( *end != '\0' )
		nl_error( 3,
		  "mlf_convert_fname: Subdir '%s' at level %d not numeric",
		  num, level );
	}
  }
  if ( del == '.' ) {
	mlfn->suffix = mlf_strtok( NULL, "/", &del );
	for ( s = mlfn->suffix; *s; s++ )
	  if ( ! isalnum(*s) && *s != '_' )
		nl_error( 3, "mlf_convert_fname: Illegal char in suffix" );
  }

  return mlfn;
}

void mlf_free_mlfn( mlf_ntup_t *mlfn ) {
  free( mlfn->ntup );
  free( mlfn );
}

/* Set the file path from an array of ints.
   Called from mlf_set_ntup() which parses a filename and
   mlf_set_index() which parses a long int.
 */
static void mlf_set_ixs( mlf_def_t *mlf, int *ixs ) {
  mlf->index = 0;
  { int end, i;
  
	for ( i = 0; i <= mlf->n_levels; i++ ) {
	  if (  mlf->flags & MLF_INITIALIZE ||
			mlf->flvl[i].index != ixs[i] ) {
		mlf->flvl[i].index = ixs[i];
		mlf->flags |= MLF_INITIALIZE;
		if ( mlf->flags & MLF_WRITING ) {
		  struct stat buf;
		  *(mlf->flvl[i].s) = '\0';
		  if ( stat( mlf->fpath, &buf ) || ! S_ISDIR(buf.st_mode) ) {
			if ( mkdir( mlf->fpath, 0775 ) != 0 )
			  nl_error( 3, "Unable to create directory %s", mlf->fpath );
		  }
		}
		end = sprintf( mlf->flvl[i].s, "/%02d", mlf->flvl[i].index );
		if ( i < mlf->n_levels )
		  mlf->flvl[i+1].s = mlf->flvl[i].s + end;
	  }
	  mlf->index =
		mlf->index * mlf->n_files + mlf->flvl[i].index;
	}
  }
  mlf->index++;
  mlf->flags &= ~(MLF_INC_FIRST | MLF_INITIALIZE);
}

void mlf_set_index( mlf_def_t * mlf, unsigned long index ) {
  int ixs[MLF_MAX_LEVELS+1], i, nf = mlf->n_files;
  unsigned long ix = index > 0 ? index - 1 : 0;
  for ( i = mlf->n_levels; i > 0; i-- ) {
	ixs[i] = ix % nf;
	ix = ix/nf;
  }
  ixs[0] = ix;
  mlf_set_ixs( mlf, ixs );
}
/*
=Name mlf_set_index(): Set next file by index
=Subject Multi-level File Routines
=Synopsis

  #include "mlf.h"
  void mlf_set_index( mlf_def_t * mlf, unsigned long index );

=Description
  
  mlf_set_index() defines the next file to be read or written by
  index. Given the definitions in the mlf_def_t structure, there
  is a natural one-to-one mapping between the non-negative
  integers and the multi-level paths generated by these routines.
  If for some reason you know the index number of a file (because
  perhaps it was stored in telemetry in that form), you can
  retrieve the file by calling mlf_set_index() followed by
  =mlf_next_file=().
  
=Returns

  Nothing.

=SeeAlso

  =Multi-level File Routines=.

=End  
*/

/* mlf_set_ntup() copies an n-tuple into the mlf_def_t.
   If writing, makes sure the directories exist.
 */
void mlf_set_ntup( mlf_def_t *mlf, mlf_ntup_t *mlfn ) {
  if ( mlfn->mlf != mlf )
	nl_error( 4, "mlf_set_ntup: Invalid n-tuple" );
  if ( mlfn->suffix != NULL )
	mlf->fsuffix = mlfn->suffix;
  if ( mlfn->base != NULL ) {
	int end = sprintf( mlf->fpath, "%s", mlfn->base );
	mlf->flvl[0].s = mlf->fpath + end;
  }
  mlf_set_ixs( mlf, mlfn->ntup );
}

mlf_def_t *mlf_init( int n_levels, int n_files, int writing,
	const char *fbase, const char *fsuffix, const char *config ) {
  mlf_def_t *mlf;
  mlf_ntup_t *mlfn;

  if ( n_levels < 1 || n_levels > MLF_MAX_LEVELS ) {
	nl_error( 3, "mlf_init: n_levels must be >= 1 and <= %d",
		MLF_MAX_LEVELS );
	return NULL;
  }
  if ( n_files < 2 ) {
	nl_error( 3, "mlf_init: n_files must be >= 2" );
	return NULL;
  }
  n_levels--;
  mlf = new_memory( sizeof(mlf_def_t) );
  if ( mlf )
	mlf->flvl = new_memory( (n_levels+1) * sizeof(mlf_elt_t) );
  if ( mlf == NULL || mlf->flvl == NULL ) return NULL;
  mlf->n_levels = n_levels;
  mlf->n_files = n_files;
  mlf->flags = ( writing ? MLF_WRITING : 0 ) | MLF_INITIALIZE;
  { int end = sprintf( mlf->fpath, "%s", "DATA" );
	mlf->flvl[0].s = mlf->fpath+end;
  }
  mlf->fsuffix = (fsuffix == NULL) ? "log" : nl_strdup(fsuffix);
  mlfn = mlf_convert_fname( mlf, fbase, config );
  mlf_set_ntup( mlf, mlfn );
  mlf_free_mlfn( mlfn );
  return mlf;
}
/*
=Name mlf_init(): Initialize multi-level file operations
=Subject Multi-level File Routines
=Synopsis

  #include "mlf.h"
  mlf_def_t *mlf_init( int n_levels, int n_files, int writing,
	  char *fbase, char *fsuffix, char *config );

=Description
  
  The multi-level file routines are designed for efficiently
  storing a large number of sequential files. Most hierarchical
  file systems become seriously inefficient as the number of
  files in a single directory becomes large. This is due in large
  part to the fact that any search for a specific filename
  requires a linear search through the directory, and hence
  sequentially accessing each file in a directory is an order n^2
  operation.
  
  The MLF routines address this issue by using multiple directory
  levels to store sequential files. The number of levels and the
  number of entries at each level is configurable, depending on
  the total expected number of files.

  mlf_init() establishes the parameters for subsequent
  multi-level file operations. n_levels specifies how many levels
  of directories should be used. n_files specifies the number of
  files per directory. the writing argument should be non-zero
  for write operations, zero for read operations. fbase is the
  name of the first level directory. fsuffix is a suffix that is
  appended to each file. config is an optional string defining
  the first file to access. The base and suffix in the config
  string takes precedence over the fbase and fsuffix parameters.
  
  Generated file names are of the form:

  $fbase/\d\d+(/\d\d)* /\d\d\.$fsuffix
  
=Returns

  mlf_init() returns a pointer to a dynamically allocated
  structure which holds the definitions. Most errors are syntax
  errors and are fatal, although they can be made non-fatal
  by manipulating =nl_response=.

=SeeAlso

  =Multi-level File Routines=.

=End  
*/

/* returns < 0 if current file position preceeds mlfn,
   0 if equal, >0 if later
*/
int mlf_compare( mlf_def_t *mlf, mlf_ntup_t *mlfn ) {
  int i, diff = 0;
  for ( i = 0; diff == 0 && i <= mlf->n_levels; i++ )
	diff = mlf->flvl[i].index - mlfn->ntup[i];
  if ( diff == 0 && ( mlf->flags & MLF_INC_FIRST ) == 0 )
	diff = -1;
  return diff;
}

static next_file( mlf_def_t *mlf, int level ) {
  if ( mlf->flags & MLF_INC_FIRST ) {
	if ( level == mlf->n_levels ) mlf->index++;
	if ( ++mlf->flvl[level].index >= mlf->n_files && level > 0 ) {
	  mlf->flvl[level].index = 0;
	  next_file( mlf, level-1 );
	}
  } else mlf->flags |= MLF_INC_FIRST;
  if ( level < mlf->n_levels ) {
	int n;
	
	n = sprintf( mlf->flvl[level].s, "/%02d", mlf->flvl[level].index );
	mlf->flvl[level+1].s = mlf->flvl[level].s + n;
	if ( mlf->flags & MLF_WRITING && mkdir( mlf->fpath, 0775 ) != 0 )
	  nl_error( 2, "Unable to create directory %s", mlf->fpath );
  } else {
	sprintf( mlf->flvl[level].s, "/%02d.%s", mlf->flvl[level].index,
		mlf->fsuffix );
  }
}

FILE *mlf_next_file( mlf_def_t *mlf ) {
  FILE *fp;
  
  next_file( mlf, mlf->n_levels );
  fp = fopen( mlf->fpath, (mlf->flags & MLF_WRITING) ? "w" : "r" );
  if ( fp == 0 && nl_response > 0 )
    nl_error( 1, "Unable to open file '%s'", mlf->fpath );
  return fp;
}
/*
=Name mlf_next_file(): Open the next mlf file
=Subject Multi-level File Routines
=Synopsis

  #include "mlf.h"
  FILE *mlf_next_file( mlf_def_t *mlf );

=Description

  Given an mlf definition, mlf_next_file() opens the next file in
  the sequence. After calling mlf_next_file(), the path of the
  open file is held in mlf->fpath, and the index is in
  mlf->index. (See =mlf_set_index=())
  
=Returns

  A FILE pointer to the newly opened file or NULL if the file
  could not be opened.
  
=SeeAlso

  =Multi-level File Routines=.

=End  
*/


/* Returns 1 if the directory exists. If we're writing,
   then we'll try to create it if it doesn't exist.
*/   
int mlf_next_dir( mlf_def_t *mlf ) {
  struct stat buf;
  next_file( mlf, mlf->n_levels );
  if ( stat( mlf->fpath, &buf ) || ! S_ISDIR(buf.st_mode) ) {
	if ( mlf->flags & MLF_WRITING ) {
	  if ( mkdir( mlf->fpath, 0775 ) != 0 )
		nl_error( 1, "Unable to create directory %s", mlf->fpath );
	  else return 1;
	}
	return 0;
  }
  return 1;
}

