#include <stdlib.h>
#include <stdio.h>
#include <sys/name.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <das_utils.h>
#include <dbr_utils.h>
#include <globmsg.h>

dbr_info_type init_msg = {{{'F'}, 32, 16, 16, 1, 16, 8, 9, 0xB4AB, 0 },
			    2, 16, 0, 1, {0, 0},DRC };

unsigned char muco_meco[] = {DCDATA, 4, 
221, 221, 221, 221, 221, 221, 221, 221, 221, 221,221,221,221,221,221,221,
221, 221, 221, 221, 221, 221, 221, 221, 221, 221,221,221,221,221,221,221,
221, 221, 221, 221, 221, 221, 221, 221, 221, 221,221,221,221,221,221,221,
221, 221, 221, 221, 221, 221, 221, 221, 221, 221,221,221,221,221,221,221};


struct {
	unsigned char b;
	unsigned int s;
	tstamp_type t;
}	ts = {TSTAMP,0,{0,0}};

token_type tkn;

main ()
	{
	int hid;	/* id of hex module */
	int mid;    /* id of this module */
	int delay;  /* delay value */
	unsigned int counter = 0;
	char hex_msg[100];
	struct _mxfer_entry rep[2];
	unsigned char okmsg = DAS_OK;
	char name[FILENAME_MAX+1];
	/* This module will simply store itself in memory, wait for an init msg,
	 * return a response and then wait forever.
	 */
	mid = qnx_name_attach (getnid(),LOCAL_SYMNAME(DG_NAME,name));
	
	init_msg.next_tid = getpid();

	hid = Receive (0, (void *) hex_msg, 100);
/*	hid = Receivemx (0, 1, rep);*/
	_setmx(&rep[0],&okmsg,1);
	_setmx(&rep[1],&init_msg,sizeof(init_msg));


/*	reply (hid, (void *) &init_msg, sizeof(init_msg));*/
	Replymx(hid, 2, rep);
	
	while (1) {


		if (!(counter%32)) {
			ts.t.secs++;
			ts.t.mfc_num = counter;			
			if (Send (hid, &ts, hex_msg, sizeof(ts), 100) == -1)
		       		exit(0);		   
		    Receive(0, &tkn, sizeof(tkn));
		    Reply(hid,&okmsg,1);
		}

		muco_meco[10] = counter & 0xFF; 
		muco_meco[11] = counter >> 0x08;
	
		counter++;
		muco_meco[26] = counter & 0xFF;
		muco_meco[27] = counter >> 0x08;

/*		counter++;*/
		muco_meco[42] = counter &  0xFF;
		muco_meco[43] = counter >> 0x08;

		counter++;
		muco_meco[58] = counter &  0xFF;
		muco_meco[59] = counter >> 0x08;	    
		
/*		counter++;*/

		if (Send (hid, muco_meco, hex_msg, sizeof(muco_meco), 100) == -1)
	   		exit(0);

		Receive(0, &tkn, sizeof(tkn));
		Reply(hid,&okmsg,1);
   	}

    for (delay = 0; delay < 20000; delay++);

}

	


