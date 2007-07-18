#include "fd.h"

/* returns -1 on failure */
int fd_read(int fd, char *buf, int size) {
  struct _mxfer_entry m;
  setmx(&m,buf,size);
  return(fd_rdmx(fd,1,&m));
}
