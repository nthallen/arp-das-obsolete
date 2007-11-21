%{
  /* navig.cmd */
  #include "da_cache.h"
  #include "globmsg.h"
  #include "nortlib.h"
  #include "msg.h"
  #include "navigutil.h"
  #include "navvars.h"
%}
&command
	: &navcmd
	: &navfilecmd
	;
&navcmd
	: Set Pressure &nav_Pchannel to &nav_Pform * {
	cache_lwrite( vars[ $3 ].addr, ascii_navig( $5 , vars[ $3 ].precision));
	}
	: Set Lattitude &nav_LATchannel to &nav_LATform * {
	cache_lwrite( vars[ $3 ].addr, ascii_navig( $5 , vars[ $3 ].precision));
	}
	: Set Longitude &nav_LONGchannel to &nav_LONGform * {
	cache_lwrite( vars[ $3 ].addr, ascii_navig( $5 , vars[ $3 ].precision));
	}
	: Set Channel &nav_channel to &nav_form * {
	cache_lwrite( vars[ $3 ].addr, ascii_navig( $5 , vars[ $3 ].precision));
	}
	;
&nav_Pchannel <int>
	: Static { $0 = 0; }
	: Total  { $0 = 1; }
	;
&nav_LATchannel <int>
	: INU { $0 = 4; }
	: GPS { $0 = 6; }
	;
&nav_LONGchannel <int>
	: INU { $0 = 5; }
	: GPS { $0 = 7; }
	;
&nav_channel <int>
	: Static Temperature { $0 = 2; }
	: True Air Speed { $0 = 3; }
	: GPS Altitude { $0 = 8; }
	: True Heading { $0 = 9; }
	: Pitch { $0 = 10; }
	: Roll { $0 = 11; }
	: Sun Elevation { $0 = 12; }
	: Sun Azimuth { $0 = 13; }
	;
&nav_Pform <char *>
	: %s (Enter Format XXXX.XXX) { $0 = $1; }
	;
&nav_LATform <char *>
	: %s (Enter Format N/S XX.XXXXX) { $0 = $1; }
	;
&nav_LONGform <char *>
	: %s (Enter Format E/W XXX.XXXXX) { $0 = $1; }
	;
&nav_form <char *>
	: %s (Enter Nav Data) { $0 = $1; }
	;

