%{
  #include <string.h>
  #include "globmsg.h"
  #include "nortlib.h"
  #include "cltsrvr.h"
%}
&start
	: &commands &&Quit
	;
&&Quit
	: Quit * {
		unsigned char rv;
		CltSend( &PBdef, "pbFF", &rv, 5, 1 );
		send_dascmd(DCT_QUIT, DCV_QUIT, 1);
	  }
	;
&commands
	:
	: &commands &command
	;
&command
	: *
	: Log %s ( Enter String to Log to Memo ) * {}
	: Telemetry &&tm_cmd
	: &&PBcmds
	: &&local
	;
&&tm_cmd
	: Start * { send_dascmd(DCT_TM, DCV_TM_START, 1); }
	: End * { send_dascmd(DCT_TM, DCV_TM_END, 1); }
	;
&&PBcmds
	: &&PBcmd * {
		unsigned char rv;
		CltSend( &PBdef, $1, &rv, strlen($1)+1, 1 );
	  }
	;
&&PBcmd < char * >
	: Fast Forward { $0 = "pbFF"; }
	: Play { $0 = "pbRT"; }
%{ /* : Realtime { $0 = "pbRT"; }
	: Readable { $0 = "pbRD"; } */
%}
	: Slow Motion { $0 = "pbSL"; }
	: Stop { $0 = "pbPS"; }
	;
&&local
	: IOMODE %d (Backspace=1 Space=2 Always=4 Word=8 WordSkip=16) *
		{ iomode = $2; }
	;
