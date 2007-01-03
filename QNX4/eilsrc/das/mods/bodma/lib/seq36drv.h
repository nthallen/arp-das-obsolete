/* SEQ36DRV.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 SEQ36DRV.H 10-Jul-95,12:44:08,`TBUIJS'
     Driver definition for seq36 fast DMA card.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#ifndef __SEQ36DRV_H
#define __SEQ36DRV_H

#include "bo_intr.h"
#include "bo_time.h"
#include "bo_bmdma.h"
#include "bodmabuf.h"
#include "bomikest.h"
#include "bomemory.h"
#include "bofifo.h"
#include "driver.h"


class Seq36 : public BoDriver
	{
		// possible private states for seq01 driver
		enum
			{
			FIRST, NO_STAT, IDLE, START_ALIGN, ALIGN_FIRST,
			ALIGNMENT, START_COLLECT, COLLECT_DELAY, COLLECT_FIRST, COLLECT,
			COLLECT_DONE, FAILLURE
			};

		// This structure is used to describe the instrument status	for the
		// interrupt routine and is subset of Status 
		struct Int_status
			{
			volatile long sequence;  // sequence number (read only)
			volatile long scans0;    // scans in direction 0 (read only)
			volatile long scans1;    // scans in direction 1 (read only)
			volatile long bad_scans; // bad scans in sequence (read only)
			volatile long scantime;  // time at end of last scan (read only)
			volatile long npts;      // number of points in interferogram
			volatile short res;      // resolution of the interferogram
			volatile short ov;       // oversampling factor for interferogram
			};

		volatile short state;			// internal driver state

		volatile short lock_flag;		// lock flag for interrupt
		volatile short scan_bad; 		// internal bad scan flag
		volatile short next_dir;		// next direction flag for half scans
		volatile short set_new_seq_flag;// flag to advance FIFO buffer

		BoInterrupt intr;				// interrupt controller object

                pid_t dta_rdy_proxy;
                pid_t p_proxy_set;
                pid_t p_proxy_clr;
                unsigned char pen;

		BoBmdma dmachan;				// Interface to Bus master DMA
		BoDmabuf dmabuf;				// Special memory for DMA buffer
		BoMemory<short> dmacopy;		// Copy of DMA buffer
		BoMemory<short> dcopy0, dcopy1; // Copy of DMA buffer for each dir
		unsigned short statword;		// Raw Status word after DMA
		BoBlockfifo fifo;
		long *coad0, *coad1;	// Ptrs to coadd buffers in FIFO

		BoDatetime _wait;				// initial wait before acquisition
		long _scans;					// number of scans to collect
		long _sequences;				// number of sequences to collect
		BoDatetime _delay;				// scan to scan delay
		long tick_at_start;				// tick at start of prev sequence
		long tick_to_start;				// tick at which to start sequence
		BoDatetime start_acq_time; 		// acquisition start time
		long ticks_to_wait;				// scan to scan delay in timer ticks
		long end_acq_tick;				// acquisition end time

		Int_status istat;				// internal status for sequence
		Int_status *pistat;             // pointer to internal status in FIFO

		// this group of variables is used for instrument scan speed
 		// determination
		BoClock timer;						// access to PC-clock
		volatile char scan_count;			// scan countdown counter
		volatile char cur_count;			// scans per time measument
		volatile long start_ticks;			// ticks at start of measurement
		volatile long ticks;				// ticks for a group of scans
		volatile short start_res;			// resolution at measurement start

		BoMikestatus stat, ostat;			// instrument status routine
	public:
		Seq36 (BoDatetime timeout, short instrument, double laser,
				short inter, short dma, short io_adr, long fifo_size
#ifdef __QNX__
, pid_t proxy, pid_t do_proxy, pid_t pen_proxy_set, pid_t pen_proxy_clr
#endif
);
		unsigned long types ();
		void set_status ();
		void get_status ();
		void get_fifo_status ();
		void align (unsigned long type, short scan_code);
		void start (unsigned long type, BoDatetime wait, long sequences,
					long scans, BoDatetime delay);
                int work ();
		void stop ();
		void copy (long det_num,
#ifdef NO_FLOAT
 BoMemory<long> &buf);
#else
 BoMemory<float> &buf);
#endif
		void data (
#ifdef NO_FLOAT
BoMemory<long> &buf);
#else
BoMemory<float> &buf);
#endif
		~Seq36 ();
};

#endif

