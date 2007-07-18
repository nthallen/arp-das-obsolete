#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

/* gets a name entry that is at file's current prosition */
int get_name_entry(int fd, char *buf) {
  int j=-1;
  int i=0;
  struct flock lock;
  lock.l_type=F_RDLCK;
  lock.l_whence=SEEK_CUR;
  lock.l_start=0;
  lock.l_len=39;
  while (fcntl(fd,F_SETLKW,&lock)==-1 && errno==EINTR);
  while (i!=39) {
    /* get an entry */
    j=read(fd,buf+i,39-i);
    if (j==0) break;
    if (j==-1) {
      if (errno == EINTR) {
	errno=0;
	continue;
      } else break;
    } else i+=j;
  }
  lock.l_type=F_UNLCK;
  if (i>0) lock.l_start=-i;
  while (fcntl(fd,F_SETLKW,&lock)==-1 && errno==EINTR);
  return(j);
}

