#include "plot.h"
int fig_window_callback( PtWidget_t *widget, void *data, 
           PtCallbackInfo_t *info) {
  assert( data != 0 );
  plot_obj *fig = (plot_obj *)data;
  int done = 0;
  int rv = fig->callback( done, info );
  if ( done == 0 ) {
	if ( info->reason == Pt_CB_WINDOW ) {
	  PhWindowEvent_t *PhWE = (PhWindowEvent_t*)info->cbdata;
	  printf( "fig_window_callback: Pt_CB_WINDOW event %d\n", PhWE->event_f );
	} else {
	  printf( "fig_window_callback: reason %d\n", info->reason );
    }
  }
  return( rv );
}

figure::figure() : plot_obj( po_figure ) {
  size.w = 800; size.h = 350;
  PhDim_t mindim = { 0, 0 };
  PtArg_t args[4];
  PtSetArg( &args[0], Pt_ARG_DIM, &size, 0 );
  PtSetArg( &args[1], Pt_ARG_FILL_COLOR, Pg_BLACK, 0 );
  PtSetArg( &args[2], Pt_ARG_MINIMUM_DIM, &mindim, 0 );
  PtSetArg( &args[3], Pt_ARG_RESIZE_FLAGS, Pt_FALSE,
				 Pt_RESIZE_X_BITS|Pt_RESIZE_Y_BITS);
  window = PtCreateWidget(PtWindow, Pt_NO_PARENT, 4, args );
  PtAddCallback( window, Pt_CB_WINDOW, fig_window_callback, this );
}

void figure::plot( f_matrix *xdata, f_matrix *ydata ) {
  if ( current_axes == 0 ) figaxes();
  current_axes->plot( xdata, ydata );
}

void figure::figaxes() {
  current_axes = new axes(this);
  add_child( current_axes );
}

void figure::render() {
  plot_obj::render();
  PtRealizeWidget(window);
}

int figure::callback( int &done, PtCallbackInfo_t *info ) {
  if ( info->reason == Pt_CB_WINDOW ) {
	PhWindowEvent_t *PhWE = (PhWindowEvent_t*)info->cbdata;
	if ( PhWE->event_f == Ph_WM_RESIZE ) {
	  size = PhWE->size;
	  plot_obj::callback( done, info );
	  done = 1;
	}
  }
  return Pt_CONTINUE;
}
