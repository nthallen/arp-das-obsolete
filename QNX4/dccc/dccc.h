#define DCCC "dccc"

/*	dccc receives msgs of maximum size MAX_DCCC_MSG_SZ.
	they can be of the following form:

		DASCMD DCT_QUIT DCV_QUIT
		DASCMD DCT_DCCC STROBE/STEP
		MULTCMD STROBE ... 0xFF
		MULTCMD STEP ... 0xFF		
		MULTCMD SET value ... 0xFF
*/	
