/* rtgapi.c Functions to access RTG in realtime
 * $Log$
 */
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <sys/name.h>
#include <sys/kernel.h>
#include "nortlib.h"
#include "rtgapi.h"

#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

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
	stored_pid = qnx_name_locate(getnid(), RTG_NAME, 0, NULL);
	if (stored_pid == -1 && last_try < 0)
	  nl_error(1, "Unable to locate RTG");
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

/* returns 0 on success, non-zero on failure */
int rtg_report(rtg_t *rtg, double X, double Y) {
  if (rtg == 0)
	return 1;
  if (rtg->pid != stored_pid) {
	rtg->pid = stored_pid;
	rtg->initialized = 0;
	rtg->deleted = 0;
  }
  if ((rtg->initialized || rtg_register(rtg, X)) && !rtg->deleted) {
	int rv;
	short int dltd;

	rtg->msg.u.pt.X = X;
	rtg->msg.u.pt.Y = Y;
	rv = Send(rtg->pid, &rtg->msg, &dltd, sizeof(rtg->msg), sizeof(dltd));
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
  }
  return 1;
}

/* returns 0 on success, non-zero on failure */
int rtg_sequence(rtg_t *rtg, double X0, double dX, int n_pts, double *Y) {
  if (rtg == 0)
	return 1;
  if (rtg->pid != stored_pid) {
	rtg->pid = stored_pid;
	rtg->initialized = 0;
	rtg->deleted = 0;
  }
  if ((rtg->initialized || rtg_register(rtg, X0)) && !rtg->deleted) {
	###
  }
}
