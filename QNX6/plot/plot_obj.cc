#include "plot.h"
#include "nortlib.h"

plot_obj::plot_obj( plot_obj_type po_type ) {
  type = po_type;
  first = last = next = 0;
}

void plot_obj::add_child( plot_obj *child ) {
  assert( child->next == 0 );
  if ( last == 0 ) {
    first = last = child;
  } else {
    last->next = child;
    last = child;
  }
}

void plot_obj::render() {
  plot_obj *child;
  for ( child = first; child != 0; child = child->next )
    child->render();
}

int plot_obj::callback( int &done, PtCallbackInfo_t *info ) {
  plot_obj *child;
  for ( child = first; child != 0; child = child->next )
    child->callback( done, info );
  return Pt_CONTINUE;
}

void plot_obj::plot( f_matrix *xdata, f_matrix *ydata ) {
  nl_error( 3, "plot function not supported for object" );
}

