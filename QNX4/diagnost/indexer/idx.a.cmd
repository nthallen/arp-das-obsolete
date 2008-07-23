%{
  /* idx.a.cmd
   * $Log$
   */
  #include <signal.h>
  #include <sys/kernel.h>
  #include <sys/name.h>
  #include "globmsg.h"
  #include "cmdctrl.h"
  #include "indexer.h"
  #include "cc.h"
  #include "msg.h"
  #include "subbus.h"
  #pragma off (unreferenced)
	static char idxrcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
%}
&start
	: &commands Quit * { send_dascmd(DCT_QUIT, DCV_QUIT, 1); }
	: &commands Exit *
	;
&commands
	:
	: &commands &command
	;
&command
	: *
	: Telemetry &tm_cmd
	: &&local
	;
&tm_cmd
	: Start * { send_dascmd(DCT_TM, DCV_TM_START, 1); }
	: End * { send_dascmd(DCT_TM, DCV_TM_END, 1); }
	: Clear Errors * { send_dascmd(DCT_TM, DCV_TM_CLEAR, 1); }
	: Logging Suspend * { send_dascmd(DCT_TM, DCV_TM_SUSLOG, 1); }
	: Logging Resume * { send_dascmd(DCT_TM, DCV_TM_RESLOG, 1); }
	;
&&local
	: IOMODE %d (Backspace=1 Space=2 Always=4 Word=8) * { iomode = $2; }
	;
