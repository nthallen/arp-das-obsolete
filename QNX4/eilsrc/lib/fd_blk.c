#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "fd.h"

/* set a file descriptor to be non-blocking */
int fd_unblock(int fd) {
  return(fcntl(fd,F_SETFL,fcntl(fd, F_GETFL) | O_NONBLOCK));
}
/* set a file descriptor to be blocking */
int fd_block(int fd) {
  return(fcntl(fd,F_SETFL,fcntl(fd, F_GETFL) & ~O_NONBLOCK));
}
/* asks if is unblocked */
int fd_isunblock(int fd) {
  if (fcntl(fd,F_GETFL) & O_NONBLOCK) return(1);
  else return(0);
}


