#include "nortlib.h"
#include "plot.h"

int main( int argc, char **argv ) {
  f_matrix *data = new f_matrix( "sample.dat", FM_FMT_TEXT );
  if (PtInit(NULL) == -1)
    PtExit(EXIT_FAILURE);
  figure *fig = new figure();
  fig->plot( data );
  //fig->current_axes->zoom();
  fig->render();
  PtMainLoop();
  return 0;
}

