/* dmmdrv.h
   Header for DMM32 Driver Library Interface
*/
#ifndef DMMDRV_H_INC
#define DMMDRV_H_INC

#ifdef __cplusplus
extern "C" {
#endif

/* Initialization */
int dmm_init( void );
unsigned short dmm_shutdown( void );

/* SCDC Functions */
unsigned short dmm_scdc_command( unsigned char cmd_no );
unsigned short dmm_scdc_multcmd( unsigned char *cmds );

/* Internal Definitions */

#define DMM_NAME "dmmdriver"

typedef struct {
  unsigned short signature;   /* 'dm' */
  unsigned short function; /* dmmmsgtype */
  unsigned short address;
  unsigned short data;
} Send_to_dmm;

typedef struct {
  unsigned short signature;   /* 'dm' */
  unsigned short function; /* dmmmsgtype */
  unsigned short address;
  double data;
} Send_gain_to_dmm;

typedef struct {
  unsigned short signature;
  unsigned short function;
  unsigned short fromMass;
  unsigned short toMass;
  unsigned short byMass;
  unsigned short dwell;
} Send_scan_to_dmm;

typedef enum {
  DMMMSG_READ,
  DMMMSG_WRITE,
  DMMMSG_QUIT,
  DMMMSG_SCDC,
  DMMMSG_R_RF_AD,
  DMMMSG_R_RF_DA,
  DMMMSG_W_RF_DA,
  DMMMSG_R_500V_AD,
  DMMMSG_R_500V_DA,
  DMMMSG_W_500V_DA,
  DMMMSG_SET_MASS,
  DMMMSG_SET_GAIN,
  DMMMSG_SCAN_MASS,
  DMMMSG_SCAN_FREQ,
  DMMMSG_MAX
} dmmmsgtype;

typedef struct {
  unsigned short signature; /* 'dm' */
  union {
	unsigned short shrt;
	unsigned long lng;
  } value;
} Reply_from_dmm;

unsigned short dmm_command( unsigned short function,
                           unsigned short address,
						   unsigned short data );
unsigned short dmm_scan_Mass(
		  unsigned short fromMass,
		  unsigned short toMass,
		  unsigned short byMass,
		  unsigned short dwell );
unsigned short dmm_scan_Freq(
		  unsigned short fromFreq,
		  unsigned short toFreq,
		  unsigned short byFreq,
		  unsigned short dwell );

#define dmm_shutdown() dmm_command( DMMMSG_QUIT, 0, 0 )

#define dmm_read(a) dmm_command( DMMMSG_READ, (a), 0 )
#define dmm_write(a,v) dmm_command( DMMMSG_WRITE, (a), (v))
#define dmm_scdc_command(a) dmm_command( DMMMSG_SCDC, (a), 0 )
#define dmm_quit()  dmm_command( DMMMSG_QUIT, 0, 0 )
#define NCAR_read_rf_a2d(a) dmm_command(DMMMSG_R_RF_AD,(a),0)
#define NCAR_read_rf_d2a(a) dmm_command(DMMMSG_R_RF_DA,(a),0)
#define NCAR_write_rf_d2a(a,v) dmm_command(DMMMSG_W_RF_DA,(a),(v))
#define NCAR_read_500V_a2d(a) dmm_command(DMMMSG_R_500V_AD,(a),0)
#define NCAR_read_500V_d2a(a) dmm_command(DMMMSG_R_500V_DA,(a),0)
#define NCAR_write_500V_d2a(a,v) dmm_command(DMMMSG_W_500V_DA,(a),(v))
#define NCAR_set_mass(m) dmm_command(DMMMSG_SET_MASS,0,(m))
#define NCAR_set_gain(a,g) dmm_gain(a,g)

#define DMM_N_ATODS 32
#define DMM_MIN_DTOA (DMM_N_ATODS*16)

#define DMM_N_DTOAS 4
#define DMM_MIN_DIO (DMM_MIN_DTOA+DMM_N_DTOAS)
#define DMM_N_DIOS 3
#define DMM_MIN_UNDEF (DMM_MIN_DIO+DMM_N_DIOS)
#define DMM_BRDINC 0x300
#define DMM_N_BRDS 3

#ifdef __cplusplus
};
#endif

#if defined __386__
#  pragma library (nortlib3r)
#elif defined __SMALL__
#  pragma library (nortlibs)
#elif defined __COMPACT__
#  pragma library (nortlibc)
#elif defined __MEDIUM__
#  pragma library (nortlibm)
#elif defined __LARGE__
#  pragma library (nortlibl)
# endif

#endif
