#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* modtime reports verbosely the modification time of a file.
   This is used in doctex to add the modification time to
   a printed manual page. The output is simple the long
   integer which can then be formatted using date.
*/

void main(int argc, char **argv) {
  int fildes, rc;
  struct stat buf;
  
  if (argc < 2) exit(1);
  fildes = open(argv[1], O_RDONLY);
  if (fildes != -1) {
	rc = fstat(fildes, &buf);
	if (rc != -1) printf("%ld\n", buf.st_mtime);
	else exit(1);
  }
  exit(0);
}
