/* startdbr.c debugging utility for DBR apps.
 * $Log$
 * Revision 1.3  1993/09/15  19:29:21  nort
 * *** empty log message ***
 *
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/name.h>
#include <sys/kernel.h>
#include "dbr.h"
#include "das.h"
#include "globmsg.h"
#include "collect.h"
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

#ifdef __USAGE

%C	<command>

  Issues commands directly to cmdctrl or DG via low-level
  messaging. Useful for controlling an experiment or extraction
  in the absence of a command server or client.
  Available commands are:

	<command>   Client/Server Equivalent
	---------   --------------------------
	start       Telemetry Start
	end         Telemetry End
	quit        Quit
	suspend     Telemetry Logging Suspend
	resume      Telemetry Logging Resume

#endif

int main (int argc, char **argv) {
  int i;
  char rv;
  struct dgreg_rep dgreg;
  pid_t kick_proxy;
  
  for (i = 1; i < argc; i++) {
	if (stricmp(argv[i], "start") == 0)
	  rv = send_dascmd(DCT_TM, DCV_TM_START, 1);
	else if (stricmp(argv[i], "end") == 0)
	  rv = send_dascmd(DCT_TM, DCV_TM_END, 1);
	else if (stricmp(argv[i], "quit") == 0)
	  rv = send_dascmd(DCT_QUIT, DCV_QUIT, 1);
	else if (stricmp(argv[i], "suspend") == 0)
	  rv = send_dascmd(DCT_TM, DCV_TM_SUSLOG, 1);
	else if (stricmp(argv[i], "resume") == 0)
	  rv = send_dascmd(DCT_TM, DCV_TM_RESLOG, 1);	  
	else if (stricmp(argv[i], "kick") == 0) {
	  dgreg.reply_code = COL_REGULATE;
	  if (kick_proxy == -1) {
		if (send_DG(&dgreg, sizeof(dgreg)) == 0) {
		  if (dgreg.reply_code == DAS_OK
			  || dgreg.reply_code == DAS_BUSY)
			kick_proxy = dgreg.proxy;
		}
	  }
	  if (kick_proxy == -1)
		nl_error(3, "Unable to get proxy for kicking\n");
	  else Trigger(kick_proxy);
	  rv = DAS_OK;
	} else nl_error(3, "Unknown instruction: \"%s\"\n", argv[i]);
	if (rv != DAS_OK)
	  printf("Return value from %s was %d\n", argv[i], rv);
  }
  return(0);
}
