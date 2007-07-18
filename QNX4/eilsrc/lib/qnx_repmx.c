#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <malloc.h>
#include <string.h>
#include "qnx_ipc.h"
#include "port_types.h"
#include "fd.h"

int qnx_repmx(pid_t pid, unsigned parts, struct _mxfer_entry *msgmx) {
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC: return(Replymx(pid, parts, msgmx)); break;
#endif
  case FIFO_QNX_IPC:
    {
      BYTE4 fromtid;
      struct _mxfer_entry *b;
      int fd, ret;
      b=NULL; fd=-1; ret=0;
      if (pid<1) {
	errno=ESRCH;
	goto out;
      }
      /* open "pidR" */
      if ((fd=open(qnx_ipc_tmp(pid,'R',0),O_WRONLY)) == -1) {
	if (errno == ENOENT) errno = ESRCH;
	goto out;
      }
      if ( (b=(struct _mxfer_entry *)malloc(MX_SZ*(parts+1)))==NULL) goto out;
      fromtid=getpid();
      setmx(&b[0],&fromtid,4);
      memcpy(&b[1], msgmx, parts*MX_SZ);
      if ((ret=fd_wrmx(fd,parts+1,b))==-1) goto out;
    out:
      if (fd<0 || b==NULL || ret<0) ret=-1;
      if (ret>-1) ret=0;
      if (fd>-1) close(fd);
      if (b!=NULL) free(b);
      return(ret);
    }
  }
}


