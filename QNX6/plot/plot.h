#ifndef PLOT_H_INCLUDED
#define PLOT_H_INCLUDED

#include <assert.h>
#include <Pt.h>
#include <photon/Pf.h>
#include "f_matrix.h"
#include "nortlib.h"

enum plot_obj_type { po_figure, po_axes, po_line, po_poly, po_text, po_max };

class plot_obj {
  public:
	plot_obj_type type;
	plot_obj *first;
	plot_obj *last;
	plot_obj *next;

	plot_obj::plot_obj( plot_obj_type po_type );
	void add_child( plot_obj *child );
	virtual void plot( f_matrix *xdata, f_matrix *ydata );
	inline void plot( f_matrix *ydata ) { plot( 0, ydata ); }
	virtual void render(); // default just renders children
	virtual int callback( int &done, PtCallbackInfo_t *info);
};

class figure : public plot_obj {
  public:
    PtWidget_t *window;
    PhDim_t size;
    plot_obj *current_axes;

    figure(void);
    void figaxes(void);
	inline void plot( f_matrix *ydata ) { plot( 0, ydata ); }
	void plot( f_matrix *xdata, f_matrix *ydata );
	void render();
	int callback( int &done, PtCallbackInfo_t *info);
};

class scale {
  public:
	float min, max;
	int pixels;
	int direction;
    float scalev;

    scale();
    void set();
    void set( f_matrix *data );
    void set( float min, float max );
    void set( int pixels );
    int evaluate( float val );
};

class axes : public plot_obj {
  public:
    figure *fig;
    PhArea_t area;
    scale xscale, yscale;

    axes( figure *fig );
	void plot( f_matrix *xdata, f_matrix *ydata );
	void zoom();
	// void render();
	int callback( int &done, PtCallbackInfo_t *info);
};

class line : public plot_obj {
  public:
    axes *ax;
    f_matrix *xdata;
    f_matrix *ydata;

    inline line::line( axes *axs, f_matrix *ydata ) : plot_obj( po_line ) {
      line( axs, 0, ydata );
    }
    line::line( axes *axs, f_matrix *xdata, f_matrix *ydata );
	void render();
};

class polyline : public plot_obj {
  public:
    axes *ax;
    float *x;
    float *y;
    int n_pts;
    PgColor_t color;
    PhPoint_t *idata;
    PtWidget_t *widget;
    // color, linestyle, etc.
    polyline( axes *axs, float *x, float *y, int n_pts, PgColor_t color );
    void render();
	int callback( int &done, PtCallbackInfo_t *info);
};

class zoom : public plot_obj {
  public:
    zoom( axes *ax );
    axes *ax;
	int callback( int &done, PtCallbackInfo_t *info);
};

#endif

