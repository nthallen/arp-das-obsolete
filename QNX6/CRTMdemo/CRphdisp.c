/* extmain.skel include file for extraction
 * $Log$
 * Revision 1.1  2001/10/23 12:28:43  nort
 * Initial revision
 *
 * Revision 1.19  1999/11/18 19:30:33  nort
 * Added <math.h> for floor() as generated for conversions
 *
 * Revision 1.18  1995/10/18  02:05:48  nort
 * Changes to nlcon_display()
 *
 * Revision 1.17  1995/10/06  16:40:02  nort
 * Changes to nlcon_disp()
 * Old change to ROWS() and FRACSECS() macros
 *
 * Revision 1.16  1994/08/29  20:29:28  nort
 * Changes for OUI
 *
 * Revision 1.15  1993/09/27  18:55:31  nort
 * Cleaned up RCS log
 * Cleaned up rcsid[] with Watcom pragmas
 * Added #include <string.h> for memcpy prototype.
 *
 * Revision 1.14  1993/05/19  19:04:51  nort
 * Changed OPT_CON_INIT to OPT_CONSOLE_INIT to allow judicious
 * redefinition for TMA.
 */
#include <stdio.h>
#include <string.h>
#ifndef __QNXNTO__
  #include <sys/kernel.h>
#endif
#include <unistd.h>
#include <math.h>
#include "oui.h"
#include "dbr.h"
#include "globmsg.h"
#ifndef __QNXNTO__
  #include "msg.h"
#endif
#pragma off (unreferenced)
  static char emrcsid[] =
	"$Id$";
#pragma on (unreferenced)

 typedef double VOLTS;
 typedef double CELCIUS;

 typedef unsigned short AD12;
 typedef unsigned char UCHAR;
 typedef UCHAR dstat;

 typedef unsigned short UINT;
 typedef UINT DASPt;
  #define StBit_text(x) ((x)?" On":"Off")
  #define S3Way_text(x) ((x)?" Calib":"Sample")
  #define RORIS_text(x) ((x)?"RgDn":"ICOS")
  #define RateS_text(x) ((x)?"50Hz":"10Hz")
  #define Clsd_text(x)  ((x)?"X":" ")



 typedef UCHAR StBit;
 typedef UCHAR S3Way;
 typedef UCHAR RORIS_t;
 typedef UCHAR RateS_t;
 typedef UCHAR Clsd_t;

 typedef AD12 AD12_T30K75KU;
 typedef AD12 AD12_T30K30KD;
 typedef UINT SW_US;
 typedef unsigned long ULONG;
 typedef ULONG SW_UL;
  char *cpcitext[] = { "------", "Ready ", "Armed ", "Trig  ",
						"Stop  ", "Quit  ", "Signal" };


 typedef UCHAR SW_UC;
 typedef SW_UC CPCI_Stat;

typedef struct {
 char header[3]; 
 char time[13];
 char lat[10];
 char lon[11];
 char tru_heading[7];
 char pitch[9];
 char roll[9];
 char gnd_speed[7];
 char tru_trk_angle[7];
 char wind_speed[5];
 char wind_dirctn[6];
 char long_accl[7];
 char lat_accl[7];
 char norm_accl[7];
 char trk_angle_rate[6];
 char pitch_rate[6];
 char roll_rate[6];
 char vert_speed[7];
 char gps_alt[8];
 char gps_lat[10];
 char gps_lon[11];
 char static_presr[9];
 char total_presr[9];
 char diff_presr[7];
 char total_temp[7];
 char static_temp[7];
 char baro_alt[8];
 char mach_no[6];
 char tru_air_speed[7];
 char calc_wind_speed[5];
 char calc_wind_dir[6];
 char ephemeris_elevatn[7];
 char ephemeris_azimuth[7];
 char anlg_chnl[12][7];
 char crlf[2]; 
} __attribute__((packed)) Nav_frame;

 typedef double Angle;
 typedef signed short NAV_Angle;


 typedef unsigned short NAV_Alttd;

 typedef unsigned short Knots;
 typedef unsigned short NAV_Knots;
  #define N_PRS_REQ
  #define N_TEMP_REQ


 typedef signed short SINT;
 typedef double kPa;
 typedef unsigned short NAV_Pres;
 typedef unsigned short NAV_Head;
 typedef short NAV_Temp;

   Clsd_t CalFS;

   Clsd_t InltS;



   StBit QCLIS;
   RORIS_t RORIS;
   RateS_t RateS;
   StBit LN2MS;

















   S3Way S_C_S;
   StBit ScrlS;


   StBit IOSw0;
   StBit IOSw1;
   StBit Fail0;
  #include "tmctime.h"


 typedef long int flttime_t;
flttime_t flttime;
void name_init( void );

/* phtable output */
#include "tablelib.h"
PtWidget_t *name_tbl_fld[63];
void name_init( void ) {
  PtWidget_t *w;
  w = tbl_window( "name", 716, 295 );
  tbl_label( w, "Gas Deck", 40, 19 ); /* Attr 5 */
  tbl_label( w, "GasDT", 9, 38 ); /* Attr 4 */
  name_tbl_fld[0] = tbl_field( w, "X", 67, 38, 53, 15 );
  tbl_label( w, "C", 130, 38 ); /* Attr 4 */
  tbl_label( w, "A_RgT", 9, 57 ); /* Attr 4 */
  name_tbl_fld[1] = tbl_field( w, "X", 67, 57, 53, 15 );
  tbl_label( w, "C", 130, 57 ); /* Attr 4 */
  tbl_label( w, "AirHP", 9, 76 ); /* Attr 4 */
  name_tbl_fld[2] = tbl_field( w, "X", 76, 76, 44, 15 );
  tbl_label( w, "V", 130, 76 ); /* Attr 4 */
  tbl_label( w, "AirLP", 9, 95 ); /* Attr 4 */
  name_tbl_fld[3] = tbl_field( w, "X", 76, 95, 44, 15 );
  tbl_label( w, "V", 130, 95 ); /* Attr 4 */
  tbl_label( w, "Table", 438, 19 ); /* Attr 5 */
  tbl_label( w, "1", 380, 38 ); /* Attr 4 */
  name_tbl_fld[13] = tbl_field( w, "X", 399, 38, 53, 15 );
  tbl_label( w, "2", 380, 57 ); /* Attr 4 */
  name_tbl_fld[14] = tbl_field( w, "X", 399, 57, 53, 15 );
  tbl_label( w, "3", 380, 76 ); /* Attr 4 */
  name_tbl_fld[15] = tbl_field( w, "X", 399, 76, 53, 15 );
  tbl_label( w, "4", 380, 95 ); /* Attr 4 */
  name_tbl_fld[16] = tbl_field( w, "X", 399, 95, 53, 15 );
  tbl_label( w, "5", 462, 38 ); /* Attr 4 */
  name_tbl_fld[17] = tbl_field( w, "X", 481, 38, 53, 15 );
  tbl_label( w, "6", 462, 57 ); /* Attr 4 */
  name_tbl_fld[18] = tbl_field( w, "X", 481, 57, 53, 15 );
  tbl_label( w, "7", 462, 76 ); /* Attr 4 */
  name_tbl_fld[19] = tbl_field( w, "X", 481, 76, 53, 15 );
  tbl_label( w, "Flow", 99, 209 ); /* Attr 5 */
  name_tbl_fld[31] = tbl_field( w, "X", 9, 228, 53, 15 );
  tbl_label( w, "Cal", 88, 228 ); /* Attr 4 */
  tbl_label( w, "Sample", 141, 228 ); /* Attr 4 */
  tbl_label( w, "SetPt", 9, 247 ); /* Attr 4 */
  name_tbl_fld[32] = tbl_field( w, "X", 83, 247, 8, 15 );
  name_tbl_fld[33] = tbl_field( w, "X", 91, 247, 35, 15 );
  name_tbl_fld[34] = tbl_field( w, "X", 157, 247, 8, 15 );
  name_tbl_fld[35] = tbl_field( w, "X", 165, 247, 35, 15 );
  tbl_label( w, "Flow", 9, 266 ); /* Attr 4 */
  name_tbl_fld[36] = tbl_field( w, "X", 82, 266, 44, 15 );
  name_tbl_fld[37] = tbl_field( w, "X", 156, 266, 44, 15 );
  tbl_label( w, "V", 210, 266 ); /* Attr 4 */
  tbl_label( w, "Cell", 184, 19 ); /* Attr 5 */
  tbl_label( w, "1", 151, 38 ); /* Attr 4 */
  name_tbl_fld[4] = tbl_field( w, "X", 170, 38, 53, 15 );
  tbl_label( w, "C", 233, 38 ); /* Attr 4 */
  tbl_label( w, "2", 151, 57 ); /* Attr 4 */
  name_tbl_fld[5] = tbl_field( w, "X", 170, 57, 53, 15 );
  tbl_label( w, "C", 233, 57 ); /* Attr 4 */
  tbl_label( w, "3", 151, 76 ); /* Attr 4 */
  name_tbl_fld[6] = tbl_field( w, "X", 170, 76, 53, 15 );
  tbl_label( w, "C", 233, 76 ); /* Attr 4 */
  tbl_label( w, "4", 151, 95 ); /* Attr 4 */
  name_tbl_fld[7] = tbl_field( w, "X", 170, 95, 53, 15 );
  tbl_label( w, "C", 233, 95 ); /* Attr 4 */
  tbl_label( w, "FB_T", 252, 19 ); /* Attr 4 */
  name_tbl_fld[8] = tbl_field( w, "X", 296, 19, 53, 15 );
  tbl_label( w, "C", 359, 19 ); /* Attr 4 */
  tbl_label( w, "Cv1", 252, 38 ); /* Attr 4 */
  name_tbl_fld[9] = tbl_field( w, "X", 296, 38, 53, 15 );
  tbl_label( w, "C", 359, 38 ); /* Attr 4 */
  tbl_label( w, "Cv2", 252, 57 ); /* Attr 4 */
  name_tbl_fld[10] = tbl_field( w, "X", 296, 57, 53, 15 );
  tbl_label( w, "C", 359, 57 ); /* Attr 4 */
  tbl_label( w, "Pres", 252, 76 ); /* Attr 4 */
  name_tbl_fld[11] = tbl_field( w, "X", 305, 76, 44, 15 );
  tbl_label( w, "V", 359, 76 ); /* Attr 4 */
  tbl_label( w, "CPT", 252, 95 ); /* Attr 4 */
  name_tbl_fld[12] = tbl_field( w, "X", 296, 95, 53, 15 );
  tbl_label( w, "C", 359, 95 ); /* Attr 4 */
  tbl_label( w, "Pump", 543, 19 ); /* Attr 5 */
  name_tbl_fld[20] = tbl_field( w, "X", 595, 19, 26, 15 );
  name_tbl_fld[21] = tbl_field( w, "X", 649, 19, 35, 15 );
  tbl_label( w, "RPM", 543, 38 ); /* Attr 4 */
  name_tbl_fld[22] = tbl_field( w, "X", 640, 38, 44, 15 );
  tbl_label( w, "V", 694, 38 ); /* Attr 4 */
  tbl_label( w, "Current", 543, 57 ); /* Attr 4 */
  name_tbl_fld[23] = tbl_field( w, "X", 640, 57, 44, 15 );
  tbl_label( w, "V", 694, 57 ); /* Attr 4 */
  tbl_label( w, "Plate T", 543, 76 ); /* Attr 4 */
  name_tbl_fld[24] = tbl_field( w, "X", 631, 76, 53, 15 );
  tbl_label( w, "C", 694, 76 ); /* Attr 4 */
  tbl_label( w, "Motor T", 543, 95 ); /* Attr 4 */
  name_tbl_fld[25] = tbl_field( w, "X", 631, 95, 53, 15 );
  tbl_label( w, "C", 694, 95 ); /* Attr 4 */
  tbl_label( w, "QCL", 101, 114 ); /* Attr 5 */
  tbl_label( w, "QCL Dewar", 9, 133 ); /* Attr 4 */
  name_tbl_fld[26] = tbl_field( w, "X", 102, 133, 53, 15 );
  tbl_label( w, "C", 165, 133 ); /* Attr 4 */
  tbl_label( w, "Detector", 9, 152 ); /* Attr 4 */
  name_tbl_fld[27] = tbl_field( w, "X", 102, 152, 53, 15 );
  tbl_label( w, "C", 165, 152 ); /* Attr 4 */
  tbl_label( w, "Dwr P", 9, 171 ); /* Attr 4 */
  name_tbl_fld[28] = tbl_field( w, "X", 64, 171, 26, 15 );
  name_tbl_fld[29] = tbl_field( w, "X", 111, 171, 44, 15 );
  tbl_label( w, "V", 165, 171 ); /* Attr 4 */
  tbl_label( w, "Etalon", 9, 190 ); /* Attr 4 */
  name_tbl_fld[30] = tbl_field( w, "X", 102, 190, 53, 15 );
  tbl_label( w, "C", 165, 190 ); /* Attr 4 */
  tbl_label( w, "QCL Control", 291, 114 ); /* Attr 5 */
  name_tbl_fld[38] = tbl_field( w, "X", 240, 133, 26, 15 );
  name_tbl_fld[39] = tbl_field( w, "X", 231, 148, 35, 15 );
  name_tbl_fld[40] = tbl_field( w, "X", 231, 163, 35, 15 );
  tbl_label( w, "Offset", 275, 133 ); /* Attr 4 */
  name_tbl_fld[41] = tbl_field( w, "X", 361, 133, 44, 15 );
  tbl_label( w, "Ramp", 275, 152 ); /* Attr 4 */
  name_tbl_fld[42] = tbl_field( w, "X", 361, 152, 44, 15 );
  tbl_label( w, "Current", 275, 171 ); /* Attr 4 */
  name_tbl_fld[43] = tbl_field( w, "X", 361, 171, 44, 15 );
  tbl_label( w, "V", 415, 171 ); /* Attr 4 */
  tbl_label( w, "Pwr Amp", 275, 190 ); /* Attr 4 */
  name_tbl_fld[44] = tbl_field( w, "X", 352, 190, 53, 15 );
  tbl_label( w, "C", 415, 190 ); /* Attr 4 */
  tbl_label( w, "Data System", 587, 114 ); /* Attr 5 */
  tbl_label( w, "PS__T", 561, 133 ); /* Attr 4 */
  name_tbl_fld[54] = tbl_field( w, "X", 631, 133, 53, 15 );
  tbl_label( w, "C", 694, 133 ); /* Attr 4 */
  tbl_label( w, "FD__T", 561, 152 ); /* Attr 4 */
  tbl_label( w, "FD__T", 642, 152 ); /* Attr 6 */
  tbl_label( w, "PC__T", 561, 171 ); /* Attr 4 */
  name_tbl_fld[55] = tbl_field( w, "X", 631, 171, 53, 15 );
  tbl_label( w, "C", 694, 171 ); /* Attr 4 */
  tbl_label( w, "Amb_P", 561, 190 ); /* Attr 4 */
  name_tbl_fld[56] = tbl_field( w, "X", 640, 190, 44, 15 );
  tbl_label( w, "V", 694, 190 ); /* Attr 4 */
  tbl_label( w, "28V Power", 459, 114 ); /* Attr 5 */
  tbl_label( w, "I1", 446, 133 ); /* Attr 4 */
  name_tbl_fld[49] = tbl_field( w, "X", 476, 133, 44, 15 );
  tbl_label( w, "V", 530, 133 ); /* Attr 4 */
  tbl_label( w, "I2", 446, 152 ); /* Attr 4 */
  name_tbl_fld[50] = tbl_field( w, "X", 476, 152, 44, 15 );
  tbl_label( w, "V", 530, 152 ); /* Attr 4 */
  tbl_label( w, "I3", 446, 171 ); /* Attr 4 */
  name_tbl_fld[51] = tbl_field( w, "X", 476, 171, 44, 15 );
  tbl_label( w, "V", 530, 171 ); /* Attr 4 */
  tbl_label( w, "V1", 446, 190 ); /* Attr 4 */
  name_tbl_fld[52] = tbl_field( w, "X", 476, 190, 44, 15 );
  tbl_label( w, "V", 530, 190 ); /* Attr 4 */
  tbl_label( w, "V2", 446, 209 ); /* Attr 4 */
  name_tbl_fld[53] = tbl_field( w, "X", 476, 209, 44, 15 );
  tbl_label( w, "V", 530, 209 ); /* Attr 4 */
  tbl_label( w, "Status", 554, 228 ); /* Attr 5 */
  tbl_label( w, "I/O:", 446, 247 ); /* Attr 4 */
  name_tbl_fld[58] = tbl_field( w, "X", 481, 247, 26, 15 );
  name_tbl_fld[59] = tbl_field( w, "X", 517, 247, 26, 15 );
  tbl_label( w, "Fail", 446, 266 ); /* Attr 4 */
  name_tbl_fld[60] = tbl_field( w, "X", 481, 266, 26, 15 );
  tbl_label( w, "SWStat", 552, 247 ); /* Attr 4 */
  name_tbl_fld[61] = tbl_field( w, "X", 631, 247, 26, 15 );
  tbl_label( w, "MFCtr", 552, 266 ); /* Attr 4 */
  name_tbl_fld[62] = tbl_field( w, "X", 613, 266, 44, 15 );
  tbl_label( w, "CPCI", 317, 209 ); /* Attr 5 */
  tbl_label( w, "CPCI14", 231, 228 ); /* Attr 4 */
  name_tbl_fld[45] = tbl_field( w, "X", 321, 228, 53, 15 );
  name_tbl_fld[46] = tbl_field( w, "X", 384, 228, 53, 15 );
  tbl_label( w, "CPCI16", 231, 247 ); /* Attr 4 */
  name_tbl_fld[47] = tbl_field( w, "X", 321, 247, 53, 15 );
  name_tbl_fld[48] = tbl_field( w, "X", 384, 247, 53, 15 );
  tbl_label( w, "HD__T", 231, 266 ); /* Attr 4 */
  tbl_label( w, "HD__T", 329, 266 ); /* Attr 6 */
  tbl_label( w, "Time", 561, 209 ); /* Attr 4 */
  name_tbl_fld[57] = tbl_field( w, "X", 606, 209, 80, 15 );
  tbl_label( w, "Cavity Ringdown", 296, 0 ); /* Attr 3 */
  tbl_horiz_sep( w, 3, 289, 709, 1 );
  tbl_vert_sep( w, 710, 114, 174, 1 );
  tbl_vert_sep( w, 556, 123, 105, 0 );
  tbl_vert_sep( w, 441, 123, 165, 0 );
  tbl_vert_sep( w, 226, 123, 165, 0 );
  tbl_vert_sep( w, 4, 114, 174, 1 );
  tbl_vert_sep( w, 710, 11, 103, 1 );
  tbl_vert_sep( w, 538, 11, 103, 0 );
  tbl_vert_sep( w, 375, 19, 95, 0 );
  tbl_vert_sep( w, 146, 11, 112, 0 );
  tbl_vert_sep( w, 4, 11, 103, 1 );
  tbl_horiz_sep( w, 419, 9, 293, 1 );
  tbl_horiz_sep( w, 3, 9, 293, 1 );
  tbl_horiz_sep( w, 133, 218, 93, 0 );
  tbl_horiz_sep( w, 6, 218, 93, 0 );
  tbl_vert_sep( w, 247, 38, 57, 0 );
  tbl_horiz_sep( w, 131, 123, 95, 0 );
  tbl_horiz_sep( w, 6, 123, 95, 0 );
  tbl_vert_sep( w, 270, 133, 76, 0 );
  tbl_horiz_sep( w, 377, 123, 64, 0 );
  tbl_horiz_sep( w, 227, 123, 64, 0 );
  tbl_horiz_sep( w, 680, 123, 29, 0 );
  tbl_horiz_sep( w, 557, 123, 30, 0 );
  tbl_horiz_sep( w, 540, 123, 16, 0 );
  tbl_horiz_sep( w, 442, 123, 17, 0 );
  tbl_vert_sep( w, 547, 247, 38, 0 );
  tbl_horiz_sep( w, 599, 237, 110, 0 );
  tbl_horiz_sep( w, 442, 237, 112, 0 );
  tbl_horiz_sep( w, 351, 218, 90, 0 );
  tbl_horiz_sep( w, 227, 218, 90, 0 );
  PtRealizeWidget(w);
}

void phinitfunc( void ) {
   if (PtInit(NULL) == -1)
      PtExit(EXIT_FAILURE);
   name_init();
}
#define phdisplay(x,y) tbl_dispfield(x,y)
#define timetext(x) "00:00:00"
#define Reply(x,y,z)

#ifdef CONSOLE_INIT
  #error CONSOLE_INIT is obsolete
#endif
#ifdef MSG_LABEL
  #error MSG_LABEL is obsolete
#endif

#ifdef N_CONSOLES
  #include "nl_cons.h"
  #ifndef DATA_ATTR
	#define DATA_ATTR 7
  #endif

  #define cdisplay(d,r,c,s) nlcon_display(d,r,c,s,DATA_ATTR)
  #define display_(d,r,c,t,v,s) {\
	static t sv_;\
	if (v != sv_) { cdisplay(d,r,c,s); sv_ = v; }\
  }
#endif


union homerow_s {
  struct {
    UINT MFCtr;
    AD12_T30K75KU  OT_3T;
    AD12_T30K75KU  OT_4T;
    AD12_T30K75KU  OT_5T;
    AD12_T30K75KU  OT_6T;
    AD12_T30K75KU A_RgT;
    AD12  AirHP;
    AD12  AirLP;
    AD12  Amb_P;
    AD12  Cal_F;
    DASPt CalSt;
    AD12_T30K75KU CCv1T;
    AD12_T30K75KU CCv2T;
    AD12  CellP;
    AD12_T30K75KU CellT;
    AD12_T30K75KU SPC_T;
    AD12_T30K75KU Eta_T;
    AD12_T30K75KU Gas1T;
    AD12_T30K75KU Gas2T;
    AD12_T30K75KU Gas3T;
    AD12_T30K75KU Gas4T;
    AD12_T30K75KU GasDT;
    AD12  InltF;
    DASPt InltSt;
    AD12  LN2MP;
    dstat DS82A;
    dstat IOSwS;
    AD12_T30K75KU  OT_1T;
    AD12_T30K75KU  OT_2T;
    UINT  Synch;
  } __attribute__((packed)) U0;
  struct {
    char U2[2];
    SW_UL CPCI14;
    SW_UL CPCI16;
    AD12_T30K75KU  OT_7T;
    AD12_T30K75KU PAmpT;
    AD12_T30K75KU PC__T;
    AD12_T30K75KU PS__T;
    AD12  QCL_I;
    AD12_T30K75KU QCL_T;
    AD12_T30K75KU QCLDT;
    SW_US RampSt;
    AD12  ScrlI;
    AD12_T30K75KU ScrlT;
    AD12_T30K30KD CFB_T;
    AD12  SPRPM;
    DASPt SPSet;
    AD12  V28_1;
    AD12  V28_2;
    AD12  V28I_1;
    AD12  V28I_2;
    AD12  V28I_3;
    CPCI_Stat CPCI14S;
    CPCI_Stat CPCI16S;
    dstat DS80A;
    dstat DS815;
    SW_US OffSt;
    SW_UC SWStat;
    SINT      TDrft;
    dstat FailS;
  } __attribute__((packed)) U1;
  struct {
    char U4[53];
    NAV_Angle Lattd;
  } __attribute__((packed)) U3;
  struct {
    char U6[53];
    NAV_Angle Lngtd;
  } __attribute__((packed)) U5;
  struct {
    char U8[53];
    NAV_Alttd Alttd;
  } __attribute__((packed)) U7;
  struct {
    char U10[53];
    NAV_Pres  TPres;
  } __attribute__((packed)) U9;
  struct {
    char U12[53];
    NAV_Temp  TTemp;
  } __attribute__((packed)) U11;
} __attribute__((packed)) *home;
/* Text array for UCHAR -> UCHAR */
static char _CVT_0[256][4] = {
  "  0", "  1", "  2", "  3", "  4", "  5", "  6", "  7", "  8", "  9",
  " 10", " 11", " 12", " 13", " 14", " 15", " 16", " 17", " 18", " 19",
  " 20", " 21", " 22", " 23", " 24", " 25", " 26", " 27", " 28", " 29",
  " 30", " 31", " 32", " 33", " 34", " 35", " 36", " 37", " 38", " 39",
  " 40", " 41", " 42", " 43", " 44", " 45", " 46", " 47", " 48", " 49",
  " 50", " 51", " 52", " 53", " 54", " 55", " 56", " 57", " 58", " 59",
  " 60", " 61", " 62", " 63", " 64", " 65", " 66", " 67", " 68", " 69",
  " 70", " 71", " 72", " 73", " 74", " 75", " 76", " 77", " 78", " 79",
  " 80", " 81", " 82", " 83", " 84", " 85", " 86", " 87", " 88", " 89",
  " 90", " 91", " 92", " 93", " 94", " 95", " 96", " 97", " 98", " 99",
  "100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
  "110", "111", "112", "113", "114", "115", "116", "117", "118", "119",
  "120", "121", "122", "123", "124", "125", "126", "127", "128", "129",
  "130", "131", "132", "133", "134", "135", "136", "137", "138", "139",
  "140", "141", "142", "143", "144", "145", "146", "147", "148", "149",
  "150", "151", "152", "153", "154", "155", "156", "157", "158", "159",
  "160", "161", "162", "163", "164", "165", "166", "167", "168", "169",
  "170", "171", "172", "173", "174", "175", "176", "177", "178", "179",
  "180", "181", "182", "183", "184", "185", "186", "187", "188", "189",
  "190", "191", "192", "193", "194", "195", "196", "197", "198", "199",
  "200", "201", "202", "203", "204", "205", "206", "207", "208", "209",
  "210", "211", "212", "213", "214", "215", "216", "217", "218", "219",
  "220", "221", "222", "223", "224", "225", "226", "227", "228", "229",
  "230", "231", "232", "233", "234", "235", "236", "237", "238", "239",
  "240", "241", "242", "243", "244", "245", "246", "247", "248", "249",
  "250", "251", "252", "253", "254", "255"
};
char *UL_6_0_u_8( unsigned long int x) {
  static char obuf[7];
  int iov;

  if (x > 999999) return("******");
  obuf[6] = '\0';
  obuf[5] = (x % 10) + '0';
  x /= 10;
  if (x == 0) goto space4;
  obuf[4] = (x % 10) + '0';
  iov = x/10;
  if (iov == 0) goto space3;
  obuf[3] = (iov % 10) + '0';
  iov /= 10;
  if (iov == 0) goto space2;
  obuf[2] = (iov % 10) + '0';
  iov /= 10;
  if (iov == 0) goto space1;
  obuf[1] = (iov % 10) + '0';
  iov /= 10;
  if (iov == 0) goto space0;
  obuf[0] = (iov % 10) + '0';
  goto nospace;
  space4: obuf[4] = ' ';
  space3: obuf[3] = ' ';
  space2: obuf[2] = ' ';
  space1: obuf[1] = ' ';
  space0: obuf[0] = ' ';
  nospace:
  return(obuf);
}
/* _CVT_1() int tcvt ULONG -> ULONG */
char *_CVT_1(ULONG x) {
  if ( x > 999999 )
	return "******";
  return UL_6_0_u_8( x );
}
char *US_5_0_u_0( unsigned short int x) {
  static char obuf[6];
  int iov;

  obuf[5] = '\0';
  obuf[4] = (x % 10) + '0';
  iov = x/10;
  if (iov == 0) goto space3;
  obuf[3] = (iov % 10) + '0';
  iov /= 10;
  if (iov == 0) goto space2;
  obuf[2] = (iov % 10) + '0';
  iov /= 10;
  if (iov == 0) goto space1;
  obuf[1] = (iov % 10) + '0';
  iov /= 10;
  if (iov == 0) goto space0;
  obuf[0] = (iov % 10) + '0';
  goto nospace;
  space3: obuf[3] = ' ';
  space2: obuf[2] = ' ';
  space1: obuf[1] = ' ';
  space0: obuf[0] = ' ';
  nospace:
  return(obuf);
}

/* int icvt for AD12_T30K30KD -> CELCIUS */
static long int _CVT_2( AD12_T30K30KD x) {
  long int ov;

  if (x < 3630) {
    if (x < 2020) {
      if (x < 805) {
        if (x < 319) {
          if (x < 182) {
            ov = (x*(246351L)+18595)/(23000)-5444;
          } else {
            if (x < 243) {
              ov = (x*(92L)-16735)/(11)-3497;
            } else {
              ov = (x*(391L)-94998)/(59)-2988;
            }
          }
        } else {
          if (x < 525) {
            if (x < 410) {
              ov = (x*(237L)-75574)/(44)-2486;
            } else {
              ov = (x*(63681L)-26096397)/(14375)-1997;
            }
          } else {
            if (x < 653) {
              ov = (x*(59837L)-31403993)/(16000)-1488;
            } else {
              ov = (x*(185L)-120757)/(57)-1010;
            }
          }
        }
      } else {
        if (x < 1169) {
          if (x < 820) {
            ov = (x*(2003L)-1612415)/(667)-517;
          } else {
            if (x < 987) {
              ov = (x*(118833L)-97408675)/(41750)-472;
            } else {
              ov = (x*(142)-8869)/(55);
            }
          }
        } else {
          if (x < 1382) {
            if (x < 1200) {
              ov = (x*(412)-22776)/(167)+473;
            } else {
              ov = (x*(8739L)-8484558)/(3640);
            }
          } else {
            if (x < 1594) {
              ov = (x*(39293L)-37394188)/(17133);
            } else {
              ov = (x*(3827L)-3590133)/(1704);
            }
          }
        }
      }
    } else {
      if (x < 3007) {
        if (x < 2476) {
          if (x < 2080) {
            ov = (x*(98)-1316)/(43)+2429;
          } else {
            if (x < 2263) {
              ov = (x*(37)-11414)/(16)+2566;
            } else {
              ov = (x*(43027L)-44298287)/(17750);
            }
          }
        } else {
          if (x < 2673) {
            if (x < 2658) {
              ov = (x*(18957L)-21410470)/(7280);
            } else {
              ov = (x*(370)-366)/(137)+3980;
            }
          } else {
            if (x < 2840) {
              ov = (x*(59182L)-74252319)/(20875);
            } else {
              ov = (x*(99169L)-139304862)/(31666);
            }
          }
        }
      } else {
        if (x < 3372) {
          if (x < 3265) {
            if (x < 3144) {
              ov = (x*(12019L)-18953598)/(3425);
            } else {
              ov = (x*(239853L)-421379426)/(60500);
            }
          } else {
            if (x < 3280) {
              ov = (x*(1733)-21979)/(404)+5979;
            } else {
              ov = (x*(267)-23787)/(59)+6044;
            }
          }
        } else {
          if (x < 3478) {
            if (x < 3387) {
              ov = (x*(923)-32070)/(194)+6460;
            } else {
              ov = (x*(359)+29283)/(69)+6532;
            }
          } else {
            if (x < 3562) {
              ov = (x*(279398L)-645884155)/(46507);
            } else {
              ov = (x*(271)+17750)/(39)+7512;
            }
          }
        }
      }
    }
  } else {
    if (x < 6910) {
      if (x < 3895) {
        if (x < 3751) {
          if (x < 3691) {
            ov = (x*(8)-21055);
          } else {
            if (x < 3706) {
              ov = (x*(748)-8280)/(83)+8474;
            } else {
              ov = (x*(218)-21463)/(23)+8610;
            }
          }
        } else {
          if (x < 3835) {
            if (x < 3797) {
              ov = (x*(375)-30348)/(34)+9038;
            } else {
              ov = (x*(884)-14158)/(69)+9547;
            }
          } else {
            if (x < 3865) {
              ov = (x*(607)+31475)/(41)+10036;
            } else {
              ov = (x*(1123)-14955)/(66)+10482;
            }
          }
        }
      } else {
        if (x < 3978) {
          if (x < 3941) {
            if (x < 3918) {
              ov = (x*(317)+10471)/(16)+10996;
            } else {
              ov = (x*(1340)-7197)/(59)+11454;
            }
          } else {
            if (x < 3963) {
              ov = (x*(1179)+6621)/(44)+11981;
            } else {
              ov = (x*(1963)+19418)/(63)+12575;
            }
          }
        } else {
          if (x < 4008) {
            if (x < 3993) {
              ov = (x*(1998)-18165)/(55)+13047;
            } else {
              ov = (x*(861)-30101)/(20)+13599;
            }
          } else {
            if (x < 4016) {
              ov = (x*(2251)+21981)/(46)+14250;
            } else {
              ov = (x*(11276L)-42252302)/(207);
            }
          }
        }
      }
    } else {
      if (x < 31410) {
        if (x < 17410) {
          if (x < 10410) {
            ov = (x*(11276L)-42252303)/(207);
          } else {
            if (x < 13910) {
              ov = (x*(11276L)-42252304)/(207);
            } else {
              ov = (x*(11276L)-42252305)/(207);
            }
          }
        } else {
          if (x < 24410) {
            if (x < 20910) {
              ov = (x*(11276L)-42252306)/(207);
            } else {
              ov = (x*(11276L)-42252307)/(207);
            }
          } else {
            if (x < 27910) {
              ov = (x*(11276L)-42252308)/(207);
            } else {
              ov = (x*(11276L)-42252309)/(207);
            }
          }
        }
      } else {
        if (x < 45410) {
          if (x < 38410) {
            if (x < 34910) {
              ov = (x*(11276L)-42252310)/(207);
            } else {
              ov = (x*(11276L)-42252311)/(207);
            }
          } else {
            if (x < 41910) {
              ov = (x*(11276L)-42252312)/(207);
            } else {
              ov = (x*(11276L)-42252313)/(207);
            }
          }
        } else {
          if (x < 50961) {
            if (x < 47254) {
              ov = (x*(100449L)-266419951)/(1844)+2269520;
            } else {
              ov = (x*(111725L)-984485847)/(2051)+2369970;
            }
          } else {
            if (x < 55082) {
              ov = (x*(145553L)+1172408165)/(2672)+2571903;
            } else {
              ov = (x*(190657L)-1911834279)/(3500)+2796388;
            }
          }
        }
      }
    }
  }
  return ov;
}
char *SL_6_2_f_c( long int x) {
  static char obuf[7];
  int neg;
  int iov;

  if (x < -9999 || x > 99999) return("******");
  if (x < 0) { neg = 1; x = -x; }
  else neg = 0;
  obuf[6] = '\0';
  obuf[5] = (x % 10) + '0';
  iov = x/10;
  obuf[4] = (iov % 10) + '0';
  iov /= 10;
  obuf[3] = '.';
  obuf[2] = (iov % 10) + '0';
  iov /= 10;
  if (iov == 0) {
    if (neg) { obuf[1] = '-'; goto space0; }
    else goto space1;
  }
  obuf[1] = (iov % 10) + '0';
  iov /= 10;
  if (iov == 0) {
    if (neg) { obuf[0] = '-'; goto nospace; }
    else goto space0;
  }
  obuf[0] = (iov % 10) + '0';
  goto nospace;
  space1: obuf[1] = ' ';
  space0: obuf[0] = ' ';
  nospace:
  return(obuf);
}
/* _CVT_3() int tcvt AD12_T30K30KD -> CELCIUS */
char *_CVT_3(AD12_T30K30KD x) {
  long int iv;

  iv = _CVT_2( x );
  if ( iv > 99999 )
	return "******";
  return SL_6_2_f_c( iv );
}

/* int icvt for AD12_T30K75KU -> CELCIUS */
static long int _CVT_4( AD12_T30K75KU x) {
  long int ov;

  if (x < 617) {
    if (x < 149) {
      if (x < 69) {
        if (x < 46) {
          if (x < 39) {
            ov = (x*(-491679L)-2099)/(4000)+18665;
          } else {
            ov = (x*(-5375L)+209619)/(56)+13898;
          }
        } else {
          if (x < 54) {
            ov = (x*(-1281L)+58926)/(16)+13242;
          } else {
            if (x < 61) {
              ov = (x*(-5217L)+281680)/(77)+12614;
            } else {
              ov = (x*(-3229L)+196928)/(55)+12149;
            }
          }
        }
      } else {
        if (x < 98) {
          if (x < 83) {
            ov = (x*(-1189L)+82034)/(24)+11688;
          } else {
            ov = (x*(-2319L)+192463)/(58)+11004;
          }
        } else {
          if (x < 113) {
            ov = (x*(-2250L)+220449)/(67)+10411;
          } else {
            if (x < 128) {
              ov = (x*(-1487L)+168028)/(51)+9911;
            } else {
              ov = (x*(-1525L)+195147)/(62)+9479;
            }
          }
        }
      }
    } else {
      if (x < 311) {
        if (x < 201) {
          if (x < 172) {
            ov = (x*(-1401L)+208701)/(67)+8966;
          } else {
            ov = (x*(-881L)+151514)/(50)+8488;
          }
        } else {
          if (x < 237) {
            ov = (x*(-926L)+186100)/(63)+7980;
          } else {
            if (x < 274) {
              ov = (x*(-777L)+184126)/(62)+7453;
            } else {
              ov = (x*(-560L)+153434)/(53)+6991;
            }
          }
        }
      } else {
        if (x < 376) {
          if (x < 325) {
            ov = (x*(-2251L)+699927)/(222)+6601;
          } else {
            ov = (x*(-293L)+95214)/(33)+6460;
          }
        } else {
          if (x < 442) {
            ov = (x*(-414L)+155621)/(55)+6009;
          } else {
            if (x < 523) {
              ov = (x*(-229L)+101203)/(36)+5513;
            } else {
              ov = (x*(-257L)+134379)/(48)+4999;
            }
          }
        }
      }
    }
  } else {
    if (x < 1817) {
      if (x < 1013) {
        if (x < 851) {
          if (x < 727) {
            ov = (x*(-73L)+45038)/(16)+4496;
          } else {
            ov = (x*(-242737L)+176441172)/(62000)+3995;
          }
        } else {
          if (x < 881) {
            ov = (x*(-695L)+591359)/(202)+3510;
          } else {
            if (x < 998) {
              ov = (x*(-7603L)+6696846)/(2250)+3407;
            } else {
              ov = (x*(-1289L)+1286144)/(418)+3012;
            }
          }
        }
      } else {
        if (x < 1364) {
          if (x < 1173) {
            ov = (x*(-105421L)+106762606)/(35649)+2966;
          } else {
            ov = (x*(-171105L)+200665032)/(65167)+2493;
          }
        } else {
          if (x < 1568) {
            ov = (x*(-10087L)+13754954)/(4250)+1992;
          } else {
            if (x < 1787) {
              ov = (x*(-112L)+175572)/(51)+1508;
            } else {
              ov = (x*(-693L)+1238153)/(326)+1027;
            }
          }
        }
      }
    } else {
      if (x < 3002) {
        if (x < 2080) {
          if (x < 2021) {
            ov = (x*(-24881L)+45203399)/(12000)+963;
          } else {
            ov = (x*(-181L)+365767)/(89)+540;
          }
        } else {
          if (x < 2548) {
            ov = (x*(-16876L)+35099082)/(8383)+420;
          } else {
            if (x < 2782) {
              ov = (x*(-109231L)+250930742)/(52414);
            } else {
              ov = (x*(-132333L)+307717963)/(59813);
            }
          }
        }
      } else {
        if (x < 3396) {
          if (x < 3206) {
            ov = (x*(-56L)+168105)/(23)-1497;
          } else {
            if (x < 3382) {
              ov = (x*(-60793L)+151026422)/(22000);
            } else {
              ov = (x*(-2041L)+6902363)/(688)-2480;
            }
          }
        } else {
          if (x < 3542) {
            ov = (x*(-94597L)+247588408)/(29200);
          } else {
            if (x < 3674) {
              ov = (x*(-46681L)+129387987)/(12000);
            } else {
              ov = (x*(-30649L)+90223346)/(6375);
            }
          }
        }
      }
    }
  }
  return ov;
}
char *SS_6_2_f_c( short int x) {
  static char obuf[7];
  int neg;

  if (x < -9999) return("******");
  if (x < 0) { neg = 1; x = -x; }
  else neg = 0;
  obuf[6] = '\0';
  obuf[5] = (x % 10) + '0';
  x /= 10;
  obuf[4] = (x % 10) + '0';
  x /= 10;
  obuf[3] = '.';
  obuf[2] = (x % 10) + '0';
  x /= 10;
  if (x == 0) {
    if (neg) { obuf[1] = '-'; goto space0; }
    else goto space1;
  }
  obuf[1] = (x % 10) + '0';
  x /= 10;
  if (x == 0) {
    if (neg) { obuf[0] = '-'; goto nospace; }
    else goto space0;
  }
  obuf[0] = (x % 10) + '0';
  goto nospace;
  space1: obuf[1] = ' ';
  space0: obuf[0] = ' ';
  nospace:
  return(obuf);
}
/* _CVT_5() int tcvt AD12_T30K75KU -> CELCIUS */
char *_CVT_5(AD12_T30K75KU x) {
  long int iv;

  iv = _CVT_4( x );
  if ( iv < -9999 )
	return "******";
  return SS_6_2_f_c( iv );
}
char *US_4_0_u_0( unsigned short int x) {
  static char obuf[5];

  if (x > 9999) return("****");
  obuf[4] = '\0';
  obuf[3] = (x % 10) + '0';
  x /= 10;
  if (x == 0) goto space2;
  obuf[2] = (x % 10) + '0';
  x /= 10;
  if (x == 0) goto space1;
  obuf[1] = (x % 10) + '0';
  x /= 10;
  if (x == 0) goto space0;
  obuf[0] = (x % 10) + '0';
  goto nospace;
  space2: obuf[2] = ' ';
  space1: obuf[1] = ' ';
  space0: obuf[0] = ' ';
  nospace:
  return(obuf);
}
/* _CVT_6() int tcvt DASPt -> DASPt */
char *_CVT_6(DASPt x) {
  if ( x > 9999 )
	return "****";
  return US_4_0_u_0( x );
}

/* int icvt for AD12 -> VOLTS */
static unsigned short int _CVT_7( AD12 x) {
  unsigned short int ov;

  ov = x*(1L);
  return ov;
}
char *US_5_3_f_c( unsigned short int x) {
  static char obuf[6];

  if (x > 9999) return("*****");
  obuf[5] = '\0';
  obuf[4] = (x % 10) + '0';
  x /= 10;
  obuf[3] = (x % 10) + '0';
  x /= 10;
  obuf[2] = (x % 10) + '0';
  x /= 10;
  obuf[1] = '.';
  obuf[0] = (x % 10) + '0';
  return(obuf);
}
/* _CVT_8() int tcvt AD12 -> VOLTS */
char *_CVT_8(AD12 x) {
  unsigned short int iv;

  iv = _CVT_7( x );
  if ( iv > 9999 )
	return "*****";
  return US_5_3_f_c( iv );
}

static void nullfunc(void);
static void BF1_0(void) {
  phdisplay( name_tbl_fld[62],US_5_0_u_0(home->U0.MFCtr));
}

static void BF2_0(void) {
  BF1_0();
  {
    flttime = itime(); {
    phdisplay( name_tbl_fld[57],timetext(flttime));
  }
  }
  phdisplay( name_tbl_fld[0],_CVT_5(home->U0.GasDT));
  phdisplay( name_tbl_fld[1],_CVT_5(home->U0.A_RgT));
  phdisplay( name_tbl_fld[2],_CVT_8(home->U0.AirHP));
  phdisplay( name_tbl_fld[3],_CVT_8(home->U0.AirLP));
  phdisplay( name_tbl_fld[13],_CVT_5(home->U0.OT_1T));
  phdisplay( name_tbl_fld[14],_CVT_5(home->U0.OT_2T));
  phdisplay( name_tbl_fld[33],_CVT_6(home->U0.CalSt));
  phdisplay( name_tbl_fld[35],_CVT_6(home->U0.InltSt));
  phdisplay( name_tbl_fld[36],_CVT_8(home->U0.Cal_F));
  phdisplay( name_tbl_fld[37],_CVT_8(home->U0.InltF));
  phdisplay( name_tbl_fld[4],_CVT_5(home->U0.Gas1T));
  phdisplay( name_tbl_fld[5],_CVT_5(home->U0.Gas2T));
  phdisplay( name_tbl_fld[6],_CVT_5(home->U0.Gas3T));
  phdisplay( name_tbl_fld[7],_CVT_5(home->U0.Gas4T));
  phdisplay( name_tbl_fld[8],_CVT_3(home->U1.CFB_T));
  phdisplay( name_tbl_fld[9],_CVT_5(home->U0.CCv1T));
  phdisplay( name_tbl_fld[10],_CVT_5(home->U0.CCv2T));
  phdisplay( name_tbl_fld[11],_CVT_8(home->U0.CellP));
  phdisplay( name_tbl_fld[12],_CVT_5(home->U0.CellT));
  phdisplay( name_tbl_fld[29],_CVT_8(home->U0.LN2MP));
  phdisplay( name_tbl_fld[30],_CVT_5(home->U0.Eta_T));
  phdisplay( name_tbl_fld[41],US_5_0_u_0(home->U1.OffSt));
  phdisplay( name_tbl_fld[56],_CVT_8(home->U0.Amb_P));
  phdisplay( name_tbl_fld[45],_CVT_1(home->U1.CPCI14));
  phdisplay( name_tbl_fld[47],_CVT_1(home->U1.CPCI16));
}

static void BF2_1(void) {
  BF1_0();
  { CalFS = ( home->U1.DS80A >> 4 ) & 1; {
    phdisplay( name_tbl_fld[32],Clsd_text(CalFS));
  }
  }
  { InltS = ( home->U1.DS815 >> 0 ) & 1; {
    phdisplay( name_tbl_fld[34],Clsd_text(InltS));
  }
  }
  { QCLIS = ( home->U1.DS80A >> 1 ) & 1; {
    phdisplay( name_tbl_fld[38],StBit_text(QCLIS));
  }
  }
  { RORIS = ( home->U1.DS80A >> 2 ) & 1; {
    phdisplay( name_tbl_fld[39],RORIS_text(RORIS));
  }
  }
  { RateS = ( home->U1.DS80A >> 6 ) & 1; {
    phdisplay( name_tbl_fld[40],RateS_text(RateS));
  }
  }
  { LN2MS = ( home->U0.DS82A >> 4 ) & 1; {
    phdisplay( name_tbl_fld[28],StBit_text(LN2MS));
  }
  }
  { S_C_S = ( home->U1.DS80A >> 3 ) & 1; {
    phdisplay( name_tbl_fld[31],S3Way_text(S_C_S));
  }
  }
  { ScrlS = ( home->U1.DS815 >> 4 ) & 1; {
    phdisplay( name_tbl_fld[20],StBit_text(ScrlS));
  }
  }
  { IOSw0 = ( home->U0.IOSwS >> 0 ) & 1; {
    phdisplay( name_tbl_fld[58],StBit_text(IOSw0));
  }
  }
  { IOSw1 = ( home->U0.IOSwS >> 1 ) & 1; {
    phdisplay( name_tbl_fld[59],StBit_text(IOSw1));
  }
  }
  phdisplay( name_tbl_fld[15],_CVT_5(home->U0.OT_3T));
  phdisplay( name_tbl_fld[16],_CVT_5(home->U0.OT_4T));
  phdisplay( name_tbl_fld[17],_CVT_5(home->U0.OT_5T));
  phdisplay( name_tbl_fld[18],_CVT_5(home->U0.OT_6T));
  phdisplay( name_tbl_fld[19],_CVT_5(home->U1.OT_7T));
  phdisplay( name_tbl_fld[21],_CVT_6(home->U1.SPSet));
  phdisplay( name_tbl_fld[22],_CVT_8(home->U1.SPRPM));
  phdisplay( name_tbl_fld[23],_CVT_8(home->U1.ScrlI));
  phdisplay( name_tbl_fld[24],_CVT_5(home->U0.SPC_T));
  phdisplay( name_tbl_fld[25],_CVT_5(home->U1.ScrlT));
  phdisplay( name_tbl_fld[26],_CVT_5(home->U1.QCL_T));
  phdisplay( name_tbl_fld[27],_CVT_5(home->U1.QCLDT));
  phdisplay( name_tbl_fld[42],US_5_0_u_0(home->U1.RampSt));
  phdisplay( name_tbl_fld[43],_CVT_8(home->U1.QCL_I));
  phdisplay( name_tbl_fld[44],_CVT_5(home->U1.PAmpT));
  phdisplay( name_tbl_fld[54],_CVT_5(home->U1.PS__T));
  phdisplay( name_tbl_fld[55],_CVT_5(home->U1.PC__T));
  phdisplay( name_tbl_fld[49],_CVT_8(home->U1.V28I_1));
  phdisplay( name_tbl_fld[50],_CVT_8(home->U1.V28I_2));
  phdisplay( name_tbl_fld[51],_CVT_8(home->U1.V28I_3));
  phdisplay( name_tbl_fld[52],_CVT_8(home->U1.V28_1));
  phdisplay( name_tbl_fld[53],_CVT_8(home->U1.V28_2));
  phdisplay( name_tbl_fld[61],_CVT_0[home->U1.SWStat]);
  phdisplay( name_tbl_fld[46],cpcitext[home->U1.CPCI14S]);
  phdisplay( name_tbl_fld[48],cpcitext[home->U1.CPCI16S]);
}

static void BF32_1(void) {
  BF2_1();
  { Fail0 = ( home->U1.FailS >> 0 ) & 1; {
    phdisplay( name_tbl_fld[60],StBit_text(Fail0));
  }
  }
}

static void (*efuncs[32])() = {
  BF2_0,
  BF32_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1,
  BF2_0,
  BF2_1
};

#define TRN 20
#define TRD 1
#define LOWLIM (-180)
#define HIGHLIM (180)
#define NBROW 58
#define NROWMINF 1
#define NSECSPER 1
#define NROWSPER 2
#define SYNCHVAL 0xABB4
#define NROWMAJF 32
#define MFSECNUM 2
#define MFSECDEN 1
#define SECDRIFT 90
#define COPY_MFCtr
#define MFCtr home->U0.MFCtr

#ifndef EXTRACTION_INIT
  #define EXTRACTION_INIT
#endif
#ifndef OPT_EXT_INIT
  #define OPT_EXT_INIT
#endif
#ifndef EXTRACTION_TERM
  #define EXTRACTION_TERM
#endif
#ifndef TMINITFUNC
  #define TMINITFUNC
#endif

static unsigned int next_minor_frame, majf_row;
#if (NROWMINF != 1)
  static unsigned int minf_row = 0;
#endif
#ifdef NEED_TIME_FUNCS
  static unsigned short CurMFC;
#endif


int main(int argc, char **argv) {
  /* oui_init_options(argc, argv); */

  TMINITFUNC;
  /* BEGIN_MSG; */
  EXTRACTION_INIT;
  printf( "CR: sizeof home is %d\n", sizeof(union homerow_s));
  printf( "CR: CPCI14 offset %d\n", offsetof(union homerow_s, U1.CPCI14 ) );
  printf( "CR: PAmpT offset %d\n", offsetof(union homerow_s, U1.PAmpT ) );
  printf( "CR: QCLDT offset %d\n", offsetof(union homerow_s, U1.QCLDT ) );
  printf( "CR: SWStat offset %d\n", offsetof(union homerow_s, U1.SWStat ) );
  DC_operate();
  EXTRACTION_TERM;
  /* DONE_MSG; */
  return 0;
}


#define MKDCTV(x,y) ((x<<8)|y)
#define DCTV(x,y) MKDCTV(DCT_##x,DCV_##y)

/* The main thing we need to do is reset our row counter on TM START */
void DC_DASCmd(unsigned char type, unsigned char val) {
  switch (MKDCTV(type, val)) {
	case DCTV(TM,TM_START):
	  next_minor_frame = majf_row = 0;
	  #if (NROWMINF != 1)
		minf_row = 0;
	  #endif
	  break;
	case DCTV(TM,TM_END):
	case DCTV(QUIT,QUIT):
	default:
	  break;
  }
}

#define incmod(x,y) if (x==((y)-1)) x = 0; else x++

void DC_data(dbr_data_type *dr_data) {
  unsigned short nrows;

  for (nrows = dr_data->n_rows, home = (void *) &dr_data->data[0];
	   nrows > 0;
	   nrows--, home ++) {

	/* First check the minor frame counter if necessary */
	#if (NROWMINF != 1)
	if (minf_row == 0) {
	#endif
	  COPY_MFCtr /* Needs to be defined by TMC */
	  #ifdef NEED_TIME_FUNCS
		CurMFC = MFCtr;
	  #endif
	  {
		unsigned short delta;
		
		delta = MFCtr - next_minor_frame;
		if (delta != 0) {
		  #ifdef IVFUNCS
			if (delta > NROWMAJF/NROWMINF) delta = NROWMAJF;
			else delta *= NROWMINF;
			/* delta is now the number of rows skipped up to a maximum of
			  nrowmajf. While invalidating, I don't need to update
			  minf_row, since the final will be 0, same as before.
			  MFCtr is already set, and majf_row is updated.
			*/
			while (delta-- > 0)
			  ivfuncs[majf_row]();
		  #endif
		  majf_row = (((unsigned short)MFCtr) % (NROWMAJF/NROWMINF))
							  * NROWMINF;
		}
	  }
	  next_minor_frame = MFCtr+1;
	#if (NROWMINF != 1)
	}
	#endif
	
	/* Now process the row */
	efuncs[majf_row]();
	incmod(majf_row, NROWMAJF);
	#if (NROWMINF != 1)
	  incmod(minf_row, NROWMINF);
	#endif
  }
}

#ifdef __WATCOMC__
  #pragma off (unreferenced)
#endif

void DC_tstamp(tstamp_type *tstamp) { /* Nothing to do! */ }

void DC_other(unsigned char *msg_ptr, int sent_tid) {
  unsigned char rep_msg = DAS_UNKN;

  Reply(sent_tid, &rep_msg, 1);
}

#ifdef __WATCOMC__
  #pragma on (unreferenced)
#endif

#ifdef NEED_TIME_FUNCS
  #if (NROWMINF != 1)
	#define ROWS(x) (((unsigned long)(x))*NROWMINF+minf_row)
  #else
	#define ROWS(x) (x)
  #endif
  #if (NSECSPER != 1)
	#define FRACSECS(x) (((unsigned long)ROWS(x))*NSECSPER)
  #else
	#define FRACSECS(x) ROWS(x)
  #endif
  long itime(void) {
	  return(dbr_info.t_stmp.secs +
		FRACSECS(CurMFC-dbr_info.t_stmp.mfc_num)
	#if (NROWSPER != 1)
		  / NROWSPER
	#endif
		  );
  }
  double dtime(void) {
	  return(dbr_info.t_stmp.secs +
		(double) FRACSECS(CurMFC-dbr_info.t_stmp.mfc_num)
	#if (NROWSPER != 1)
		  / NROWSPER
	#endif
		  );
  }
  double etime(void) {
	double t;
	static double t0 = -1e9;
	
	t = dtime();
	if (t0 == -1e9) t0 = t;
	return(t - t0);
  }
#endif

dbr_info_type dbr_info = {
  "", /* tmid */
  58, /* NBMINF */
  58, /* NBROW */
  32, /* NROWSMAJF */
  1, /* NSECSPER */
  2, /* NROWSPER */
  0, 1, /* MFC lsb col, msb col */
  0xABB4, /* Synch Value */
  0 /* not inverted */
};
