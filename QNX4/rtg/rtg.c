/* rtg.c The top level!
 * $Log$
 */
#include <stdlib.h>
#include <windows/Qwindows.h>
#include "rtg.h"

#pragma off (unreferenced)
  static char
	rcsid[] = "$Id$";
#pragma on (unreferenced)

QW_WNDPROP wnd;

void main(void) {
  /* Initialize communication with qwindows */
  if (!GraphicsOpen(NULL)) exit(1);
  SetName("RTG", NULL);
  
  /* Initialize any objects which might require it */
  /* Create the first base window (if necessary) */
  New_Base_Window();

  /* Enter Receive Loop */
  Receive_Loop();

  /* Terminate any objects which require it */
  /* Terminate communication with qwindows */
  exit(0);
}
