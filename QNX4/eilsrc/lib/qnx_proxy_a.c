#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifdef __QNX__
#include <sys/kernel.h>
#include <sys/proxy.h>
#endif
#include "qnx_ipc.h"
#include "port_types.h"

int qnx_proxy_a(int pid, char *data, int nbytes, int priority) {
  int i;
  pid_t proxy;
  BYTE4 p;
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC: return(qnx_proxy_attach(pid, data, nbytes, priority));
#endif
  case FIFO_QNX_IPC:
    if (pid<0) return(-1);
    p = pid==0 ? getpid() : pid;
    proxy=0;
    do
      i=open(qnx_ipc_tmp(++proxy,'P',0), O_WRONLY|O_CREAT|O_EXCL,
	     S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    while (i==-1 && errno==EEXIST);
    if (i==-1) return(-1);
    if (write(i,&p,4)!=4) return(-1);
    if (write(i,data,nbytes)!=nbytes) return(-1);
    close(i);
    return(proxy);
  }
}
