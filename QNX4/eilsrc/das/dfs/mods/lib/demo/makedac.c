#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbr_utils.h>


main(int argc, char **argv) {
	tm_info_type t;
	
	of_open(argv[1]);	
	strcpy(t.tmid.ident,argv[2]);
	t.nbminf=atoi(argv[3]);
	t.nbrow=atoi(argv[4]);
	t.nrowmajf=atoi(argv[5]);
	t.nsecsper=atoi(argv[6]);
	t.nrowsper=atoi(argv[7]);
	t.mfc_lsb=atoi(argv[8]);
	t.mfc_msb=atoi(argv[9]);	
	t.synch=atoh(argv[10]);
	t.isflag=atoi(argv[11]);

	of_rec_beg(OFF_HDR);
	of_rec_data((unsigned char *)&t,sizeof(tm_info_type));
	of_rec_end();
	of_close();
}
