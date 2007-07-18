#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include "qnx_ipc.h"
#include "port_types.h"
#include "fd.h"

int qnx_sndmx(int pid, unsigned sparts, unsigned rparts,
	      struct _mxfer_entry *smsg,
	      struct _mxfer_entry *rmsg) {
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC: return(Sendmx(pid, sparts, rparts, smsg, rmsg)); break;
#endif
  case FIFO_QNX_IPC:
    {
      BYTE4 fromtid;
      struct _mxfer_entry *b;
      int fd, fdr, ret;
      pid_t child;

      b=NULL; fd=-1; fdr=-1; ret=0; child=0;
      if (pid<1) {
	errno=ESRCH;
	goto out;
      }
      if (b_fifo_out!=0) {
	/* Posix dosn't take care of zombies like this, but common */
	signal(SIGCHLD, SIG_IGN); 
	if ((child=fork())==-1)
	  return -1;
      }
      if (child==0) {		/* child process or no fork */
	/* open pid */
	if ( (fd=open(qnx_ipc_tmp(pid,'\0',0),O_WRONLY))==-1) {
	  if (errno==ENOENT) errno = ESRCH;
	  goto out;
	}
	if ((b=(struct _mxfer_entry *)malloc(MX_SZ*(sparts+1)))==NULL)
	  goto out;
	fromtid = relay_pid ? relay_pid : getpid();
	fromtid = b_fifo_out > 0 ? b_fifo_out : fromtid;
	setmx(&b[0],&fromtid,4);
	memcpy(&b[1], smsg, sparts*MX_SZ);
	if ((ret=fd_wrmx(fd,sparts+1,b))==-1) goto out;
	if (b_fifo_out==0) {
	  if ((fdr=open(qnx_ipc_tmp(getpid(),'R',0),O_RDONLY))==-1) {
	    if (errno == ENOENT) errno = ESRCH;
	    ret=-1;
	    goto out;
	  }
	  if ((b=(struct _mxfer_entry *)malloc(MX_SZ*(rparts+1)))==NULL)
	    goto out;
	  setmx(&b[0],&fromtid,sizeof(BYTE4));
	  memcpy(&b[1], rmsg, rparts*MX_SZ);
	  if ((ret=fd_rdmx(fdr,rparts+1,b))==-1) goto out;
	}
      out:
	if (fd<0 || b==NULL || ret==-1) ret=-1;
	if (ret>=0) ret=0;
	if (fd>-1) close(fd);
	if (fdr>-1) close(fdr);
	if (b!=NULL) free(b);
	if (b_fifo_out!=0) _exit(0);
	return(ret);
      } else {
	return(0);
      }
    }
  }
}


