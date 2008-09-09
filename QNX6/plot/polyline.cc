#include "plot.h"

const int polyline::pts_per_polyline = 500;

polyline::polyline( axes *axs, float *x, float *y, int n_pts, PgColor_t color ) :
     plot_obj( po_poly ) {
  ax = axs;
  this->x = x;
  this->y = y;
  this->n_pts = n_pts;
  n_widgets = (n_pts + pts_per_polyline - 1)/pts_per_polyline;
  this->color = color;
  idata = new PhPoint_t[n_pts];
  widget = new PtWidget_p[n_widgets];
}

void polyline::render() {
  int i, wn;
  int start = 0, np = pts_per_polyline;
  for ( i = 0; i < n_pts; i ++ ) {
    float xx = x != 0 ? x[i] : i;
    float yy = y[i];
    idata[i].x = ax->xscale.evaluate( xx );
    idata[i].y = ax->yscale.evaluate( yy );
  }
  PhDim_t minsize = { 0,0};
  PtArg_t args[4];
  PtSetArg( &args[1], Pt_ARG_AREA, &ax->area, 0 );
  PtSetArg( &args[2], Pt_ARG_COLOR, color, 0 );
  PtSetArg( &args[3], Pt_ARG_MINIMUM_DIM, &minsize, 0 );
  for ( start = 0, wn = 0; start < n_pts; ++wn) {
    np = pts_per_polyline;
    if ( start + np > n_pts ) np = n_pts - start;
    PtSetArg( &args[0], Pt_ARG_POINTS, &idata[start], np );
    if ( widget[wn] == 0 )
      widget[wn] = PtCreateWidget(PtPolygon, ax->fig->window, 4, args );
    else PtSetResources( widget[wn], 4, args );
    start += np;
    if ( start < n_pts ) --start;
  }
}

int polyline::callback( int &done, PtCallbackInfo_t *info ) {
  if ( info->reason == Pt_CB_WINDOW ) {
	PhWindowEvent_t *PhWE = (PhWindowEvent_t*)info->cbdata;
	if ( PhWE->event_f == Ph_WM_RESIZE ) render();
  }
  return Pt_CONTINUE;
}

