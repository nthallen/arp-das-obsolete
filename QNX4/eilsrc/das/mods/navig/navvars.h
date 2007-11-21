#define SIGN_FMT ' ', '-'
#define PRESSURE_FMT 8, 3, SIGN_FMT
#define LAT_FMT 9, 5, 'N', 'S'
#define LONG_FMT 10, 5, 'E', 'W'
#define SIGNED_FMT 6, 2, SIGN_FMT

typedef struct {
  char *name;
  unsigned short addr;
  int offset, width, precision;
  char pos, neg;
} var_def;

var_def vars[] = {
  "sPress", 0x1000, 162, PRESSURE_FMT,
  "tPress", 0x1002, 171, PRESSURE_FMT,
  "sTemp",  0x1004, 194, 6, 2, SIGN_FMT,
  "pTAS",   0x1006, 215, 6, 2, SIGN_FMT,
  "iLat",   0x1008,  16, LAT_FMT,
  "iLong",  0x100A,  26, LONG_FMT,
  "gLat",   0x100C, 141, LAT_FMT,
  "gLong",  0x100E, 151, LONG_FMT,
  "gAlt",   0x1010, 133, 7, 1, SIGN_FMT,
  "tHead",  0x1012,  37, 6, 2, SIGN_FMT,
  "Pitch",  0x1014,  44, 8, 4, SIGN_FMT,
  "Roll",   0x1016,  53, 8, 4, SIGN_FMT,
  "sElev",  0x1018, 233, 6, 2, SIGN_FMT,
  "sAzim",  0x101A, 240, 6, 2, SIGN_FMT
};
#define N_VARS (sizeof(vars)/sizeof(var_def))
