/* chanprop.c handles channel properties menus.
 * This is work very much in progress. I would like to have the
 * nested properties handled entirely by the dialog manager, but
 * that doesn't seem to work at present, so I will have to handle
 * dispatching of the sub-dialogs.
 
 Present Strategy:
 I will allow only one instance of the channel properties dialog.
 A second invocation will instead update the first. Correspondingly,
 I should allow only one instance each of X and Y axis properties.
 This won't work, since I need to access axis props from graph
 props also.
*/
#include <windows/Qwindows.h>
#include "rtg.h"

/* These are the variables which define the state of this module.
   I think I will support only one copy of each dialog. If a
   second call comes in for one, I will simply activate the
   existing window and change the parameters.
*/
static chandef *channel;

/* These #defines identify the dialog windows and pictures */
#define WIND    SRCDIR "chanprop.wnd"
#define WINDNM  "chprwn"
#define PICT    SRCDIR "chanprop.pict"
#define PICTNM  "chprpt"
#define CP_LABEL "PC"

/* Standard Property Dialog buttons: */
static char *bars[4] = { NULL, "Apply|A;Reset|CANCEL", NULL, NULL };

static void chanprop_cancel(char *options) {
  if (DialogCancel(CP_LABEL, options)) {
	axisprop_delete(AP_CHANNEL_X);
	axisprop_delete(AP_CHANNEL_Y);
  }
}

static int chanprop_handler(QW_EVENT_MSG *msg, char *label) {
  switch (label[1]) {
	case 'C': /* Channel Prop responses */
	  if (msg->hdr.action == QW_DISMISS) {
		chanprop_cancel("P");
	  } else switch (msg->hdr.key[0]) {
		case 'A': /* Apply changes */
		  chanprop_cancel(NULL);
		  break;
		case 'X':
		  axisprop_dialog(AP_CHANNEL_X, channel->name);
		  break;
		case 'Y':
		  axisprop_dialog(AP_CHANNEL_Y, channel->name);
		  break;
		default:
		  switch (msg->hdr.action) {
			case QW_CLOSED:
			case QW_CANCELLED:
			  if (!DialogCurrent(CP_LABEL)) {
				axisprop_delete(AP_CHANNEL_X);
				axisprop_delete(AP_CHANNEL_Y);
			  }
			  break;
			default:
			  EventNotice("ChanProp PC", msg);
		  }
		  break;
	  }
	  break;
	default:
	  EventNotice("ChanProp P[^C]", msg);
	  break;
  }
  return 1;
}

static void init_handler(void) {
  static int handler_set = 0;
  
  if (!handler_set) {
	set_key_handler('P', chanprop_handler);
	handler_set = 1;
  }
}

void chanprop_dialog(const char *chname) {
  static int dialog_pict, window_pict;

  channel = channel_props(chname);
  if (channel == 0) return;
  if (DialogCurrent(CP_LABEL)) {
	/* Want to make the existing dialog active eventually */
	int wind_id;
	if (wind_id = WindowFind(CP_LABEL, "G")) {
	  WindowCurrent(wind_id);
	  WindowBarCurrent('T', NULL);
	  ChangeText("SName", chname, 0, -1, 0);
	  WindowState("AS");
	  axisprop_update(AP_CHANNEL_X, chname);
	  axisprop_update(AP_CHANNEL_Y, chname);
	} else {
	  Tell("CP_dialog", "No DialogID");
	  return;
	}
  } else {
	/* Find the channel */
	if (window_pict == 0)
	  window_pict = PictureOpen(WINDNM, WIND, NULL, 0, 0, NULL, "R");
	else
	  PictureCurrent(window_pict);
	ChangeText("SName", chname, 0, -1, 0);
	if (dialog_pict == 0)
	  dialog_pict = PictureOpen(PICTNM, PICT, NULL, 0, 0, NULL, "R");
	else
	  PictureCurrent(dialog_pict);
	Dialog(CP_LABEL, "Channel Properties", "$" WINDNM, bars,
										   "$" PICTNM, "cp-b");
	init_handler();
  }
}

void chanprop_delete(chandef *chan) {
  if (chan == channel && DialogCurrent(CP_LABEL))
	chanprop_cancel("P");
}
