#define SCDC "scdc"

/*
	scdc receives msgs of maximum size MAX_SCDC_SZ of the following form:
	DASCMD DCT_SCDC strobe_command
	SC_MULTCMD #cmds strobe_command ...
*/
