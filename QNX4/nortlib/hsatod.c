#include <sys/vc_msg.h>
#include <malloc.h>
#include "hsatod.h"
#include "nortlib.h"

/* High Speed A/D Board Client Routines:
  Unlike many of our clients, these actually need the ability to
  support multiple servers, since we are likely to run at least
  two different servers. As such, we'll have to dynamically
  create server definitions which the client will use to access
  the server.
  
*/

Server_Def *hsatod_init( char *name ) {
  Server_Def *hsatod = new_memory( sizeof(Server_Def) );
  hsatod->name = name;
  hsatod->expand = 1;
  hsatod->global = 1;
  hsatod->response = 2;
  hsatod->node = 0;
  hsatod->pid = 0;
  hsatod->connected = 0;
  hsatod->disconnected = 0;
  return hsatod;
}

int hsatod_setup( Server_Def *hsatod, hsatod_setup_t *setup ) {
  hsatod_msg_t msg, rep;
  struct _mxfer_entry sx[2], rx;
  _setmx( &sx[0], &msg, sizeof(msg) );
  _setmx( &sx[1], setup, sizeof(hsatod_setup_t));
  _setmx( &rx, &rep, sizeof(rep) );
  msg.header = ANLGC_HEADER;
  msg.type = ANLGC_SETUP;
  if ( CltSendmx( hsatod, 2, 1, sx, &rx ) )
	return ANLGC_E_SEND;
  else return rep.type;
}

static int hsatod_short_msg( Server_Def *hsatod, signed short mtype ) {
  hsatod_msg_t msg, rep;
  msg.header = ANLGC_HEADER;
  msg.type = mtype;
  if ( CltSend( hsatod, &msg, &rep, sizeof(msg), sizeof(rep) ) )
	return ANLGC_E_SEND;
  else return rep.type;
}

int hsatod_stop( Server_Def *hsatod ) {
  return hsatod_short_msg( hsatod, ANLGC_STOP );
}

int hsatod_quit( Server_Def *hsatod ) {
  return hsatod_short_msg( hsatod, ANLGC_QUIT );
}

int hsatod_nolog( Server_Def *hsatod ) {
  return hsatod_short_msg( hsatod, ANLGC_NOLOG );
}

int hsatod_log( Server_Def *hsatod ) {
  return hsatod_short_msg( hsatod, ANLGC_LOG );
}

int hsatod_report( Server_Def *hsatod, int raw, unsigned short index,
		hsatod_rpt_t *rpt, void **data, float **fit, size_t size ) {
  hsatod_msg_t rep;
  struct {
	hsatod_msg_t hdr;
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
		  "Insufficient memory in hsatod_report" );
	  return ANLGC_E_SEND;
	}
	msgbuf = newbuf;
	msgbufsize = newsize;
  }
  msg.hdr.header = ANLGC_HEADER;
  msg.hdr.type = raw ? ANLGC_RAW : ANLGC_REPORT;
  msg.index = index;
  _setmx( &sx, &msg, raw ? sizeof(msg) : sizeof(hsatod_msg_t) );
  _setmx( &rx[0], &rep, sizeof(rep) );
  _setmx( &rx[1], rpt, sizeof(hsatod_rpt_t) );
  _setmx( &rx[2], msgbuf, msgbufsize );
  if ( CltSendmx( hsatod, 1, 3, &sx, rx ) )
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
		nl_error( 1, "Message buf too small in hsatod_report" );
	  return ANLGC_E_MSG;
	}
  }
  return rep.type;
}
/*
=Name hsatod_report(): Request realtime data from driver
=Subject High Speed A/D Drivers

=Synopsis

#include "hsatod.h"

int hsatod_report( Server_Def *hsatod, int raw, unsigned short index,
		hsatod_rpt_t *rpt, void **data, float **fit, size_t size ) {

=Description

hsatod_report() requests realtime data from the driver. Two formats
of data are supported, raw and reduced. The raw format returns
the raw samples from a single trigger in their native format. The
reduced format returns data that has been analysed, binned and
averaged according to the parameters passed in hsatod_setup().

On success, the rpt structure is filled in defining the contents
of the data buffer and data and/or fit are set to point to
the relevant data. The hsatod_rpt_t is:

=Code
typedef struct {
  unsigned short Format;
  unsigned short NChannels;
  unsigned short NReport;
  unsigned short NFit;
} hsatod_rpt_t;
=Text

Format is a bit-mapped word defining the raw data format,
and which if any fit is being used. The fit definitions
are int hsatod.h

ANLGC_FMT_16IL is 16-bit words interleaved. If NChannels were 2,
then the data would be reported as A0, B0, A1, B1, A2, ...

ANLGC_FMT_FLOAT is 32-bit floating-point values non-interleaved.
If NChannels were 2, then the data would be reported as A0, A1,
A2, ..., An, B0, B1, B2, ..., Bn.

If NFit > 0, and fit != NULL, fit will point to the fit parameters
which are driver-specific.

=Returns

hsatod_report() returns 0 on success or:
=Code
ANLGC_E_SEND - CltSendmx returned an error. See errno
ANLGC_E_MSG - An error occurred reading the message
ANLGC_E_UNKN - The message type was unknown
ANLGC_E_BUSY - Request cannot be handled at the moment
ANLGC_E_SETUP - Setup parameters were inconsistent or illegal.
=Text

=SeeAlso

=hsatod_init=().

=End

=Name hsatod_init(): Identify High Speed Server
=Subject High Speed A/D Drivers
=Name hsatod_setup(): Begin A/D Operation
=Subject High Speed A/D Drivers
=Name hsatod_stop(): End A/D Operation
=Subject High Speed A/D Drivers
=Name hsatod_quit(): Request driver to terminate
=Subject High Speed A/D Drivers

=Synopsis

#include "hsatod.h"

Server_Def *hsatod_init( char *name );
int hsatod_setup( Server_Def *hsatod, hsatod_setup_t *setup );
int hsatod_stop( Server_Def *hsatod );
int hsatod_quit( Server_Def *hsatod );

=Description

These functions provide an API for controlling the drivers for
high-speed A/D boards. These functions were originally designed
for use with the Analogic CPCI-14 and CPCI-16 boards, but they
have been generalized a bit to support the Chase Scientific
CS210 and the Anderson SSP Board.

Since we anticipate supporting more than one board in a system,
we will likewise anticipate more than one server in a system.
Whereas other client APIs in nortlib can maintain a single static
Server_Def structure out of view, we'll need to create them
dynamically. hsatod_init() performs this dynamic creation, taking a
single argument, the server's short name, e.g. "cs210". The
programmer may choose to call =CltInit=() after calling
hsatod_init(), although that is optional.

hsatod_setup() requests that the driver configure the board for
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
} hsatod_setup_t;
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

hsatod_stop() requests the driver to suspend data acquisition. This
is required before reconfiguring the board.

hsatod_quit() requests the driver to terminate.

=Returns

hsatod_init() returns the Server_Def structure. It will only fail
if it is unable to allocate memory for the structure, and that is
a fatal error.

All the other functions return 0 on success and non-zero on
error.

=SeeAlso

=CltInit=().

=End
*/

int hsatod_status( Server_Def *hsatod, hsatod_status_t *status ) {
  hsatod_msg_t msghdr, rep;
  struct _mxfer_entry sx, rx[2];
  
  msghdr.header = ANLGC_HEADER;
  msghdr.type = ANLGC_STATUS;
  _setmx( &sx, &msghdr, sizeof(hsatod_msg_t) );
  _setmx( &rx[0], &rep, sizeof(rep) );
  _setmx( &rx[1], status, sizeof(hsatod_status_t) );
  if ( CltSendmx( hsatod, 1, 2, &sx, rx ) )
	return ANLGC_E_SEND;
  return rep.type;
}

/*
=Name hsatod_status(): Get Driver Status
=Subject High Speed A/D Drivers

=Synopsis

#include "hsatod.h"

int hsatod_status( Server_Def *hsatod, hsatod_status_t *status );

=Description

hsatod_status() returns basic status information about the
driver in the status structure.

=Code
typedef struct {
  unsigned long index;
  unsigned short status;
  unsigned short Vin1, Vin2;
} hsatod_status_t;
=Text

The index member is the most recently written file index.
(See =Multi-level File Routines=.) The status member is
the status code as defined in hsatod.h as ANLGC_S_*.

=Returns

hsatod_status() returns 0 on success or:
=Code
ANLGC_E_SEND - CltSendmx returned an error. See errno
ANLGC_E_MSG - An error occurred reading the message
ANLGC_E_UNKN - The message type was unknown
=Text

=SeeAlso

=hsatod_init=().

=End
*/
