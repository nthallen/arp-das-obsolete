/* DAC Functions: dac.c */
#include <conio.h>
#include "ttdriver.h"
#include "ttintern.h"

static unsigned short saved_outputs[ TT_N_DAC_CHANS ];

/* D/A output:
  CFG  <- 0  All ports for output, even Ch!
  CTRL <- 4
  ADDR <- channel address
  DATA <- LSB
  CTRL <- 5   latches LSB
  DATA <- MSN
  CTRL <- 6   latches MSN
  CTRL <- 7
  ADDR <- output address
  CTRL <- 3
  CTRL <- 7
  ADDR <- dummy address
*/
unsigned short tti_dac_out( unsigned short channel, unsigned short value ) {
  if ( channel >= TT_N_DAC_CHANS ) return 0xFFFF;
  outp( TT_CFG_PORT, 0 ); /* All ports for output, even Ch! */
  outp( TT_CTRL_PORT, 4 );
  outp( TT_ADDR_PORT, channel + TT_DAC_BASE );
  outp( TT_DATA_PORT, value & 0xFF );         /* LSB */
  outp( TT_CTRL_PORT, 5 );
  outp( TT_DATA_PORT, ( value >> 8 ) & 0xF ); /* MSN */
  outp( TT_CTRL_PORT, 6 );
  outp( TT_CTRL_PORT, 7 );
  outp( TT_ADDR_PORT, TT_DAC_OUTPUTS );
  outp( TT_CTRL_PORT, 3 );
  outp( TT_CTRL_PORT, 7 );
  outp( TT_ADDR_PORT, TT_DUMMY_ADDR );
  saved_outputs[ channel ] = value & 0xFFF;
  return 0;
}

/* This returns the last value output. It's only software, but
   it might be convenient!
*/
unsigned short tti_dac_in( unsigned short channel ) {
  if ( channel >= TT_N_DAC_CHANS ) return 0xFFFF;
  return saved_outputs[ channel ];
}

/* If safe != 0, sets hardware to use the SAFE potentiometer.
   If safe == 0, hardware enables the value of the DAC.
   
   CFG  <- 0x10  Ain, Cout, Bout
   CTRL <- 7
   ADDR <- Output Address
       safe:          unsafe:
   CTRL <- 0x1F    CTRL <- 0x0F
   CTRL <- 0x17
       both:
   CTRL <- 7
   ADDR <- dummy address
*/
unsigned short tti_dac_enbl( unsigned short safe ) {
  outp( TT_CFG_PORT, 0x10 ); /* Ain, Cout, Bout */
  outp( TT_CTRL_PORT, 7 );
  outp( TT_ADDR_PORT, TT_DAC_OUTPUTS );
  if ( safe != 0 ) {
	outp( TT_CTRL_PORT, 0x1F );
	outp( TT_CTRL_PORT, 0x17 );
  } else outp( TT_CTRL_PORT, 0x0F );
  outp( TT_CTRL_PORT, 7 );
  outp( TT_ADDR_PORT, TT_DUMMY_ADDR );
  return 0;
}

