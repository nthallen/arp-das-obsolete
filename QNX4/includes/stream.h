#ifndef _DS_H_INCLUDED
#define _DS_H_INCLUDED
#include <globmsg.h>
int bus_nonblock(int fd);
/* returns -1 on failure, number of bytes read otherwise */
int bus_read(int fd, char *buf);
/* returns -1 on failure, number of bytes written otherwise */
int bus_write(int fd, msg_hdr_type h, char *buf, int nbytes);
#endif

