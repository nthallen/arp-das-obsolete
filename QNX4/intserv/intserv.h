/* intserv.h Defines interfaces for intserv program */
#ifndef INTSERV_H_INCLUDED
#define INTSERV_H_INCLUDED

#include <sys/types.h>

#define cardID_MAX 35

typedef struct {
  msg_t type;
  char cardID[ cardID_MAX ];
  union {
	short int irq;
	short int region;
  } u;
  mpid_t proxy;
  unsigned short address;
} IntSrv_msg;

typedef struct {
  msg_t status;
} IntSrv_reply;

/*
  Message types
*/
#define ISRV_IRQ_ATT 0xFA01
#define ISRV_IRQ_DET 0xFA02
#define ISRV_INT_ATT 0xFA03
#define ISRV_INT_DET 0xFA04
#define ISRV_QUIT    0xFA05

/* Define the name by which we will be known.
    Since we cannot tolerate more than one, we won't expand the
	name by Experiment.
 */
#include "company.h"
#define ISRV_NAME COMPANY "/intserv"

/* This is the API: */
extern int IntSrv_IRQ_attach( char *cardID, int IRQ, pid_t Proxy );
extern int IntSrv_IRQ_detach( char *cardID, int IRQ );
extern int IntSrv_Int_attach( char *cardID, unsigned short address,
						int region, pid_t Proxy );
extern int IntSrv_Int_detach( char *cardID );

#define ISRV_REGION_A 0x40
#define ISRV_REGION_B 0x42
#define ISRV_REGION_C 0x44
#define ISRV_REGION_D 0x46
#define ISRV_IRQ_SPARE 0
#define ISRV_IRQ_PFAIL 1
#define ISRV_MAX_IRQS 2

#endif
