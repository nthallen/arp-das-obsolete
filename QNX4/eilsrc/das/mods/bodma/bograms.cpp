/* BOGRAMS.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BOGRAMS.CPP 26-Jun-95,20:40:26,`TBUIJS'
     Glue layer to allow GRAMS to call the collect device drivers.
1:1 BOGRAMS.CPP 4-Jul-95,19:02:58,`TBUIJS'
     Added support for acquisition cards that can return a raw_spectrum as their
     data type. Also make sure that a card that can return both interferogram and
     raw_spectrum always choses raw spectrum when spectra are called for.
1:2 BOGRAMS.CPP 12-Jul-95,15:08:50,`TBUIJS'
     Fixed some interfacing problems with GRAMS, the DLL now protects each entry
     point by resetting the coprocessor and mask floating point exceptions.
     Also some problems could occur during aling if the resolution was changed
     and the interrupt happened to occur between the call to get_status and
     the call to get_data. Now the results from get_data are used to determine
     the status of the instrument instead of those from get_status.
1:3 BOGRAMS.CPP 21-Aug-95,14:49:22,`TBUIJS'
     Fixed a problem in the close function that prevented open being used a
     second time after a close.
1:4 BOGRAMS.CPP 11-Dec-95,12:01:22,`TBUIJS'
     Change flags from simple 0/1 mode to using a magic number. This avoids
     potential initialization bugs. Added some debugging code to save intermediate
     results to disk.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mem.h>
#include <malloc.h>
#include <dos.h>
#include <float.h>
#include <signal.h>

#include "bo_proto.h"
#include "bomemory.h"
#include "bo_error.h"
/* #include "seq01drv.h"*/
/*#include "dsp56drv.h"*/
#include "seq36drv.h"
#include "spectrum.h"
//#include "file_gra.h"

//get rid of the min max macros in windows.h!!!
/* #define NOMINMAX
#include <windows.h>
#include <windowsx.h>
*/

//#define DEBUG

// magic number to be used instead of 1 for flags
// because they may be uninitialized
#define MAGIC 15793

// structure that represents the status as sent to BGRAMS
/* struct BoGrams_status
	{
	double sn;	// sigma min or minimum x value
	double sx;	// sigma max or maximum x value
	double tm;	// absolute of spectrum being collected
	double et;	// elapsed time since acquisition start
	double tl;	// estimated time left to acquisition end
	double tmf;	// absolute time of spectrum in FIFO
	long s0;	// number of scans in direction 0
	long s1;	// number of scans in direction 1
	long sb;	// number of bad scans in this coad
	long seq;	// sequence number of this coad
	long s0f;	// number of scans in direction 0 in FIFO
	long s1f;	// number of scans in direction 1 in FIFO
	long sbf;	// number of bad scans in FIFO
	long seqf;	// sequence number of coad in FIFO
	long npts;	// number of points in result
	float spd;	// current scan speed in scans/minute
	short res;	// current instrument resolution 
 	short sp;  	// samples per fringe
	short done;	// completion flag, this is a bit field
			// bit 0 is on when an acquisition is in progress
			// bit 1 is on when the FIFO contains a complete coad
			// bit 2 is on if a FIFO overrun occured
	float zpd_pos;	// largest positive value around interferogram center
	float zpd_neg;	// largest negative value around interferogram center
	char pad[154];	// reserved for future use (set to zero)
	};
*/
/*
extern "C" short FAR PASCAL _export open (short board, short instrument,
	 short irq, short dma, short ioadr, double laser, long bufsiz,
	 char *path, double timeout);
extern "C" short FAR PASCAL _export close ();
extern "C" short FAR PASCAL _export start (short mode, double wait,
	long scans, long runs,
	double delay, short type, double sn,
	double sx, short apod, short phase_res,
	short phase_apod);
extern "C" short FAR PASCAL _export get_data (short source,
	BoGrams_status *status, long npts, float huge *data);
extern "C" short FAR PASCAL _export get_status (BoGrams_status *status);
extern "C" short FAR PASCAL _export stop ();


int FAR PASCAL LibMain (HINSTANCE, WORD, WORD, LPSTR)
{
    return( 1 );
}
*/


BoDriver *driver = NULL;

static void bocopy (long size, 
#ifdef NO_FLOAT
long *inbuf, long *outbuf)
#else
float *inbuf, float *outbuf)
#endif
{
	for (long i = size + 1; --i;)
		{
		*outbuf++ = *inbuf++;
		}
}

// math function exceptions
int _matherr (struct _exception *)
{
	return (1);
}

short open_flag;
double open_laser;

/*extern*/ /*"C"*/ short /*FAR PASCAL _export*/ bo_open (short board, short instrument,
							short irq, short dma, short ioadr,
							double laser, long bufsiz, char *path,
							double timeout
#ifdef __QNX__
, pid_t proxy, pid_t proxy_do, pid_t pen_proxy_set, pid_t pen_proxy_clr
#endif
)
{
	/* asm cld; */
	_fpreset ();
	(void)signal (SIGINT, (void (*)(int))SIG_IGN);
	(void)_control87 (MCW_EM, MCW_EM);

#ifdef DEBUG
	char buf[200];

	sprintf (buf, "%d, %d, %d, %d, %d, %f, %ld, %s, %f", board,instrument,irq,dma,ioadr,laser,bufsiz,path,timeout);
	MessageBox (NULL, buf, "called OPEN", MB_OK);
#endif
	try
		{
		if (open_flag == MAGIC)
			{
			return (-4); // driver already open
			}
		open_laser = laser;
		switch (board)
			{
/*			case 0: // DMA
				driver = new Seq01 (timeout, instrument, laser, irq, dma,
																	bufsiz);
				break;
*/
/*			case 1: // DSP
				driver = new Dsp56 (timeout, instrument, laser, irq, ioadr,
															  path, bufsiz);
				break;
*/
			case 6: // NDMA
				driver = new Seq36 (timeout, instrument, laser, irq, dma,
														ioadr, bufsiz, proxy, proxy_do, pen_proxy_set, pen_proxy_clr);
				break;
			default:
				break;
			}
		// magic number instead of 1 in case variable is not initialized
		open_flag = MAGIC;
		return 0;
		}
	catch (BoError err)
		{
		switch (err.number ())
			{
			case TOO_MANY_FILES:			
			case NOT_ENOUGH_MEMORY:		
			case NOT_ENOUGH_LOCKED_MEMORY:
				return (-1);
			case INVALID_FILE:
			case INVALID_DIRECTORY:
			case FILE_IO_ERROR:			
			case FILE_NOT_FOUND:
				return (-2);
			case NO_HARDWARE:
				return (-3);
			case DMA_ERROR:
				return (-5);
			case DSP_ERROR:
				return (-6);
			case INVALID_ARGUMENT:
 				return (-7);
			case TIMEOUT:
				return (-8);
			default:
				return (-9);
			}
		}
}

/*extern*/ /*"C"*/ short /*FAR PASCAL _export*/ bo_close ()
{
#ifdef DEBUG
	MessageBox (NULL, "", "called CLOSE", MB_OK);
#endif
	/* asm cld; */
	_fpreset ();
	(void)signal (SIGINT, (void (*)(int))SIG_IGN);
	(void)_control87 (MCW_EM, MCW_EM);
	try
		{
		if (open_flag != MAGIC) // driver not open
			{
			return (-100);
			}
		delete driver;
		driver = NULL;
		open_flag = 0; // reset open flag
		return 0;
		}
	catch (BoError err)
		{
		return (-101);
		}
}

short  start_mode;
double start_wait;
long   start_scans;
long   start_runs;
double start_delay;
short  start_type;
double start_sn;
double start_sx;
short  start_apod;
short  start_phase_res;
short  start_phase_apod;
short  start_dbl;

int bo_work () {
  return(driver->work ());
}

/*extern*/ /*"C"*/ short /*FAR PASCAL _export*/ bo_start (short mode, double wait,
								long scans, long runs,
								double delay, short type, double sn,
								double sx, short apod, short phase_res,
								short phase_apod)
{
	/* asm cld; */
	_fpreset ();
	(void)signal (SIGINT, (void (*)(int))SIG_IGN);
	(void)_control87 (MCW_EM, MCW_EM);

#ifdef DEBUG
	char buf[200];

	sprintf (buf, "%d, %f, %ld, %ld, %f, %d, %f, %f, %d, %d, %d", mode,wait,scans,runs,delay,type,sn,sx,apod,phase_res,phase_apod);
	MessageBox (NULL, buf, "called START", MB_OK);
#endif
	try
		{
		if (open_flag != MAGIC)
			{
			return (-201); // driver not open
			}

		start_mode  = mode;
		start_wait  = wait;
		start_scans = scans;
		start_runs  = runs;
		start_delay = delay;
		start_type  = type;
		start_sn    = sn;
		start_sx    = sx;
		start_apod  = apod;
		start_phase_res  = phase_res;
		start_phase_apod = phase_apod;
		start_dbl = mode ? (scans ? 1 : 0) : 0;

		switch (mode)
			{
			case 0: // align
				switch (type)
					{
					case INTERFEROGRAM:
						driver->align (INTERFEROGRAM, (short) scans);
						break;
					case RAW_SPECTRUM:
						if (driver->types () & RAW_SPECTRUM)
							{
							driver->get_status ();
							driver->inst_stat.detectors.p[0].apodization =
																		apod;
							driver->inst_stat.detectors.p[0].phase_apod =
																phase_apod;
							driver->inst_stat.detectors.p[0].phase_res =
																phase_res;
							driver->inst_stat.detectors.p[0].sn = sn;
							driver->inst_stat.detectors.p[0].sx = sx;
							driver->inst_stat.detectors.p[1].apodization =
																		apod;
							driver->inst_stat.detectors.p[1].phase_apod =
																phase_apod;
							driver->inst_stat.detectors.p[1].phase_res =
																phase_res;
							driver->inst_stat.detectors.p[1].sn = sn;
							driver->inst_stat.detectors.p[1].sx = sx;
							driver->align (RAW_SPECTRUM, (short) scans);
							}
						else if (driver->types () &	COMPLEX_RAW)
							{
							driver->get_status ();
							driver->inst_stat.detectors.p[0].apodization =
																		apod;
							driver->inst_stat.detectors.p[0].phase_res =
																phase_res;
							driver->inst_stat.detectors.p[0].sn = sn;
							driver->inst_stat.detectors.p[0].sx = sx;
							driver->inst_stat.detectors.p[1].apodization =
																		apod;
							driver->inst_stat.detectors.p[1].phase_res =
																phase_res;
							driver->inst_stat.detectors.p[1].sn = sn;
							driver->inst_stat.detectors.p[1].sx = sx;
							driver->align (COMPLEX_RAW, (short) scans);
							}
						else
							{
							driver->align (INTERFEROGRAM, (short) scans);
							}
						break;
					}
				break;
			case 1: // acquire
				switch (type)
					{
					case INTERFEROGRAM:
						driver->start (INTERFEROGRAM, wait, runs, scans,
																	delay);
						break;
					case RAW_SPECTRUM:
						if (driver->types () & RAW_SPECTRUM)
							{
							driver->get_status ();
							driver->inst_stat.detectors.p[0].apodization =
																		apod;
							driver->inst_stat.detectors.p[0].phase_apod =
																phase_apod;
							driver->inst_stat.detectors.p[0].phase_res =
																phase_res;
							driver->inst_stat.detectors.p[0].sn = sn;
							driver->inst_stat.detectors.p[0].sx = sx;
							driver->inst_stat.detectors.p[1].apodization =
																		apod;
							driver->inst_stat.detectors.p[1].phase_apod =
																phase_apod;
							driver->inst_stat.detectors.p[1].phase_res =
																phase_res;
							driver->inst_stat.detectors.p[1].sn = sn;
							driver->inst_stat.detectors.p[1].sx = sx;
							driver->start (RAW_SPECTRUM, wait, runs, scans,
																	delay);
							}
						else if (driver->types () &	COMPLEX_RAW)
							{
							driver->get_status ();
							driver->inst_stat.detectors.p[0].apodization =
																		apod;
							driver->inst_stat.detectors.p[0].phase_res =
																phase_res;
							driver->inst_stat.detectors.p[0].sn = sn;
							driver->inst_stat.detectors.p[0].sx = sx;
							driver->inst_stat.detectors.p[1].apodization =
																		apod;
							driver->inst_stat.detectors.p[1].phase_res =
																phase_res;
							driver->inst_stat.detectors.p[1].sn = sn;
							driver->inst_stat.detectors.p[1].sx = sx;
							driver->start (COMPLEX_RAW, wait, runs, scans,
																	delay);
							}
						else
							{
							driver->start (INTERFEROGRAM, wait, runs, scans,
																	delay);
							}
						break;
					}
				break;
			}
		return 0;
		}

	catch (BoError err)
		{
		switch (err.number ())
			{
			case NOT_ENOUGH_MEMORY:		
			case NOT_ENOUGH_LOCKED_MEMORY:
				return (-200);
			case TIMEOUT:
				return (-206);		
			case ACQUISITION_ERROR:
			case DMA_ERROR:
			case DSP_ERROR:			
				return (-204);
			case FIFO_TOO_SMALL:
				return (-207);
			default:
				return (-205);
			}
		}
}

/*extern*/ /*"C"*/ short /*FAR PASCAL _export*/ bo_get_data (short source,
					BoGrams_status *status, long npts,
#ifdef NO_FLOAT
 long *data
#else
 float *data
#endif
, short zpd_flag)
{
  short dbl;
  long i;

  /* asm cld; */
  _fpreset ();
  (void)signal (SIGINT, (void (*)(int))SIG_IGN);
  (void)_control87 (MCW_EM, MCW_EM);

  try
    {
      if (open_flag != MAGIC)
	{
	  return (-301); // driver not open
	  }
#ifdef NO_FLOAT
      BoMemory<long> buf;
#else
      BoMemory<float> buf;
#endif

      driver->get_status ();

      switch (driver->inst_stat.drv_stat)
	{
	case INIT:
	case READY:
	case FAIL:
	  if (source == 0) // data from FIFO when FIFO empty
	    {
	      return (-307);
	    }
	case ALIGN:
	case ACQUIRE:
	case ACQUIRE_DATA:
	case DONE:
	  short d = driver->inst_stat.current_det;
	  if (source == 1) // data from current acquisition
	    {
	      driver->copy (d, buf);
	    }

	  status->sn = driver->inst_stat.detectors.p[d].sn;
	  status->sx = driver->inst_stat.detectors.p[d].sx;
	  status->tm = driver->inst_stat.scantime.total_seconds ();
	  status->et = driver->inst_stat.elapsed.total_seconds ();
	  status->tl = driver->inst_stat.left.total_seconds ();
	  status->s0 = max (driver->inst_stat.scans0,1L);
	  status->s1 = max (driver->inst_stat.scans1,1L);
	  status->sb = driver->inst_stat.bad_scans;
	  status->seq = driver->inst_stat.sequence;
	  status->npts = driver->inst_stat.detectors.p[d].npts;
	  status->spd	= driver->inst_stat.speed;
	  status->res = driver->inst_stat.resolution;
	  status->sp = driver->inst_stat.detectors.p[d].samples;

	  switch (driver->inst_stat.drv_stat)
	    {
	    case ALIGN:
	    case ACQUIRE:
	      status->done = 1; // bit 0 on bit 1 off
		break;
	    case ACQUIRE_DATA:
	      status->done = 3; // bit 0 & 1 on
		break;
	    case DONE:
	      status->done = 2; // bit 0 off bit 1 on
		break;
	    default:
	      status->done = 0; // bit 0 & 1 off
		break;
	    }
	  status->done |= (driver->inst_stat.fifo_stat << 2);

	  dbl = (!!driver->inst_stat.scans0) +
	    ((!!driver->inst_stat.scans1) << 1);
	  dbl = start_dbl ? (dbl ? dbl : 1) : 0;

	  switch (source)
	    {
	    case 0: // data in FIFO
	      driver->data (buf);
	      if (dbl) dbl = 3; // always complete when in FIFO

#if 0
		{
		  // write interferograms to disk as a multifile
		    static short first = 1;
		  static char name[100] = "c:\\bgrams\\data\\interf.spc";
		  static Gr_handle hdl;
		  char xt, yt, zt;
		  short year;
		  char month, day, hour, mins;
		  char gal_form;

		  if (first)
		    {
		      xt = XDBLIGM;
		      yt = YIGRAM;
		      zt = XSECS;
		      year = 1995;
		      month = 10;
		      day = 29;
		      hour = 2;
		      mins = 0;
		      gal_form = 1;
		      gra_format (1);
		      gra_open (1, name, &hdl, buf.size (),
				status->sn, status->sx*(buf.size ()-3.0)/buf.size (),
				&xt, &yt, &zt, NULL, NULL, NULL, &year, &month, &day,
				&hour, &mins, "", "", "", &gal_form);
		      first = 0;
		    }
		  gra_write (&hdl, driver->inst_stat.sequence, buf,
			     driver->inst_stat.sequence, 0);
		  if (driver->inst_stat.sequence == start_runs-1)
		    {
		      gra_close (hdl);
		    }
		}
#endif
	      break;
	    case 1: // data from current acquisition
	      driver->get_fifo_status ();
	      break;
	    }

	  status->tmf = driver->inst_stat.scantime.total_seconds ();
	  status->s0f = max (driver->inst_stat.scans0,1L);
	  status->s1f = max (driver->inst_stat.scans1,1L);
	  status->sbf = driver->inst_stat.bad_scans;
	  status->seqf = driver->inst_stat.sequence;

	  // perform any needed computations on data
	    switch (start_type)
	      {
	      case INTERFEROGRAM:
		if (start_dbl)
		  {
		    status->npts *= 2;
		    // correct interferogram frequency for
		      // Galactic GRAMS convention
			status->sx *= (status->npts-3.0)/status->npts;
		  }
		else
		  {
		    // correct interferogram frequency for
		      // Galactic GRAMS convention
			status->sx *= (status->npts-2.0)/status->npts;
		  }

		// compute zpd max amplitudes
#ifdef NOMUX
		status->zpd_pos = status->zpd_neg = 0.0f;
#else
		status->A_zpd_pos = status->A_zpd_neg = buf.p[0];
		status->B_zpd_pos = status->B_zpd_neg = buf.p[0];
		status->A_l_zpd_pos = status->A_l_zpd_neg = 0L;
		status->B_l_zpd_pos = status->B_l_zpd_neg = 0L;
#endif
		if (zpd_flag) {
#ifdef NOMUX
		  for (i = 0; i < buf.size (); i++)
		    {
		      if (buf.p[i] > status->zpd_pos)
			{
			  status->zpd_pos = buf.p[i];
			  status->l_zpd_pos = i;
			}
		      if (buf.p[i] < status->zpd_neg)
			{
			  status->zpd_neg = buf.p[i];
			  status->l_zpd_neg = i;
			}
		    }
#else
		  for (i = 0; i < (buf.size () /4); i+=2)
		    {
		      if (buf.p[i] > status->A_zpd_pos)
			{
			  status->A_zpd_pos = buf.p[i];
			  status->A_l_zpd_pos = i/2;
			}
		      if (buf.p[i] < status->A_zpd_neg)
			{
			  status->A_zpd_neg = buf.p[i];
			  status->A_l_zpd_pos = i/2;
			}
  		    } 
		  for (i = 1; i <= (buf.size () / 4); i+=2)
		    {
		      if (buf.p[i] > status->B_zpd_pos)
			{
			  status->B_zpd_pos = buf.p[i];
			  status->B_l_zpd_pos = (i+1)/2;
			}
		      if (buf.p[i] < status->B_zpd_neg)
			{
			  status->B_zpd_neg = buf.p[i];
			  status->B_l_zpd_pos = (i+1)/2;
			}
	            }
#endif
		}
		break;
	      case RAW_SPECTRUM:
		switch (driver->types () &
			(INTERFEROGRAM|COMPLEX_RAW|RAW_SPECTRUM))
		  {
		  case INTERFEROGRAM:
		    {
#ifdef NOMUX
		      // compute zpd max amplitudes
			status->zpd_pos = status->zpd_neg = 0.0f;
		      for (i = 0; i < buf.size (); i++)
			{
			  if (buf.p[i] > status->zpd_pos)
			    {
			      status->zpd_pos = buf.p[i];
			    }
			  if (buf.p[i] < status->zpd_neg)
			    {
			      status->zpd_neg = buf.p[i];
			    }
			}
#endif

		      // compute spectrum
			double sn = start_sn;
		      double sx = start_sx;
#ifndef NO_FLOAT
		      spectrum (buf, dbl, status->sx, &sn,
				&sx, start_apod, start_phase_apod,
				short (status->sp * 32768L /
				       start_phase_res));
#endif
		      status->sn = sn;
		      status->sx = sx;
		      status->npts = buf.size ();
		      break;
		    }
		  case INTERFEROGRAM|COMPLEX_RAW:
		    {
		      short pnpts =
			short (status->sp * 32768L /
			       start_phase_res);
#ifdef NOMUX
		      // compute zpd max amplitudes
			status->zpd_pos = status->zpd_neg = 0.0f;
		      for (i = status->npts*2;
			   i < status->npts*2 + pnpts; i++)
			{
			  if (buf.p[i] > status->zpd_pos)
			    {
			      status->zpd_pos = buf.p[i];
			    }
			  if (buf.p[i] < status->zpd_neg)
			    {
			      status->zpd_neg = buf.p[i];
			    }
			}
		      if (dbl)
			{
			  for (i = status->npts*4 + pnpts;
			       i < status->npts*4 + pnpts*2; i++)
			    {
			      if (buf.p[i] > status->zpd_pos)
				{
				  status->zpd_pos = buf.p[i];
				}
			      if (buf.p[i] < status->zpd_neg)
				{
				  status->zpd_neg = buf.p[i];
				}
			    }
			}
#endif
		      // compute phase correction
			double sn = start_sn;
		      double sx = start_sx;
		      double sf = open_laser/2 * (start_dbl+1) *
			status->sp;
#ifndef NO_FLOAT
		      spectrum (buf, status->sp * 32768L /
				status->res, status->npts, dbl, sf,
				&sn, &sx, start_phase_apod,	pnpts);
#endif
		      break;
		    }
		  case INTERFEROGRAM|COMPLEX_RAW|RAW_SPECTRUM:
		    // Warning this format does not allow zpd
		      // value extraction, this function will have
			// to be part of the low level driver
			  // in the next release!!!
			    break;
		  }
		break;
	      }
	  // avoid transiant in align mode
	    npts = __min (npts, status->npts);
	  npts = __min (npts, buf.size ());
	  bocopy (npts, buf.p, data);

#if 0
	  if (source == 0)
	    {
	      // write spectra to disk as a multifile
		static short first = 1;
	      static char name[100] = "c:\\bgrams\\data\\spec.spc";
	      static Gr_handle hdl;
	      char xt, yt, zt;
	      short year;
	      char month, day, hour, mins;
	      char gal_form;

	      if (first)
		{
		  xt = XWAVEN;
		  yt = YSB;
		  zt = XSECS;
		  year = 1995;
		  month = 10;
		  day = 29;
		  hour = 2;
		  mins = 0;
		  gal_form = 1;
		  gra_format (1);
		  gra_open (1, name, &hdl, buf.size (),
			    status->sn, status->sx,
			    &xt, &yt, &zt, NULL, NULL, NULL, &year, &month, &day,
			    &hour, &mins, "", "", "", &gal_form);
		  first = 0;
		}
	      gra_write (&hdl, status->seqf, buf, status->seqf, 0);
	      if (status->seqf == start_runs-1)
		{
		  gra_close (hdl);
		}
	    }
#endif


	  break;
	}
      // activate LED on DSB48 for debug
	// outpw (0x278, status->seq%10);
      return 0;
    }

  catch (BoError err)
    {
      switch (err.number ())
	{
	case NOT_ENOUGH_MEMORY:		
	case NOT_ENOUGH_LOCKED_MEMORY:
	  return (-300);
	case TIMEOUT:
	  return (-306);		
	case DMA_ERROR:
	case DSP_ERROR:			
	case ACQUISITION_ERROR:
	  return (-304);
	case FIFO_UNDERRUN:
	  return (-308);
	default:
	  return (-305);
	}
    }
}

/*extern*/ /*"C"*/ short /*FAR PASCAL _export*/ bo_get_status (BoGrams_status *status)
{
	/* asm cld; */
	_fpreset ();
	(void)signal (SIGINT, (void (*)(int))SIG_IGN);
	(void)_control87 (MCW_EM, MCW_EM);

	try
		{
		if (open_flag != MAGIC)
			{
			return (-401); // driver not open
			}
		driver->get_status ();
		short d = driver->inst_stat.current_det;
		status->sn = driver->inst_stat.detectors.p[d].sn;
		status->sx = driver->inst_stat.detectors.p[d].sx;
		status->tm = driver->inst_stat.scantime.total_seconds ();
		status->et = driver->inst_stat.elapsed.total_seconds ();
		status->tl = driver->inst_stat.left.total_seconds ();
		status->s0 = max (driver->inst_stat.scans0,1L);
		status->s1 = max (driver->inst_stat.scans1,1L);
		status->sb = driver->inst_stat.bad_scans;
		status->seq = driver->inst_stat.sequence;
		status->npts = driver->inst_stat.detectors.p[d].npts;
		status->spd	= driver->inst_stat.speed;
		status->res = driver->inst_stat.resolution;
		status->sp = driver->inst_stat.detectors.p[d].samples;
#ifdef NOMUX
		status->zpd_pos = 0.0f;
		status->zpd_neg = 0.0f;
#endif

		switch (driver->inst_stat.drv_stat)
			{
			case ALIGN:
			case ACQUIRE:
				status->done = 1; // bit 0 on bit 1 off
				break;
			case ACQUIRE_DATA:
				status->done = 3; // bit 0 & 1 on
				break;
			case DONE:
				status->done = 2; // bit 0 off bit 1 on
				break;
			default:
				status->done = 0; // bit 0 & 1 off
				break;
			}
		status->done |= (driver->inst_stat.fifo_stat<<2);

		driver->get_fifo_status ();
		status->tmf = driver->inst_stat.scantime.total_seconds ();
		status->s0f = max (driver->inst_stat.scans0, 1L);
		status->s1f = max (driver->inst_stat.scans1, 1L);
		status->sbf = driver->inst_stat.bad_scans;
		status->seqf = driver->inst_stat.sequence;
		// Adjust sn, sx and npts if needed
		switch (start_type)
			{
			case INTERFEROGRAM:
				if (start_dbl)
					{
					status->npts *= 2;
					// correct interferogram frequency for
					// Galactic GRAMS convention
					status->sx *= (status->npts-3.0)/status->npts;
					}
				else
					{
					// correct interferogram frequency for
					// Galactic GRAMS convention
					status->sx *= (status->npts-2.0)/status->npts;
					}
				break;
			case RAW_SPECTRUM:
				if (driver->types () == INTERFEROGRAM)
					{
					if (start_dbl)
						{
						status->sx = status->sx / 2;
						}
					long t;
					double sn = start_sn;
					double sx = start_sx;
					comp_limits (&sn, &sx, &status->npts, status->sx, &t, &t);

					status->sn = sn;
					status->sx = sx;
					}
				break;
			}
		// activate LED on DSB48 for debug
		// outpw (0x278, status->seq%10);
		return 0;
		}

	catch (BoError err)
		{
		switch (err.number ())
			{
			case NOT_ENOUGH_MEMORY:		
			case NOT_ENOUGH_LOCKED_MEMORY:
				return (-400);
			case TIMEOUT:
				return (-406);		
			case DMA_ERROR:
			case DSP_ERROR:			
			case ACQUISITION_ERROR:
				return (-404);
			default:
				return (-405);
			}
		}
}

/*extern*/ /*"C"*/ short /*FAR PASCAL _export*/ bo_stop ()
{
	/* asm cld; */
	_fpreset ();
	(void)signal (SIGINT, (void (*)(int))SIG_IGN);
	(void)_control87 (MCW_EM, MCW_EM);

	try
		{
		if (open_flag != MAGIC)
			{
			return (-501); // driver not open
			}
		driver->stop ();
		return (0);
		}

	catch (BoError err)
		{
		switch (err.number ())
			{
			case NOT_ENOUGH_MEMORY:		
			case NOT_ENOUGH_LOCKED_MEMORY:
				return (-500);
			case TIMEOUT:
				return (-505);		
			case DMA_ERROR:
			case DSP_ERROR:			
			case ACQUISITION_ERROR:
				return (-504);
			default:
				return (-506);
			}
		}
}

