#include "plot.h"

axes::axes( figure *fig ) : plot_obj( po_axes ) {
  this->fig = fig;
  area.pos.x = 0;
  area.pos.y = 0;
  area.size = fig->size;
  // PtGetResource( fig->window, Pt_ARG_AREA, &area, 0 );
  xscale.direction = 1;
  xscale.set(area.size.w);
  yscale.direction = 0;
  yscale.set(area.size.h);
}

void axes::plot( f_matrix *xdata, f_matrix *ydata ) {
  assert(ydata != 0);
  yscale.set( ydata );
  if ( xdata != 0 ) xscale.set( xdata );
  else xscale.set( 0, ydata->nrows-1 );
  add_child( new line( this, xdata, ydata ) );
}

// void axes::render() { }
int axes::callback( int &done, PtCallbackInfo_t *info ) {
  if ( info->reason == Pt_CB_WINDOW ) {
	PhWindowEvent_t *PhWE = (PhWindowEvent_t*)info->cbdata;
	if ( PhWE->event_f == Ph_WM_RESIZE ) {
	  area.size = fig->size;
	  xscale.set(area.size.w);
	  yscale.set(area.size.h);
	  plot_obj::callback( done, info );
	  done = 1;
	}
  }
  return Pt_CONTINUE;
}

void axes::zoom() {
  // add_child( new zoom(this) );
}

