#include <semaphore.h>
#include <sys/stat.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "subbus.h"
#include "nortlib.h"

#define LIBRARY_SUB SB_SYSCON104
#define SUBBUS_VERSION 0x400 /* subbus version 4.00 QNX6 */

//---------------------------------------------------------------------
//Subbus Features:
//---------------------------------------------------------------------
#define SBF_SIC 1 // SIC Functions (SICFUNC)
#define SBF_LG_RAM 2 // Large NVRAM (SYSCON)
#define SBF_HW_CNTS 4 // Hardware rst & pwr Counters (SIC)
#define SBF_WD 8 // Watchdog functions (always)
#define SBF_SET_FAIL 0x10 // Set failure lamp (Comes w/ SICFUNC)
#define SBF_READ_FAIL 0x20 // Read failure lamps (SYSCON)
#define SBF_READ_SW 0x40 // Read Switches (was SICFUNC, now indep.)
#define SBF_NVRAM 0x80 // Any NVRAM at all!
#define SBF_CMDSTROBE 0x100 // Cmdstrobe Function


#if LIBRARY_SUB == 1
  #define PCICC 1
  #define SIC 0
  #define SYSCON 0
  #define SICFUNC 0
  #define SC104 0
  #define READSWITCH 0
  #define NVRAM 0
#endif

#if LIBRARY_SUB == 2
  #define PCICC 1
  #define SIC 1
  #define SYSCON 0
  #define SICFUNC 1
  #define SC104 0
  #define READSWITCH 1
  #define NVRAM 1
#endif

#if LIBRARY_SUB == 3
  #define PCICC 0
  #define SIC 0
  #define SYSCON 1
  #define SICFUNC 1
  #define SC104 0
  #define READSWITCH 1
  #define NVRAM 1
#endif

#if LIBRARY_SUB == 4
  #define PCICC 0
  #define SIC 0
  #define SYSCON 1
  #define SICFUNC 0
  #define SC104 1
  #define READSWITCH 1
  #define NVRAM 0
#endif

#if PCICC
  #define SC_SB_RESET 0x310
  #define SC_SB_LOWA 0x308
  #define SC_SB_HIGHA 0x30C
  #define SC_SB_LOWB 0x309
  #define SC_SB_HIGHB 0x30D
  #define SC_SB_LOWC 0x30A
  #define SC_SB_HIGHC 0x30E
  #define SC_SB_LOWCTRL 0x30B
  #define SC_SB_HIGHCTRL 0x30F
  #define SC_SB_CONFIG 0x0C1C0
  #define SC_CMDENBL 0x311
  #define SC_DISARM 0x318
  #define SC_TICK 0x319
  #define WAIT_COUNT 10
#endif

#if SIC
  #define SC_RES_CNT 0x31A
  #define SC_PWR_CNT 0x31B
  #define SC_RAMADDR 0x31C
  #define SC_NMI_ENABLE 0x31C
  #define SC_NMIE_VAL 0x20
  #define SC_RAMDATA 0x31D
  #define SC_SWITCHES 0x31E
  #define SC_LAMP 0x31F
#endif

#if SYSCON
  #define SC_SB_RESET 0x310
  #define SC_SB_LOWA 0x308
  #define SC_SB_LOWB 0x30A
  #define SC_SB_LOWC 0x30C
  #define SC_SB_LOWCTRL 0x30E
  #define SC_SB_HIGHA 0x309
  #define SC_SB_HIGHB 0x30B
  #define SC_SB_HIGHC 0x30D
  #define SC_SB_HIGHCTRL 0x30F
  #define SC_SB_CONFIG 0x0C1C0
  #define SC_CMDENBL 0x318
  #define SC_DISARM 0x311
  #define SC_TICK 0x319
  #define SC_LAMP 0x317
	#if SC104
    #define SC_SWITCHES 0x316
    #define WAIT_COUNT 5
    #define SET_FAIL 1
	#else
    #define SC_RAMADDR 0x31A
    #define SC_RAMDATA 0x31D
    #define SC_NMI_ENABLE 0x31C
    #define SC_NMIE_VAL 1 
    #define WAIT_COUNT 1
    #define SC_SWITCHES 0x31C
    #define LG_RAM 1
	#endif
#endif

#if SICFUNC
  #define TICKFAIL 6 // novram addr for tick fail info
  #ifndef SET_FAIL
    #define SET_FAIL 1
  #endif
#endif

//----------------------------------------------------------------
// SC104 is the first unit without any NVRAM
//----------------------------------------------------------------
#ifndef SET_FAIL
  #define SET_FAIL 0
#endif
#ifndef LG_RAM
  #define LG_RAM 0
#endif

#define SUBBUS_FEATURES (SBF_WD | (SIC*SBF_HW_CNTS) | (SET_FAIL*SBF_SET_FAIL) | \
 (LG_RAM*SBF_LG_RAM) | (SYSCON*(SBF_READ_FAIL|SBF_CMDSTROBE)) | \
 (SICFUNC*SBF_SIC) | (READSWITCH*SBF_READ_SW) | (NVRAM*SBF_NVRAM))


static sem_t *sb_sem = (sem_t *)(-1);

// We assume we have the semaphore locked at this point
static void init_subbus(void) {
  out16( SC_SB_RESET, 0 );
  #if !SC104
    #if SYSCON
      out16( SC_SB_LOWCTRL, SC_SB_CONFIG );
    #else
      out8( SC_SB_LOWCTRL, SC_SB_CONFIG & 0xFF );
      out8( SC_SB_HIGHCTRL, (SC_SB_CONFIG >> 8) & 0xFF );
    #endif
  #endif
}

int load_subbus(void) {
  if (ThreadCtl(_NTO_TCTL_IO,0) == -1 )
    nl_error( 3, "Error requesting I/O priveleges: %s", strerror(errno) );
  // We won't do mmap_device_io()
  // http://www.qnx.com/developers/articles/article_304_2.html
  while ( ((int)sb_sem) == -1 ) {
    sb_sem = sem_open("/subbus", 0 );
    if ( ((int)sb_sem) == -1 ) {
      if ( errno == ENOENT ) {
        // it doesn't exist, try creating it locked
        sb_sem = sem_open("/subbus", O_CREAT|O_EXCL, S_IRWXU, 0);
        if ( ((int)sb_sem) == -1 ) {
          if ( errno == EEXIST )
            nl_error( 2, "Subbus semaphore created before we got to it: retrying" );
          else
            nl_error( 3, "Failed to create semaphore: %s", strerror(errno));
        } else {
          init_subbus();
          if (sem_post(sb_sem))
            nl_error( 3, "Error unlocking subbus semaphore after initialization" );
          nl_error( 0, "Subbus initialized" );
        }
      } else nl_error( 3, "Error opening semaphore: %s", strerror(errno));
    }
  }
  return LIBRARY_SUB;
}

unsigned int subbus_version = SUBBUS_VERSION;
unsigned int subbus_features = SUBBUS_FEATURES;
unsigned int subbus_subfunction = LIBRARY_SUB;


#define SB_BASE_NAME "Subbus Library V4.00"
char *get_subbus_name(void) {
  #if LIBRARY_SUB == SB_PCICC
    return SB_BASE_NAME ": PC/ICC";
  #endif
  #if LIBRARY_SUB == SB_PCICCSIC
    return SB_BASE_NAME ": PC/ICC+SIC";
  #endif
  #if LIBRARY_SUB == SB_SYSCON
    return SB_BASE_NAME ": Syscon";
  #endif
  #if LIBRARY_SUB == SB_SYSCON104
    return SB_BASE_NAME ": Syscon104";
  #endif
}


#if SC104
int read_ack( unsigned short addr, unsigned short *data ) {
  int i, status;
  if (sem_wait( sb_sem ))
    nl_error( 3, "Error from sem_wait() in read_ack: %s", strerror(errno));
  out16(SC_SB_LOWB, addr); // Output address
  out8(SC_SB_LOWC, 1); // assert read
  for ( i = WAIT_COUNT; i > 0; i-- ) {
    status = in16(SC_SB_LOWC); // Check read+write bit
    if ( !(status | 0x800) ) break;
  }
  *data = in16(SC_SB_LOWA);
  sem_post(sb_sem);
  return((status&0x40) ? i+1 : 0 );
}
#endif

unsigned short read_subbus(unsigned short addr) {
  unsigned short data;
  read_ack(addr, &data);
  return data;
}

unsigned int sbb(unsigned short addr) {
  unsigned int word;
  
  word = read_subbus(addr);
  if (addr & 1) word >>= 8;
  return(word & 0xFF);
}

/* returns zero if no acknowledge */
unsigned int sbba(unsigned short addr) {
  unsigned short word;
  
  if ( read_ack( addr, &word ) ) {
  	if (addr & 1) word >>= 8;
  	return( word & 0xFF );
  } else return 0;
}

/* returns zero if no acknowledge */
unsigned int sbwa(unsigned short addr) {
  unsigned short word;
  
  if ( read_ack( addr, &word ) )
    return word;
  else return 0;
}

#if SC104
int write_ack(unsigned short addr, unsigned short data) {
  int i, status;
  if (sem_wait( sb_sem ))
    nl_error( 3, "Error from sem_wait() in write_ack: %s", strerror(errno));
  out16(SC_SB_LOWB, addr); // Output address
  out16(SC_SB_LOWA, data); // Output data
  for ( i = WAIT_COUNT; i > 0; i-- ) {
    status = in16(SC_SB_LOWC);
    if ( !(status & 0x800) ) break;
  }
  sem_post(sb_sem);
  return ((status&0x40) ? i+1 : 0 );
}
#endif

/** don't need to serialize access to cmdenbl, since it's I/O port mapped.
 */
void set_cmdenbl(int val) {
  out16(SC_CMDENBL, val);
}

unsigned int read_switches(void) {
  #if READSWITCH
    return in8(SC_SWITCHES) | 0x8000;
  #else
    return 0x8000;
  #endif
}

void set_failure(int value) {
  #if SIC
    if ( value ) in8(SC_LAMP);
    else out8(SC_LAMP,0);
  #endif
  #if SYSCON
    out8(SC_LAMP,value);
  #endif
}

unsigned char read_failure(void) {
  #if SYSCON
    return in8(SC_LAMP);
  #else
    return 0;
  #endif
}

/**
 * Function to set cmdstrobe value.
 * @param val non-zero value asserts cmdstrobe.
 * @return non-zero no success, zero if operation is not supported.
 *
 *  Function did not exist at all before version 3.10, so
 *  programs intending to use this function should verify that
 *  the resident library version is at least 3.10. The feature
 *  word can also be checked for support, and that is consistent
 *  back to previous versions.
 */
short int set_cmdstrobe(short int value) {
  #if SYSCON
    #if SC104
      out8(SC_SB_LOWC, 8 | (value?0:2));
    #else
      out8(SC_SB_LOWCTRL, 2 | (value?1:0));
    #endif
    return 1;
  #else
    return 0;
  #endif
}

int  tick_sic(void) {
  out8(SC_TICK, 0);
}

void disarm_sic(void) {
  out8(SC_DISARM, 0);
}
