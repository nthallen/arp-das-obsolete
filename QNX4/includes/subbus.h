/* subbus.h defines the interface to the subbus resident library
 * Before calling the subbus routines, you must first call
 * load_subbus().  This returns the subfunction of the resident
 * subbus library or 0 if none is installed.  If you call a
 * subbus function without initializing, or if the initialization
 * fails, you are guaranteed to crash.
 ****************************************************************
 * $Log$
 * Revision 1.5  1992/09/09  18:45:23  nort
 * Latest version
 *
 * Revision 1.4  1992/08/20  20:56:40  nort
 * Add explicit cast to read_ack #define
 *
 * Revision 1.3  1992/08/03  16:29:41  nort
 * Added definitions for sbb() and sbw()
 *
 * Revision 1.2  1992/06/18  15:58:50  nort
 * Major revisions for QNX4.
 *
 * Revision 1.1  1992/06/16  17:24:50  nort
 * Initial revision
 *
 * Added late version functions and included SIC definitions from
 * sic.h on October 15, 1991
 */
#ifndef _SUBBUS_H
#define _SUBBUS_H
#ifndef __TYPES_H_INCLUDED
  #include <sys/types.h>
#endif

#define SB_PCICC 1
#define SB_PCICCSIC 2
#define SB_SYSCON 3

struct sbf {
  unsigned int subbus_version;
  unsigned int subbus_features;
  unsigned int subfunction;
  unsigned int (far *read_subbus)(unsigned int);
  int (far *writeack)(unsigned int, unsigned int);
  int (far *read_ack)(unsigned int, unsigned int far *data);
  void (far *set_cmdenbl)(int value);
  unsigned char (far *read_novram)(unsigned int address);
  void (far *write_novram)(unsigned int address, unsigned char val);
  unsigned int (far *read_switches)(void);
  void (far *set_failure)(int value);
  unsigned char (far *read_rstcnt)(void);
  unsigned char (far *read_pwrcnt)(void);
  unsigned char (far *read_failure)(void);
};
						/* subbus_features: */
#define SBF_SIC 1		/* SIC Functions */
#define SBF_LG_RAM 2	/* Large NVRAM */
#define SBF_HW_CNTS 4	/* Hardware rst & pwr Counters */
#define SBF_WD 8		/* Watchdog functions */
#define SBF_SET_FAIL 0x10 /* Set failure lamp */
#define SBF_READ_FAIL 0x20 /* Read failure lamps */

extern struct sbf sbfs;
extern pid_t sb_pid;
int load_subbus(void);
void far sbsnload(void);

#define SIG_NOSLIB SIGABRT

/* here are the redefinitions of the subbus functions */
#define read_subbus(x,y)    sbfs.read_subbus(y)
#define read_ack(x,y,z)     sbfs.read_ack(y,(unsigned int far *) z)
#define write_subbus(x,y,z) ((void)sbfs.writeack(y,z))
#define write_ack(x,y,z)    sbfs.writeack(y,z)
#define set_cmdenbl(v)	    sbfs.set_cmdenbl(v)
#define read_novram(a)	    sbfs.read_novram(a)
#define write_novram(a,v)   sbfs.write_novram(a,v)
#define read_switches()	    sbfs.read_switches()
#define set_failure(v)	    sbfs.set_failure(v)
#define read_rstcnt()	    sbfs.read_rstcnt()
#define read_pwrcnt()	    sbfs.read_pwrcnt()
#define read_failure()		sbfs.read_failure()
#define subbus_version      sbfs.subbus_version
#define subbus_features		sbfs.subbus_features
#define subbus_subfunction	sbfs.subfunction

#define sbw(x) read_subbus(0,x)
#define sbwr(x,y) write_subbus(0,x,y)
#define sbwra(x,y) write_ack(0,x,y)

/* These functions will be implemented via normal message IPC */
void enable_nmi(void (far *func)(void));
int  set_tps(unsigned int tps);
int  tick_sic(void);
int  tick_check(char id, unsigned int secs);
void disable_nmi(void);
void disarm_sic(void);
void reboot(unsigned char critstat);
char *get_subbus_name(void);
#define subbus_name get_subbus_name()
unsigned int sbb(unsigned int);
int get_nvrdir(unsigned char ID, unsigned short size, struct nvram_reply *rep);

#define SBMSG_LOAD     129
#define SBMSG_ENA_NMI  130
#define SBMSG_DIS_NMI  131
#define SBMSG_TICK     132
#define SBMSG_DIS_TICK 133
#define SBMSG_TICK_CHK 134
#define SBMSG_SET_TPS  135
#define SBMSG_GET_NAME 136
#define SBMSG_REBOOT   137
#define SBMSG_QUIT     138
#define SBMSG_NVRR     139
#define SBMSG_NVR_INIT 140
struct sb_tps {
  unsigned char type;
  unsigned int tps;
};
struct sb_tchk {
  unsigned char type;
  unsigned char id;
  unsigned int secs;
};
struct sb_ena_nmi {
  unsigned char type;
  void (far *func)(void);
  unsigned short ds;
};
struct sb_reboot {
  unsigned char type;
  unsigned char critstat;
};
struct nvram_dir_rqst {
  unsigned char type;
  unsigned char ID;
  unsigned short size;
};
struct nvram_dir_entry {
  unsigned char ID;
  unsigned short start;
  unsigned short end;
};
struct nvram_reply {
  signed char code;
  struct nvram_dir_entry dir;
};
#define NVRR_NO_SUBBUS (-1)
#define NVRR_NOT_SUPPORTED (-2)
#define NVRR_MISMATCH (-3)
#define NVRR_NO_SPACE (-4)
#define NVRR_NO_DIR_SPACE (-5)
#define NVRR_NOT_INITD (-6)
#define NVRR_NEW_ENTRY 1
#define NVRR_OLD_ENTRY 2
/* NVRR codes:
   NVRR_NO_SUBBUS subbus library is not resident
   NVRR_NOT_SUPPORTED resident library does not support this function
   NVRR_MISMATCH A previously defined entry for this ID had a different size
   NVRR_NO_SPADE There isn't enough NVRAM to fill this request
   NVRR_NEW_ENTRY A new entry was created to fill this request
   NVRR_OLD_ENTRY An existing entry matched this description
*/

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
#define SIC_TICKFAIL_ADDR 6

#if defined __386__
#  pragma library (subbus3r)
#elif defined __SMALL__
#  pragma library (subbuss)
#elif defined __COMPACT__
#  pragma library (subbusc)
#elif defined __MEDIUM__
#  pragma library (subbusm)
#elif defined __LARGE__
#  pragma library (subbusl)
# endif

#endif
