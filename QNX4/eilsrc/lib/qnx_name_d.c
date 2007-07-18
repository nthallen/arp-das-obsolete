#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __QNX__
#include <sys/name.h>
#endif
#include "qnx_ipc.h"
#include "port_types.h"

int get_name_entry(int fd, char *buf);
int put_name_entry(int fd, char *buf);

int qnx_name_d(int nid, int name_id) {
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC: return(qnx_name_detach(nid,name_id));
#endif
  case FIFO_QNX_IPC:
    {
      int fd;
      BYTE4 p;
      BYTE2 id;
      int j;
      char buf[39];
      int lines, nulllines;
      int ret;
      p=-1; ret=-1; lines=0; nulllines=0;
      if ((fd=open(qnx_ipc_tmp(0,'N',NULL),O_RDWR))<0) goto out;
      while (1) {
	j=get_name_entry(fd,buf);
	if (j==-1 || j==0) break;
	/* got a line */
	lines++;
	memcpy(&id,buf,2);
	if (id==0) nulllines++;
	else if (id==name_id) {
	  if (lseek(fd,(name_id-1)*39,SEEK_SET)==-1) goto out;
	  memcpy(&p,buf+2,4);
	  if (p==getpid()) {
	    buf[0]=0; buf[1]=0;
	    j=put_name_entry(fd,buf);
	    if (j==-1 || j==0) break;
	    if (j==39) {
	      ret=0;
	      nulllines++;
	    }
	  } else {
	    errno=EINVAL;
	    goto out;
	  }
	}
      }
    out:
      if (j==0 && lines==nulllines) remove(qnx_ipc_tmp(0,'N',NULL));
      if (fd>-1) close(fd);
      return(ret);
    }
  }
}





