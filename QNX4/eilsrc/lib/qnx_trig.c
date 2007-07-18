#include <sys/types.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#ifdef __QNX__
#include <sys/proxy.h>
#endif
#include "port_types.h"
#include "qnx_ipc.h"
#include "fd.h"

int qnx_trig(pid_t proxy) {
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC: return(Trigger(proxy));
#endif
  case FIFO_QNX_IPC:
    {
      int i;
      int fd;
      BYTE4 apid;
      off_t l;
      char *buf;
      buf=NULL; i=-1; l=-1; fd=-1;
      if ((fd=open(qnx_ipc_tmp(proxy,'P',0),O_RDONLY))==-1) goto out;
      if (read(fd,&apid,4)!=4) goto out;
      if ((l=lseek(fd,0,SEEK_END))==-1) goto out;
      l-=4;
      if ((buf=malloc(l))==0) goto out;
      if (lseek(fd,4,SEEK_SET)==-1) goto out;
      if (read(fd,buf,l)==-1) goto out;
      b_fifo_out=proxy;
      i=qnx_snd(apid, buf, 0, l, 0);
      b_fifo_out=0;
    out:
      if (fd>-1) close(fd);
      if (buf!=NULL) free(buf);
      return(i);
    }
  }
}

