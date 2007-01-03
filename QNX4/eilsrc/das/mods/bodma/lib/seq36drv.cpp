/* SEQ36DRV.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1995 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 SEQ36DRV.CPP 13-Jul-95,12:06:18,`TBUIJS'
     Added function headers to all functions, and fixed some small problems.
     Added a destructor to correctly shut down DMA and the interrupt service
     before releasing other resources, otherwise there was a small probability of
     a crash if an interrupt occured during the shutdown process.
1:1 SEQ36DRV.CPP 22-Jan-96,18:05:22,`TBUIJS'
     Change the start acquisition command to wait for a valid speed before
     starting in order to correct a problem with GRAMS noting the speed at the
     start of an acquisition.

     Also remove some commented debugging code and explicitly stop the DMA
     transfer when a bad scan is detected.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <stdlib.h>
#include "bo_intr.h"
#include "driver.h"
#include "seq36drv.h"
#ifdef __QNX__
#include <sys/kernel.h>
#endif

extern pid_t do_proxy;

// pointer that allows access to the object from the isr
static Seq36 *seq36;

int Seq36::work () {
  pid_t ret_proxy = 0;

  // If an error occured, driver is disabled
  if (seq36->state == FAILLURE)
    {
      //		seq36->intr.eoi ();
      return -1;
    }
  seq36->scan_bad  = 0;			// always intially assume scan good
  seq36->lock_flag = 0;			// signal interrupt

  if (seq36->state == FIRST)		// forget first intr, often a glitch
    {
      seq36->state = NO_STAT;
      //		seq36->intr.eoi ();
      return 0;
    }

  // check status of DMA transfer if acquiring data
  if (seq36->state == ALIGNMENT || seq36->state == ALIGN_FIRST ||
      seq36->state == COLLECT || seq36->state == COLLECT_FIRST ||
      seq36->state == COLLECT_DELAY)
    {
      // if status != 0: not enough pts
	if (seq36->dmachan.status (seq36->dmabuf))
	  {
	    seq36->dmachan.stop ();
	    seq36->scan_bad = 1;
	  }
	else
	  {
	      // Copy the DMA buffer
	      if (seq36->dmabuf.copy_from_dma_isr (seq36->dmacopy, 0, 0,
						     seq36->ostat.npts))
		{
		  seq36->state = FAILLURE;
		  //				seq36->intr.eoi ();
		  return -2;
		}
	    }
    }

    // get instrument status
    seq36->statword = seq36->dmachan.mike_stat ();

    // check for valid status word
    if (seq36->stat.new_status (seq36->statword))
      {
	seq36->scan_bad = 2;
      }
    else // status is OK
      {
	if (seq36->stat.error)
	  {
	    seq36->scan_bad = 3;
	  }
	if (seq36->stat.resolution != seq36->ostat.resolution &&
	    seq36->state != NO_STAT)
	  {
	    seq36->scan_bad = 4;
	  }
      }

    // Evaluate instrument scan speed
    static char scan_table[8] = {20,10,5,3,2,1,1,1};

    // scan count will be zero on the first pass and after each time
    // measurement is complete. Compute the number of scans over which to
    // measure the elapsed time as a function of resolution
    if (!seq36->scan_count || seq36->stat.resolution != seq36->start_res)
      {
	seq36->scan_count  = scan_table [seq36->stat.res_code];
	seq36->cur_count   = seq36->scan_count;
	seq36->start_ticks = seq36->timer.get_tick ();
	seq36->start_res   = seq36->stat.resolution;
	seq36->ticks = 0; // signals that there is no valid speed available
	}
    else // the clock is running
      {
	if (!--seq36->scan_count) // wait for n scans to go by
	  {
	    seq36->ticks = seq36->timer.tick_sub (seq36->start_ticks,
						  seq36->timer.get_tick ());
	    seq36->scan_count  = scan_table [seq36->stat.res_code];
	    seq36->cur_count   = seq36->scan_count;
	    seq36->start_ticks = seq36->timer.get_tick ();
	    seq36->start_res   = seq36->stat.resolution;
	  }
      }

  static long tick_cur;
  tick_cur = seq36->timer.get_tick ();	// get current tick time

    static unsigned long tnpts;
  switch (seq36->state)
    {
    case START_ALIGN:
    case ALIGN_FIRST:
    case ALIGNMENT:
    case START_COLLECT:
      tnpts = seq36->stat.npts; // number of points for collect
	seq36->dmachan.transfer (seq36->dmabuf, tnpts);
      break;
    case COLLECT_DELAY:
      // check if the delay is over or not
	if (seq36->timer.tick_cmp (seq36->tick_at_start, tick_cur,
				   seq36->tick_to_start))
	  {
	    seq36->state = COLLECT_FIRST;
	    seq36->tick_at_start = tick_cur;	// start clock
	    }
    case COLLECT_FIRST:
    case COLLECT:
      tnpts = seq36->ostat.npts; // number of points for collect
	seq36->dmachan.transfer (seq36->dmabuf, tnpts);
      break;
    default:
      break;
    }

  // deal with bad scans
    if (seq36->scan_bad)
      {
	if (seq36->state == START_ALIGN || seq36->state == ALIGN_FIRST ||
	    seq36->state == ALIGNMENT || seq36->state == START_COLLECT ||
	    seq36->state == COLLECT_FIRST || seq36->state == COLLECT ||
	    seq36->state == COLLECT_DELAY)
	  {
	    seq36->pistat->bad_scans++;
	  }
	if (seq36->state != COLLECT && seq36->state != COLLECT_FIRST &&
	    seq36->state != COLLECT_DELAY)
	  {
	    seq36->ostat = seq36->stat;
	  }
	//		seq36->intr.eoi ();
	return (seq36->scan_bad);
      }

  // cannot get here unless everything is OK

  seq36->ostat = seq36->stat;
  switch (seq36->state)
    {
    case NO_STAT:		// goto IDLE
    case START_ALIGN:	// goto ALIGN_FIRST
    case START_COLLECT:	// goto COLLECT_DELAY
      seq36->state ++;
    case IDLE:
    case COLLECT_DELAY:
    case COLLECT_DONE:
      //			seq36->intr.eoi ();
      return 0;
    default:
      break;
    }


  // from this point on we are in ALIGN_FIRST, ALIGNMENT, COLLECT_FIRST or
  // COLLECT mode!!!

  // penultimate signal clear
  if (seq36->p_proxy_clr && pen && seq36->pistat->scans0 == 0 &&
      seq36->pistat->scans1 == 0) {
    Trigger(seq36->p_proxy_clr);
    pen = 0;
  }

  // get time of current scan
  seq36->istat.scantime = seq36->timer.get_tick ();

  // pointers for copying and coadding
  static long *c;
  static short *dc, *dcc;
  static long i; // loop index

  dc = (short *) seq36->dmacopy.p;

#ifndef NO_ALIGN
  // align mode
    if (seq36->state == ALIGNMENT || seq36->state == ALIGN_FIRST)
      {
	if (seq36->stat.direction)
	  {
	    seq36->pistat->scans1++;
	    // copy to buffer for direction 1
	      dcc = seq36->dcopy1.p;
	    for (i = seq36->stat.npts + 1; --i; )
	      {
		*dcc++ = *dc++;
	      }
	  }
	else
	  {
	    seq36->pistat->scans0++;
	    // copy to buffer for direction 0
	      dcc = seq36->dcopy0.p;
	    for (i = seq36->stat.npts + 1; --i; )
	      {
		*dcc++ = *dc++;
	      }
	  }
	seq36->state = ALIGNMENT;
	//		seq36->intr.eoi ();
	return 0;
      }
#endif
  // advance the FIFO entry and reset scan counters if needed for new
    // sequence. signaled by the set_new_seq_flag 
      if (seq36->set_new_seq_flag)
	{
	  seq36->set_new_seq_flag = 0;
	  seq36->fifo.unlock ();
	  seq36->fifo.allocate ();
	  seq36->pistat = (Int_status *)((char *)seq36->fifo.inptr +
					 seq36->fifo.offs.p[0]);
	  seq36->pistat->sequence = seq36->istat.sequence;
	  seq36->pistat->scans0 = 0;
	  seq36->pistat->scans1 = 0;
	  seq36->pistat->bad_scans = 0;

	  if (seq36->_scans == 0) // half scan mode
	    {
	      seq36->coad0  = (long *)((char *)
				       seq36->fifo.inptr + seq36->fifo.offs.p[1]);
	      seq36->coad1  = (long *)((char *)
				       seq36->fifo.inptr + seq36->fifo.offs.p[1]);
	    }
	  else
	    {
	      seq36->coad0  = (long *)((char *)
				       seq36->fifo.inptr + seq36->fifo.offs.p[1]);
	      seq36->coad1  = (long *)((char *)
				       seq36->fifo.inptr + seq36->fifo.offs.p[2]);
	    }
#ifdef __QNX__
	  ret_proxy = seq36->dta_rdy_proxy;
#endif
	}

  // copy scantime, npts, resolution and oversampling for this coad
    seq36->pistat->scantime = seq36->istat.scantime;
  seq36->pistat->npts = seq36->stat.npts;
  seq36->pistat->res  = seq36->stat.resolution;
  seq36->pistat->ov   = seq36->stat.samples;

  // coad data into coad buffers
    if (seq36->stat.direction)
      {
	if ((seq36->_scans == 0 && seq36->next_dir) || seq36->_scans != 0)
	  {
	    c = (long *) seq36->coad1;
	    if (seq36->pistat->scans1++ == 0)
	      {
		for (i = seq36->stat.npts + 1; --i; )
		  {
		    *c++ = long (*dc++);
		  }
	      }
	    else if (seq36->pistat->scans1 <= seq36->_scans)
	      {
		for (i = seq36->stat.npts + 1; --i; )
		  {
		    *c++ += long (*dc++);
		  }
	      }
	  }
      }
    else
      {
	if ((seq36->_scans == 0 && !seq36->next_dir) || seq36->_scans != 0)
	  {
	    c = (long *) seq36->coad0;
	    if (seq36->pistat->scans0++ == 0)
	      {
		for (i = seq36->stat.npts + 1; --i; )
		  {
		    *c++ = long (*dc++);
		  }
	      }
	    else if (seq36->pistat->scans0 <= seq36->_scans)
	      {
		for (i = seq36->stat.npts + 1; --i; )
		  {
		    *c++ += long (*dc++);
		  }
	      }
	  }
      }

  // trigger penultimate proxies
  if ( seq36->p_proxy_set && seq36->_scans > 1 ) {
    if ( (seq36->pistat->scans0 + seq36->pistat->scans1) == 
	((seq36->_scans * 2) -2) ) {
	Trigger(seq36->p_proxy_set);
	pen=1;
    }
  }

  seq36->state = COLLECT;
  if ((seq36->_scans == 0 && (seq36->pistat->scans0 >= 1 ||
			      seq36->pistat->scans1 >= 1)) ||
      (seq36->_scans != 0 &&
       seq36->pistat->scans0 >= seq36->_scans &&
       seq36->pistat->scans1 >= seq36->_scans))
    {
      // End of one measurement
	seq36->istat.sequence++; // new sequence
	  if (seq36->istat.sequence == seq36->_sequences)
	    {
	      seq36->state = COLLECT_DONE;
	      seq36->end_acq_tick = seq36->timer.get_tick ();
	      seq36->fifo.unlock ();
#ifdef __QNX__
	      ret_proxy = seq36->dta_rdy_proxy;
#endif

	    }
	  else
	    {
	      seq36->state = COLLECT_DELAY;
	      if (seq36->_scans == 0) // half scan mode
		{
		  // flip expected direction flag
		    seq36->next_dir = !seq36->next_dir;
		}
	      seq36->set_new_seq_flag = 1; // flag to advance FIFO buffer
		seq36->tick_to_start =
		  seq36->tick_at_start + seq36->ticks_to_wait;
	    }
    }
  //	seq36->intr.eoi ();
#ifdef __QNX__
  if (ret_proxy) Trigger(ret_proxy);
#endif
  return 0;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   Seq36::Seq36
File:   SEQ01DRV.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        Seq36::Seq36 (BoDatetime timeout, short instrument, double laser,
					  short inter, short dma, short io_adr, long fifo_size)

		timeout         Timeout delay before declaring an error on a status
		                or data request
		instrument      Instrument identifier: (only MB=1 is supported)
		laser           Laser sampling frequency (normal: 15799.7)
		inter          	Hardware interrupt number
		dma             Dma channel to use
		io_adr          I/O address of the fast DMA card
		fifo_size       Size in byte of acquisition FIFO

		Exception       BoError (ACQUISITION_ERROR) if the driver signals
		                                     a FAILLURE code.

Description
		This constructor intializes the SEQ36 acquisition driver and
		starts the interrupt running in monitor mode. After this constructor
		is executed the get_status function can be used to obtain the status
		of the instrument.

 #$%!........................................................................*/

extern pid_t far isr ();

Seq36::Seq36 (BoDatetime timeout, short instrument, double laser,
			  short inter, short dma, short io_adr, long fifo_size
#ifdef __QNX__
, pid_t proxy, pid_t proxy_do, pid_t pen_proxy_set, pid_t pen_proxy_clr
#endif
) :
			  BoDriver (timeout, instrument, laser),
#ifdef __QNX__
			  intr (inter),
#else
			  intr (inter, isr), 
#endif
#ifdef SMALL_MEM
                          dmachan (dma, io_adr), dmabuf (32768L),
			  dmacopy (32768L, BoAlloc::LOCK),
#else
                          dmachan (dma, io_adr), dmabuf (65536L),
			  dmacopy (65536L, BoAlloc::LOCK),
#endif
#ifndef NO_ALIGN
			  dcopy0 (65536L, BoAlloc::LOCK),
			  dcopy1 (65536L, BoAlloc::LOCK),
#endif
			  statword (1), fifo (fifo_size, BoAlloc::LOCK)
{
	state = FIRST;

#ifdef __QNX__
dta_rdy_proxy = proxy;
do_proxy = proxy_do;
p_proxy_set = pen_proxy_set;
p_proxy_clr = pen_proxy_clr;
pen = 0;
#endif
	scan_count  = 0;
	start_ticks = 0;
	ticks       = 0;
	start_res   = 0;

	inst_stat.number_of_det = 2;
	inst_stat.current_det   = 0;
	inst_stat.detectors.resize (2);
	inst_stat.detectors.p[0].type        = 0;
	inst_stat.detectors.p[0].active      = 1;
	inst_stat.detectors.p[0].position    = 0;
	inst_stat.detectors.p[0].auto_gain   = 0;
	inst_stat.detectors.p[0].gain1       = 0;
	inst_stat.detectors.p[0].gain2       = 0;
	inst_stat.detectors.p[0].amp_sat     = 0;
	inst_stat.detectors.p[0].det_sat     = 0;
	inst_stat.detectors.p[0].delay       = 0.0f;
	inst_stat.detectors.p[0].samples     = 1;
	inst_stat.detectors.p[0].apodization = COSINE;
	inst_stat.detectors.p[0].phase_apod  = GAUSSIAN;
	inst_stat.detectors.p[0].phase_res   = 128; // cm-1
	inst_stat.detectors.p[0].phase_npts  = 256;
	inst_stat.detectors.p[0].phase_log2  = 8;
	inst_stat.detectors.p[0].sn          = 0.0;
	inst_stat.detectors.p[0].sx          = laser/2;
	inst_stat.detectors.p[0].s1          = 0;
	inst_stat.detectors.p[0].s2          = 32768L;
	inst_stat.detectors.p[0].npts        = 0;
	inst_stat.detectors.p[1].type        = 0;
	inst_stat.detectors.p[1].active      = 1;
	inst_stat.detectors.p[1].position    = 1;
	inst_stat.detectors.p[1].auto_gain   = 0;
	inst_stat.detectors.p[1].gain1       = 0;
	inst_stat.detectors.p[1].gain2       = 0;
	inst_stat.detectors.p[1].amp_sat     = 0;
	inst_stat.detectors.p[1].det_sat     = 0;
	inst_stat.detectors.p[1].delay       = 0.0f;
	inst_stat.detectors.p[1].samples     = 1;
	inst_stat.detectors.p[1].apodization = COSINE;
	inst_stat.detectors.p[1].phase_apod  = GAUSSIAN;
	inst_stat.detectors.p[1].phase_res   = 128; // cm-1
	inst_stat.detectors.p[1].phase_npts  = 256;
	inst_stat.detectors.p[1].phase_log2  = 8;
	inst_stat.detectors.p[1].sn          = 0.0;
	inst_stat.detectors.p[1].sx          = laser/2;
	inst_stat.detectors.p[1].s1          = 0;
	inst_stat.detectors.p[1].s2          = 32768L;
	inst_stat.detectors.p[1].npts        = 0;

	inst_stat.number_of_src = 1;
	inst_stat.current_src   = 0;
	inst_stat.sources.resize (1);
	inst_stat.sources.p[0].type     = 0;
	inst_stat.sources.p[0].position = 0;

	inst_stat.sequence = 0;
	inst_stat.scans0 = 0;
	inst_stat.scans1 = 0;
	inst_stat.bad_scans = 0;
	inst_stat.scantime = 0;
	inst_stat.elapsed = 0;
	inst_stat.left = 0;
	inst_stat.resolution = 0;
	inst_stat.speed = 0;
	inst_stat.drv_stat = INIT;

	internal_stat = inst_stat;

	seq36 = this;
	dmacopy.init (0);
	dmabuf.copy_to_dma (dmacopy, 0, 0,
#ifdef SMALL_MEM
 32768L);
#else
 65536L);
#endif
#ifndef NO_ALIGN
	dcopy0.init (0);
	dcopy1.init (0);
#endif

	pistat = &istat; // avoids problem with get_status

#ifdef __QNX__
	intr.unmask (&isr);
#else
	intr.unmask ();
#endif

	if (state == FAILLURE)
		{
		inst_stat.drv_stat = FAIL;
		internal_stat.drv_stat = FAIL;
		throw (BoError (ACQUISITION_ERROR));
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   Seq36::types
File:   SEQ01DRV.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        Seq36::types ()

		Exception       BoError (ACQUISITION_ERROR) if the driver signals
		                                     a FAILLURE code.

Description
		This function returns the types of data that the driver can produce,
		in the case of the SEQ36 driver this is only INTERFEROGRAM.

 #$%!........................................................................*/

unsigned long Seq36::types ()
{
	if (state == FAILLURE)
		{
		inst_stat.drv_stat = FAIL;
		internal_stat.drv_stat = FAIL;
		throw (BoError (ACQUISITION_ERROR));
		}

	return (INTERFEROGRAM);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   Seq36::set_status
File:   SEQ01DRV.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void Seq36::set_status ()

		Exception       BoError (ACQUISITION_ERROR) if the driver signals
		                                     a FAILLURE code.

Description
		This function reads the public parameters in inst_stat and sets
		the driver into an appropriate mode. Once this is done the instrument
		status is read and inst_stat is updated to reflect the current
		operating parameters. In the case of the SEQ36
		there are no software controlled parameters that can be changed so
		set_status does nothing more than get_status.

 #$%!........................................................................*/

void Seq36::set_status ()
{
	// There is no status to set with an seq36 card
	if (state == FAILLURE)
		{
		inst_stat.drv_stat = FAIL;
		internal_stat.drv_stat = FAIL;
		throw (BoError (ACQUISITION_ERROR));
		}
	get_status ();
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   Seq36::get_status
File:   SEQ01DRV.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void Seq36::get_status ()

		Exception       BoError (ACQUISITION_ERROR) if the driver signals
		                                     a FAILLURE code.

Description
		This functions updates the inst_stat structure to reflect the current
		instrument operating conditions.

 #$%!........................................................................*/

void Seq36::get_status ()
{
	if (state == FAILLURE)
		{
		inst_stat.drv_stat = FAIL;
		internal_stat.drv_stat = FAIL;
		throw (BoError (ACQUISITION_ERROR));
		}

	timer.countdown (_timeout);
	while (state == FIRST || state == NO_STAT)
		{
		if (timer.countdown () <= 0)
			{
			throw (BoError (TIMEOUT));
			}
		}

	short i = ostat.aux_det;
	internal_stat.current_det = ostat.aux_det;
	internal_stat.detectors.p[i].active = 1;
	internal_stat.detectors.p[i==0 ? 1 : 0].active = 0;	 // 2 detectors	on MB
	internal_stat.detectors.p[i].samples = ostat.samples;
	internal_stat.detectors.p[i].npts = ostat.npts;
	internal_stat.sequence = pistat->sequence;
	internal_stat.scans0 = pistat->scans0;
	internal_stat.scans1 = pistat->scans1;
	internal_stat.bad_scans = pistat->bad_scans;

	switch (state)
		{
		case START_COLLECT:
		case COLLECT_DELAY:
		case COLLECT_FIRST:
		case COLLECT:
		case COLLECT_DONE:
			if (_scans == 0 && ostat.samples == 1)
				{
				internal_stat.detectors.p[i].sx = _laser/2;
				}
			else if ((_scans == 0 && ostat.samples == 2) ||
					 (_scans > 0 && ostat.samples == 1))
				{
				internal_stat.detectors.p[i].sx = _laser;
				}
			else if (_scans > 0 && ostat.samples == 2)
				{
				internal_stat.detectors.p[i].sx = _laser*2;
				}
			break;
		default:
			if (ostat.samples == 1)
				{
				internal_stat.detectors.p[i].sx = _laser/2;
				}
			else
				{
				internal_stat.detectors.p[i].sx = _laser;
				}
			break;
		}

	// compute the time at end of scan based on the tick number of the
	// end of scan. Take care of 24 hour wrap around correctly
	long tick_cur = timer.get_tick ();
	internal_stat.scantime = timer.time () -
			BoDatetime (timer.tick_sub (istat.scantime, tick_cur) /
			timer.tick_rate);

	// speed in scans / second, we divide by 2 because a scan is 2 sweeps
	internal_stat.speed = ticks ? cur_count * timer.tick_rate/ticks/2 : 1.0f;

	BoDatetime seq_time; // time for one measurement
	double time_diff;	 // difference between measurement and time delay

	double tscans = _scans ? _scans : 0.5;

	if (_delay.total_seconds () > tscans/internal_stat.speed)
		{
		seq_time = _delay;
		time_diff = _delay.total_seconds () - tscans/internal_stat.speed;
		}
	else
		{
		seq_time = BoDatetime (tscans/internal_stat.speed);
		time_diff = 0;
		}

	// scans left in current measurement
	double scans_left = tscans - 0.5 * (pistat->scans0 + pistat->scans1);

	switch (state)
		{
		case COLLECT_DELAY:
		case START_COLLECT:
		case COLLECT_FIRST:
		case COLLECT:
			internal_stat.elapsed = timer.time () - start_acq_time;

			// case 1: no measurements done, this is the initial wait period
			if (internal_stat.elapsed < _wait)
				{
				// time left in initial wait period
				internal_stat.left = _wait - internal_stat.elapsed;
				// time to perform measurements
				internal_stat.left = internal_stat.left +
						BoDatetime (_sequences *
									seq_time.total_seconds () - time_diff);
				}
			// case 2: in the middle of a coadd sequence
			else if (scans_left > 0)
				{
				// time left in current measurement
				// should be +time_diff, but the next should be -time_diff
				// so it cancels out.
				internal_stat.left = BoDatetime (
									scans_left / internal_stat.speed);
				// time left in rest of measurements
				internal_stat.left = internal_stat.left + 
					BoDatetime ((_sequences-pistat->sequence-1) *
												seq_time.total_seconds ());
				}
			// case 3: between coadds, in the measurement delay
			else
				{
				// compute what's left of the time delay between scans
				internal_stat.left = time_diff -
									(timer.time () - internal_stat.scantime);
				// time left in rest of measurements
				internal_stat.left = internal_stat.left +
					BoDatetime ((_sequences-pistat->sequence-1) *
									seq_time.total_seconds () - time_diff);
				}
			break;
		case COLLECT_DONE:
			// compute total elapsed based on tick number of the end of
			// acquisition. Take care of 24 hour wrap around correctly
			internal_stat.elapsed = timer.time () - BoDatetime
				(timer.tick_sub (end_acq_tick, tick_cur) / timer.tick_rate) -
				start_acq_time;

			internal_stat.left = BoDatetime (0.0);
			break;
		default:
			internal_stat.elapsed = BoDatetime (0.0);
			internal_stat.left = BoDatetime (0.0);
			break;

		}
	internal_stat.resolution = ostat.resolution;
	switch (state)
		{
		case IDLE:
			internal_stat.drv_stat = READY;
			break;
		case START_ALIGN:
		case ALIGN_FIRST:
		case ALIGNMENT:
			internal_stat.drv_stat = ALIGN;
			break;
		case COLLECT_DELAY:
		case START_COLLECT:
		case COLLECT_FIRST:
		case COLLECT:
			if (fifo.access_out () == BoBlockfifo::FULL)
				{
				// data pending in FIFO
				internal_stat.drv_stat = ACQUIRE_DATA;
				}
			else
				{
				// no data waiting in FIFO
				internal_stat.drv_stat = ACQUIRE;
				}
			break;
		case COLLECT_DONE:
			internal_stat.drv_stat = DONE;
			break;
		default:
			internal_stat.drv_stat = FAIL;
			break;
		}

	internal_stat.speed *= 60; // convert to scans/minute
	internal_stat.fifo_stat = fifo.overrun;

	inst_stat = internal_stat; // make public copy of status
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   Seq36::get_fifo_status
File:   SEQ01DRV.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void Seq36::get_fifo_status ()

		Exception       BoError (ACQUISITION_ERROR) if the driver signals
		                                     a FAILLURE code.

Description
		This functions updates the inst_stat structure to reflect the
		instrument operating conditions that were in effect at the time the
		first entry in the FIFO was collected. If the driver is not in 
		COLLECT mode this function is identical to get_status.

 #$%!........................................................................*/

void Seq36::get_fifo_status ()
{
	Int_status *fpistat;

	get_status ();

	switch (state)
		{
		case COLLECT_DELAY:
		case COLLECT_FIRST:
		case COLLECT:
		case COLLECT_DONE:
			{
			if (fifo.access_out () == BoBlockfifo::EMPTY)
				{
				if (state == COLLECT_DONE)
					{
					inst_stat.drv_stat = READY;
					return;
					}
				throw (BoError (GENERAL_ERROR));
				}
			fpistat = (Int_status *)((char *)fifo.outptr +
															fifo.offs.p[0]);
			short i = inst_stat.current_det;
			inst_stat.detectors.p[i].samples = fpistat->ov;
			inst_stat.detectors.p[i].npts = fpistat->npts;
			inst_stat.sequence = fpistat->sequence;
			inst_stat.scans0 = fpistat->scans0;
			inst_stat.scans1 = fpistat->scans1;
			inst_stat.bad_scans = fpistat->bad_scans;

			long tick_cur = timer.get_tick ();
			inst_stat.scantime = timer.time () -
					BoDatetime (timer.tick_sub (fpistat->scantime, tick_cur) /
					timer.tick_rate);

 			inst_stat.elapsed =	inst_stat.scantime - start_acq_time;
			inst_stat.left = _wait + BoDatetime (_sequences *
				   max ((_scans ? _scans : 0.5) / (internal_stat.speed/60.0),
								_delay.total_seconds ()));
			inst_stat.left = inst_stat.left - inst_stat.elapsed;
			if (inst_stat.left < 0)
				{
				inst_stat.left = 0;
				}

			inst_stat.resolution = fpistat->res;

			if (fifo.access_out () != BoBlockfifo::FULL)
				{
				inst_stat.drv_stat = ACQUIRE;
				}
			else
				{
				if (state == COLLECT_DONE)
					{
					inst_stat.drv_stat = DONE;
					}
				else
					{
					inst_stat.drv_stat = ACQUIRE_DATA;
					}
				}

			return;
			}
		case START_ALIGN:
		case START_COLLECT:
		case ALIGN_FIRST:
		case ALIGNMENT:
		default:
			break;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   Seq36::align
File:   SEQ01DRV.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void Seq36::align (unsigned long type, short scan_code)

		type            Type of DATA to return, for SEQ36 only
						INTERFEROGRAM is valid
		scan_code       Code that indicates which scans to show:
							0 - show only direction zero
							1 - show only direction one
							2 - show both directions

		Exception       BoError (ACQUISITION_ERROR) if the driver signals
		                                     a FAILLURE code.
						BoError (INVALID_ARGUMENT) if the type is anything
						                     but INTERFEROGRAM
						BoError (TIMEOUT) if the status is not available
											 after the timeout delay is
											 elapsed.

Description
		This function starts align mode; a continuous series of non-coadded
 		single scans.

 #$%!........................................................................*/

void Seq36::align (unsigned long type, short scan_code)
{
	if (state == FAILLURE)
		{
		inst_stat.drv_stat = FAIL;
		internal_stat.drv_stat = FAIL;
		throw (BoError (ACQUISITION_ERROR));
		}

	if (type != INTERFEROGRAM)
		{
		throw (BoError (INVALID_ARGUMENT));
		}

	timer.countdown (_timeout);
	while (state == FIRST || state == NO_STAT)
		{
		if (timer.countdown () <= 0)
			{
			throw (BoError (TIMEOUT));
			}
		}

	state = IDLE;

	_scans = scan_code;
	_sequences = 0;
	istat.sequence = 0;
	istat.scans0 = istat.scans1 = istat.bad_scans = 0;
	pistat = &istat;

	state = START_ALIGN;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   Seq36::start
File:   SEQ01DRV.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void Seq36::start (unsigned long type, BoDatetime wait,
						   long sequences, long scans, boDatetime delay)

		type            Type of DATA to return, for SEQ36 only
						INTERFEROGRAM is valid
		wait            Time to wait before starting the acquisition
		sequences       Number of coads to perform
		scans           Number of scans per coad (0 scans is 1 sweep, 1
		                    scan is composed of a forward and a reverse
							sweep)
		delay           Minimum time from the start of a coad to the start
						of the next coad.

		Exception       BoError (ACQUISITION_ERROR) if the driver signals
		                                     a FAILLURE code.
						BoError (INVALID_ARGUMENT) if the type is anything
						                     but INTERFEROGRAM
						BoError (TIMEOUT) if the status is not available
											 after the timeout delay is
											 elapsed.

Description
		This function is used to start an acquisition. If the number of
		coads is larger than what can fit in the FIFO the FIFO will have
		to be emptied as the acquisition progresses to avoid an overflow.

 #$%!........................................................................*/

void Seq36::start (unsigned long type, BoDatetime wait, long sequences,
					long scans, BoDatetime delay)
{
	if (state == FAILLURE)
		{
		inst_stat.drv_stat = FAIL;
		internal_stat.drv_stat = FAIL;
		throw (BoError (ACQUISITION_ERROR));
		}

	if (type != INTERFEROGRAM)
		{
		throw (BoError (INVALID_ARGUMENT));
		}

	timer.countdown (_timeout);
	while (state == FIRST || state == NO_STAT || ticks == 0)
		{
		if (timer.countdown () <= 0)
			{
			throw (BoError (TIMEOUT));
			}
		}

	state = IDLE;

	get_status ();
	internal_stat.elapsed = 0;
	internal_stat.left = wait +	BoDatetime (sequences *
					max ((scans ? scans : 0.5) / (internal_stat.speed/60.0),
						delay.total_seconds ()));

	inst_stat.elapsed = 0;
	inst_stat.left = internal_stat.left;

	_wait = wait;
	_scans = scans;
	_sequences = sequences;
	_delay = delay;
	istat.sequence = 0;
	istat.scans0 = istat.scans1 = istat.bad_scans = 0;

	tick_at_start = timer.get_tick ();
	tick_to_start = timer.tick_add
				(tick_at_start, wait.total_seconds () * timer.tick_rate);
	ticks_to_wait = delay.total_seconds () * timer.tick_rate;

	// record start time
	internal_stat.scantime = timer.time ();
	inst_stat.scantime = internal_stat.scantime;
	start_acq_time = internal_stat.scantime;

	if (_scans == 0) // half scan mode
		{
		BoMemory<long> sizes (2);
		sizes.p[0] = sizeof (Int_status);
		sizes.p[1] = stat.npts * sizeof (long);
		fifo.init (4, sizes);
		fifo.allocate ();
		pistat = (Int_status *)((char *)fifo.inptr + fifo.offs.p[0]);
		pistat->sequence = seq36->istat.sequence;
		pistat->scans0 = 0;
		pistat->scans1 = 0;
		pistat->bad_scans = 0;

		// coad0 and coad1 point to the same location
		coad0  = (long *)((char *)fifo.inptr + fifo.offs.p[1]);
		coad1  = (long *)((char *)fifo.inptr + fifo.offs.p[1]);

		// always start with direction zero
		next_dir = 0; 
		}
	else
		{
		BoMemory<long> sizes (3);
		sizes.p[0] = sizeof (Int_status);
		sizes.p[1] = stat.npts * sizeof (long);
		sizes.p[2] = stat.npts * sizeof (long);
		fifo.init (4, sizes);
		fifo.allocate ();
		pistat = (Int_status *)((char *)fifo.inptr + fifo.offs.p[0]);
		pistat->sequence = seq36->istat.sequence;
		pistat->scans0 = 0;
		pistat->scans1 = 0;
		pistat->bad_scans = 0;

		coad0  = (long *)((char *)fifo.inptr + fifo.offs.p[1]);
		coad1  = (long *)((char *)fifo.inptr + fifo.offs.p[2]);
		}
	set_new_seq_flag = 0;

	state = START_COLLECT;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   Seq36::stop
File:   SEQ01DRV.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void Seq36::stop ()

		Exception       BoError (ACQUISITION_ERROR) if the driver signals
		                                     a FAILLURE code.
						BoError (INVALID_ARGUMENT) if the type is anything
						                     but INTERFEROGRAM
						BoError (TIMEOUT) if the status is not available
											 after the timeout delay is
											 elapsed.

Description
		This functions stops the current acquisition. If the driver is
		in ALIGN mode the align function is simply stopped. If the driver
		is collecting data the acquisition is interrupted but the data
		already in the FIFO remains available.

 #$%!........................................................................*/

void Seq36::stop ()
{
	if (state == FAILLURE)
		{
		inst_stat.drv_stat = FAIL;
		internal_stat.drv_stat = FAIL;
		throw (BoError (ACQUISITION_ERROR));
		}

	switch (state)
		{
		// stop has no effect on these states
		case FIRST:
		case NO_STAT:
		case COLLECT_DONE:
			break;

		// finish the current collect run
		case COLLECT_DELAY:
		case COLLECT_FIRST:
		case COLLECT:
			state = COLLECT_DONE;
			end_acq_tick = timer.get_tick ();
			fifo.unlock ();
		        Trigger(seq36->dta_rdy_proxy);
			break;

		// drop everything and return to IDLE
		default:
			state = IDLE;
			break;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   Seq36::copy
File:   SEQ01DRV.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void Seq36::copy (long det_num, BoMemory<float> &buf)

		det_num         Number of the detector
		buf             Memory buffer to hold the data

		Exception       BoError (ACQUISITION_ERROR) if the driver signals
		                                     a FAILLURE code.
						BoError (INVALID_ARGUMENT) if the det_num is not
						                     valid.
						BoError (TIMEOUT) if the status is not available
											 after the timeout delay is
											 elapsed.
						BoError (INVALID_CONTEXT) if the driver is not in
						                     state to returns data.

Description
		This functions takes a copy of the latest acquisition buffer
		state. This function is the only function used to get results in
		align mode. It is also used to copy the state of the coad buffer
		during acquistion.

 #$%!........................................................................*/

void Seq36::copy (long det_num, 
#ifdef NO_FLOAT
BoMemory<long> &buf)
#else
BoMemory<float> &buf)
#endif
{
	if (state == FAILLURE)
		{
		inst_stat.drv_stat = FAIL;
		internal_stat.drv_stat = FAIL;
		throw (BoError (ACQUISITION_ERROR));
		}

#ifdef NO_FLOAT
	long *pbuf;
#else
	float *pbuf;
#endif
	long  *pcoa;
	short *pdma;
	long i;

	timer.countdown (_timeout);

	while (1)
		{
		lock_flag = 1;

		get_status ();
		if (inst_stat.current_det != det_num)
			{
			throw (BoError (INVALID_ARGUMENT));
			}

		switch (state)
			{
#ifndef NO_ALIGN
			case ALIGNMENT:
				{
				if (scan_bad) break;
				buf.resize (stat.npts);
				pbuf = buf.p;
				// select buffer depending on the scan_code
				switch (_scans)
					{
					case 0:	// only direction 0
						if (pistat->scans0 == 0) // any dir 0 scans ?
							{
							goto next;
							}
						pdma = dcopy0.p;
						break;
					case 1: // only direction 1
						if (pistat->scans1 == 0) // any dir 1 scans ?
							{
							goto next;
							}
						pdma = dcopy1.p;
						break;
					case 2:	// both directions
					default:
						pdma = dmacopy.p;
						break;
					}
				for (i = stat.npts+1; --i; pbuf++, pdma++)
					{
#ifdef NO_FLOAT
					*pbuf = *pdma;
#else
					*pbuf = float (*pdma * 5.0/32768.0);
#endif
					}
				if (!lock_flag)
					{
					break;
					}
				return;
				}
#endif
			case COLLECT_DELAY:
			case COLLECT_FIRST:
			case COLLECT:
			case COLLECT_DONE:
				{
				if (_scans != 0)
					{
					buf.resize (stat.npts*2);
					}
				else
					{
					buf.resize (stat.npts);
					}
				buf.init (0);
				pbuf = buf.p;
				pcoa = coad0;
				long s0 = pistat->scans0;
				long s1 = pistat->scans1;
				if (s0 != 0)
					{
					for (i = stat.npts+1; --i; pbuf++, pcoa++)
						{
#ifdef NO_FLOAT
						*pbuf = *pcoa;
#else
						*pbuf = float (*pcoa * 5.0/32768.0/s0);
#endif
						}
					}
				else if (_scans != 0)
					{
					pbuf += stat.npts;
					}
				pcoa = coad1;
				if (s1 != 0)
					{
					for (i = stat.npts+1; --i; pbuf++, pcoa++)
						{
#ifdef NO_FLOAT
						*pbuf = *pcoa;
#else
						*pbuf = float (*pcoa * 5.0/32768.0/s1);
#endif
						}
					}
				if (!lock_flag) break;
				return;
				}
			case START_ALIGN:
			case START_COLLECT:
			case ALIGN_FIRST:
				break;
			default:
				throw (BoError (INVALID_CONTEXT));
			}
next:
		if (timer.countdown () <= 0)
			{
			throw (BoError (TIMEOUT));
			}
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   Seq36::data
File:   SEQ01DRV.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void Seq36::data (BoMemory<float> &buf)

		buf             Buffer to hold the returned results.

		Exception       BoError (ACQUISITION_ERROR) if the driver signals
		                                     a FAILLURE code.
						BoError (TIMEOUT) if the status is not available
											 after the timeout delay is
											 elapsed.
						BoError (FIFO_UNDERRUN) if there is no data ready
						                     in the FIFO.
						BoError (INVALID_CONTEXT) if the driver is not in
											 a mode where the FIFO is used
											 such as align mode.

Description
		This function read data from the FIFO buffer.

 #$%!........................................................................*/

void Seq36::data (
#ifdef NO_FLOAT
BoMemory<long> &buf
#else
BoMemory<float> &buf
#endif
)
{
	Int_status *fpistat;
	long *fcoad0, *fcoad1;
#ifdef NO_FLOAT
	long *pbuf;
#else
	float *pbuf;
#endif
	long  *pcoa;
	long i;

	get_status ();

	switch (state)
		{
		case COLLECT_DELAY:
		case COLLECT_FIRST:
		case COLLECT:
		case COLLECT_DONE:
			{
			if (fifo.access_out () != BoBlockfifo::FULL)
				{
				throw (BoError (FIFO_UNDERRUN));
				}
			if (_scans != 0)
				{
				fpistat = (Int_status *)((char *)fifo.outptr +
														fifo.offs.p[0]);
				fcoad0  = (long *)((char *)fifo.outptr +
														fifo.offs.p[1]);
				fcoad1  = (long *)((char *)fifo.outptr +
														fifo.offs.p[2]);
				buf.resize (fpistat->npts*2);
				}
			else
				{
				fpistat = (Int_status *)((char *)fifo.outptr +
														fifo.offs.p[0]);
				fcoad0  = (long *)((char *)fifo.outptr +
														fifo.offs.p[1]);
				fcoad1  = (long *)((char *)fifo.outptr +
														fifo.offs.p[1]);
				buf.resize (fpistat->npts);
				}
			buf.init (0);
			pbuf = buf.p;
			pcoa = fcoad0;
			if (fpistat->scans0 != 0)
				{
				for (i = fpistat->npts+1; --i; pbuf++, pcoa++)
					{
#ifdef NO_FLOAT
					*pbuf = *pcoa;
#else
					*pbuf = float (*pcoa * 5.0/32768.0/fpistat->scans0);
#endif
					}
				}
			else if (_scans != 0)
				{
				pbuf += fpistat->npts;
				}
			pcoa = fcoad1;
			if (fpistat->scans1 != 0)
				{
				for (i = fpistat->npts+1; --i; pbuf++, pcoa++)
					{
#ifdef NO_FLOAT
					*pbuf = *pcoa;
#else
					*pbuf = float (*pcoa * 5.0/32768.0/fpistat->scans1);
#endif
					}
				}

			i = inst_stat.current_det;
			inst_stat.detectors.p[i].samples = fpistat->ov;
			inst_stat.detectors.p[i].npts = fpistat->npts;
			inst_stat.sequence = fpistat->sequence;
			inst_stat.scans0 = fpistat->scans0;
			inst_stat.scans1 = fpistat->scans1;
			inst_stat.bad_scans = fpistat->bad_scans;

			long tick_cur = timer.get_tick ();
			inst_stat.scantime = timer.time () -
					BoDatetime (timer.tick_sub (fpistat->scantime, tick_cur) /
					timer.tick_rate);

 			inst_stat.elapsed =	inst_stat.scantime - start_acq_time;
			inst_stat.left = _wait + BoDatetime (_sequences *
				   max ((_scans ? _scans : 0.5) / (internal_stat.speed/60.0),
								_delay.total_seconds ()));
			inst_stat.left = inst_stat.left - inst_stat.elapsed;
			if (inst_stat.left < 0)
				{
				inst_stat.left = 0;
				}

			inst_stat.resolution = fpistat->res;

			if (fifo.access_out (1) != BoBlockfifo::FULL)
				{
				if (state == COLLECT_DONE)
					{
					state = IDLE;
					inst_stat.drv_stat = READY;
					}
				else
					{
					inst_stat.drv_stat = ACQUIRE;
					}
				}
			else
				{
				if (state == COLLECT_DONE)
					{
					inst_stat.drv_stat = DONE;
					}
				else
					{
					inst_stat.drv_stat = ACQUIRE_DATA;
					}
				}

			fifo.release ();
			return;
			}
		case START_ALIGN:
		case START_COLLECT:
		case ALIGN_FIRST:
		case ALIGNMENT:
		default:
			throw (BoError (INVALID_CONTEXT));
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   Seq36::~Seq36
File:   SEQ01DRV.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void Seq36::~Seq36 ()

Description
		This destructor does an orderly shutdown of the SEQ36 driver.
		Once the interrupt and DMA tasks are shut down the other resource
		will be freed automatically by their respective destructors.

 #$%!........................................................................*/

Seq36::~Seq36 ()
{
	// make sure the interrupt is masked before closing down the
	// driver because otherwise transients can occur.
	intr.mask ();
	// shut down any DMA transfers in progress
	dmachan.stop ();
}
