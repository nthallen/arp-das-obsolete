#ifndef DG_CMD_H
#define DG_CMD_H

#include <signal.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

class DG_cmd {
  private:
    struct sigevent cmd_ev;
    int cmd_fd;
    static int const BUFSIZE = 80;
  public:
    DG_cmd( dispatch_t *dpp );
    ~DG_cmd();
    int service(int triggered); // return non-zero on quit
};

int DG_cmd_pulse_func( message_context_t *ctp, int code,
			unsigned flags, void *handle );

#endif

