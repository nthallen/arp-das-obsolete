/* channels.c provides channel-specific functions
 */

/* channel_menu() brings up the channels menu (or possibly the
   "create channel" dialog if no channels are defined).
   If a callback function is specified, it is called when a
   channel is selected from the menu with the channel name.
   I haven't yet figured out the boundary conditions (what
   to do when the dialog is cancelled, e.g.) A logical thing
   to do is call the callback with NULL.

   Being a menu, multiple instantiations are pretty much ruled
   out by the server. It's not clear what the implications of
   that are... But the first is that I don't have to keep
   track of invocations. If a second is requested, I don't
   need to worry whether the first is still active.
   If I select menu, it might bring up "properties,delete,create"
   Menu may be too restrictive: try other options.
*/
#include <windows/Qwindows.h>
#include <unistd.h>
#include <string.h>
#include "nortlib.h"
#include "rtg.h"

void ss_channels(char *name);

static void (*cbmenufunc)(const char *, char) = NULL;

static int chan_key_handler(QW_EVENT_MSG *msg, char *label) {
  switch (label[1]) {
	case 'M':
	  if (cbmenufunc != NULL)
		cbmenufunc(msg->hdr.key, label[2]);
	  return(1);
	case 'C':
	  dummy_channel_create(EventText(msg));
	  return 1;
	case 'F': /* FileMenu Response */
	  { char *filename, *path, *ss_name;
		int i;
		
		filename = EventText(msg);
		path = EventPath(msg);
		if (filename != 0 && path != 0) {
		  chdir(path);
		  ss_name = nl_strdup(filename);
		  i = strlen(ss_name);
		  if (i < 4 || ss_name[i-4] != '.' || ss_name[i-3] != 's' ||
			  ss_name[i-2] != 'p' || ss_name[i-1] != 's')
			Tell("Spreadsheet Open", "Invalid Filename");
		  else {
			ss_name[i-4] = '\0';
			ss_channels(ss_name);
		  }
		  free_memory(ss_name);
		}
	  }
	  return 1;
	default:
	  return 0;
  }
}

static void init_handler(void) {
  static int handler_set = 0;
  
  if (!handler_set) {
	set_key_handler('C', chan_key_handler);
	handler_set = 1;
  }
}

void channel_menu( char *title, void (* callback)(const char *, char),
					char bw_ltr) {
  char menu_label[4];

  menu_label[0] = 'C'; menu_label[1] = 'M'; menu_label[2] = bw_ltr;
  menu_label[3] = '\0';
  if (title == NULL) title = "Channels";
  cbmenufunc = callback;
  DialogAt(-420, -200, "cC", NULL);
  /* Menu( "CM", title, 1, "A;B;C", "O"); */
  Draw_channel_menu( menu_label, title );
  init_handler();
}

static void edit_channel_props(const char *channel, char bw_ltr) {
  if (channel != NULL)
	Tell("Channel Properties", channel);
}

static void chandelete(const char *channel, char bw_ltr) {
  channel_delete(channel);
}

/* This is the function called from basewin menu */
void channel_opts( int key, char bw_ltr ) {
  char *function;

  switch (key) {
	case 'C':
	  Prompt("CC", NULL, 20, NULL, "OK|DONE;CANCEL", NULL,
		  "Enter New Channel Name:");
	  init_handler();
	  return;
	case 'D':
	  channel_menu("Delete Channel", chandelete, bw_ltr);
	  return;
	case 'P':
	  channel_menu("Properties", edit_channel_props, bw_ltr);
	  return;
	case 'S': /* Channels/Spreadsheet: bring of file menu */
	  FileMenu("CF", "Open Spreadsheet", NULL, ".", "*.sps", "Cb-s", NULL, NULL);
	  init_handler();
	  return;
	default:
	  function = "unknown!";
  }
  Tell("Channel Opts", function);
}


#ifdef DOESNOTWORK
  static int pict_id = 0;
  /* Dialog( "C", NULL, NULL, NULL, "<Channels>A;B|B;CDE|C", "AOcb"); */
  /* Try the picture approach: Things to try:
     1: try different picture types
	 2: try opening window with WindowOpen() and Dialog() what's the diff?
	 3: In dialog, try types CMD, PROP, and MENU
   */
  if (pict_id == 0) {
	pict_id = PictureOpen( "chmenu", NULL, "Channels", 0, 0, NULL, ";MENU");
	DrawGroup(0, 0, 7, "LXV", "SmiN", "ChGrp");
	DrawText("One", 0, 0, 0, NULL, "One");
	DrawText("Two", 0, 0, 0, NULL, "Two");
	DrawText("Three", 0, 0, 0, NULL, "Three");
	DrawEnd("ChGrp");
  }
  WindowAt(0,0,"cC",NULL);
  WindowOpen("ChMnW", 0, 0, "bOPQs-i;s-S", "C", NULL, pict_id);
  
  /* Dialog( "C", NULL, NULL, NULL, "$chmenu", "AOcb"); */
#endif
