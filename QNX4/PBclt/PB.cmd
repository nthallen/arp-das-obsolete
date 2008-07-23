%{
  #include <string.h>
  #include "globmsg.h"
  #include "nortlib.h"
  #include "cltsrvr.h"
  
  void PBSearch( char *hdr, char *str ) {
	struct _mxfer_entry sx[2];
	_setmx( &sx[0], hdr, strlen(hdr) );
	_setmx( &sx[1], str, strlen(str)+1 );
	CltSendmx( &PBdef, 2, 0, sx, NULL );
  }
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
	: Advance to MFCtr %s * { PBSearch( "pbSM", $4 ); }
	: Advance to Time %s * { PBSearch( "pbST", $4 ); }
	;
&&PBcmd < char * >
	: Go Slower { $0 = "pb/2"; }
	: Go Faster { $0 = "pb*2"; }
	: Fast Forward { $0 = "pbFF"; }
	: Play { $0 = "pbRT"; }
	: Stop { $0 = "pbPS"; }
	: Row Step { $0 = "pb1R"; }
	;
&&local
	: IOMODE %d (Backspace=1 Space=2 Always=4 Word=8 WordSkip=16) *
		{ iomode = $2; }
	;
