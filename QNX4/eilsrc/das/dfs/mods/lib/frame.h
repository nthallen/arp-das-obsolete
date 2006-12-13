#ifndef _FRAME_H_INCLUDED
#define _FRAME_H_INCLUDED
#include <dbr.h>

#define MFC_TIME(TIMESTAMP,MFC) (TIMESTAMP.secs + ((long)(MFC-TIMESTAMP.mfc_num) * dbr_info.nrowminf * tmi(nsecsper)) / tmi(nrowsper))

#define MFC(X) ( (X[tmi(mfc_msb)] << 8) | (X[tmi(mfc_lsb)]) )

extern int check_synch(unsigned char *databuf);
extern int check_maj_synch(unsigned char *databuf);
extern int check_any_synch(unsigned char *databuf);
extern unsigned int getmfc(unsigned char *databuf);

#endif
