/* rtg.c The top level!
 * $Log$
 * Revision 1.1  1994/10/31  18:49:11  nort
 * Initial revision
 *
 */
#include <stdlib.h>
#include <windows/Qwindows.h>
#include <sys/name.h>
#include "nortlib.h"
#include "rtg.h"
#include "rtgapi.h"

#pragma off (unreferenced)
  static char
	rcsid[] = "$Id$";
#pragma on (unreferenced)

QW_WNDPROP wnd;

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
