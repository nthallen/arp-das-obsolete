#include "DC.h"
#include "nortlib.h"

class DCtap : public data_client {
  public:
    DCtap(char *srcfile);
  protected:
    void process_data();
    void process_init();
    void process_tstamp();
}

DCtap::DCtap( char *srcfile ) : data_client( 4096, 0, srcfile ) {}

void DCtap::process_init() {
  data_client::process_init();
  nl_error( 0, "process_init()" );
}

void DCtap::process_tstamp() {
  data_client::process_tstamp();
  nl_error( 0, "process_tstamp()" );
}

void DCtap::process_data() {
  data_client::process_data();
  nl_error( 0, "process_data()" );
}
