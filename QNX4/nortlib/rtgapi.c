/* rtgapi.c Functions to access RTG in realtime
 */
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <sys/name.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <stdlib.h>
#include "nortlib.h"
#include "rtgapi.h"
char rcsid_rtgapi_c[] =
  "$Header$";

/* rtg_register() attempts to contact RTG
   currently attempts only on the current node, although
   it will be desirable to contact another node, just need
   to figure out the API for that...
   This routine will also handle retries, actually looking
   every, say, 20 seconds.
*/
static pid_t stored_pid = -1;
static pid_t rtg_pid(double time) {
  static double last_try = -100.;
  
  if (stored_pid == -1 &&  time - last_try > 20.) {
	nid_t rtg_node = 0;
	char *ntext = getenv( "RTGNODE" );
	
	if ( ntext != NULL )
	  rtg_node = strtol( ntext, NULL, 0 );
	if ( rtg_node <= 0 ) rtg_node = getnid();
	stored_pid = qnx_name_locate(rtg_node, RTG_NAME, 0, NULL);
	if (stored_pid == -1 && last_try < 0)
	  nl_error(1, "Unable to locate RTG on node %d", rtg_node );
	if (stored_pid != -1 && last_try >= 0)
	  nl_error(0, "Restablished connection to RTG");
	last_try = time;
  }
  return stored_pid;
}

/* If we can locate RTG, create a registration message and send it */
static int rtg_register(rtg_t *rtg, double time) {
  rtg_msg_t *rtmsg;
  
  rtg->pid = rtg_pid(time);
  if (rtg->pid != -1) {
	int length, rv;
	
	length = offsetof(rtg_msg_t, u.name) + strlen(rtg->name) + 1;
	rtmsg = new_memory(length);
	rtmsg->id = RTG_MSG_ID;
	rtmsg->ver = RTG_VERSION;
	rtmsg->module[0] = RTG_MOD_CDB;
	rtmsg->module[1] = RTG_CDB_CREATE;
	strcpy(rtmsg->u.name, rtg->name);
	rv = Send(rtg->pid, rtmsg, &rtg->msg.u.pt.channel_id, length,
	  sizeof(rtg->msg.u.pt.channel_id));
	if (rv == 0) {
	  rtg->initialized = 1;
	  if (rtg->msg.u.pt.channel_id == -1)
		rtg->deleted = 1;
	  else {
		rtg->deleted = 0;
		return 1;
	  }
	} else {
	  nl_error(1, "RTG connection lost on register");
	  rtg->pid = -1;
	}
  }
  return 0;
}

/* Create rtg_t structure and then call rtg_register */
rtg_t * rtg_init(char *name) {
  rtg_t *rtg;

  if (name == 0) return NULL;
  rtg = new_memory(sizeof(rtg_t));
  rtg->name = nl_strdup(name);
  rtg->pid = -1;
  rtg->msg.id = RTG_MSG_ID;
  rtg->msg.ver = RTG_VERSION;
  rtg->msg.module[0] = RTG_MOD_CDB;
  rtg->msg.module[1] = RTG_CDB_REPORT;
  rtg->deleted = 0;
  rtg->initialized = 0;
  rtg_register(rtg, 0.);
  return(rtg);
}

/* returns 0 on success, non-zero if RTG cannot be found */
static int rtg_check( rtg_t *rtg, double X ) {
  if (rtg == 0)
	return 1;
  if (rtg->pid != stored_pid) {
	rtg->pid = stored_pid;
	rtg->initialized = 0;
	rtg->deleted = 0;
  }
  if ((rtg->initialized || rtg_register(rtg, X)) && !rtg->deleted)
	return 0;
  return 1;
}

static int rtg_check_reply( int rv, short int dltd, rtg_t *rtg ) {
  if (rv == 0) {
	if (dltd == 1) return 0;
	else {
	  if (dltd == 0)
		rtg->deleted = 1;
	  else nl_error(1, "RTG replied %d to rtg_report", dltd);
	}
  } else {
	stored_pid = -1;
	nl_error(1, "Lost connection to RTG in rtg_report");
  }
  return 1;
}

/* returns 0 on success, non-zero on failure */
int rtg_report(rtg_t *rtg, double X, double Y) {
  int rv;
  short int dltd;
  
  if ( rtg_check( rtg, X ) )
	return 1;

  rtg->msg.u.pt.X = X;
  rtg->msg.u.pt.dXorY = Y;
  rv = Send(rtg->pid, &rtg->msg, &dltd, sizeof(rtg->msg), sizeof(dltd));
  return rtg_check_reply( rv, dltd, rtg );
}

/* returns 0 on success, non-zero on failure */
int rtg_sequence(rtg_t *rtg, double X0, double dX, int n_pts, float *Y) {
  struct _mxfer_entry mx[2], rx;
  short int dltd;
  int rv;

  if ( rtg_check( rtg, X0 ) )
	return 1;
  rtg->msg.u.pt.X = X0;
  rtg->msg.u.pt.dXorY = dX;
  rtg->msg.u.pt.n_pts = n_pts;
  _setmx( &mx[0], &rtg->msg, offsetof(rtg_msg_t, u.pt.Y) );
  _setmx( &mx[1], Y, n_pts * sizeof(float) );
  _setmx( &rx, &dltd, sizeof( dltd ) );
  rtg->msg.module[1] = RTG_CDB_SEQUENCE;
  rv = Sendmx( rtg->pid, 2, 1, &mx[0], &rx );
  rtg->msg.module[1] = RTG_CDB_REPORT;
  return rtg_check_reply( rv, dltd, rtg );
}

/*
=Name rtg_init(): Initialize connection to RTG
=Subject Realtime Graphics

=Synopsis

  #include "rtgapi.h"

  rtg_t * rtg_init(char *name);

=Description

  Establishes a connection between the client application and the
  RTG realtime graphics server. The name specified is the channel
  name. rtg_init() must be called once for each distinct data
  channel.

  The RTG server is located by means of its registered name,
  "huarp/rtg". rtg_init() will look for this name on the current
  node or on the node specified by the environment variable
  RTGNODE.

=Returns

  A pointer to a structure identifying the channel. This identifier
  is passed to either the =rtg_report=() or =rtg_sequence=()
  function when data is ready. Even if the server is not located,
  an identifier will be returned. If a server subsequently comes
  up, the connection will be reattempted.

=SeeAlso

  =rtg_report=(), =rtg_sequence=(),
  <a HREF="http://www.arp.harvard.edu/das/manuals/rtg.html">
  RTG</A>,
  <a HREF="http://www.arp.harvard.edu/das/manuals/tmg.html">
  TMG: A TMC Preprocessor for interfacing to RTG</A>.

=End

=Name rtg_report(): Report Single Data Point to RTG
=Subject Realtime Graphics

=Synopsis

  #include "rtgapi.h"

  int rtg_report(rtg_t *rtg, double X, double Y);

=Description

  Sends a single data point to RTG.

=Returns

  Returns 0 on success, non-zero
  on error. All errors are non-fatal. Even if the RTG server dies,
  subsequent calls to rtg_report() can connect to a new invocation
  of the server.

=SeeAlso

  =rtg_init=(), =rtg_sequence=(),
  <a HREF="http://www.arp.harvard.edu/das/manuals/rtg.html">
  RTG</A>,
  <a HREF="http://www.arp.harvard.edu/das/manuals/tmg.html">
  TMG: A TMC Preprocessor for interfacing to RTG</A>.

=End

=Name rtg_sequence(): Report Entire Sequence to RTG
=Subject Realtime Graphics

=Synopsis

  #include "rtgapi.h"

  int rtg_sequence(rtg_t *rtg, double X0, double dX,
		int n_pts, float *Y);

=Description

  Reports a sequence of data points to RTG. The rtg argument is the
  identifier returned by =rtg_init=(). The data points have an
  implicit X-coordinate, based on the X0 and dX paramaters. Y
  points to an array of n_pts float values.

=Returns

  Returns 0 on success, non-zero on failure. As with
  =rtg_report=(), all errors are non-fatal.

=SeeAlso

  =rtg_init=(), =rtg_report=(),
  <a HREF="http://www.arp.harvard.edu/das/manuals/rtg.html">
  RTG</A>,
  <a HREF="http://www.arp.harvard.edu/das/manuals/tmg.html">
  TMG: A TMC Preprocessor for interfacing to RTG</A>.

=End
*/
