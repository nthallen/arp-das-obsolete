/* cic.c Defines functions used by Command Interpreter Clients
 * $Log$
 * Revision 1.4  1994/02/15  19:01:55  nort
 * Added -p option from tma.c
 * Improved syntax error reporting
 *
 * Revision 1.3  1993/09/15  19:25:53  nort
 * Using nl_make_name()
 *
 * Revision 1.2  1993/07/01  15:35:04  nort
 * Eliminated "unreferenced" via Watcom pragma
 *
 * Revision 1.1  1993/02/10  02:05:16  nort
 * Initial revision
 *
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include "nortlib.h"
#include "cmdalgo.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static pid_t cis_pid = 0;
static char cic_header[CMD_PREFIX_MAX] = "CIC";
static nid_t cis_node = 0;
static unsigned char cic_msg_type;
static unsigned short cic_reply_code;
static struct _mxfer_entry cic_msgs[3], cic_reply;
static int playback = 0;

void cic_options(int argcc, char **argvv, const char *def_prefix) {
  int c;
  extern char *opt_string;

  if (def_prefix != NULL) {
	strncpy(cic_header, def_prefix,	CMD_PREFIX_MAX);
	cic_header[CMD_PREFIX_MAX-1] = '\0';
  }
  optind = 0;
  opterr = 0;
  if (argcc > 0) do {
	c=getopt(argcc,argvv,opt_string);
	switch (c) {
	  case 'C':
		cis_node = atoi(optarg);
		break;
	  case 'h':
		strncpy(cic_header, optarg, CMD_PREFIX_MAX);
		cic_header[CMD_PREFIX_MAX-1] = '\0';
		break;
	  case 'p':
		playback = 1;
		break;
	  case '?':
		nl_error(3, "Unknown option -%c", optopt);
	  default:
		break;
	}
  } while (c!=-1);
  opterr = 1;
}

/* cic_init() Locates the Command Interpreter Server (CIS) using
   either the default node information or the information set
   by cic_options -C <node>. Once located, if ci_version is
   non-empty, the version is queried. cic_init() uses the
   standard nl_response codes to determine whether fatal
   response is required. Returns zero on success.
*/
int cic_init(void) {
  cis_pid = nl_find_name(cis_node, nl_make_name(CMDINTERP_NAME, 0));
  if (cis_pid == -1) {
	cis_pid = 0;
	return(1);
  }
  
  /* Initialize the first two message parts once and for all */
  _setmx(&cic_msgs[0], &cic_msg_type, 1);
  _setmx(&cic_msgs[1], &cic_header, CMD_PREFIX_MAX);
  _setmx(&cic_reply, &cic_reply_code, sizeof(cic_reply_code));
  
  /* If specified, verify version */
  if (ci_version[0] != '\0') {
	char version[CMD_VERSION_MAX];

	if (cic_query(version) == 0) {
	  if ( version[0] != '\0' &&
		   strncmp(version, ci_version, CMD_VERSION_MAX) != 0 ) {
		if (nl_response)
		  nl_error(nl_response, "Incorrect Command Server Version");
		return(1);
	  }
	} else return(1);
  }
  return(0);
}

/* cic_query queries the command server for version information.
   Returns zero on success.
*/
int cic_query(char *version) {
  ci_ver v;
  int rv;

  if (cis_pid == 0 && cic_init() != 0) return(1);
  cic_msg_type = CMDINTERP_QUERY;
  _setmx(&cic_msgs[2], &v, sizeof(v));
  rv = Sendmx(cis_pid, 2, 1, &cic_msgs[0], &cic_msgs[2]);
  if (rv == -1) {
	if (nl_response)
	  nl_error(nl_response, "Error sending to Command Server");
  } else if (v.msg_type != 0) {
	if (nl_response)
	  nl_error(nl_response, "Invalid response from CI Server");
  } else {
	strncpy(version, v.version, CMD_VERSION_MAX);
	return(0);
  }
  return(1);
}

/* ci_sendcmd() Sends a command to the CIS.
   If cmdtext==NULL, sends CMDINTERP_QUIT,0
    mode == 0 ==> CMDINTERP_SEND
	mode == 1 ==> CMDINTERP_TEST
	mode == 2 ==> CMDINTERP_SEND_QUIET
   Possible errors:
	 Unable to locate CIS: Normally fatal: return 1
	 CMDREP_QUIT from CIS: Reset cis_pid: return it
	 CMDREP_SYNERR from CIS: Normally fatal: return it
	 CMDREP_EXECERR from CIS: Normally warning: return it
*/
int ci_sendcmd(const char *cmdtext, int mode) {
  unsigned sparts, clen;
  int rv;
  
  if (!playback && cis_pid == 0 && cic_init() != 0) return(1);
  if (cmdtext == NULL) {
	cic_msg_type = CMDINTERP_QUIT;
	sparts = 2;
	clen = 0;
	nl_error(-3, "Sending Quit to Server");
  } else {
	switch (mode) {
	  case 1: cic_msg_type = CMDINTERP_TEST; break;
	  case 2: cic_msg_type = CMDINTERP_SEND_QUIET; break;
	  default: cic_msg_type = CMDINTERP_SEND; break;
	}
	clen = strlen(cmdtext);
	{ int len = clen;
	  if (len > 0 && cmdtext[len-1]=='\n') len--;
	  nl_error(-3, "%*.*s", len, len, cmdtext);
	}
	clen++;
	if (clen > CMD_INTERP_MAX) {
	  if (nl_response)
		nl_error(nl_response, "Command too long: %s", cmdtext);
	  return(CMDREP_SYNERR + CMD_INTERP_MAX);
	}
	_setmx(&cic_msgs[2], cmdtext, clen);
	sparts = 3;
  }
  if (playback) return(0);
  rv = Sendmx(cis_pid, sparts, 1, cic_msgs, &cic_reply);
  if (rv == -1) {
	switch (errno) {
	  case EFAULT:
		nl_error(4, "Ill-formed Sendmx in cic_sendcmd");
	  case EINTR:
		if (nl_response)
		  nl_error(1, "cic_sendcmd interrupted");
		break;
	  case ESRCH:
		if (nl_response)
		  nl_error(nl_response, "Cmd Server Terminated");
		cis_pid = 0;
		break;
	}
	return(1);
  }
  
  if (cic_reply_code != 0) {
	if (cic_reply_code == CMDREP_QUIT) cis_pid = 0;
	else if (cic_reply_code >= CMDREP_EXECERR) {
	  if (nl_response)
		nl_error(1, "Execution error %d from Cmd Server",
				cic_reply_code - CMDREP_EXECERR);
	} else if (cic_reply_code >= CMDREP_SYNERR) {
	  if (nl_response && clen) {
		clen--;
		if (clen > 0 && cmdtext[clen-1]=='\n') clen--;
		nl_error(2, "Syntax Error in ci_sendcmd:");
		nl_error(2, "%*.*s", clen, clen, cmdtext);
		nl_error(2, "%*s", cic_reply_code - CMDREP_SYNERR, "^");
	  }
	} else nl_error(4, "Unexpected reply %d from Cmd Server",
				cic_reply_code);
  }
  return(cic_reply_code);
}
