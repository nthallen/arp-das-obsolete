#ifdef __QNX__
#include <sys/kernel.h>
#endif
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <types.h>
#include "qnx_ipc.h"
#include "mx.h"

int snd(pid_t pid, void __far *smsg,void __far *rmsg, 
	unsigned snbytes, unsigned rnbytes) {
  struct _mxfer_entry b[2];
  static pid_t spid;
  char nm[12];
  pid_t fromtid;

  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC: return(Send(pid, smsg, rmsg, snbytes, rnbytes)); break;
#endif
  case IPC_FIFO:
    if (pid != spid) {
      if (fifo_out>=0) close(fifo_out);
      sprintf(nm,"%d",pid);
      /* open pid */
      if ( (fifo_out=open(nm,O_WRONLY))==-1) {
	if (errno==ENOENT) errno = ESRCH;
	spid = 0;
	return(-1);
      }
      spid = pid;
    }
    fromtid=getpid();
    setmx(&b[0],&fromtid,sizeof(BYTE4));
    setmx(&b[1],smsg,snbytes);
    if (fd_wrmx(fifo_out,2,b)==-1) return(-1);
    setmx(&b[0],&fromtid,sizeof(BYTE4));
    setmx(&b[1],rmsg,rnbytes);
    if (fd_rdmx(fifo_r,2,b)==-1) return(-1);
  }
  return(0);
}
