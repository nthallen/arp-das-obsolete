/* colsend.c contains Col_send_init() and Col_send(), since one is
   useless without the other. Col_send_reset() will probably go in
   here also because I'm a little lazy, though it should be used
   together with the other two, it is kinda optional also.
   
   See also collect.h for a description of the message-level protocol.
   
   My strategy with these functions is to be verbose with the init,
   but to be quiet with the other functions. i.e. once a connection
   is established, these functions will not fail in a big way.
   If the DG were to go away, the Sendmx() would fail and I would
   return an error code, but I won't be dying due to nl_response
   for that kind of failure.
*/
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <stddef.h>
#include <string.h>
#include "collect.h"
#include "nortlib.h"
#include "globmsg.h"
char rcsid_colsend_c[] =
  "$Header$";

static pid_t dg_tid = -1;

/* Establishes connection with collection */
send_id Col_send_init(const char *name, void *data, unsigned short size) {
  struct colmsg msg;
  int length;
  send_id sender = NULL;
  
  length = strlen(name) + offsetof(struct colmsg, u.name) + 1;
  if (length <= MAX_COLMSG_SIZE) {
	/* build up the message */
	msg.type = COL_SEND;
	msg.id = COL_SEND_INIT;
	strcpy(msg.u.name, name);

	/* send it */
	if (dg_tid == -1)
	  dg_tid = find_DG();
	if (dg_tid == -1) return NULL;
	if (send_DG(&msg, length) == 0) {
	  if (msg.type == DAS_OK) {
		struct colmsg *hdr;
		/* build up the send_id */
		sender = new_memory(sizeof(send_id_struct));
		sender->hdr = hdr =
		  new_memory(offsetof(struct colmsg, u.data.data));
		hdr->type = COL_SEND;
		hdr->id = COL_SEND_SEND;
		hdr->u.data.id = msg.u.data.id;
		hdr->u.data.size =
		  size < msg.u.data.size ? size : msg.u.data.size;
		_setmx(&sender->mx[0], hdr, offsetof(struct colmsg, u.data.data));
		_setmx(&sender->mx[1], data, hdr->u.data.size);
		_setmx(&sender->mx[2], &sender->rv, 1);
		return sender;
	  } else if (msg.type == DAS_BUSY && nl_response) {
		nl_error(nl_response,
		  "Send Registration for %s in use", name);
		return NULL;
	  }
	}
	if (nl_response)
	  nl_error(nl_response,
		"Send Registration for %s failed to send to DG", name);
  }
  return NULL;
}

/* return 0 on success, non-zero otherwise
   Failure of the send is quiet, but causes an error return.
 */
int Col_send(send_id sender) {
  if (dg_tid != -1 && sender != 0) {
	if (Sendmx(dg_tid, 2, 1, sender->mx, &sender->mx[2]) == 0 &&
		sender->rv == DAS_OK)
	  return 0;
  }
  return 1;
}

/* returns zero on success, non-zero otherwise. Quiet */
int Col_send_reset(send_id sender) {
  if (sender != 0) {
	sender->hdr->id = COL_SEND_RESET;
	if (dg_tid != -1 &&
		Sendmx(dg_tid, 1, 1, sender->mx, &sender->mx[2]) == 0 &&
		sender->rv == DAS_OK) {
	  if (sender->hdr != 0) free_memory(sender->hdr);
	  free_memory(sender);
	  return 0;
	}
  }
  return 1;
}
/*
=Name Col_send_init(): Initialize communication with collection
=Subject Data Collection
=Subject Startup
=Name Col_send(): Send data to collection
=Subject Data Collection
=Name Col_send_reset(): Terminate communication with collection
=Subject Data Collection

=Synopsis

#include "collect.h"
send_id Col_send_init(const char *name, void *data, unsigned short size);
int Col_send(send_id sender);
int Col_send_reset(send_id sender);

=Description

  These functions define the standard approach for sending data
  to collection. This is necessary whenever an auxilliary program
  is generating data which needs to be fed into the telemetry
  frame.<P>
  
  Col_send_init() initializes the connection, creating a data
  structure to hold all the details. The name string must match the
  name specified in the corresponding 'TM "Receive"' statement in
  the collection program. Data points to where the data will be
  stored, and size specifies the size of the data.<P>
  
  Whenever the data is updated, Col_send() should be called to
  forward the data to collection.<P>
  
  On termination, Col_send_reset() may be called to provide an
  orderly shutdown.

=Returns

  Col_send_init() returns a pointer to a structure which holds
  the state of the connection. This pointer must be passed to
  Col_send() or Col_send_reset(). On error, Col_send_init() will
  return NULL unless =nl_response= indicates a fatal response.<P>
  
  Col_send() returns 0 on success, non-zero otherwise.
  Failure of the send is quiet, but causes an error return.<P>
  
  Col_send_reset() returns 0 on success, non-zero otherwise. It
  does not issue any diagnostic messages, since failure is
  expected to be fairly common (Collection may quit before we get
  around to our termination.)

=SeeAlso

  =Data Collection= functions.

=End
*/
