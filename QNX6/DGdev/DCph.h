#ifndef DCPH_H_INCLUDED
#define DCPH_H_INCLUDED
#include "DC.h"

class ph_data_client : public data_client {
  ph_data_client( int bufsize_in );
  void operate();
};

#endif
