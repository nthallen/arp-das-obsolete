#include <frame.h>
#include <dbr.h>

unsigned int getmfc(unsigned char *databuf) {
unsigned short mfctemp;	

	/* get mfc of this data */
	mfctemp = 0;
	mfctemp = (unsigned char )databuf[dbr_info.tm.mfc_msb] << 8;
	mfctemp |= (unsigned char)databuf[dbr_info.tm.mfc_lsb];
	return(mfctemp);
	
}

int check_synch(unsigned char *databuf) {
unsigned short synchtemp;

    synchtemp=0;
    synchtemp = databuf[tmi(nbminf)-1] << 8;
    synchtemp |= databuf[tmi(nbminf)-2];
    return(synchtemp==tmi(synch));
}

int check_maj_synch(unsigned char *databuf) {
unsigned short synchtemp;

    if (!tmi(isflag)) return(check_synch(databuf));
    synchtemp=0;
    synchtemp = databuf[tmi(nbminf)-1] << 8;
    synchtemp |= databuf[tmi(nbminf)-2];
    return(synchtemp == ~(tmi(synch)));
}

int check_any_synch(unsigned char *databuf) {
unsigned short synchtemp;
    if (!tmi(isflag)) return(check_synch(databuf));
    synchtemp=0;
    synchtemp = databuf[tmi(nbminf)-1] << 8;
    synchtemp |= databuf[tmi(nbminf)-2];
    return(synchtemp == tmi(synch) || synchtemp == ~(tmi(synch)) );
}
