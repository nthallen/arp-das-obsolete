#ifndef DC_H_INCLUDED
#define DC_H_INCLUDED
#include "tm.h"

class data_client {
  public:
    data_client(int bufsize_in, int fast = 0, int non_block = 0);
    void operate(); // event loop
    static unsigned int next_minor_frame, majf_row, minf_row;
  protected:
    void process_data();
    int bfr_fd;
    void read();
    bool quit;
  private:
    void process_message();
    void process_init();
    void process_tstamp();
    int nQrows;
    int bufsize;
    unsigned int bytes_read; /// number of bytes currently in buf
    unsigned int toread; /// number of bytes needed before next action
    char *buf;
    tm_msg_t *msg;
    tm_hdrw_t output_tm_type;
    int nbQrow; // may differ from nbrow if stripping MFCtr & Synch
    int nbDataHdr;
    bool tm_info_ready;
};

void tminitfunc();


#endif
