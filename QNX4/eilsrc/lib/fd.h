#ifndef _DS_H_INCLUDED
#define _DS_H_INCLUDED
int fd_unblock(int fd);
int fd_block(int fd);
int fd_isunblock(int fd);
int fd_read(int fd, char *buf, int size, int used);
#define fd_write(A,B,C) write(A,B,C)
#endif

