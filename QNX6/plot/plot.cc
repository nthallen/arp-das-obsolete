#include "nortlib.h"
#include "plot.h"

int main( int argc, char **argv ) {
  char *filename = "sample.dat";
  if ( argc > 1 ) filename = argv[1];
  f_matrix *data = new f_matrix( filename, FM_FMT_ICOS );
  if (PtInit(NULL) == -1)
    PtExit(EXIT_FAILURE);
  figure *fig = new figure();
  fig->plot( data );
  //fig->current_axes->zoom();
  fig->render();
  PtMainLoop();
  return 0;
}

