%{
  #include "globmsg.h"
  #include "nortlib.h"
%}
&start
	: &commands Quit * {
		cis_terminate();
		send_dascmd(DCT_QUIT, DCV_QUIT, 1);
	  }
	: &commands &&Exit
	;
&&Exit
	: Exit * { cgc_forwarding = 0; cgc_exit_code = 1; }
	;
&commands
	:
	: &commands &command
	;
&command
	: *
	: Log %s ( Enter String to Log to Memo ) * {}
	: Telemetry &tm_cmd
	: &&local
	;
&tm_cmd
	: Start * { send_dascmd(DCT_TM, DCV_TM_START, 1); }
	: Logging Suspend * { send_dascmd(DCT_TM, DCV_TM_SUSLOG, 1); }
	: Logging Resume * { send_dascmd(DCT_TM, DCV_TM_RESLOG, 1); }
	;
&&local
	: IOMODE %d (Backspace=1 Space=2 Always=4 Word=8 WordSkip=16) *
		{ iomode = $2; }
	;
