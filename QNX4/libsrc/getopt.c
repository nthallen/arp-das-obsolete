#include <unistd.h>
  int c;

  optind = 0; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt(argc, argv, "options")) != -1) {
	switch (c) {
	  case '?':
		nl_error(3, "Unrecognized Option -%c", optopt);
	  default:
		nl_error(4, "Unsupported Option -%c", c);
	}
  }
