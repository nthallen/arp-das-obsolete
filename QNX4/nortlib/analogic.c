#include <sys/vc_msg.h>
#include <malloc.h>
#include "analogic.h"
#include "nortlib.h"

/* Analogic A/D Board Client Routines:
  Unlike many of our clients, these actually need the ability to
  support multiple servers, since we are likely to run at least
  two different servers. As such, we'll have to dynamically
  create server definitions which the client will use to access
  the server.
  
  Would be handy to have a short mnemonic to use as a common base
  for these functions. I suppose cpci would be appropriate,
  although it was a poor choice on Analogic's part, since it
  refers to the form factor, not the board function, but I could
  claim it refers to the boards.
  
*/

Server_Def *cpci_init( char *name ) {
  Server_Def *cpci = new_memory( sizeof(Server_Def) );
  cpci->name = name;
  cpci->expand = 1;
  cpci->global = 1;
  cpci->response = 2;
  cpci->node = 0;
  cpci->pid = 0;
  cpci->connected = 0;
  cpci->disconnected = 0;
  return cpci;
}

int cpci_setup( Server_Def *cpci, analogic_setup_t *setup ) {
  analogic_msg_t msg, rep;
  struct _mxfer_entry sx[2], rx;
  _setmx( &sx[0], &msg, sizeof(msg) );
  _setmx( &sx[1], setup, sizeof(analogic_setup_t));
  _setmx( &rx, &rep, sizeof(rep) );
  msg.header = ANLGC_HEADER;
  msg.type = ANLGC_SETUP;
  if ( CltSendmx( cpci, 2, 1, sx, &rx ) )
	return ANLGC_E_SEND;
  else return rep.type;
}

int cpci_stop( Server_Def *cpci ) {
  analogic_msg_t msg, rep;
  msg.header = ANLGC_HEADER;
  msg.type = ANLGC_STOP;
  if ( CltSend( cpci, &msg, &rep, sizeof(msg), sizeof(rep) ) )
	return ANLGC_E_SEND;
  else return rep.type;
}

int cpci_quit( Server_Def *cpci ) {
  analogic_msg_t msg, rep;
  msg.header = ANLGC_HEADER;
  msg.type = ANLGC_QUIT;
  if ( CltSend( cpci, &msg, &rep, sizeof(msg), sizeof(rep) ) )
	return ANLGC_E_SEND;
  else return rep.type;
}

int cpci_report( Server_Def *cpci, int raw, unsigned short index,
		analogic_rpt_t *rpt, void **data, float **fit, size_t size ) {
  analogic_msg_t rep;
  struct {
	analogic_msg_t hdr;
	unsigned short index;
  } msg;
  struct _mxfer_entry sx, rx[3];
  static char *msgbuf = NULL;
  static size_t msgbufsize = 0;

  if ( msgbuf == 0 || size > msgbufsize ) {
	char *newbuf;
	size_t newsize = size ? size : _MAX_VID_SIZE;
	newbuf = (char *)realloc( msgbuf, newsize );
	if ( newbuf == 0 ) {
	  if ( nl_response )
		nl_error( nl_response,
		  "Insufficient memory in cpci_report" );
	  return ANLGC_E_SEND;
	}
	msgbuf = newbuf;
	msgbufsize = newsize;
  }
  msg.hdr.header = ANLGC_HEADER;
  msg.hdr.type = raw ? ANLGC_RAW : ANLGC_REPORT;
  msg.index = index;
  _setmx( &sx, &msg, raw ? sizeof(msg) : sizeof(analogic_msg_t) );
  _setmx( &rx[0], &rep, sizeof(rep) );
  _setmx( &rx[1], rpt, sizeof(analogic_rpt_t) );
  _setmx( &rx[2], msgbuf, msgbufsize );
  if ( CltSendmx( cpci, 1, 3, &sx, rx ) )
	return ANLGC_E_SEND;
  if ( rep.type == ANLGC_OK ) {
	int factor;
	size_t rep_size;
	
	if ( fmt_float(rpt->Format) ) factor = sizeof(float);
	else factor = sizeof(short);
	rep_size = rpt->NChannels * rpt->NReport * factor;
	if ( data != NULL )
	  *data = (void *)msgbuf;
	if ( fit != NULL ) {
	  if ( rpt->NFit ) {
		*fit = (void *)(msgbuf + rep_size);
		rep_size += rpt->NFit * sizeof(float);
	  } else *fit = NULL;
	}
	if ( rep_size > msgbufsize ) {
	  if ( nl_response )
		nl_error( 1, "Message buf too small in cpci_report" );
	  return ANLGC_E_MSG;
	}
  }
  return rep.type;
}
/*
=Name cpci_report(): Request realtime data from driver
=Subject Analogic A/D Drivers

=Synopsis

#include "analogic.h"

int cpci_report( Server_Def *cpci, int raw, unsigned short index,
		analogic_rpt_t *rpt, void **data, float **fit, size_t size ) {

=Description

cpci_report() requests realtime data from the driver. Two formats
of data are supported, raw and reduced. The raw format returns
the raw samples from a single trigger in their native format. The
reduced format returns data that has been analysed, binned and
averaged according to the parameters passed in cpci_setup().

On success, the rpt structure is filled in defining the contents
of the data buffer and data and/or fit are set to point to
the relevant data. The analogic_rpt_t is:

=Code
typedef struct {
  unsigned short Format;
  unsigned short NChannels;
  unsigned short NReport;
  unsigned short NFit;
} analogic_rpt_t;
=Text

Format is a bit-mapped word defining the raw data format,
and which if any fit is being used. The fit definitions
are int analogic.h

ANLGC_FMT_16IL is 16-bit words interleaved. If NChannels were 2,
then the data would be reported as A0, B0, A1, B1, A2, ...

ANLGC_FMT_FLOAT is 32-bit floating-point values non-interleaved.
If NChannels were 2, then the data would be reported as A0, A1,
A2, ..., An, B0, B1, B2, ..., Bn.

If NFit > 0, and fit != NULL, fit will point to the fit parameters
which are driver-specific.

=Returns

cpci_report() returns 0 on success or:
=Code
ANLGC_E_SEND - CltSendmx returned an error. See errno
ANLGC_E_MSG - An error occurred reading the message
ANLGC_E_UNKN - The message type was unknown
ANLGC_E_BUSY - Request cannot be handled at the moment
ANLGC_E_SETUP - Setup parameters were inconsistent or illegal.
=Text

=SeeAlso

=cpci_init=().

=End

=Name cpci_init(): Identify Analogic Server
=Subject Analogic A/D Drivers
=Name cpci_setup(): Begin A/D Operation
=Subject Analogic A/D Drivers
=Name cpci_stop(): End A/D Operation
=Subject Analogic A/D Drivers
=Name cpci_quit(): Request driver to terminate
=Subject Analogic A/D Drivers

=Synopsis

#include "analogic.h"

Server_Def *cpci_init( char *name );
int cpci_setup( Server_Def *cpci, analogic_setup_t *setup );
int cpci_stop( Server_Def *cpci );
int cpci_quit( Server_Def *cpci );

=Description

These functions provide an API for controlling the drivers for
the Analogic high-speed A/D boards. The boards we are supporting
are called the CPCI-16 and the CPCI-14, so I have used the 'cpci'
prefix for these functions. Of course 'cpci' refers to the form
factor, "Compact PCI", and is not specific to Analogic or A/D
conversion, but I'll ignore that for lack of a better short
mnemonic.

Since we anticipate supporting more than one board in a system,
we will likewise anticipate more than one server in a system.
Whereas other client APIs in nortlib can maintain a single static
Server_Def structure out of view, we'll need to create them
dynamically. cpci_init() performs this dynamic creation, taking a
single argument, the server's short name, e.g. "cpci14". The
programmer may choose to call =CltInit=() after calling
cpci_init(), although that is optional.

cpci_setup() requests that the driver configure the board for
data acquisition using the parameters in the setup structure.
The structure is defined as:

=Code
typedef struct {
  unsigned long FSample;
  unsigned long NSample;
  unsigned long NReport;
  unsigned long NAvg;
  unsigned short NCoadd;
  unsigned short FTrigger;
  unsigned short Options;
} analogic_setup_t;
=Text

FSample is the sample frequency in Hz. This rate must be
supported by the selected board.

NSample is the number of samples to take per trigger at the
specified sample frequency.

NReport is how many points to report. This should be equal
to or evenly divide NSample.

NAvg applies when Fit Analysis is selected. It defines the number
of sequential reduced data points that are averaged together into
each report bin. NAvg*NReport*NCoadd*NSample is the total number of
raw points that would go into a report with Fit Analysis. Without
Fit Analsys, only NCoadd*NSample raw samples are required.

NCoadd is the number of passes that should be coadded into the
report before it is recorded. Coadding is essentially vector
addition for summing repeating waveforms.

Options is a bit-mapped word that selects which channels should
be collected and/or reported and whether a specific analysis
should be performed on the data. The bits are:

=Code
ANLG_OPT_A:   Collect Channel A
ANLG_OPT_B:   Collect Channel B
ANLG_OPT_C:   Collect Channel C
ANLG_OPT_D:   Collect Channel D
ANLG_OPT_FIT: Perform time constant analysis
=Text

OPT_FIT applies to the CPCI14 only and is used in ringdown modes
to reduce the ringdowns to a single time constant. OPT_C and
OPT_D are only valid with the CPCI16, since the CPCI14 only has
two channels.

cpci_stop() requests the driver to suspend data acquisition. This
is required before reconfiguring the board.

cpci_quit() requests the driver to terminate.

=Returns

cpci_init() returns the Server_Def structure. It will only fail
if it is unable to allocate memory for the structure, and that is
a fatal error.

All the other functions return 0 on success and non-zero on
error.

=SeeAlso

=CltInit=().

=End
*/
