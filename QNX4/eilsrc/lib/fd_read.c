#include <stdio.h>
#include <string.h>
#include "fd.h"

/* returns -1 on failure */
int fd_read(int fd, char *buf, int size, int used) {
  memmove(buf, buf+used, size-used);
  return(read(fd,buf+used,size-used));
}
