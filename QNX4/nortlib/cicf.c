/* cicf.c defines ci_sendfcmd() */
#include <stdarg.h>
#include "nortlib.h"
#include "cmdalgo.h"

#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

int ci_sendfcmd(int mode, char *fmt, ...) {
  va_list arg;
  char cmdbuf[CMD_INTERP_MAX];

  va_start(arg, fmt);
  vsprintf(cmdbuf, fmt, arg);
  va_end(arg);
  return(ci_sendcmd(cmdbuf, mode));
}
