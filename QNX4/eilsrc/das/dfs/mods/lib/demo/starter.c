#include <stdlib.h>
#include <process.h>
#include <systids.h>
#include <magic.h>
#include <stdio.h>
#include <dring.h>
#include <globmsg.h>
#include <cmdctrl.h>

main() {
dasc_msg_type msg = {DASCMD,0,0};
unsigned char replycode;
int cmd_tid;

	if ( (cmd_tid = name_locate(CMD_CTRL, My_nid, 1000)) == 0) exit(-1);
	
	/* send a TM START to reader through cmdctrl */
	msg.command_type = DCT_TM;
	msg.command_value = DCV_TM_START;
	if (send(cmd_tid, &msg, &replycode, sizeof(msg),1)<1)
		printf("starter: Cant send to cmdctrl\n");
	if (replycode != DAS_OK)
		printf("Bad response from datagen %d\n",replycode);
	
	/* send a TM RESLOG to logger through reader through cmdctrl */
	msg.command_type = DCT_TM;
	msg.command_value = DCV_TM_RESLOG;
	if (send(cmd_tid, &msg, &replycode, sizeof(msg), 1) <1)
		printf("starter: Cant send to cmdctrl again\n");
	if (replycode != DAS_OK)
		printf("Bad response from datagen again %u\n",replycode);		
}
