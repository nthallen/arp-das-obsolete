#include <stdio.h>
#include "qnx_ipc.h"

int qnx_ipc_end(void) {
  switch (ipc) {
  default:
  case MSG_QNX_IPC: return(0);
  case FIFO_QNX_IPC:
    {
      int i,j;
      if (fifo_in>-1) close(fifo_in);
      i=remove(qnx_ipc_tmp(getpid(),'\0',0));
      j=remove(qnx_ipc_tmp(getpid(),'R',0));
      if (i || j) return(-1);
      else return(0);
    }
  }
}
