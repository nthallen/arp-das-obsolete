/* idx64.c formats messages to be sent to the idx64
*/
#include <errno.h>
#include "nortlib.h"
#include "idx64.h"
#include "cltsrvr.h"
char rcsid_idx64_c[] =
  "$Header$";

static Server_Def Idx64Def = { IDX64_NAME, 1, 0, 1, 0, 0, 0, 0 };

/* Returns 0 on success, -1 otherwise with errno set to error
   value.
*/
int idx64_cmd(byte_t cmd, byte_t drive, step_t steps, step_t dsteps) {
  idx64_msg imsg;
  idx64_reply rep;
  
  imsg.type = IDX64_MSG_TYPE;
  imsg.ix.dir_scan = cmd;
  imsg.ix.drive = drive;
  imsg.ix.steps = steps;
  imsg.ix.dsteps = dsteps;
  if ( CltSend( &Idx64Def, &imsg, &rep,
		sizeof(imsg), sizeof(rep) ) == 0 ) {
	if ( rep.status == EOK ) return 0;
	else {
	  errno = rep.status;
	  return -1;
	}
  } else return -1;
}
