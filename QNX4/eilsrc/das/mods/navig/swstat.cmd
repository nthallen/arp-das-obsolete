%{
  #ifdef SERVER
	#include "da_cache.h"
   #endif
%}
&command
	: Software &SWStatus * { cache_write(0x2000,$2); }
	;
&SWStatus <unsigned char>
	: Set to %d ( Enter value from 0-255 ) { $0 = $3; }
	: Simulate Nav Takeoff { $0 = 1; }
	: Simulate Nav High Altitude { $0 = 2; }
	: Simulate Nav Low Altitude { $0 = 3; }
	: Simulate Nav Landing { $0 = 4; }
	: Simulate Suspend { $0 = 252; }
	: Simulate from NavFile { $0 = 253; }
	: ReadFile navig.tmas { $0 = 254; }
	: Shutdown { $0 = 255; }
	;




