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

#! /bin/sh
while getopts "f:Fx" option; do
  case $option in
	f) echo Option -f arg $OPTARG;;
	F) echo Option -F;;
	\?) echo; exit 1;;
	*) echo Unsupported option: -$option; exit 1;;
  esac
done
let sval=$OPTIND-1
shift $sval
echo Args remaining: $*
