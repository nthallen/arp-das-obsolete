#define PIF_FLAG_ID 10

typedef struct {
  	unsigned char RollA,
		PtchA,
		PtchR,
		YawR,
		AttA,
		SideA,
		AcclZ,
		IndAS;
	unsigned short AltB;
	unsigned char VertS,
		Head;
	unsigned long int Latt,
		Lnttd;
	unsigned short AltG,
		GndS,
		Cours;
	unsigned char TruAS,
		AmbT,
		WindS,
		WindD;
	union {
		unsigned short TDrft;
		struct {
			unsigned char secs;
			unsigned char mins;
			unsigned char hrs;
		} GPStime;
	} u;
} PIF_FRAME;
