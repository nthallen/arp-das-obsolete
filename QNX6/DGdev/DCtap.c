#include <string.h>
#include <time.h>
#include "DC.h"
#include "nortlib.h"

class DCtap : public data_client {
  public:
    DCtap(char *srcfile);
  protected:
    void process_data();
    void process_init();
    void process_tstamp();
};

DCtap::DCtap( char *srcfile ) : data_client( 4096, 0, srcfile ) {}

void DCtap::process_init() {
  memcpy(&tm_info, &msg->body.init.tm, sizeof(tm_dac_t));
  data_client::process_init();
  nl_error( 0, "process_init()" );
}

void DCtap::process_tstamp() {
  struct tm *tm = gmtime( &msg->body.ts.secs );
  char *ttext = asctime(tm);
  int tlen = strlen(ttext);
  ttext[tlen-1] = '\0';
  nl_error( 0, "process_tstamp(%s, %5d)", ttext, msg->body.ts.mfc_num );
  data_client::process_tstamp();
}

void DCtap::process_data() {
  //data_client::process_data();
  nl_error( 0, "DCtap::process_data()" );
}

void data_client::process_data() {
  nl_error( 0, "data_client::process_data()" );
}

void tminitfunc() {}

int main( int argc, char **argv ) {
  char *srcfile;
  srcfile = argc > 1 ? argv[1] : tm_dev_name("TM/DCf");
  DCtap DC( srcfile );
  DC.operate();
  return 0;
}

