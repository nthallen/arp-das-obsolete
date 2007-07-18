#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#ifdef __QNX__
#include <sys/name.h>
#endif
#include "qnx_ipc.h"
#include "port_types.h"

int get_name_entry(int fd, char *buf);
int put_name_entry(int fd, char *buf);

int qnx_name_a(int nid, char *name) {
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC: return(qnx_name_attach(nid,name)); break;
#endif
  case FIFO_QNX_IPC:
    {
      int fd;
      BYTE2 line, nullid, id;
      BYTE4 p;
      int j;
      char buf[39];
      fd=-1; j=-1; nullid=-1; line=0;
      /* open file 0N */
      if ((fd=open(qnx_ipc_tmp(0,'N',NULL),O_RDWR|O_CREAT,
	      S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))==-1) goto out;
      while (1) {
	j=get_name_entry(fd,buf);
	if (j==-1 || j==0) break;
	/* got a line */
	line++;
	memcpy(&id,buf,2);
	if (id==0 && nullid==-1) nullid=line;
	if (strncmp(name,buf+6,32)==0) {
	  errno=EBUSY;
	  nullid=-1;
	  goto out;
	}
      }
      if (nullid<1 && line) nullid=line;
      else nullid=1;
      if (lseek(fd,(nullid-1)*39,SEEK_SET)==-1) {
	nullid=-1;
	goto out;
      }
      memcpy(buf,&nullid,2);
      p=getpid();
      memcpy(buf+2,&p,4);
      j=strlen(name) > 32 ? 32 : strlen(name);
      memcpy(buf+6,name,j);
      buf[6+j]='\0';
      if ((j=put_name_entry(fd,buf))==-1) nullid=-1;
    out:
      if (fd!=-1) close(fd);
      return(nullid);
    }
  }
}

