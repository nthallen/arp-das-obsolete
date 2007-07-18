#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#ifdef __QNX__
#include <sys/name.h>
#endif
#include "qnx_ipc.h"
#include "port_types.h"

int get_name_entry(int fd, char *buf);
int put_name_entry(int fd, char *buf);

int qnx_name_l(int nid, char *name, unsigned size, unsigned *copies) {
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC: return(qnx_name_locate(nid,name,size,copies)); break;
#endif
  case FIFO_QNX_IPC:
    {
      int fd;
      BYTE4 p;
      int j;
      char buf[39];
      p=-1;
      if ((fd=open(qnx_ipc_tmp(0,'N',NULL),O_RDONLY))<0) goto out;
      while (1) {
	j=get_name_entry(fd,buf);
	if (j==-1 || j==0) break;
	/* got a line */
	if (strncmp(name,buf+6,32)==0) {
	  memcpy(&p,buf+2,4);
	  break;
	}
      }
    out:
      if (copies!=NULL) *copies = p>0 ? 1 : 0;
      if (fd>-1) close(fd);
      if (p==-1) errno=ESRCH;
      return(p);
    }
  }
}

