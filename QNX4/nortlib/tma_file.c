/* This file provides the file input support for TMCALGO R2 */
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "nortlib.h"
#include "tma.h"
#include "cmdalgo.h"

static char *yy_filename;
static int yy_lineno;
static char *yy_text;
static long int yy_val;

static void synt_err( char *txt ) {
  nl_error( 2, "%s:%d %s", yy_filename, yy_lineno, txt );
}

#define TK_NUMBER 256
#define TK_TMCCMD 257
#define TK_QSTR 258
#define KW_VALIDATE 259 
#define TK_NAME 260
#define TK_EOF 261
#define TK_ERR 262

static int lexbufsize;
static int lexbufpos;

static int buffer_char( char c ) {
  if ( lexbufpos >= lexbufsize ) {
	if ( lexbufsize == 0 ) lexbufsize = 128;
	else lexbufsize *= 2;
	yy_text = realloc( yy_text, lexbufsize );
	if ( yy_text == 0 ) {
	  synt_err( "Out of memory reading tma file" );
	  return 1;
	}
  }
  yy_text[ lexbufpos++ ] = c;
  return 0;
}

#define BEGIN_BUFFER lexbufpos = 0
#define BUFFER_CHAR(c) if ( buffer_char( c ) ) return TK_ERR
#define END_BUFFER BUFFER_CHAR(0)

/* yylex() recognizes the following tokens
  [ \t]* nothing
  [\n] increment yy_lineno
  "#.*$" nothing
  TK_NUMBER [0-9]+
  TK_TMCCMD ">.*$"
  TK_QSTR "..."
  KW_VALIDATE 
  TK_NAME [A-Z_][A-Z_0-9]*
  TK_EOF  EOF
  ':'
  ';'
  '+'
*/
static int yylex( FILE *fp ) {
  int c;

  for (;;) {  
	switch ( c = getc(fp) ) {
	  case EOF:	return TK_EOF;
	  case '\n': yy_lineno++; break;
	  case ':': return ':';
	  case ';': return ';';
	  case '+': return '+';
	  case '#':
		for (;;) {
		  c = getc( fp );
		  if ( c == '\n' ) {
			ungetc( c, fp );
			break;
		  } else if ( c == EOF ) break;
		}
		break;
	  case '>':
		BEGIN_BUFFER;
		BUFFER_CHAR( c );
		for (;;) {
		  c = getc(fp);
		  if ( c != ' ' && c != '\t' ) break;
		}
		while ( c != '\n' && c != EOF ) {
		  BUFFER_CHAR( c );
		  c = getc(fp);
		}
		BUFFER_CHAR( '\n' );
		END_BUFFER;
		if ( c == '\n' ) ungetc( c, fp );
		return TK_TMCCMD;
	  case '"':
		BEGIN_BUFFER;
		BUFFER_CHAR( c );
		for (;;) {
		  c = getc(fp);
		  if ( c == '"' ) break;
		  else if ( c == '\\' ) c = getc(fp);
		  if ( c == '\n' || c == EOF ) {
			synt_err( "Runaway quote" );
			return TK_ERR;
		  }
		  BUFFER_CHAR( c );
		}
		END_BUFFER;
		return TK_QSTR;
	  default:
		if ( isspace( c ) ) break;
		else if ( isdigit(c) ) {
		  yy_val = 0;
		  do {
			yy_val = yy_val*10 + c - '0';
			c = getc(fp);
		  } while ( isdigit(c) );
		  ungetc( c, fp );
		  return TK_NUMBER;
		} else if ( isalpha(c) || c == '_' ) {
		  BEGIN_BUFFER;
		  do {
			BUFFER_CHAR(c); c = getc(fp);
		  } while( isalnum(c) || c == "_" );
		  ungetc( c, fp );
		  END_BUFFER;
		  if ( stricmp( yy_text, "validate" ) == 0 )
			return KW_VALIDATE;
		  return TK_NAME;
		} else {
		  synt_err( "syntax error" );
		  return TK_ERR;
		}
	}
  }
}

static int tma_strdup( char **ptr, const char *str ) {
  *ptr = strdup( str );
  if ( *ptr == 0 ) {
	synt_err( "Out of memory reading tma file" );
	return 1;
  } else return 0;
}

static long int last_time;
/* returns non-zero on error. dt=-1 and cmd = NULL on EOF.
   otherwise cmd is assigned a newly allocated string.
   Checks ">" command syntax, validate validity,
   time monotonicity. Supports '>', '"' and Validate
   commands plus # comments
	  timecommand : timespec command
	    : command
	  timespec : delta time
	  delta :
	    : '+'
	  time : TK_NUMBER
	    : time ':' TK_NUMBER
	  command : TK_TMCCMD
	    : TK_QSTR
		: KW_VALIDATE TK_NAME ';'
*/
static int read_a_cmd( FILE *fp, long int *dtp, char **cmd ) {
  int token, oldresp, rv;
  int delta = 0, i;
  long int dt = 0;
  
  token = yylex( fp );
  switch ( token ) {
	case '+':
	  delta = 1;
	  token = yylex( fp );
	  if ( token != TK_NUMBER ) {
		synt_err( "Expected Number after +" );
		return 1;
	  }
	case TK_NUMBER:
	  for (;;) {
		dt = dt * 60 + yy_val;
		token = yylex( fp );
		if ( token != ':' ) break;
		token = yylex( fp );
		if ( token != TK_NUMBER ) {
		  synt_err( "Expected Number after :" );
		  return 1;
		}
	  }
	  if ( delta ) last_time += dt;
	  else if ( last_time > dt ) {
		synt_err( "Specified absolute time earlier than previous time" );
		return 1;
	  } else last_time = dt;
	  break;
	case TK_EOF:
	  *cmd = NULL;
	  *dtp = -1;
	  return 0;
	default:
	  break;
  }
  *dtp = last_time;
  switch ( token ) {
	case TK_TMCCMD:
	  oldresp = set_response( 0 );
	  rv = ci_sendcmd( yy_text+1, 1 );
	  set_response( oldresp );
	  if ( rv >= CMDREP_SYNERR ) {
		synt_err( "Syntax Error reported by command server" );
		return 1;
	  }
	  return tma_strdup( cmd, yy_text );
	case TK_QSTR:
	  if ( tma_strdup( cmd, yy_text ) == 0 ) {
		if ( yylex(fp) == ';' ) return 0;
		synt_err( "Semicolon required after quoted string" );
	  }
	  return 1;
	case KW_VALIDATE:
	  token = yylex( fp );
	  if ( token != TK_NAME ) {
		synt_err( "Expected State Name after validate" );
		return 1;
	  }
	  for ( i = 0; slurp_vals[i].state != 0; i++ ) {
		if ( strcmp( yy_text, slurp_vals[i].state ) == 0 ) {
		  if ( yylex( fp ) != ';' ) {
			synt_err( "Expected ; after validate <state>" );
			return 1;
		  }
		  return tma_strdup( cmd, slurp_vals[i].cmdstr );
		}
	  }
	  synt_err( "Unknown state in validate" );
	  return 1;
	default:
	  synt_err( "Expected Command" );
	  return 1;
  }
}

static void free_tmacmds( tma_ifile *spec ) {
  tma_state *cmd;

  assert( spec->cmds != 0 );
  for ( cmd = spec->cmds; cmd->cmd != 0; cmd++ )
	free( cmd->cmd );
  free( spec->cmds );
  spec->cmds = NULL;
}

/* for read_tmafile() I need to build an array of tma_state's 
with pointers to a bunch of strings. I'll use the obvious 
approach of malloc/realloc on the tma_state array and simply use 
malloc/free on the strings. Returns 0 on success.
*/
static int read_tmafile( tma_ifile *spec, FILE *fp ) {
  int max_cmds = 32, n_cmds = 0;

  spec->cmds = malloc( max_cmds * sizeof( tma_state ) );
  if ( spec->cmds == 0 ) {
	nl_error( 2, "Out of memory reading tma file" );
	return 1;
  }
  yy_lineno = 1;
  yy_filename = spec->filename;
  last_time = 0;
  for (;;) {
	char *cmd;
	long int dt;

	if ( read_a_cmd( fp, &dt, &cmd ) ) {
	  spec->cmds[n_cmds].cmd = NULL;
	  free_tmacmds( spec );
	  return 1;
	}
	spec->cmds[n_cmds].dt = dt;
	spec->cmds[n_cmds].cmd = cmd;
	if ( ++n_cmds == max_cmds ) {
	  tma_state *ncmds;
	  max_cmds *= 2;
	  ncmds = realloc( spec->cmds, max_cmds * sizeof( tma_state ) );
	  if ( ncmds == 0 ) {
		if ( cmd != 0 ) {
		  free(cmd);
		  spec->cmds[n_cmds-1].cmd = NULL;
		}
		free_tmacmds( spec );
		nl_error( 2, "Out of memory reading tma file" );
		return 1;
	  }
	}
	if ( cmd == 0 ) break;
  }
  return 0;
}

void tma_read_file( tma_ifile *ifilespec ) {
  FILE *fp;
  tma_state *cmds;
  long int modtime = 0;
  struct stat buf;

  assert( ifilespec != 0 && ifilespec->filename != 0 );
  assert( ifilespec->statename != 0 );
  /* check modtime of the file. If newer, free cmds */
  if ( stat( ifilespec->filename, &buf ) == -1 )
	modtime = -1;
  else modtime = buf.st_mtime;
  if ( modtime != ifilespec->modtime && ifilespec->cmds != 0 )
	free_tmacmds( ifilespec );
  if ( modtime != ifilespec->modtime && ifilespec->cmds == 0 ) {
	fp = fopen( ifilespec->filename, "r" );
	if ( fp == 0 ) {
	  nl_error( 1, "Unable to open algo file %s for state %s", 
					ifilespec->filename, ifilespec->statename );
	} else {
	  nl_error( -2, "Reading algo file %s", ifilespec->filename );
	  read_tmafile( ifilespec, fp );
	  fclose( fp );
	}
	ifilespec->modtime = modtime;
  }
  cmds = ifilespec->cmds;
  if ( cmds == 0 ) cmds = ifilespec->def_cmds;
  tma_init_state( ifilespec->partno, cmds, ifilespec->statename );
}
