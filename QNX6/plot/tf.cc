#include "f_matrix.h"
#include "nortlib.h"

int main( int argc, char **argv ) {
  f_matrix *data = new f_matrix( "sample.dat", FM_FMT_TEXT );
  printf("Sample matrix has %d rows and %d columns\n",
    data->nrows, data->ncols );
  return 0;  
}
