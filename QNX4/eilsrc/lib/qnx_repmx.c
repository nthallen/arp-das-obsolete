#ifdef __QNX__
#include <sys/kernel.h>
#else
#include <malloc.h>
#endif
#include "mx.h"
#include "qnx_ipc.h"

int repmx(pid_t pid, unsigned parts, struct _mxfer_entry __far *msgmx) {
  int i, byts;
  char *buf;
  switch (ipc) {
#ifdef __QNX__
  case MSG_QNX_IPC: return(Replymx(pid, parts, msgmx)); break;
#endif
  default:
  case FIFO_QNX_IPC:
    for (i=0,byts=0;i<parts;i++) byts+=msgmx[i]->mxfer_len;
    if ((buf=malloc(byts))==0) return(-1);
    i=rep(pid,buf,byts);
    free(buf);
    return(i);
  }
  return (0);
}


