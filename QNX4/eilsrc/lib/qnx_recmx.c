#ifdef __QNX__
#include <sys/kernel.h>
#else
#include <malloc.h>
#include <string.h>
#endif
#include "mx.h"
#include "qnx_ipc.h"

int recmx(int pid,unsigned parts,struct _mxfer_entry __far *msgmx) {
  int byts, i,j;
  char *buf;
  switch (ipc) {
#ifdef __QNX__
  case MSG_QNX_IPC:
    return(Receivemx(pid, parts, msgmx)); break;
#endif
  default:
  case FIFO_QNX_IPC:
    for (i=0,byts=0;i<parts;i++) byts+=msgmx[i]->mxfer_len;
    if ((buf=malloc(byts))==-1) return(-1);
    if ( (j=rec(pid,buf,byts))!=-1)
      for (i=0; i<parts;buf+=msgmx[i]->mxfer_len,i++)
        memcpy(msgmx[i]->mxfer_off,buf,msgmx[i]->mxfer_len);
    free(buf);
    return(j);
  }  
}
