#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

/* puts a name entry at file's current prosition */
int put_name_entry(int fd, char *buf) {
  int j=-1;
  int i=0;
  struct flock lock;
  lock.l_type=F_WRLCK;
  lock.l_whence=SEEK_CUR;
  lock.l_start=0;
  lock.l_len=39;
  while (fcntl(fd,F_SETLKW,&lock)==-1 && errno==EINTR);
  while (i!=39) {
    j=write(fd,buf+i,39-i);
    if (j==-1) {
      if (errno!=EINTR) break;
      if (i==0) break;
      errno=0;
    } else i+=j;
  }
  lock.l_type=F_UNLCK;
  if (i>0) lock.l_start=-i;
  while (fcntl(fd,F_SETLKW,&lock)==-1 && errno==EINTR);
  return(j);
}








