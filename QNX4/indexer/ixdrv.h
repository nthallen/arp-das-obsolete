/* ixdrv.h is an include file internal to the indexer driver.
 * $Log$
 * Revision 1.2  1993/11/17  17:55:56  nort
 * made chandef.scan_bit short to support STV scan
 *
 * Revision 1.1  1992/09/24  20:39:31  nort
 * Initial revision
 *
 */
#ifndef IXDRV_H_INCLUDED
#define IXDRV_H_INCLUDED

#ifndef INDEXER_H_INCLUDED
  #error Must include indexer.h before ixdrv.h
#endif

typedef struct {
  step_t base_addr;
  unsigned char EIR;
  unsigned short scan_bit;
  unsigned char on_bit;
  unsigned char supp_bit;
} chandef;

typedef struct ixcmdlist {
  struct ixcmdlist *next;
  ixcmd c;
} ixcmdl;

typedef struct {
  ixcmdl *first;
  ixcmdl *last;
  step_t online;
  step_t online_delta;
  step_t offline_delta;
  step_t altline_delta;
  step_t scan_addr;
  step_t scan_amount;
  step_t w_amount;
  step_t to_go;
} drvstat;
#define IX_SCAN_PROXY 255
#define IX_CHAN_0_PROXY (IX_SCAN_PROXY-N_CHANNELS)

extern drvstat drive[N_CHANNELS];
#endif
