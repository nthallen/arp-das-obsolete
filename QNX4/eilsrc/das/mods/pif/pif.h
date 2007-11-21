#include "port_types.h"

typedef struct {
  UBYTE1
    pif_sync,
    pif_ias,
    pif_vsi,
    pif_hdg;
  UBYTE4
    pif_lat,
    pif_long;
  UBYTE2
    pif_gs,
    pif_course,
    pif_gps_alt,
    pif_baro_alt;
  UBYTE1
    pif_roll,
    pif_pitch,
    pif_pitch_rate,
    pif_yaw_rate,
    pif_alpha,
    pif_beta,
    pif_accel_z,
    pif_tas,
    pif_ambtemp,
    pif_wind_speed,
    pif_wind_crs,
    pif_time_hh,
    pif_time_mm,
    pif_time_ss;
  UBYTE2
    pif_crc;
} Pif_frame;

unsigned long int check_crc(unsigned char *array, unsigned long int sz);
