#ifdef __QNX__
#include <sys/kernel.h>
#endif
#include <malloc.h>
#include "qnx_ipc.h"

int rec(int pid, void __far *msg, unsigned nbytes) {
  BYTE4 fromtid;
  switch (ipc) {
#ifdef __QNX__
  case MSG_QNX_IPC:
    return(Receive(pid, msg, nbytes));
    break;
#endif
  default:
  case FIFO_QNX_IPC:
    /* read fromtid */
    if (fd_readmx(fifo_in,&fromtid,sizeof(BYTE4),msg,bytes)==-1) return(-1);
  }
  /* get fromtid */
  return(fromtid);
}
