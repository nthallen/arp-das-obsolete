#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/fsys.h>

void main( int argc, char **argv ) {
  int i;
  
  for ( i = 1; i < argc; i++ ) {
	char dev[_POSIX_NAME_MAX];
	char mtpt[_POSIX_PATH_MAX];
	if ( fsys_get_mount_dev( argv[i], dev ) == 0 &&
		 fsys_get_mount_pt( dev, mtpt ) == 0 ) {
	  printf( "%s\n", mtpt );
	} else exit(1);
  }
  exit(0);
}
