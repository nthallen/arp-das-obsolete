#ifndef DG_CMD_H
#define DG_CMD_H

#include <signal.h>

class DG_cmd {
  private:
    struct sigevent cmd_ev;
    int cmd_fd;
    static int const BUFSIZE = 80;
  public:
    DG_cmd( int coid, int pulse_code );
    ~DG_cmd();
    int service(int triggered); // return non-zero on quit
};

#endif

