/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		getpf -- get program file name
*
* synopsis	error = getpf(file,prog);
*		int error;	non-zero if error
*		char *file;	actual file name
*		char *prog;	program name
*
* description	This function gets the file name that corresponds to the
*		specified program name.  This is done by first looking for
*		"prog.COM" and then looking for "prog.EXE".  However, if
*		the program name has an extension, that is the only form
*		used. ^^^^^ for qnx 4, just tries and opens "prog" itself ^^^^^
*
**/
/* $Revision$ $Date$ */

#include <string.h>

getpf(file,prog)
char *file;
const char *prog;
{
strcpy(file,prog);
return(0);
}
