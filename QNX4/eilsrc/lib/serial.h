#ifndef _SERIAL_H_INCLUDED
#define _SERIAL_H_INCLUDED

int serial_init(int fd);
int serial_init_options(int argcc, char *argvv[], int fd);

#define OPT_SERIAL_INIT "t:"

#endif
