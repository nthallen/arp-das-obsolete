%{
  #include <string.h>
  #include "nortlib.h"
  #include "bodma.h"

  Seq36_Req req = { SEQ36, 1, 1, 0, 0, '\000' };

%}

&command
	: Bomem &bomem_cmd
	;
&bomem_cmd
	: IR *  {
		req.hdr = SEQ36;
		req.david_pad[7] = '\000';
		send_CC(&req, sizeof(Seq36_Req),0); 
		}
	: Coadd %d (Enter No. of Scans to Coadd) * { 
		req.scans = $2;
		}
	: Runs %d (Enter No. of Consecutive Runs) * { 
		req.runs = $2;
		}
	: ZPD On * { req.zpd = 1; }
	: ZPD Off * { req.zpd = 0; }
	: Code %d (Enter Code for Bomem Log Files) * { req.david_code = $2; }
	: String %s (Enter String for Bomem Log Files) * {
		strncpy(req.david_pad,$2,7);
		}
	;

