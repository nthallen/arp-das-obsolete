/* cic.c Defines functions used by Command Interpreter Clients
 * $Log$
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include "nortlib.h"
#include "cmdalgo.h"
static char rcsid[] = "$Id$";

static pid_t cis_pid = 0;
static char cic_header[CMD_PREFIX_MAX] = "CIC";
static nid_t cis_node = 0;
static unsigned char cic_msg_type;
static unsigned short cic_reply_code;
static struct _mxfer_entry cic_msgs[3], cic_reply;

void cic_options(int argcc, char **argvv, char *def_prefix) {
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
  cis_pid = nl_find_name(cis_node, CMDINTERP_NAME);
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
	ci_ver v;
	int rv;
	
	cic_msg_type = CMDINTERP_QUERY;
	_setmx(&cic_msgs[2], &v, sizeof(v));
	rv = Sendmx(cis_pid, 2, 1, &cic_msgs[0], &cic_msgs[2]);
	if (rv == -1) {
	  if (nl_response)
		nl_error(nl_response, "Error sending to Command Server");
	  return(1);
	} else {
	  if (strncmp(v.version, ci_version, CMD_VERSION_MAX) != 0) {
		if (nl_response)
		  nl_error(nl_response, "Incorrect Command Server Version");
		return(1);
	  }
	}
  }
  return(0);
}

/* ci_sendcmd() Sends a command to the CIS.
   If cmdtext==NULL, sends CMDINTERP_QUIT,0
   Possible errors:
	 Unable to locate CIS: Normally fatal: return 1
	 CMDREP_QUIT from CIS: Reset cis_pid: return it
	 CMDREP_SYNERR from CIS: Normally fatal: return it
	 CMDREP_EXECERR from CIS: Normally warning: return it
*/
int ci_sendcmd(char *cmdtext, int test) {
  unsigned sparts, clen;
  int rv;
  
  if (cis_pid == 0 && cic_init() != 0) return(1);
  if (cmdtext == NULL) {
	cic_msg_type = CMDINTERP_QUIT;
	sparts = 2;
  } else {
	cic_msg_type = test ? CMDINTERP_TEST : CMDINTERP_SEND;
	clen = strlen(cmdtext) + 1;
	if (clen > CMD_INTERP_MAX) {
	  if (nl_response)
		nl_error(nl_response, "Command to long: %s", cmdtext);
	  return(CMDREP_SYNERR + CMD_INTERP_MAX);
	}
	_setmx(&cic_msgs[2], cmdtext, clen);
	sparts = 3;
  }
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
		  nl_error(nl_response, "Cmd Interpreter Terminated");
		cis_pid = 0;
		break;
	}
	return(1);
  }
  
  if (cic_reply_code != 0) {
	if (cic_reply_code == CMDREP_QUIT) cis_pid = 0;
	else if (cic_reply_code >= CMDREP_EXECERR && nl_response)
	  nl_error(1, "Execution error %d from Cmd Interpreter",
				cic_reply_code - CMDREP_EXECERR);
	else if (cic_reply_code >= CMDREP_SYNERR && nl_response)
	  nl_error(nl_response, "CMDREP_SYNERR: char %d in \"%s\"",
				cic_reply_code - CMDREP_SYNERR, cmdtext);
	else nl_error(4, "Unexpected reply %d from Cmd Interpreter",
				cic_reply_code);
  }
  return(cic_reply_code);
}
