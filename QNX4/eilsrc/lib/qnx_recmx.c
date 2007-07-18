#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "qnx_ipc.h"
#include "fd.h"
#include "port_types.h"

int qnx_recmx(int pid,unsigned parts,struct _mxfer_entry *msgmx) {
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC:
    return(Receivemx(pid, parts, msgmx)); break;
#endif
  case FIFO_QNX_IPC:
    {
      struct _mxfer_entry *b;
      BYTE4 fromtid;
      int bts;
      b=NULL; fromtid=-1; bts=-1;
      if (fifo_in==-1)
	if ((fifo_in=open(qnx_ipc_tmp(getpid(),'\0',0),O_RDONLY))==-1)
	  goto out;
      if ((b=(struct _mxfer_entry *)malloc(MX_SZ*(parts+1)))==NULL) goto out;
      setmx(&b[0],&fromtid,4);
      memcpy(&b[1],msgmx,MX_SZ*parts);
      if ((bts=fd_rdmx(fifo_in,parts+1,b))==-1) goto out;
      memcpy(msgmx,&b[1],parts*MX_SZ);
    out: 
      if (b!=NULL) free(b);
      if (bts==0) {
	close(fifo_in);
	fifo_in=-1;
	fromtid=qnx_recmx(pid,parts,msgmx);
      }
      return(fromtid);
    }
  }  
}
