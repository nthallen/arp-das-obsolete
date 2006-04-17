#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* #include <fcntl.h> */
#include <time.h>
/* #include <sys/types.h> */
#include <sys/stat.h>

/* modtime reports verbosely the modification time of a file.
   This is used in doctex to add the modification time to
   a printed manual page. The output is simple the long
   integer which can then be formatted using date.
*/

void main(int argc, char **argv) {
  int rc, i, maxl = 0;
  struct stat buf;
  
  for ( i = 1; i < argc; i++ ) {
	int len = strlen( argv[i] );
	if ( len > maxl ) maxl = len;
  }
  for ( i = 1; i < argc; i++ ) {
	rc = stat( argv[i], &buf );
	if (rc != -1) printf( "%-*s %s", maxl, argv[i],
							ctime(&buf.st_ftime) );
	else printf( "%-*s Not found\n", maxl, argv[i] );
  }
  exit(0);
}
