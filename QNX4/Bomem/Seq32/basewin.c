#include <windows/Qwindows.h>
#include <stdlib.h>
#include <math.h>
#include "bomem.h"
#include "nortlib.h"

int base_wind_id, base_pict_id;

int n_scans = 10;
int sequence = 0;
int plot_dir = 0;
int plot_chan = 0;
int plot_imag = 0; /* 0 = real, 1 = imag */

void rubber_band(QW_EVENT_MSG *msg) {
  QW_EXTENT area;
  int rv;

  PictureCurrent(base_pict_id);
  DrawAt(msg->hdr.row, msg->hdr.col);
  SetColor("l", QW_TRANSPARENT);
  SetLineThickness("l", 0);
  DrawRect(0, 0, NULL, "rect");
  DrawAt(msg->hdr.row, msg->hdr.col);
  rv = (RubberBand("rect", 7, NULL, &area) == YES);
  Erase("rect");
  if (rv) sub_plot(area.col, area.width);
}

void update_sequence(void) {
  if (windows) {
	WindowCurrent(base_wind_id);
	WindowBarCurrent('T', NULL);
	ChangeNumber("C", (long) sequence);
  }
}

void update_n_scans(void) {
  if (windows) {
	WindowCurrent(base_wind_id);
	WindowBarCurrent('B', NULL);
	ChangeNumber("B", (long)n_scans);
	Draw();
  }
}

void update_int_spec(void) {
  if (windows) {
	WindowCurrent(base_wind_id);
	WindowBarCurrent('B', NULL);
	ChangeState(collect_spec ? "I1" : "I0", 1);
	Draw();
  }
}

void dim_acquire(int dim) {
  if (windows) {
	WindowCurrent(base_wind_id);
	WindowBarCurrent('B', NULL);
	ChangeOptions("A", dim ? "d-S" : "S-d");
	Draw();
  }
}

/* Tags in the basewindow:
  Top Pane Bar
    L   Log Data button
	C   Sequence Number
	m   Minimum Value
	M   Maximum Value
	  Plotting options
	D   Direction
	c   Channel
	R   Real/Imaginery

  Bottom Pane Bar	
	A   Acquire Button
	B   Number of Scans to perform
	I   Interferogram/Spectrum selection
	Q   Quit Button
*/
int BW_handler(QW_EVENT_MSG *msg) {
  void *picture;

  switch (msg->hdr.key[0]) {
	case 'A':
	  WindowCurrent(base_wind_id);
	  acquire_data();
	  WindowCurrent(base_wind_id);
	  WindowBarCurrent('T', NULL);
	  ChangeOptions("R*", spec_collected ? "S-d" : "d-S");
	  if (spec_collected)
		ChangeState(plot_imag ? "R1" : "R0", 1);
	  else
		ChangeState("R*", 0);
	  Draw();
	  break;
	case 'B':
	  picture=EventPicture(msg);
	  if (picture==NULL) Tell("BW_handler", "No element in B msg");
	  else {
		ElementFirst(picture);
		n_scans = ElementNumber();
	  }
	  break;
	case 'c': /* Channel */
	  plot_chan = msg->hdr.key[1] - '0';
	  plot_opt();
	  break;
	case 'D': /* Direction */
	  plot_dir = msg->hdr.key[1] - '0';
	  plot_opt();
	  break;
	case 'R': /* Real/Imag */
	  plot_imag = msg->hdr.key[1] - '0'; /* 0 = real, 1 = imag */
	  if (spec_collected) plot_opt();
	  break;
	case 'I': /* Int/Spec */
	  collect_spec = msg->hdr.key[1] - '0'; /* 0 = Int, 1 = Spec */
	  break;
	case 'L':
	  if (msg->hdr.action == QW_CLICK && msg->hdr.code == 'S')
		log_data = msg->hdr.etc;
	  break;
	case 'Q':
	  exit(0);
	case '\0':
	  switch (msg->hdr.action) {
		case QW_RESIZED:
		  get_pane_size();
		  break;
		case QW_EXPOSED:
		  replot();
		  break;
		case QW_CLICK:
		  if (msg->hdr.code == 'S')
			rubber_band(msg);
		  break;
		default:
		  EventNotice("BW Handler No Key", msg);
		  break;
	  }
	  break;
	default:
	  EventNotice("BW Handler", msg);
	  break;
  }
  return 1;
}

void New_Base_Window(void) {
  base_pict_id = PictureOpen("bomem", NULL, NULL, QW_WHITE, QW_SOLID_PAT, NULL, NULL);
  base_wind_id = Window("bomem", "@bomem", NULL, NULL);
  set_win_handler(base_wind_id, BW_handler);
  get_pane_size();

  update_sequence();
  update_n_scans();
  update_int_spec();

  WindowCurrent(base_wind_id);
  WindowBarCurrent('T', NULL);
  ChangeState("L", log_data);
  ChangeState(plot_dir ? "D1" : "D0", 1);
  ChangeState(plot_chan ? "c1" : "c0", 1);
  ChangeState(plot_imag ? "R1" : "R0", 1);
  WindowBarCurrent('B', NULL);
  ChangeOptions("A", "d-S");
  ChangeState(collect_spec ? "I1" : "I0", 1);
  Draw();
}
