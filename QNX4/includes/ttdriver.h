/* ttdriver.h
   Header for Thompson Stack Driver Library Interface
*/
#ifndef TTDRIVER_H_INC
#define TTDRIVER_H_INC

#ifdef __cplusplus
extern "C" {
#endif

/* Initialization */
unsigned short tt_init( void );
unsigned short tt_shutdown( void );

/* A/D function */
unsigned short tt_read_atod( unsigned short channel );

/* Digital Functions */
unsigned short tt_write_digital( unsigned short dig_word, unsigned short value );
unsigned short tt_read_digital( unsigned short dig_word );

/* SCDC Functions */
unsigned short tt_scdc_command( unsigned char cmd_no );
unsigned short tt_scdc_multcmd( unsigned char *cmds );

/* GC functions */
#define N_GC_CHANNELS 4
#define MAX_GC_SAMPLES 17
typedef struct {
  unsigned short end_offset[ N_GC_CHANNELS ];
  unsigned long samples[ MAX_GC_SAMPLES * N_GC_CHANNELS ];
} gc_data_buf;

unsigned short tt_gc_reset( void );
unsigned short tt_gc_byte( void );
unsigned short tt_gc_read( gc_data_buf *gcbuf );
unsigned long tt_gc_chan( unsigned short chan );

/* DAC Functions */
unsigned short tt_dac_out( unsigned short channel, unsigned short value );
unsigned short tt_dac_in( unsigned short channel );
unsigned short tt_dac_enbl( unsigned short safe );

/* Counter Functions */

/* Internal Definitions */

#define TT_NAME "ttdriver"

typedef struct {
  unsigned short signature;   /* 'tt' */
  unsigned short function; /* ttmsgtype */
  unsigned short address;
  unsigned short data;
} Send_to_tt;

typedef enum {
  TTMSG_READ_ATOD,
  TTMSG_WRITE_DIGITAL,
  TTMSG_READ_DIGITAL,
  TTMSG_GC_RESET,
  TTMSG_GC_BYTE,
  TTMSG_GC_READ,
  TTMSG_GC_CHAN,
  TTMSG_DAC_OUT,
  TTMSG_DAC_IN,
  TTMSG_DAC_ENBL,
  TTMSG_QUIT,
  TTMSG_SCDC,
  TTMSG_MAX
} ttmsgtype;

typedef struct {
  unsigned short signature; /* 'tt' */
  union {
	unsigned short shrt;
	unsigned long lng;
  } value;
} Reply_from_tt;

typedef struct {
  unsigned short signature; /* 'tt' */
  gc_data_buf gcbuf;
} Reply_from_gc;

unsigned short tt_command( unsigned short function,
                           unsigned short address,
						   unsigned short data );

#define tt_shutdown() tt_command( TTMSG_QUIT, 0, 0 )
#define tt_gc_reset() tt_command( TTMSG_GC_RESET, 0, 0 )
#define tt_gc_byte() tt_command( TTMSG_GC_BYTE, 0, 0 )

#define tt_read_atod(x) tt_command( TTMSG_READ_ATOD, x, 0 )
#define tt_read_digital(x) tt_command( TTMSG_READ_DIGITAL, x, 0 )
#define tt_dac_in(x) tt_command( TTMSG_DAC_IN, x, 0 )
#define tt_dac_enbl(x) tt_command( TTMSG_DAC_ENBL, x, 0 )

#define tt_write_digital(a,v) tt_command( TTMSG_WRITE_DIGITAL, a, v )
#define tt_scdc_command( a ) tt_command( TTMSG_SCDC, a, 0 )
#define tt_dac_out(a,v) tt_command( TTMSG_DAC_OUT, a, v )

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
