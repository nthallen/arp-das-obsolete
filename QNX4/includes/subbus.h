/* subbus.h defines the interface to the subbus resident library
   Before calling the subbus routines, you must first call
   load_subbus().  This returns the subfunction of the resident
   subbus library or 0 if none is installed.  If you call a
   subbus function without initializing, or if the initialization
   fails, you are guaranteed to crash.
   
   $Log$
   Added late version functions and included SIC definitions from
   sic.h on October 15, 1991
*/
#ifndef _SUBBUS_H
#define _SUBBUS_H

#define SB_PCICC 1
#define SB_PCICCSIC 2
#define SB_SYSCON 3

struct sbf {
  unsigned int (far *read_subbus)(unsigned int);
  int (far *writeack)(unsigned int, unsigned int);
  int (far *read_ack)(unsigned int, unsigned int far *data);
  void (far *set_cmdenbl)(int value);
  int  (far *tick_sic)(void);
  void (far *disarm_sic)(void);
  unsigned char (far *read_novram)(unsigned int address);
  void (far *write_novram)(unsigned int address, unsigned char val);
  unsigned int (far *read_switches)(void);
  void (far *enable_nmi)(void (far *func)(void));
  void (far *disable_nmi)(void);
  void (far *set_failure)(int value);
  unsigned char (far *read_rstcnt)(void);
  unsigned char (far *read_pwrcnt)(void);
  /* end of version 1 structure */
  unsigned int subbus_version;
  char far *subbus_name;
  int (far *set_tps)(unsigned int tps);
  int (far *tick_check)(char id, unsigned int secs);
  void (far *reboot)(unsigned char critstat);
};

extern struct sbf sbfs;
#ifdef _RESLIB_H
  #define load_subbus() load_lib(1,sbfs)
#endif

/* here are the redefinitions of the subbus functions */
#define read_subbus(x,y)    sbfs.read_subbus(y)
#define read_ack(x,y,z)     sbfs.read_ack(y,z)
#define write_subbus(x,y,z) ((void)sbfs.writeack(y,z))
#define write_ack(x,y,z)    sbfs.writeack(y,z)
#define set_cmdenbl(v)	    sbfs.set_cmdenbl(v)
#define tick_sic()	    sbfs.tick_sic()
#define disarm_sic()	    sbfs.disarm_sic()
#define read_novram(a)	    sbfs.read_novram(a)
#define write_novram(a,v)   sbfs.write_novram(a,v)
#define read_switches()	    sbfs.read_switches()
#define enable_nmi(x)	    sbfs.enable_nmi(x)
#define disable_nmi()	    sbfs.disable_nmi()
#define set_failure(v)	    sbfs.set_failure(v)
#define read_rstcnt()	    sbfs.read_rstcnt()
#define read_pwrcnt()	    sbfs.read_pwrcnt()
#define subbus_version      sbfs.subbus_version
#define subbus_name         sbfs.subbus_name
#define set_tps(v)          sbfs.set_tps(v)
#define tick_check(i,v)     sbfs.tick_check(i,v)
#define reboot(v)          sbfs.reboot(v)

/* Formerly sic.h
   Defines the pertinent addresses used by the sic card as well as
   some important constant definitions for data stored in the non-volatile
   RAM and for the hardware switches.
   Written April 27, 1987 annexed October 15, 1991
*/

/* These are the I/O addresses of SIC functions.  These should no
   longer be addressed directly, but rather through resident libraries.
	SIC_RES_CNT	0x31A
	SIC_PWR_CNT	0x31B
	SIC_ADDR_LATCH	0x31C
	SIC_NOVRAM	0x31D
	SIC_SWITCHES	0x31E
	SIC_LAMP	0x31F
          Write Turns it Off, Read turns it on
*/

/* These are the switch masks - all of the switches are true in low */
#define SW_POWER_GOOD 1
#define SW_GROUND_POWER 2
#define SW_DUMP_DATA 4
#define SW_DIAGNOSTIC 8
#define SW_MODE 0x80
#define SW_VERSION 0x8000

/* These are the status values for NOVRAM address 0 - the main status word
   Additional status values can be added at any time.
	SIC_READY	default start-up code before a flight.
	SIC_INIT	Initialization has begun.
	SIC_RUNNING	code written to the sic when the flight program is
			started.  If read on power-up, this code indicates
			an unanticipated crash occurred - bad news.
	SIC_PFAIL_DET	This code is written out to the sic immediately on
			detecting a low power situation.  This is followed
			by emergency shut down procedures (all the off
			commands at once!).  Reading this on power up
			indicates that we saw it coming, but didn't have time
			to do much about it.
	SIC_PFAIL_OK	If after low power is detected, shut down is
			completed before losing it totally, this code is
			written out to indicate that the instrument has been
			secured for power failure.  Reading this on power up
			indicates we successfully responded to a low power
			signal.
	SIC_FLT_OVER	After the flight sequence is completed, this code
			is written out to the SIC.  If read on power up, no
			action should be taken unless the external switches
			are positioned for a tape dump.
	SIC_FLT_OVER2	This code is written after grndctrl regains control
			after the flight algorithm is terminated.
	SIC_DUMPED	After the data has been written to the tape, we write
			this code.  If read on power up, no action should be
			taken.
	SIC_PWR_CYC_REQ	The software has requested a power cycle by lighting
			the failure lamp.  On power up, the situation should
			be diagnosed.  If the situation has not been
			remedied, a failure should be declared.
	SIC_FAILURE	The software has detected an unrecovereable system
			failure.  On power up, the system failure lamp should
			be lit and no further action should be undertaken. 
			The cause of the failure should be recorded in the
			power log file.
	SIC_EARLY	The flight algorithm was terminated early.  This is
			the same as FLT_OVER except in the case that the
			mode switch reads mode 1.  In that case, the command
			count register should be zeroed and the algorithm
			should be restarted.
	SIC_NMI_SEEN	This code is written if an NMI occurs which is
			not attributable to low power.  Such an NMI may
			be routine, but the low power nmisr has not way
			of distinguishing the benign from the
			catastrophic.  If in fact the NMI turns out to
			have been a catastrophic one, such as parity
			error, presumably the watch dog will time out
			and the system will reset and grndctrl will
			find out that an NMI occurred before the system
			crashed.  As with PFAIL_DET and PFAIL_OK, the
			presumption is that the system should be up
			and running under those circumstances.
	SIC_ABNORMAL	This is written if ADOS terminates with a
			critical status other then SIC_FLT_OVER or
			SIC_EARLY. The critical status in question
			is written to novram(7).
*/
#define SIC_READY 0
#define SIC_INIT 1
#define SIC_RUNNING 2
#define SIC_PFAIL_DET 3
#define SIC_PFAIL_OK 4
#define SIC_FLT_OVER 5
#define SIC_DUMPED 6
#define SIC_PWR_CYC_REQ 7
#define SIC_FAILURE 8
#define SIC_EARLY 9
#define SIC_FLT_OVER2 10
#define SIC_NMI_SEEN 11
#define SIC_ABNORMAL 12

#endif
