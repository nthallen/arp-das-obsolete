/* winmgr.c The Qwindows manager
 * $Log$
 * Revision 1.3  1994/11/28  17:30:51  nort
 * Rough separation of acquisition and windows
 *
 * Revision 1.2  1994/11/23  21:39:30  nort
 * Figured out a good loop for checking for incoming
 * messages while doing something else.
 *
 * Revision 1.1  1994/11/23  21:25:41  nort
 * Initial revision
 *
 */
#include <windows/Qwindows.h>
#include <sys/kernel.h>
#include "bomem.h"
#include "bomemw.h"
#include "nortlib.h"

#pragma off (unreferenced)
  static char
	rcsid[] = "$Id$";
#pragma on (unreferenced)

typedef struct whdlr {
  struct whdlr *next;
  int key;
  event_handler *handler;
} winhndlr;

/* windows sorted by increasing window_ids */
static winhndlr *winhndlrs = NULL, *keyhndlrs = NULL;

static void
	add_handler(int key, winhndlr **wh, event_handler *handler) {
  winhndlr *wp, **wpp, *nw;
  
  wpp = wh;
  for (wp = *wpp; wp != NULL && wp->key < key; ) {
	wpp = &wp->next;
	wp = *wpp;
  }
  nw = (winhndlr *) new_memory(sizeof(winhndlr));
  nw->next = wp;
  *wpp = nw;
  nw->key = key;
  nw->handler = handler;
}

static void del_handler(int key, winhndlr **wh) {
  winhndlr *wp, **wpp;
  
  wpp = wh;
  for (wp = *wpp; wp != NULL && wp->key < key; ) {
	wpp = &wp->next;
	wp = *wpp;
  }
  if (wp != NULL && wp->key == key) {
	*wpp = wp->next;
	free_memory(wp);
  }
}

void set_win_handler(int window_id, event_handler handler) {
  add_handler(window_id, &winhndlrs, handler);
}

void del_win_handler(int window_id) {
  del_handler(window_id, &winhndlrs);
}

void set_key_handler(int keyltr, event_handler handler) {
  add_handler(keyltr, &keyhndlrs, handler);
}

void del_key_handler(int keyltr) {
  del_handler(keyltr, &keyhndlrs);
}

void Receive_Loop(void) {
  static QW_EVENT_MSG msg;
  int action, keyltr;
  char *label;
  pid_t pid;
  winhndlr *wp;
  
  for (;;) {
	for (pid = -1; pid == -1; ) {
	  if (plotting()) {
		pid = Creceive(0, &msg, sizeof(msg));
		if (pid == event_proxy) {
		  Trigger(event_proxy);
		  pid = GetEvent(event_proxy, &msg, sizeof(msg));
		}
	  } else pid = GetEvent(0, &msg, sizeof(msg));
	}
	action = Event(&msg);
	switch (action) {
	  case 0:
		if (msg.hdr.id == 'ob')
		  server_command(pid, (char *)&msg);
		else Tell("Receive_Loop",
		  "Unrecognized message received: %s", (char *)&msg);
		break;
	  case QW_QUIT:
	  case QW_TERMINATED:
		return;
	  case QW_CLOSED:
	  case QW_HELP:
	  default:
		/* First try the keyltr handlers */
		label = EventLabel(&msg);
		if (label != NULL) {
		  keyltr = *label;
		  for (wp = keyhndlrs;
			   wp != NULL && wp->key < keyltr;
			   wp = wp->next) ;
		  if (wp != NULL && wp->handler != NULL &&
			  wp->handler(&msg) != 0) break;
		}

		/* Now try the window handlers */
		for (wp = winhndlrs;
			 wp != NULL && wp->key < msg.hdr.window;
			 wp = wp->next) ;
		if (wp != NULL && wp->handler != NULL &&
			wp->handler(&msg) != 0) break;

		/* If all else fails... */
		if (action == QW_HELP)
		  Tell("HELP!", "Sorry, No Help is Available!");
		else EventNotice("Receive_Loop", &msg);
		break;
	}
  }
}
