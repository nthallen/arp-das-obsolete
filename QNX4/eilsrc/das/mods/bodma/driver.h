/* DRIVER.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 DRIVER.H 22-Jun-94,19:46:24,`THOMAS' File creation
1:1 DRIVER.H 5-Jul-94,12:11:24,`THOMAS'
     Made the various substructures used in the generic Driver definition part
     of the class in order to minimize the use of the global namespace
1:2 DRIVER.H 11-Aug-94,14:08:14,`THOMAS'
     Add support for putting the driver into a DLL and for driver state
     information.
1:3 DRIVER.H 23-Jun-95,15:47:22,`TBUIJS'
     The Driver object is now a BoDriver object. It has been updated to
     better support multiple detector configurations and generic status
     information.
1:4 DRIVER.H 2-Jul-95,12:33:56,`TBUIJS'
     Added the s1, s2 and phase_log2 variables in the detector description. These
     are used to be able to compute the number of points and indexes associated
     with the spectral range for each detector.
1:5 DRIVER.H 7-Jul-95,10:56:52,`TBUIJS'
     Added a destructor to the general driver class.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#ifndef __DRIVER_H
#define __DRIVER_H

#include "bomemory.h"
#include "bo_time.h"
#include "spectrum.h" // apodizations are defined in spectrum

// result type that can be returned by low level driver. Minimal
// functionality must include INTERFEROGRAM. All other type can be derived
// from the INTERFEROGRAM type. Coprocessor assisted acquisition cards will
// normally also include COMPLEX_RAW and RAW_SPECTRUM. The values are powers
// of two so that they can be combined to form a bit pattern indicating
// multiple return types by addition
enum
	{
	INTERFEROGRAM = 1,
	COMPLEX_RAW = 2,
	RAW_SPECTRUM = 4,
	TRANSMITANCE = 8,
	ABSORBANCE = 16,
	RADIANCE = 32
	};

// Driver status flags
enum
	{
	INIT,	      // Driver still initializing
	READY,	      // Ready for acquisition
	ALIGN,	      // Align mode	in progress
	ACQUIRE,      // Acquisition in progress, no data ready in FIFO yet
	ACQUIRE_DATA, // Acquisition in progress, some data ready in FIFO
	DONE,	      // Acquisition complete, data pending
	FAIL          // Unrecoverable error occured
	};

// This structure is used to describe the status of the detectors in the
// system. It is used both to set and to receive the detector status.
struct Detector
	{
	short  type;			// type code for the detector
	short  active;			// flag to turn a detector on or off
	long   position;		// position code within a detector array
	short  auto_gain;		// automatic gain adjustment on/off
	float  gain1;			// first stage gain
	float  gain2;			// second state gain
	short  amp_sat;			// amplifier saturation flag (read only)
	short  det_sat;			// detector saturation (read only)
	float  delay;			// amplifier compensation time delay
	short  samples;			// number of samples per fringe
	short  apodization;		// apodization function to use
	short  phase_apod;		// apodization function to use on phase
	short  phase_res;		// resolution to use for phase correction
	short  phase_npts;		// number of points in phase array (read only)
	short  phase_log2;      // log2 of the number of points (read only)
	double sn, sx;			// min x value and max x value of data array
	long   s1;				// first x point in spectrum at 1cm-1 (read only)
	long   s2;				// last x point in spectrum at 1cm-1 (read only)
	long   npts;			// number of points in data array (read only)
	};

// This structure is used to describe the status of the sources in the
// system. It is used both to set and to receive the source status.
struct Source
	{
	short type;				// type code for source
	short position;			// position code for the source
	};

// This structure is used to describe the complete instrument status
// including the detector and source status.
struct Status
	{
	short      number_of_det;	  // number of detectors present
	short      current_det;       // current detector
	BoMemory<Detector> detectors; // setup array, 1 entry per detector
	short      number_of_src;	  // number of sources available
	short      current_src;		  // current source being used
	BoMemory<Source> sources;	  // setup array, 1 entry per source
	long       sequence;          // sequence number (read only)
	long       scans0;            // scans in direction 0 (read only)
	long       scans1;            // scans in direction 1 (read only)
	long       bad_scans;         // bad scans in sequence (read only)
	BoDatetime scantime;   		  // time at end of last scan (read only)
	BoDatetime elapsed;	   		  // elapsed time since start (read only)
	BoDatetime left;	   		  // time left to completion (read only)
	float      resolution; 		  // Instrument resolution
	float      speed;	   		  // Instrument speed in scans/min
	short	   drv_stat;   		  // state of the driver (read only)
	short      fifo_stat;		  // FIFO Overrun flag		        
	};

// Base abstract class for all acquisition drivers
class BoDriver
	{
	protected:
		BoDatetime _timeout;
		short _instrument;
		double _laser;
		Status internal_stat; // private copy of instrument status
		
	public:
		Status inst_stat; // instrument status

		BoDriver (BoDatetime timeout, short instrument, double laser);

		// Returns the data types supported by the driver
		virtual unsigned long types () = 0;

		// The set_status routine reads the instrument conditions in the
		// inst_stat structure which can be written to and sends commands
		// to put the instrument in the specified conditions.
		// Once the commands are send set_status automatically calls
		// get_status in order to update the status structure with the
		// resulting conditions. get_status can also be called at any time
		// to obtain the latest instrument status. get_fifo_status is like
		// get_status but it obtains the status of the system at the time
		// the entry at the end of the FIFO was made. This is the same as
		// the status that is returned when data is called.
		virtual void set_status () = 0;
		virtual void get_status () = 0;
		virtual void get_fifo_status () = 0;

		// Starts align mode given the return type, and a scan_code.
		// The scan code lets either all scans or only one direction to
		// be selected for alignment
		virtual void align (unsigned long type, short scan_code) = 0;

		// Starts acquisition mode given the return type, the intial delay
		// the number of measurement sequences, the number of scans per
		// seuqence	and the delay from start to start between sequences
		virtual void start (unsigned long type, BoDatetime wait,
							long sequences, long scans, BoDatetime delay) = 0;

		virtual int work ( ) = 0;

		// The stop function interrupts the current acquisition but the
		// result collected so far are kept in the FIFO
		virtual void stop () = 0;

		// Copy the current state of the coad buffer for detector det_num.
		// The status fields are automatically updated when this function
		// is called so that they are in sync with the copy.
		// When multiple results are needed they are catenated in a single
		// array; for example 2 interferograms are send, the sequences are:
		// interferogram: interf d0, interf d1
		// complex raw: (real, imag, phase) d0, (real, imag, phase) d1
		virtual void copy (long det_num, 
#ifdef NO_FLOAT
BoMemory<long> &buf) = 0;
#else
BoMemory<float> &buf) = 0;
#endif

		// Get the oldest entry from the FIFO. When multiple detectors
		// are present they are sent in sequence into the FIFO. The det_num
		// parameter indicates the sequence number of the detector.
		// The status fields are modified to reflect the status at the time
		// the entry was collected. To return to the current status simply
		// call get_status.
		virtual void data (
#ifdef NO_FLOAT
BoMemory<long> &buf) = 0;
#else
BoMemory<float> &buf) = 0;
#endif
		virtual ~BoDriver ();
	};

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDriver::BoDriver
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDriver::BoDriver (BoDatetime timeout, short instrument,
						    double laser);

		timeout        time to wait before declaring an error
		instrument     type of instrument to acquire from
		laser          reference laser frequency (HeNe laser)

Description
		This constructor handles the parameters that are universal to
		all acquisition drivers.
 #$%!........................................................................*/

inline BoDriver::BoDriver (BoDatetime timeout, short instrument, double laser)
{
	_timeout    = timeout;
	_instrument = instrument;
	_laser      = laser;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDriver::~BoDriver
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDriver::~BoDriver ();

Description
		Destructor for acquisition drivers, this function does nothing
		but the real driver implements various behaviours and the destructor
		is a virtual fucntion.
 #$%!........................................................................*/

inline BoDriver::~BoDriver ()
{
}

#endif

