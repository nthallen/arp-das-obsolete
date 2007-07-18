#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#ifdef __QNX__
#include <sys/kernel.h>
#include <sys/proxy.h>
#endif
#include "qnx_ipc.h"
#include "port_types.h"

int qnx_proxy_d(int pid) {
  switch (ipc) {
  default:;
#ifdef __QNX__
  case MSG_QNX_IPC: return(qnx_proxy_detach(pid));
#endif
  case FIFO_QNX_IPC:
    {
      int fd;
      BYTE4 apid;
      int i;
      fd=-1; i=-1;
      if (pid<0) return(-1);
      if ((fd=open(qnx_ipc_tmp(pid,'P',0),O_RDONLY))==-1) {
	if (errno==ENOENT) errno=ESRCH;
	goto out;
      }
      if (read(fd,&apid,4)!=4) goto out;
      if (apid!=getpid()) {
	errno=EPERM;
	goto out;
      }
      /* should probably check pid that this proxy is attached to first */
      i=remove(qnx_ipc_tmp(pid,'P',0)) !=0 ? -1 : 0;
    out:
      if (fd>-1) close(fd);
      return(i);
    }
  }	
}
