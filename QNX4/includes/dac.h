#ifndef _DAC_H_INCLUDED
#define _DAC_H_INCLUDED

/* dacoff.h defines the format for the
     Data
     Acquisition
     Compiler
     Object
     File
     Format
   Written May 21, 1991

$Id$
$Log$
 * Revision 1.2  1992/05/19  15:04:02  nort
 * Removed unused record types, added Log and Id
 *

The object file format is based on a record structure similar to that
found in the Intel Object Module Format. Each record begins with a
record type and a record size followed by the record data and a
checksum of the record data. The idea is that the record structure
allows us to add more features to the output file format without
destroying the format. The current record types are:

  OFF_HDR     1
     Header data matches the "tm_info" structure found in dring.h
        char ident[9];
        unsigned int nbminf;
        unsigned int nbrow;
        unsigned int nrowmajf;
        unsigned int nrowsec;
        unsigned int mfc_lsb;
        unsigned int mfc_msb;
        unsigned int synch;
        unsigned int isflag;
           Bit 0 is '1' if inverted synch is indicated.
           All other bits are reserved.
  OFF_COMMENT 6
     Data is ignored. Should be ASCIIZ.
  Additional candidates for record types might be:
    Datum name and type information for interactive extraction.
    Display background information.
*/

/* record types: */
#define OFF_HDR     1
#define OFF_COMMENT 2

/* defined in dacoff.c */
int of_open(char *name);
void of_close(void);
void of_rec_beg(unsigned int rectype);
void of_rec_data(unsigned char *buf, unsigned int n_bytes);
void of_rec_end(void);

/* .DAC input routines defined in dacin.c: */
int dac_open(char *filename);
void dac_close(void);
int dac_next_rec(void);
unsigned int dac_rec_size(void);
unsigned int dac_rec(void *buf, unsigned int maxsize);
unsigned int dac_read(void *buf, unsigned int n_bytes);
int dac_checksum(void);

#endif
