#include <windows/Qwindows.h>
#include <windows/Qwin_etc.h>
#include <assert.h>
#include "bomem.h"
#include "nortlib.h"

#define XY_UNIT 100

typedef struct {
  QW_CXY_COORD *xy;
  float huge *y;
  long int npts;
  long int plotted;
  unsigned short pane_x, pane_y;
  float min_y, max_y;
  double mx, dy, my;
} plotdef;

plotdef plot;

static void calc_scales(void) {
  plot.mx = (plot.pane_x-1)/((double) plot.npts);
  plot.dy = plot.min_y;
  plot.my = (plot.pane_y-1)/(plot.max_y - plot.min_y);
}

static unsigned short xscale(float x) {
  return (unsigned short) (x * plot.mx);
}

static unsigned short yscale(float y) {
  return (unsigned short) ((y-plot.dy) * plot.my);
}

void replot(void) {
  WindowCurrent(base_wind_id);
  PictureCurrent(base_pict_id);
  Erase((char const __far *)QW_ALL);
  SetFill("r", QW_WHITE, QW_SOLID_PAT);
  DrawAt(0, 0);
  DrawRect(plot.pane_y, plot.pane_x, "!", NULL);
  plot.plotted = 0;
}

void get_pane_size(void) {
  int picture;
  QW_RECT_AREA region, view;

  WindowCurrent(base_wind_id);
  PaneInfo(&region, &view, &picture);
  assert(picture == base_pict_id);
  plot.pane_x = view.width;
  plot.pane_y = view.height;
  if (plot.npts > 0) {
	calc_scales();
	replot();
  }
}

void new_plot(float huge *buffer, long int npts) {
  long int i;

  plot.npts = npts;
  plot.y = buffer;
  if (plot.npts > 0) {
	/* determine min/max */
	plot.min_y = plot.max_y = buffer[0];
	for (i = 1; i < npts; i++) {
	  if (buffer[i] > plot.max_y) plot.max_y = buffer[i];
	  else if (buffer[i] < plot.min_y) plot.min_y = buffer[i];
	}
	WindowCurrent(base_wind_id);
	WindowBarCurrent('T', NULL);
	ChangeReal("m", plot.min_y);
	ChangeReal("M", plot.max_y);
	Draw();
	calc_scales();
	replot();
  }
}

/* returns 1 while there is more plotting to do */
int plotting(void) {
  int togo, i;

  if (plot.plotted < plot.npts) {
	if (plot.xy == NULL)
	  plot.xy = (QW_CXY_COORD *)new_memory(XY_UNIT*sizeof(QW_CXY_COORD));
	togo = plot.npts - plot.plotted;
	if (togo > XY_UNIT) togo = XY_UNIT;
	for (i = 0; i < togo; i++) {
	  plot.xy[i].x = xscale(plot.plotted);
	  plot.xy[i].y = yscale(plot.y[plot.plotted++]);
	}
	PictureCurrent(base_pict_id);
	SetPointArea(plot.pane_y, plot.pane_x);
	DrawAt(0, 0);
	DrawPoints(togo, (QW_XY_COORD *)plot.xy, NULL, QW_RED, "!;KN", NULL);
	Draw();
	if (plot.plotted < plot.npts) {
	  plot.plotted--;
	  return 1;
	}
  }
  return 0;
}

void sub_plot(short row, short width) {
  long int i, npts;

  if (plot.npts > 0 && plot.mx != 0) {
	i = row/plot.mx;
	npts = width/plot.mx;
	if (i < plot.npts) {
	  if (i + npts > plot.npts) {
		npts = plot.npts - i;
	  }
	  new_plot(plot.y+i, npts);
	}
  }
}
