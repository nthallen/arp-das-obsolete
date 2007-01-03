#ifdef __QNX__
#include <sys/types.h>
struct BoGrams_status
	{
	double sn;	// sigma min or minimum x value
	double sx;	// sigma max or maximum x value
	double tm;	// absolute of spectrum being collected
	double et;	// elapsed time since acquisition start
	double tl;	// estimated time left to acquisition end
	double tmf;	// absolute time of spectrum in FIFO
	long s0;	// number of scans in direction 0
	long s1;	// number of scans in direction 1
	long sb;	// number of bad scans in this coad
	long seq;	// sequence number of this coad
	long s0f;	// number of scans in direction 0 in FIFO
	long s1f;	// number of scans in direction 1 in FIFO
	long sbf;	// number of bad scans in FIFO
	long seqf;	// sequence number of coad in FIFO
	long npts;	// number of points in result
	float spd;	// current scan speed in scans/minute
	short res;	// current instrument resolution 
 	short sp;  	// samples per fringe
	short done;	// completion flag, this is a bit field
			// bit 0 is on when an acquisition is in progress
			// bit 1 is on when the FIFO contains a complete coad
			// bit 2 is on if a FIFO overrun occured
#ifdef NOMUX
	float zpd_pos;	// largest positive value around interferogram center
	float zpd_neg;	// largest negative value around interferogram center
	long l_zpd_pos; // added by eil, location of zpd
	long l_zpd_neg; // added by eil
#else
	float A_zpd_pos;// largest positive value around interferogram center
	float A_zpd_neg;// largest negative value around interferogram center
	float B_zpd_pos;// largest positive value around interferogram center
	float B_zpd_neg;// largest negative value around interferogram center
	long A_l_zpd_pos; // added by eil, location of zpd
	long A_l_zpd_neg; // added by eil
	long B_l_zpd_pos; // added by eil, location of zpd
	long B_l_zpd_neg; // added by eil
#endif
	char pad[154];	// reserved for future use (set to zero)
	};

extern short  bo_open (short board, short instrument,
	 short irq, short dma, short ioadr, double laser, long bufsiz,
	 char *path, double timeout,
	 pid_t proxy, pid_t proxy_do, pid_t pen_proxy_set,pid_t pen_proxy_clr);
extern short  bo_close ();
extern short  bo_start (short mode, double wait,
	long scans, long runs,
	double delay, short type, double sn,
	double sx, short apod, short phase_res,
	short phase_apod);
extern short  bo_get_data (short source, BoGrams_status *status, long npts,
#ifdef NO_FLOAT
 long *data
#else
 float *data
#endif
, short zpd_flag);
extern short  bo_get_status (BoGrams_status *status);
extern short  bo_stop ();
extern int    bo_work ();
#endif
