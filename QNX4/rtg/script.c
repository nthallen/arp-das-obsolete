/* script.c handles input (and output?) of scripts
 Leading whitespace can be ignored.
 General syntax is:
    key arg arg arg
 Args delimited by whitespace, leading whitespace OK
 args may be quoted strings with the usual escape sequences.
 escape sequences not supported outside quotes.
 key is two-letter code.

 --------------------------------------------------------
 Output strategy
 --------------------------------------------------------
 Keep file stuff static here. Provide functions to output
 chunks to the file. Useful chunks are "words" and maybe
 integers. Manipulate indentation also. (Indentation may
 be controlled locally if all of the driving mechanisms
 live here, or may need to be global if certain operations
 turn over more control to outside routines.)

 --------------------------------------------------------
 Input strategy
 --------------------------------------------------------
 Parse a whole line, storing elements in a global argv array.
 Any input routine may access the argv array to get additional
 arguments as necessary.

  void script_dump(void);
    This is the application-specific state dump routine called
	by script_create(). It should iterate over all objects which
	require specification in the output script.

  int script_cmd( const char *filename );
	This routine processes a single line of the input script.
	The filename is passed purely for informational messages.
	The file is already opened by script_load(). The arguments
	on this line of the script have been parsed into script_argv
	and script_argc which have the usual characteristics, but
	aren't passed as arguments (they are global!)
*/
#include <stdlib.h>
#include <string.h>
#include <windows/Qwindows.h>
#include <ctype.h>
#include <assert.h>
#include "nortlib.h"
#include "rtg.h"

static FILE *script_fp;
#define USR_PATH_LENGTH 101

static FILE *open_usrpath( const char *filename, char *mode ) {
  const char *fname;
  FILE *fp;
  
  if (filename[0] == '/') fname = filename;
  else {
	char usr_path[USR_PATH_LENGTH], *fbuf;
	
	if ( UsrPath( usr_path, USR_PATH_LENGTH ) == 0 ) {
	  nl_error(2, "Error getting UsrPath");
	  return NULL;
	}
	fbuf = new_memory(strlen(usr_path) + strlen(filename) + 1);
	sprintf( fbuf, "%s/%s", usr_path, filename );
	fname = fbuf;
  }
  fp = fopen( fname, mode );
  if (fname != filename)
	free_memory( fname );
  if (fp == 0)
	nl_error(2, "Cannot open script file %s", filename);
  return fp;
}

/* script_create() opens the script file for output and outputs to it.
   Returns non-zero on error of any sort. Errors should report themselves.
*/
int script_create( const char *filename ) {
  assert(script_fp == 0);
  script_fp = open_usrpath( filename, "w" );
  if ( script_fp == 0 )
	return 1;

  /* call the application-specific routine */
  script_dump();  
  
  /* Finally close the file */
  fclose( script_fp );
  script_fp = NULL;
  return 0;
}

/* script_word() outputs words to a script file, quoting as necessary.
   If word==NULL and there is output on the current line, the line
   will be terminated. If word == "\0", the output will be ""
*/
void script_word( const char *word ) {
  static int at_bol = 1;

  assert(script_fp != 0);
  if ( word == NULL ) {
	if ( ! at_bol ) {
	  putc('\n', script_fp);
	  at_bol = 1;
	}
	return;
  }

  /* Handle indentation and/or interword spacing */
  if ( at_bol ) {
	/* print any indentation */
  } else {
	putc(' ', script_fp);
  }
  at_bol = 0;

  /* First scan the word for characters requiring quotation
     A string needs quoting if:
	   It is empty
	   It contains any non-graphic characters (including spaces)
	   It begins with a quote
  */
  if (*word != '\0' && *word != '"') {
	const char *s;
	for (s = word; isgraph( *s ); s++);
	if (*s == '\0') {
	  fputs(word, script_fp);
	  return;
	}
  }
  
  /* If we get this far, the string needs quoting */
  putc('"', script_fp);
  for ( ; *word != '\0'; word++) {
	switch (*word) {
	  case '\"': fputs("\\\"", script_fp); break;
	  case '\\': fputs("\\\\", script_fp); break;
	  case '\a': fputs("\\a", script_fp); break;
	  case '\b': fputs("\\b", script_fp); break;
	  case '\f': fputs("\\f", script_fp); break;
	  case '\n': fputs("\\n", script_fp); break;
	  case '\r': fputs("\\r", script_fp); break;
	  case '\t': fputs("\\t", script_fp); break;
	  case '\v': fputs("\\v", script_fp); break;
	  default:
		if ( isprint(*word) )
		  putc( *word, script_fp );
		else
		  fprintf( script_fp, isdigit(word[1]) ? "\\%03o" : "\\%o", *word);
		break;
	}
  }
  putc('"', script_fp);
}

#define MIN_ALLOC 8
char **script_argv = NULL;
int script_argc;
static int argv_size = 0;
static FILE *scr_ifp;

/* read_a_word returns 1 on EOL, 2 on EOF or serious syntax errors */
static int read_a_word( const char *filename ) {
  char wordbuf[PATH_MAX+1];
  int i, c;

  if (script_argc == argv_size) {
	int i;

	argv_size = (argv_size > 0) ? argv_size * 2 : MIN_ALLOC;
	script_argv = realloc(script_argv, sizeof(char *)*argv_size);
	if (script_argv == 0)
	  nl_error(3, "Memory allocation failure in script_load");
	for (i = script_argc; i < argv_size; i++)
	  script_argv[i] = NULL;
  }

  /* swallow non-newline whitespace */
  do {
	c = getc(scr_ifp);
  } while ( c != EOF && c != '\n' && isspace( c ) );
  if ( c == '\n' ) return 1;
  if ( c == EOF ) return script_argc > 0 ? 1 : 2;
	  
  /* OK, we must be at the beginning of a word */
  if ( c == '\"' ) {
	/* swallow a quoted string */
	for (i = 0; i < PATH_MAX; ) {
	  c = getc( scr_ifp );
	  switch ( c ) {
		case '\"':
		  break;
		case '\\':
		  c = getc( scr_ifp );
		  switch ( c ) {
			case '\"': wordbuf[ i++ ] = '\"'; break;
			case '\\': wordbuf[ i++ ] = '\\'; break;
			case 'a': wordbuf[ i++ ] = '\a'; break;
			case 'b': wordbuf[ i++ ] = '\b'; break;
			case 'f': wordbuf[ i++ ] = '\f'; break;
			case 'n': wordbuf[ i++ ] = '\n'; break;
			case 'r': wordbuf[ i++ ] = '\r'; break;
			case 't': wordbuf[ i++ ] = '\t'; break;
			case 'v': wordbuf[ i++ ] = '\v'; break;
			default:
			  { char obuf[4];
				int j;
					
				for ( j = 0; j < 3 && isdigit( c ); j++) {
				  obuf[ j ] = c;
				  c = getc( scr_ifp );
				}
				if ( j > 0 ) {
				  ungetc( c, scr_ifp );
				  obuf[ j ] = '\0';
				  wordbuf[ i++ ] = strtoul( obuf, NULL, 8 );
				  break;
				} else if ( isprint( c ) ) {
				  wordbuf[ i++ ] = c;
				  break;
				} else {
				  nl_error( 2,
					"Illegal char code %d in quoted string", c );
				  return 2;
				}
			  }
		  }
		  continue;
		default: /* pick up EOF, and any non-print chars */
		  if ( isprint( c ) ) {
			wordbuf[ i++ ] = c;
			continue;
		  } else {
			nl_error( 2,
			  "Unexpected char code %d in quoted string in script %s",
			  c, filename );
			return 2;
		  }
	  }
	  break;
	}
	wordbuf[ i ] = '\0';
  } else {
	for (i = 0; i < PATH_MAX && isgraph( c ); ) {
	  /* swallow a non-quoted string */
	  wordbuf[ i++ ] = c;
	  c = getc( scr_ifp );
	}
	if ( i > 0 ) {
	  ungetc( c, scr_ifp );
	  wordbuf[ i ] = '\0';
	} else {
	  nl_error( 2, "Illegal character code %d in script file %s",
				  c, filename);
	  return 2;
	}
  }
	  
  /* The word has been swallowed into wordbuf. now duplicate it */
  script_argv[ script_argc++ ] = nl_strdup( wordbuf );
  return 0;
}

/* returns zero on EOF or serious syntax error. Errors should have
   been reported already.
*/
static int read_a_line( const char *filename ) {
  int i;

  assert( script_argc == 0 );
  do {
	i = read_a_word( filename );
  } while ( i == 0 );
  if ( i == 1 ) return 1; /* EOL */
  else return 0; /* EOF or Serious syntax problem */
}

void script_load( const char *filename ) {
  int quit_early = 0;

  assert(scr_ifp == 0);
  scr_ifp = open_usrpath( filename, "r" );
  if (scr_ifp == 0)
	return;

  while ( ( ! quit_early ) && read_a_line( filename ) ) {
	/* interpret the vector */
	quit_early = script_cmd( filename );

	/* free the vector */
	while ( script_argc > 0 ) {
	  free_memory( script_argv[ --script_argc ] );
	  script_argv[ script_argc ] = NULL;
	}
  }
  fclose( scr_ifp );
  scr_ifp = NULL;
}
