/* ttintern.h
   Header for Thompson Stack Driver Internals
*/
#ifndef TTINTERN_H_INC
#define TTINTERN_H_INC

#ifndef TTDRIVER_H_INC
  #error Must include ttdriver.h before ttintern.h
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* A/D function: atod.c */
unsigned short tti_read_atod( unsigned short channel );

/* Digital Functions: digital.c */
unsigned short tti_write_digital( unsigned short dig_word,
								  unsigned short value );
unsigned short tti_read_digital( unsigned short dig_word );

/* SCDC Functions: digital.c */
unsigned short tti_scdc_command( unsigned char cmd_no );

/* GC functions: gc.c */
unsigned short tti_gc_reset( void );
unsigned short tti_gc_byte( void );
unsigned short tti_gc_read( gc_data_buf *gcbuf );
unsigned long tti_gc_chan( unsigned short chan );

/* DAC Functions: dac.c */
unsigned short tti_dac_out( unsigned short channel, unsigned short value );
unsigned short tti_dac_in( unsigned short channel );
unsigned short tti_dac_enbl( unsigned short safe );

/* Counter Functions */

/* Timeout Functions */
void init_timer( void );     
void start_timer( void );
void stop_timer( void );
extern int timed_out;

/* Thompson Stack Constants */
#define TT_DATA_PORT (0x250)
#define TT_ADDR_PORT (TT_DATA_PORT+1)
#define TT_CTRL_PORT (TT_DATA_PORT+2)
#define TT_CFG_PORT (TT_DATA_PORT+3)
/* CFG Port is bit-mapped as:  x x x A Ch x B Cl
   The x's are don't care (set to zero to be neat)
   A, B, Ch and Cl refer to their respective ports.
     1 = input
	 0 = output
   Power-on default is 0x1B (all input)
   Standard reading config is 0x18 (address port and low ctrl out)
   Standard writing config is 0x08 (only high ctrl is in)
*/
#define TT_READ_CFG (0x18)
#define TT_WRITE_CFG (0x08)
#define TT_DUMMY_ADDR (0xFF)

#define TT_DIGITAL_BASE (0x90)
#define TT_GC_BASE (0xAF)
#define TT_DAC_BASE (0xC0)
#define TT_N_DAC_CHANS (10)
#define TT_DAC_OUTPUTS (TT_DAC_BASE+TT_N_DAC_CHANS)

#ifdef __cplusplus
};
#endif

#endif
