#ifndef TRUE
	#define	TRUE	(1 == 1)
#endif
#ifndef FALSE
	#define	FALSE	(1 == 0)
#endif

#define FLAG int

#define	SUCCESS			0
#define	INV_INTR		1
#define	INT_USED		2
#define	INT_NOT_SET_UP	3
#define	OUT_OF_MEMORY	4
#define PORT_NOT_SET_UP	5

#define	MIN_DISK_FREE	50000000L	//	Minimum required disk free space
									//	before recording begins
struct OLD_FORMAT{
	 int 		header;
	 int 		time;
	 int 		date;
	 int 		lat;
	 int 		lon;
	 int 		n_svel;
	 int 		e_wvel;
	 int 		tru_heading;
	 int 		gnd_speed;
	 int 		total_temp;
	 int 		inertial_alt;
	 int 		tru_airspeed;
	 int 		pitch;
	 int 		roll;
	 char 		crlf[3];
};


struct NAV_DATA{
	 char header[4];
	 char time[14];
	 char date[5];
	 char hours[4];
	 char min[4];
	 char sec[4];
	 char *lat;
	 char *lon;
	 char *tru_heading;
	 char *pitch;
	 char *roll;
	 char *gnd_speed;
	 char *tru_trk_angle;
	 char *wind_speed;
	 char *wind_dirctn;
	 char *long_accl;
	 char *lat_accl;
	 char *norm_accl;
	 char *trk_angle_rate;
	 char *pitch_atk_rate;
	 char *roll_atk_rate;
	 char *inertial_alt;
	 char *vert_speed;

	 char *gps_lat;
	 char *gps_lon;
	 char *gps_alt;

	 char *static_presr;
	 char *total_presr;
	 char *diff_presr;
	 char *total_temp;
	 char *static_temp;
	 char *baro_alt;
	 char *mach_no;
	 char *tru_air_speed;
	 char *calc_wind_speed;
	 char *calc_wind_dir;
	 char *ephemeris_elevatn;
	 char *ephemeris_azimuth;

	 char *anlg_chnl[12];

	 char crlf[3];
};

struct INU{
	float 	lat[8];
	float 	lon[8];
	float 	tru_heading[16];
	float 	pitch[16];
	float   roll[16];
	float 	gnd_speed;
	float 	tru_trk_angle;
	float	wind_speed;
	float	wind_dirctn;
	float	mag_trk_angle;
	float	mag_heading;
	float	drift_angle;
	float	flt_path_angle;
	float	flt_path_accl;
	float   body_pitch_rate;
	float	body_roll_rate;
	float	body_yaw_rate;
	float	body_lon_accl;
	float	body_lat_accl;
	float	body_norm_accl;
	float	platform_heading;
	float	trk_angle_rate;
	float	pitch_atk_rate;
	float	roll_atk_rate;
	float	pot_vrtcl_speed;
	float	inertial_alt;
	float	alng_trk_accl;
	float	cross_trk_accl;
	float	vrtcl_accl;
	float	inertial_vrtcl_speed;
	float	n_svel;
	float	e_wvel;
};
struct GPS{
	int		mode;
	int		visible_sats;
	int		tracked_sats;
	int		days;
	int		hours;
	int		min;
	int 	sec;
	float	alt;
	float 	hdop;
	float 	vdop;
	float	trk_angle;
	float	lat;
	float   lon;
	float 	gnd_speed;
	float	integrity_lmt;
	float	vert_fom;   		//vertical figure of merit
	float	vert_vel;
	float   n_svel;
	float	e_wvel;
	float   horz_fom;			//horizontal figure of merit

};
struct ANALOG{
	float   static_presr;
	float	total_presr;
	float 	diff_presr;
	float 	total_temp;
	float	static_temp;
	float	baro_alt;
	float 	mach_no;
	float   tru_air_speed;
	float   calc_wind_speed;
	float   calc_wind_dir;
	float	ephemeris_elevation;   //Ephemeris sun angle Elevation
	float	ephemeris_azimuth;     //Ephemeris sunangle Azimuth
};

struct STORAGE_DATA{
	 struct INU inu;
	 struct GPS gps;
	 struct ANALOG analog;

	 unsigned char  ckpit_cntl[32];
};

struct DDIR	{				//	Added 6/16/94, EAH
	int			ftime;
	int			fdate;
	long		fsize;
	char		fnm[15];
	struct DDIR	*next;
};


