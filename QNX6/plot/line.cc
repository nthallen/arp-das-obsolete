#include "plot.h"

static PgColor_t line_colors[] = {
  Pg_BLUE, Pg_GREEN, Pg_RED, Pg_YELLOW, Pg_MAGENTA,
  Pg_CYAN, Pg_DGREEN, Pg_DCYAN, Pg_DBLUE, Pg_BROWN,
  Pg_PURPLE, Pg_CELIDON
};
#define N_COLORS (sizeof(line_colors)/sizeof(PgColor_t))

line::line( axes *axs, f_matrix *xdata, f_matrix *ydata ) :
    plot_obj( po_line ) {
  ax = axs;
  this->xdata = xdata;
  this->ydata = ydata;
  assert( ydata != 0 );
  if ( xdata != 0 && xdata->nrows != ydata->nrows )
    nl_error( 3, "Vectors must be the same length in line::line" );
}

void line::render() {
  int x_inc = 1, y_inc = 1;
  int n_lines;

  if ( xdata == 0 ) {
    n_lines = ydata->ncols;
  } else if ( xdata->ncols == ydata->ncols ) {
    n_lines = ydata->ncols;
  } else if ( xdata->ncols == 1 ) {
    n_lines = ydata->ncols;
    x_inc = 0;
  } else if ( ydata->ncols == 1 ) {
    n_lines = xdata->ncols;
    y_inc = 0;
  } else nl_error( 3, "Matrix dimensions don't agree in line::line" );

  int xc = 0, yc = 0, line_num;
  for ( line_num = 0; line_num < n_lines; line_num++ ) {
    float *x = xdata != 0 ? xdata->mdata[xc] : 0;
    float *y = ydata->mdata[yc];
    add_child( new polyline( ax, x, y, ydata->nrows,
          line_colors[line_num%N_COLORS] ));
    xc += x_inc;
    yc += y_inc;
  }
  plot_obj::render();
}
