/* subbus-usb/src/subbus.c 
 */

#include <semaphore.h>
//#include <sys/stat.h>
//#include <sys/neutrino.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "subbus_usb.h"
#include "nortlib.h"
#include "nl_assert.h"

#define LIBRARY_SUB SB_SYSCONUSB
#define SUBBUS_VERSION 0x500 /* subbus version 5.00 QNX6 */

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

#if LIBRARY_SUB == 4
  #define PCICC 0
  #define SIC 0
  #define SYSCON 1
  #define SICFUNC 0
  #define SC104 1
  #define READSWITCH 1
  #define NVRAM 0
#endif

#if LIBRARY_SUB == 5
  #define PCICC 0
  #define SIC 0
  #define SYSCON 1
  #define SICFUNC 0
  #define SC104 0
  #define READSWITCH 0
  #define NVRAM 0
  #define SET_FAIL 0
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
static sb_fd;

// We assume we have the semaphore locked at this point
static void init_subbus(void) {
  int n;
  char buf[MAX_REV_LEN+1];
  write( sb_fd, "B\n", 2 );
  // readcond( sb_fd, buf, n, min, time, timeout );
  n = readcond( sb_fd, buf, MAX_REV_LEN, 1, 1, 5 );
  if ( n < 0 ) {
    nl_error( 2, "Error reading board rev: %s", strerror(errno) );
  } else if (n == 0) {
    nl_error( 2, "No response from rev query" );
  } else {
    if ( n >= MAX_REV_LEN )
      nl_error(1, "Likely overflow in rev query" );
    buf[n] = '\0';
    if ( buf[n-1] == '\n' )
      buf[n-1] = '\0';
    nl_error( 0, "SBusb Initialized: %s", buf );
  }
}

int load_subbus(void) {
  // if (ThreadCtl(_NTO_TCTL_IO,0) == -1 )
  //  nl_error( 3, "Error requesting I/O priveleges: %s", strerror(errno) );
  // We won't do mmap_device_io()
  // http://www.qnx.com/developers/articles/article_304_2.html
  sb_fd = open("/dev/serusb2", O_RDWR );
  if ( sb_fd == -1 )
    nl_error( 3, "Error opening USB subbus: %s", strerror(errno));
  while ( ((int)sb_sem) == -1 ) {
    sb_sem = sem_open("/subbus-usb", 0 ); // opening does not lock
    if ( ((int)sb_sem) == -1 ) {
      if ( errno == ENOENT ) {
        // it doesn't exist, try creating it locked
        sb_sem = sem_open("/subbus-usb", O_CREAT|O_EXCL, S_IRWXU, 0);
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


#define SB_BASE_NAME "Subbus Library V5.00"
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
  #if LIBRARY_SUB == SB_SYSCONUSB
    return SB_BASE_NAME ": SysconUSB";
  #endif
}


int read_ack( unsigned short addr, unsigned short *data ) {
  int n_out, n_in;
  char buf[8];

  sprintf( buf, "R%04X\n", addr );
  if (sem_wait( sb_sem ))
    nl_error( 3, "Error from sem_wait() in read_ack: %s", strerror(errno));
  n_out = write(sb_fd, buf, 6);
  if ( n_out == 6 ) {
    n_in = readcond( sb_fd, buf, 7, 1, 1, 1 );
  }
  sem_post(sb_sem);
  if ( n_out < 0 )
    nl_error( 3, "Error writing to usb: %s", strerror(errno) );
  else if ( n_out != 6 )
    nl_error( 3, "Unexpected output count in read_ack: %d (expected 6)", n_out );
  else if ( n_in < 1 )
    nl_error( 3, "Error reading from usb: %s", strerror(errno) );
  else if (n_in != 6 ) {
    if ( buf[0] == 'E' )
      nl_error( 1, "Error %c from syscon in read_ack", buf[1] ); 
    else nl_error( 3,
	"Unexpected input count in read_ack: %d (expected 6)", n_in );
  } else {
    int i, nv;
    unsigned short idata = 0;
    for ( i = 1; i <= 4; i++ ) {
      int c = buf[i];
      if ( isdigit(c) ) nv = c - '0';
      else if (isxdigit(c)) {
        if (isupper(c)) nv = c - 'A' + 10;
        else nv = c - 'a' + 10;
      } else {
        nl_error( 1, "Invalid character in read_ack" );
        nv = 0;
      }
      idata = (idata<<4) + nv;
    }
    *data = idata;
  }
  return(buf[0] == 'R' ? 1 : 0 );
}

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

int write_ack(unsigned short addr, unsigned short data) {
  int n_out, n_in;
  char buf[12];

  sprintf( buf, "W%04X:%04X\n", addr, data );
  if (sem_wait( sb_sem ))
    nl_error( 3, "Error from sem_wait() in write_ack: %s", strerror(errno));
  n_out = write(sb_fd, buf, 11);
  if ( n_out == 11 ) {
    n_in = readcond( sb_fd, buf, 3, 1, 1, 1 );
  }
  sem_post(sb_sem);
  if ( n_out < 0 )
    nl_error( 3, "Error writing to usb in write_ack: %s", strerror(errno) );
  else if ( n_out != 11 )
    nl_error( 3, "Unexpected output count in write_ack: %d (expected 11)", n_out );
  else if ( n_in < 1 )
    nl_error( 3, "Error reading from usb in write_ack: %s", strerror(errno) );
  else if (n_in != 2 )
    nl_error( 3, "Unexpected input count in write_ack: %d (expected 2)", n_in );
  else if ( buf[0] == 'E' )
    nl_error( 1, "Error %c from syscon in write_ack", buf[1] ); 
  return(buf[0] == 'W' ? 1 : 0 );
}

static void send_CS( char code, int val ) {
  int n_out;
  char buf[12];

  nl_assert( code == 'S' || code == 'C' );
  sprintf( buf, "%c%d\n", code, val ? 1 : 0 );
  if (sem_wait( sb_sem ))
    nl_error( 3, "Error from sem_wait() in send_CS: %s", strerror(errno));
  n_out = write(sb_fd, buf, 3);
  sem_post(sb_sem);
  if ( n_out < 0 )
    nl_error( 3, "Error writing to usb in send_CS: %s", strerror(errno) );
  else if ( n_out != 3 )
    nl_error( 3, "Unexpected output count in send_CS: %d (expected 3)", n_out );
}

/* CMDENBL "Cn\n" where n = 0 or 1. Response should be the same */
void set_cmdenbl(int val) {
  send_CS( 'C', val );
}

unsigned int read_switches(void) {
  #if READSWITCH
    return in8(SC_SWITCHES) | 0x8000;
  #else
    return 0x8000;
  #endif
}

void set_failure(int value) {
  #if SET_FAIL
    #if SIC
      if ( value ) in8(SC_LAMP);
      else out8(SC_LAMP,0);
    #endif
    #if SYSCON
      out8(SC_LAMP,value);
    #endif
  #endif
}

unsigned char read_failure(void) {
  #if SET_FAIL && SYSCON
    return in8(SC_LAMP);
  #else
    return 0;
  #endif
}

/**----------------------------------------------------------------------
 * short int set_cmdstrobe(short int val)
 *   val == 0 turns the cmdstrobe.
 *   val == 1 turns on cmdstrobe.
 *  Returns non-zero on success, zero if operation isn't supported.
 *  Function did not exist at all before version 3.10, so
 *  programs intending to use this function should verify that
 *  the resident library version is at least 3.10. The feature
 *  word can also be checked for support, and that is consistent
 *  back to previous versions.
---------------------------------------------------------------------- */
short int set_cmdstrobe(short int value) {
  send_CS('S', value);
  return 1;
}

int  tick_sic(void) {
  // out8(SC_TICK, 0);
  return 0;
}

void disarm_sic(void) {
  // out8(SC_DISARM, 0);
}
