#ifndef _DS_H_INCLUDED
#define _DS_H_INCLUDED
#include "mx.h"
extern int fd_unblock(int fd);
extern int fd_block(int fd);
extern int fd_isunblock(int fd);
extern int fd_read(int fd, char *buf, int size);
extern int fd_write(int fd, char *buf, int size);
extern int fd_wrmx(int fd, unsigned sparts, struct _mxfer_entry far *smsg);
extern int fd_rdmx(int fd, unsigned parts, struct _mxfer_entry far *msgmx);
#endif

