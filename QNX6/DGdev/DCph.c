#include "DCph.h"
#include <Pt.h>
#include "nortlib.h"

ph_data_client::ph_data_client( int bufsize_in )
    : data_client(bufsize_in, 1, 1 ) {
  if (PtInit(NULL) == -1)
    nl_error(3, "Cannot initialize Photon Connection");
}

PtFdProcF_t DCPH_fd_handler;
int DCPH_fd_handler( int fd, void *data, unsigned mode) {
  ph_data_client *DC = (ph_data_client *)data;
  DC->read();
}

void ph_data_client::operate() {
  tminitfunc();
  PtAppAddFd( NULL, bfr_fd, Pt_FD_READ, DCPH_fd_handler, NULL );
  PtMainLoop();
  return 0;
}
