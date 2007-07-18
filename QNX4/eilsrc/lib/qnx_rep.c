#ifdef __QNX__
#include <sys/kernel.h>
#endif
#include "mx.h"
#include "qnx_ipc.h"

int qnx_rep(int pid, void far *msg, unsigned nbytes) {
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC: return(Reply(pid, msg, nbytes));
#endif
  case FIFO_QNX_IPC:
    {
      struct _mxfer_entry r;
      setmx(&r,msg,nbytes);
      return(qnx_repmx(pid,1,&r));
    }
  }
}
