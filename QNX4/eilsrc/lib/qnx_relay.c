#ifdef __QNX__
#include <sys/kernel.h>
#endif
#include <sys/types.h>
#include "qnx_ipc.h"

int qnx_relay(pid_t source, pid_t target, char *msg, int size) {
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC: return(Relay(source, target));
#endif
  case FIFO_QNX_IPC:
    {
      int i;
      relay_pid=source;
      b_fifo_out=-1;
      i=qnx_snd(target,msg,0,size,0);
      relay_pid=0;
      b_fifo_out=0;
      return(i);
    }
  }
}
