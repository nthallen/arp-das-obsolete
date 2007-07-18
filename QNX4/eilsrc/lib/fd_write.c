#include <stdio.h>
#include <string.h>
#include "fd.h"

int fd_write(int fd, char *buf, int size) {
  struct _mxfer_entry msg[1];
  setmx(&msg[0],buf,size);
  return(fd_wrmx(fd,1,msg));
}
