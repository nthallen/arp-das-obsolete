/* messages.c */
#include <sys/kernel.h>
#include "nortlib.h"
#include "rtg.h"
#include "rtgapi.h"
#include "messages.h"

static int cdb_msg(rtg_msg_t *msg) {
  switch (msg->module[1]) {
	case RTG_CDB_CREATE: /* Create a realtime channel */
	  return cdb_channel_create(msg->u.name);
	case RTG_CDB_REPORT: /* Report a data point */
	  return cdb_new_point(msg->u.pt.channel_id, msg->u.pt.X, msg->u.pt.Y);
	default: return -1;
  }
}

void rt_msg(pid_t pid, rtg_msg_t *msg) {
  short rv;

  if (msg->ver != RTG_VERSION) {
	rv = -1;
  } else switch (msg->module[0]) {
	case RTG_MOD_CDB: /* cdb module */
	  rv = cdb_msg(msg);
	  break;
	default:
	  rv = -1;
  }
  Reply(pid, &rv, sizeof(rv));
}
