
#ifndef _BREAK_H_INCLUDED
#define _BREAK_H_INCLUDED

#include <signal.h>

extern sigset_t BREAK_SET_;

void signalfunction(int sig_number);
int break_init(int breaking);
int break_init_options(int argcc, char *argvv[]);

/* handy definitions for allowing and disallowing breaks and TERM signals */
#define BREAK_SETUP { \
    sigemptyset(&BREAK_SET_); \
    sigaddset(&BREAK_SET_, SIGINT); \
    sigaddset(&BREAK_SET_, SIGTERM); \
}
    
#define BREAK_PROTECT sigprocmask(SIG_BLOCK, &BREAK_SET_, 0)
#define BREAK_ALLOW sigprocmask(SIG_UNBLOCK, &BREAK_SET_, 0)

#define OPT_BREAK_INIT "B"

#endif

