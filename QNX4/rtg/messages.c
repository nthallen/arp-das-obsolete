/* messages.c */
#include <sys/kernel.h>
#include <stddef.h>
#include "nortlib.h"
#include "rtg.h"
#include "rtgapi.h"
#include "messages.h"

#define SEQ_UNIT 100
static
int cdb_sequence(int channel_id, cdb_data_t X, cdb_data_t dX, 
          short int n_pts, pid_t pid, unsigned short offset ) {
  float Y[SEQ_UNIT];
  int i = 0, j = SEQ_UNIT;
  cdb_resize( channel_id, n_pts );
  while (i++ < n_pts) {
    if ( j >= SEQ_UNIT ) {
      Readmsg( pid, offset, &Y[0], sizeof(Y) );
      offset += sizeof(Y);
      j = 0;
    }
    cdb_new_point(channel_id, X, Y[j++] );
    X += dX;
  }
  return 1;
}

static int cdb_msg(pid_t pid, rtg_msg_t *msg) {
  switch (msg->module[1]) {
    case RTG_CDB_CREATE: /* Create a realtime channel */
      return cdb_channel_create(msg->u.name);
    case RTG_CDB_REPORT: /* Report a data point */
      return cdb_new_point(msg->u.pt.channel_id, msg->u.pt.X, 
                              msg->u.pt.dXorY);
    case RTG_CDB_SEQUENCE: /* Report a sequence of data points */
      return cdb_sequence(msg->u.pt.channel_id,
                msg->u.pt.X, msg->u.pt.dXorY, 
                msg->u.pt.n_pts, pid, offsetof(rtg_msg_t, u.pt.Y) );
    case RTG_CDB_ENABLE: /* Enable Updates */
      plotting_enable( 1 );
      return 1;
    case RTG_CDB_DISABLE: /* Disable Updates */
      plotting_enable( 0 );
      return 1;
    default: return -1;
  }
}

void rt_msg(pid_t pid, rtg_msg_t *msg) {
  short rv;

  if (msg->ver != RTG_VERSION) {
    rv = -1;
  } else switch (msg->module[0]) {
    case RTG_MOD_CDB: /* cdb module */
      rv = cdb_msg(pid, msg);
      break;
    default:
      rv = -1;
  }
  Reply(pid, &rv, sizeof(rv));
}
