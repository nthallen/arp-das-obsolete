#ifndef IDX64INT_H_INCLUDED
#define IDX64INT_H_INCLUDED

#include <sys/types.h>
#ifndef IDX64_H_INCLUDED
  #error Must include idx64.h before idx64int.h
#endif

#define MAX_IDXRS 4
#define MAX_IDXR_CHANS 6
#define DFLT_CHAN_CFG 0xC20

typedef struct ixcmdlist {
  struct ixcmdlist *next;
  idx64_cmnd c;
} ixcmdl;

typedef struct {
  unsigned short base_addr;
  unsigned short *tm_ptr;
  unsigned short scan_bit;
  unsigned short on_bit;
  unsigned short supp_bit;
  step_t online;
  step_t online_delta;
  step_t offline_delta;
  step_t altline_delta;
  ixcmdl *first;
  ixcmdl *last;
} chandef;

/* This is the structure for boards actually in use */
typedef struct {
  pid_t proxy; /* proxy for intserv for this board */
  unsigned short request; /* bit-map of drive requests */
  unsigned short scans; /* bit-map of scans */
  chandef chans[ MAX_IDXR_CHANS ];
} idx64_bd;

/* This structure separates out the static board definitions */
typedef struct {
  unsigned short card_base;
  char * cardID;
} idx64_def;

extern idx64_def idx_defs[ MAX_IDXRS ];
extern idx64_bd *boards[ MAX_IDXRS ];
extern char *idx64_cfg_string;
extern int idx64_region;

#endif
