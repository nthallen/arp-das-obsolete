#include "DC.h"

data_client::data_client(int nQrows, int low_water);
void data_client::init();
void data_client::operate(); // event loop
void data_client::process_data();
