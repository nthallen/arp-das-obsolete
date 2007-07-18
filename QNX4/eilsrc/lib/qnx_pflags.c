#include <sys/psinfo.h>
#include "qnx_ipc.h"

int qnx_pflgs(long bits, long mask, long *old_bits, long *new_bits) {
  switch(ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC:return(qnx_pflags(bits, mask, old_bits, new_bits));
    break;
#endif
  case FIFO_QNX_IPC: return(0);
  }
}
