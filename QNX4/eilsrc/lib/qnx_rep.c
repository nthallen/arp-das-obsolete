#ifdef __QNX__
#include <sys/kernel.h>
#endif
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "qnx_ipc.h"

int rep(int pid, void __far *msg, unsigned nbytes) {
  static int rpid;
  char nm[12];
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC: return(Reply(pid, msg, nbytes)); break;
#endif
  case FIFO_QNX_IPC:
    if (pid<1) {
      errno=EINVAL;
      return(-1);
    }
    if (pid != rpid) {
      if (fifo_r>=0) close(fifo_r);
      /* open "pidR" */
      sprintf(nm,"%dR",pid);
      if (open(nm,O_WRONLY)==-1) {
	if (errno == ENOENT) errno = ESRCH;
	rpid = 0;
	return(-1);
      }
      rpid = pid;				
    }
    if (fd_write(fifo_r, msg, nbytes)==-1) return(-1);
  }
  return (0);
}
