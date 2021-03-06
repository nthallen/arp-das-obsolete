/* cic.c Defines functions used by Command Interpreter Clients */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include "nortlib.h"
#include "cmdalgo.h"
char rcsid_cic_c[] =
  "$Header$";

static pid_t cis_pid = 0;
static char cic_header[CMD_PREFIX_MAX] = "CIC";
static nid_t cis_node = 0;
static unsigned char cic_msg_type;
static unsigned short cic_reply_code;
static struct _mxfer_entry cic_msgs[3], cic_reply;
static int playback = 0;
int cgc_forwarding;

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
	  if ( version[0] == '\0') cgc_forwarding = 1;
	  else if ( strncmp(version, ci_version, CMD_VERSION_MAX) != 0 ) {
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

static long int ci_time = 0L;
void ci_settime( long int time ) {
  ci_time = time;
}

const char *ci_time_str( void ) {
  static char buf[11];
  int hour, min, sec;
  long int time = ci_time;

  if ( ! playback || time == 0 ) return "";
  if ( time < 0 ) return "-1: ";
  time = time % ( 24 * 3600L );
  hour = time / 3600;
  time = time % 3600;
  min = time / 60;
  sec = time % 60;
  sprintf( buf, "%02d:%02d:%02d: ", hour, min, sec );
  return buf;
}

/* ci_sendcmd() Sends a command to the CIS.
   If cmdtext==NULL, sends CMDINTERP_QUIT,0
    mode == 0 ==> CMDINTERP_SEND
	mode == 1 ==> CMDINTERP_TEST
	mode == 2 ==> CMDINTERP_SEND_QUIET
   Possible errors:
	 Unable to locate CIS: Normally fatal: return 1
	 CMDREP_QUIT from CIS: Reset cis_pid: return it
	 CMDREP_SYNERR from CIS: Normally error: return it
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
	  nl_error( cic_msg_type == CMDINTERP_SEND_QUIET ? -4 : -3,
		  "%s%*.*s", ci_time_str(), len, len, cmdtext);
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

/*
=Name cic_options(): Command line initializations for Command Clients
=Subject Command Server and Client
=Subject Startup
=Synopsis

#include "nortlib.h"
void cic_options(int argcc, char **argvv, const char *def_prefix);

=Description

  cic_options() handles command-line initializations for Command
  Clients (i.e. processes which will be sending commands to a
  Command Server generated via CMDGEN.). It handles the 'C', 'h'
  and 'p' options. ('h' actually belongs to the msg library, but
  if it has been specified, we want to know about it.)

=Returns
  Nothing.

=SeeAlso
  =Command Server and Client= functions.

=End

=Name cic_init(): Locate Command Server
=Subject Command Server and Client
=Subject Startup
=Synopsis
#include "nortlib.h"
int cic_init(void);

=Description
cic_init() Locates the Command Interpreter Server (CIS) using
either the default node information or the information set
by cic_options -C node. Once located, if ci_version is
non-empty, the version is queried. cic_init() uses the
standard =nl_response= codes to determine whether fatal
response is required.

=Returns
Returns zero on success.

=SeeAlso
  =Command Server and Client= functions.

=End

=Name cic_query(): Get version information from Command Server
=Subject Command Server and Client
=Synopsis
#include "nortlib.h"
int cic_query(char *version);

=Description
  cic_query queries the command server for version information.
  If successful, the version of the server is written into the
  buffer pointed to by the version argument.

=Returns
  Returns zero on success.

=SeeAlso
  =Command Server and Client= functions.

=End

=Name ci_settime(): Update command client's time
=Subject Command Server and Client
=Name ci_time_str(): Retrieve algorithm time string
=Subject Command Server and Client
=Synopsis
#include "nortlib.h"
void ci_settime( long int time );
const char *ci_time_str( void );

=Description

ci_settime is an internal function used to update a command
client's time. This is used during playback of algorithms to
allow outbound commands to be logged with their TM time instead
of the current time.

ci_time_str() returns a string which is appropriate for
output of algorithm time when in playback mode. If not
in playback mode, or if ci_settime() has not yet been
called, an empty string is returned.

=Returns
  ci_settime() return nothing. ci_time_str() returns a
  string.

=SeeAlso
  =Command Server and Client= functions.

=End

=Name ci_sendcmd(): Send command to Command Server
=Subject Command Server and Client
=Synopsis
#include "nortlib.h"
#include "cmdalgo.h"

int ci_sendcmd(const char *cmdtext, int mode);

=Description

ci_sendcmd() is the basic method to send a command to a
CMDGEN-generated Command Server. cmdtext is a NUL-terminated
ASCII string. It should include any terminating newline (e.g.
"Quit\n") or the command will not actually be executed. The
string must conform to the syntax specified in the CMDGEN input
for the particular server or a syntax error will occur.<P>

If cmdtext is NULL, a syntax-independent quit request is sent to
the server.<P>

The mode argument takes on the following values:

<DL>
  <DT>CMDINTERP_SEND (0)
  <DD>Execute the command and log it to memo via msg().
  <DT>CMDINTERP_TEST (1)
  <DD>Check the command for syntax only, but do not execute it
  or log it.
  <DT>CMDINTERP_SEND_QUIET (2)
  <DD>Execute the command but do not log it to memo.
</DL>

=Returns
  Returns zero on success. If the command server cannot be
  located (and =nl_response= is set below 3) returns 1.
  Otherwise, the return value is the sum of a type code and a
  value. The possible types are CMDREP_QUIT, CMDREP_SYNERR and
  CMDREP_EXECERR. CMDREP_QUIT indicates that the command you sent
  was a quit request, which probably means the client should shut
  down also. CMDREP_SYNERR indicates there was a syntax error in
  the command you sent. The value portion of the return code is
  the offset within the command where the error occurred.
  CMDREP_EXECERR indicates that an error occurred when the server
  was executing the instructions associated with the command. The
  value portion of the return code is the error code.

=SeeAlso
  =ci_sendfcmd=(), =ci_settime=(), =ci_time_str=(), and
  =Command Server and Client= functions.

=End
*/
