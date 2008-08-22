/* subbus.h defines the interface to the subbus resident library
 * Before calling the subbus routines, you must first call
 * load_subbus().  This returns the subfunction of the resident
 * subbus library or 0 if none is installed.  If you call a
 * subbus function without initializing, or if the initialization
 * fails, you are guaranteed to crash.
 */
#ifndef SUBBUS_H_INCLUDED
#define SUBBUS_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

/* Subbus version codes */
#define SB_PCICC 1
#define SB_PCICCSIC 2
#define SB_SYSCON 3
#define SB_SYSCON104 4

/* subbus_features: */
#define SBF_SIC 1		/* SIC Functions */
#define SBF_LG_RAM 2	/* Large NVRAM */
#define SBF_HW_CNTS 4	/* Hardware rst & pwr Counters */
#define SBF_WD 8		/* Watchdog functions */
#define SBF_SET_FAIL 0x10 /* Set failure lamp */
#define SBF_READ_FAIL 0x20 /* Read failure lamps */
#define SBF_READ_SW 0x40 /* Read Switches */
#define SBF_NVRAM 0x80   /* Any NVRAM at all! */
#define SBF_CMDSTROBE 0x100 /* CmdStrobe Function */

// extern struct sbf sbfs;
// extern pid_t sb_pid;
int load_subbus(void);
// void sbsnload(void);

//#define SIG_NOSLIB SIGABRT

unsigned int subbus_version;
unsigned int subbus_features;
unsigned int subbus_subfunction;
unsigned short read_subbus(unsigned short addr);
int write_ack(unsigned short addr, unsigned short data);
int read_ack(unsigned short addr, unsigned short *data);
void set_cmdenbl(int value);
unsigned int read_switches(void);
void set_failure(int value);
unsigned char read_failure(void);
short int cmdstrobe(short int value);
int  tick_sic(void);
void disarm_sic(void);
char *get_subbus_name(void);
#define subbus_name get_subbus_name()

#define sbw(x) read_subbus(x)
#define sbwr(x,y) write_ack(x,y)
#define sbwra(x,y) write_ack(x,y)
unsigned int sbb(unsigned short addr);
unsigned int sbba(unsigned short addr);
unsigned int sbwa(unsigned short addr);

// int  set_tps(unsigned int tps);
// int  tick_check(char id, unsigned int secs);
// void reboot(unsigned char critstat);
// int get_nvrdir(unsigned char ID, unsigned short size, struct nvram_reply *rep);

#ifdef __cplusplus
};
#endif

#endif
