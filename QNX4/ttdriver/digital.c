/* digital.c
   void tt_write_digital( unsigned short dig_word, unsigned char value );
   unsigned short tt_read_digital( unsigned short dig_word );

   Valid output words are 8-15
   Valid input words are 0-5, 8-15
   Reading from 8-15 returns values previously written which are
   stored in software. The board does not have a readback mechanism.
*/
#include <conio.h>
#include "ttdriver.h"
#include "ttintern.h"

#define TT_MAX_DIG_INPUT 5
#define TT_DIG_OUT_BASE 8
#define TT_MAX_DIG_OUTPUT 15
#define N_OUTPUTS ( TT_MAX_DIG_OUTPUT - TT_DIG_OUT_BASE + 1 )

/* Write a word out
  CFG  <- TT_WRITE_CFG
  CTRL <- 1
  ADDR <- word address
  DATA <- output value
  CTRL <- 0    Latch it
  CTRL <- 1    Reset Latch
  ADDR <- dummy address
  CFG  <- TT_READ_CFG
*/

static unsigned char saved_outputs[ N_OUTPUTS ];

unsigned short tti_write_digital( unsigned short dig_word, unsigned char value ) {
  if ( dig_word >= TT_DIG_OUT_BASE && dig_word <= TT_MAX_DIG_OUTPUT ) {
	outp( TT_CFG_PORT, TT_WRITE_CFG );
	outp( TT_CTRL_PORT, 1 );
	outp( TT_ADDR_PORT, dig_word + TT_DIGITAL_BASE );
	outp( TT_DATA_PORT, value );
	outp( TT_CTRL_PORT, 0 );
	outp( TT_CTRL_PORT, 1 );
	outp( TT_ADDR_PORT, TT_DUMMY_ADDR );
	outp( TT_CFG_PORT , TT_READ_CFG );
	saved_outputs[ dig_word - TT_DIG_OUT_BASE ] = value;
	return 0;
  } else return ~0;
}

/* Input
  CFG  <- TT_READ_CFG   Standard input config.
  CTRL <- 1      To enable outputs
  ADDR <- word address
  val  <- DATA
  ADDR <- dummy address
*/

unsigned short tti_read_digital( unsigned short dig_word ) {
  if ( dig_word <= TT_MAX_DIG_INPUT ) {
	unsigned char val;

	outp( TT_CFG_PORT, TT_READ_CFG );
	outp( TT_CTRL_PORT, 1 );
	outp( TT_ADDR_PORT, dig_word + TT_DIGITAL_BASE );
	val = inp( TT_DATA_PORT );
	outp( TT_ADDR_PORT, TT_DUMMY_ADDR );
	return val;
  } else if ( dig_word >= TT_DIG_OUT_BASE &&
			  dig_word <= TT_MAX_DIG_OUTPUT ) {
	return saved_outputs[ dig_word - TT_DIG_OUT_BASE ];
  } else return ~0;
}

/*  0 <= cmd_no <=  63 Clear the associated bit
  100 <= cmd_no <= 163 Set the associated bit (cmd_no - 100)
  Returns 0 unless the channel number is out of range.
*/
unsigned short tti_scdc_command( unsigned char cmd_no ) {
  unsigned short chan, addr, bit;
  unsigned char word;
  
  chan = ( cmd_no >= 100 ) ? ( cmd_no - 100 ) : cmd_no;
  if ( chan >= 64 ) return ~0;
  addr = chan / 8;
  word = saved_outputs[ addr ];
  bit = 1 << (chan % 8);
  if ( cmd_no >= 100 ) word |= bit;
  else word &= ~bit;
  return tti_write_digital( addr + TT_DIG_OUT_BASE, word );
}
