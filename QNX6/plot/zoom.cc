#include "plot.h"

/* Zoom object is attached to an axes.
   Attaches Pt_CB_ARM and Pt_CB_DISARM callbacks to the axes'
   window. Or perhaps more specifically, attaches the callbacks
   via the window, but the callback should point directly to
   the zoom object in question.

   Pt_CB_ARM: Check if mouse is within axes' area. If so,
   initiate an outline drag. I'm guessing I can do this
   without really having a widget I'm dragging, but if
   necessary, I can create a rectangle and drag it.
   PhInitDrag() PtAddFilterCallback()

   Pt_CB_FILTER: On drag complete, rescale axes and redraw.
   Also clean up the drag operation. Remove the filter callback,
   etc.
*/

zoom::zoom( axes *axx ) {
  ax = axx;
  // Add the callback
}

