#ifndef RESMGR_H_
#define RESMGR_H_

#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/resmgr.h>
//#include "nortlib.h"

//class DG_device {
//  private:
//    int dev_id;
//    class DG_dispatch *dispatch;
//    iofunc_attr_t dev_attr;
//		// static resmgr_connect_funcs_t connect_funcs;
//		// static resmgr_io_funcs_t io_funcs;
//  public:
//    DG_cmd( DG_dispatch *disp );
//    ~DG_cmd();
//    int service_pulse(int triggered); // return non-zero on quit
//    int execute(char *buf);
//    static int const DG_CMD_BUFSIZE = 80;
//};

class DG_dispatch {
  public:
    dispatch_t *dpp;
    DG_dispatch();
    ~DG_dispatch();
    void Loop();
    void ready_to_quit();
  private:
    int quit_received;
    dispatch_context_t *single_ctp;
};

#endif /*RESMGR_H_*/
