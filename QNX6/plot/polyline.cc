#include "plot.h"

polyline::polyline( axes *axs, float *x, float *y, int n_pts, PgColor_t color ) :
     plot_obj( po_poly ) {
  ax = axs;
  this->x = x;
  this->y = y;
  this->n_pts = n_pts;
  this->color = color;
  idata = new PhPoint_t[n_pts];
  widget = 0;
}

void polyline::render() {
  int i;
  for ( i = 0; i < n_pts; i ++ ) {
    float xx = x != 0 ? x[i] : i;
    float yy = y[i];
    idata[i].x = ax->xscale.evaluate( xx );
    idata[i].y = ax->yscale.evaluate( yy );
  }
  PhDim_t minsize = { 0,0};
  PtArg_t args[4];
  PtSetArg( &args[0], Pt_ARG_POINTS, idata, n_pts );
  PtSetArg( &args[1], Pt_ARG_AREA, &ax->area, 0 );
  PtSetArg( &args[2], Pt_ARG_COLOR, color, 0 );
  PtSetArg( &args[3], Pt_ARG_MINIMUM_DIM, &minsize, 0 );
  if ( widget == 0 )
	widget = PtCreateWidget(PtPolygon, ax->fig->window, 4, args );
  else PtSetResources( widget, 4, args );
}

int polyline::callback( int &done, PtCallbackInfo_t *info ) {
  if ( info->reason == Pt_CB_WINDOW ) {
	PhWindowEvent_t *PhWE = (PhWindowEvent_t*)info->cbdata;
	if ( PhWE->event_f == Ph_WM_RESIZE ) render();
  }
  return Pt_CONTINUE;
}

