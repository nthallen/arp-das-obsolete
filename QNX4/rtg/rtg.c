/* rtg.c The top level!
 * $Log$
 * Revision 1.2  1994/12/13  16:09:54  nort
 * Realtime!
 *
 * Revision 1.1  1994/10/31  18:49:11  nort
 * Initial revision
 *
 */
#include <stdlib.h>
#include <windows/Qwindows.h>
#include <sys/name.h>
#include <stdarg.h>
#include <assert.h>
#include "nortlib.h"
#include "rtg.h"
#include "rtgapi.h"

#pragma off (unreferenced)
  static char
	rcsid[] = "$Id$";
#pragma on (unreferenced)

QW_WNDPROP wnd;

static int win_err(int level, char *s, ...) {
  char buf[256], *lvlmsg;
  va_list arg;

  va_start(arg, s);
  vsprintf(buf, s, arg);
  va_end(arg);
  switch (level) {
	case -1: lvlmsg = "RTG"; break;
	case 0: lvlmsg = "RTG"; break;
	case 1: lvlmsg = "Warning: "; break;
	case 2: lvlmsg = "Error: "; break;
	case 3: lvlmsg = "Fatal: "; break;
	default:
	  if (level <= -2) lvlmsg = "Debug: ";
	  else lvlmsg = "Internal: ";
	  break;
  }
  Tell(lvlmsg, buf);
  if (level > 2 || level == -1) exit(level > 0 ? level : 0);
  return(level);
}

int (*nl_error)(int level, char *s, ...) = win_err;

#ifndef NDEBUG
void __assert(int chk, char *txt, char *file, int line) {
  if (!chk)
	nl_error(4, "%s:%d - Assert Failed:\n %s", file, line, txt);
}
#endif

void main(void) {
  /* Initialize communication with qwindows */
  if (!GraphicsOpen(getenv("WINSERVER"))) exit(1);
  SetName("RTG", NULL);
  if (qnx_name_attach(0, RTG_NAME) == -1)
	nl_error(1, "Unable to attach name: another RTG already running");

  /* Initialize any objects which might require it */
  /* Create the first base window (if necessary) */
  New_Base_Window();

  /* Enter Receive Loop */
  Receive_Loop();

  /* Terminate any objects which require it */
  /* Terminate communication with qwindows */
  exit(0);
}
