#include <malloc.h>
#include "qnx_ipc.h"
#include "port_types.h"

int qnx_rec(int pid, void far *msg, unsigned nbytes) {
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC:
    return(Receive(pid, msg, nbytes));
    break;
#endif
  case FIFO_QNX_IPC:
    {
      struct _mxfer_entry m;
      setmx(&m,msg,nbytes);
      return(qnx_recmx(pid,1,&m));
    }
  }
}
