/* stepin.c handles input of STEP format data files to a spreadsheet.
   All work herein is in reference to the undated document titled:
   "Format Specification for STEP Data Interchange."  I just hope other
   experimenters are following the same document.
   Written in the Winter of September 1, 1987
   Modified by MBM, Dec. 16, 1987 to use standard dr I/O functions and
   to observe windowing and new_line conventions.
   Made ANSI on or after October 17, 1989
   Modified by Eil, July 1991 for Ascii File Format Standards Document
   Version 1, reads FFI = 1001 or 1020.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#ifndef __QNX__
#include <dos.h>
#endif
#include "ssp.h"
#include "memlib.h"
#include "snafuerr.h"
#include "snafucmd.h"
#include "snf_dr.h"

#define MAX_VARS 45
#define LN 133
extern int no_aux_ask;
extern int not_gmt;


int set_scale_miss_name(FILE *fp, sps_ptr ss, int nv, double *sf, double *mv);

/*+ DATA REDUCTION
<sort> read_vals

int
read_vals(fp, nv, vals)
FILE *fp;
int nv;
double *vals;

-*/
int read_vals(FILE *fp, int nv, double *vals) {
  int i;
  char buf[LN];

  for (i = 0; i < nv; i++) {
	if (fscanf(fp," %lf",vals) != 1) {
	      cmderr("Bad scale factor or missing value");
	      return(1);
	  }
    vals++;
  }
  /* read a newline */
  read_line(fp, buf);
  return(0);
}


int ffi;
char ass_name[LN]={'\0'};
sps_ptr ass;


/*+ DATA REDUCTION
<sort> read_hdr

sps_ptr read_hdr(FILE *fp, char *ss_name, int *nv, int *nsph, int *nspl,
             double *bd, double *dt, double *sf, double *mv,
			int *nauxv, double *asf, double *amv);

-*/
sps_ptr read_hdr(FILE *fp, char *ss_name, int *nv, int *nsph, int *nspl,
             double *bd, double *dt, double *sf, double *mv,
	                  int *nauxv, double *asf, double *amv) {
  int nlhdr, i, yr, mo, dy, auxing;
  sps_ptr ss;
  char buf[LN];
  struct tm timeptr={0};

  *nauxv=0;
  auxing=0;
  /* read number of lines in header and ffi index */
  read_line(fp, buf);
  if (sscanf(buf, "%d%d", &nlhdr, &ffi) != 2 || nlhdr <= 0) {
    cmderr("Error reading nlhdr and ffi (line 1)");
    return(-1);
  }

  if (ffi != 1001 && ffi !=1020) {
    cmderr("Only FFI 1001 and 1020 Formats accepted");
    return(-1);
  }

  /* line 2 - Experimenter ; ONAME */
  read_line(fp, buf);
  buf[69] = '\0';
  dr_calc_cmd_line(buf, NULL);

  /* line 3 ORG, line 4 SNAME, line 5 MNAME, line 6 IVOL, NVOL */
  swlw_ln(fp);
  swlw_ln(fp);
  swlw_ln(fp);
  swlw_ln(fp);

  /* line 7, date and rdate */
  read_line(fp, buf);
  buf[69] = '\0';
  dr_calc_cmd_line(buf, NULL);
  for (;;) {
    if (sscanf(buf, "%d%d%d", &yr, &mo, &dy) == 3 &&
        yr >= 1970 &&
        mo >= 1 && mo <= 12 &&
        dy >= 1 && dy <= 31) break;
    cmderr("Error reading flight date (line 7)");
    if (dr_calc_cmd_io("Please Enter Correct Flight Date: ", buf, 0))
      return(-1);
  }

  timeptr.tm_year=yr-1900;
  timeptr.tm_mon=mo-1;
  timeptr.tm_mday=dy;
  *bd = (double) mktime(&timeptr);
  if (not_gmt==0) {
  	tzset();
  	*bd = *bd - timezone;
  }
 
  /* line 8  DX(1) */
  read_line(fp, buf);   /* line 8 */
  buf[69] = '\0';
  dr_calc_cmd_line(buf, NULL);
  if (sscanf(buf, "%lf", dt) != 1 || *dt < 0) {
    cmderr("Error reading time interval (line 8)");
    return(-1);
  }

  /* line 9, depends on ffi */
  if (ffi == 1020) {
  	read_line(fp, buf);
  	buf[69] = '\0';
  	dr_calc_cmd_line(buf, NULL);
  	if (sscanf(buf, "%d", nsph) != 1 || *nsph <= 0) {
  		cmderr("Error reading samples per hack");
  		return(-1);
  	}
  } else *nsph = 1;

  /* next line XNAME */
  swlw_ln(fp);

  /* next line NV */
  read_line(fp, buf);
  buf[69] = '\0';
  dr_calc_cmd_line(buf, NULL);
  if (sscanf(buf, "%d", nv) != 1 || *nv <= 0) {
    cmderr("Error reading number of variables NV");
    return(-1);
  }

  if (ss_error(ss = ss_create(ss_name, SPT_INCREASING, *nv+1, 1))) {
    cmderr("Cannot open spreadsheet %s", ss_name);
    return(-1);
  }

  if (ss_set_column_title(ss, 0, "Time")) {
    cmderr("Error %d setting spdsheet title", snafu);
    ss_close(ss);
    return(-1);
  } else if (ss_set_column_format(ss, 0, "%14.11t")) {
    cmderr("Error %d setting spdsheet formats", snafu);
    ss_close(ss);
    return(-1);
  }


/*  read_line(fp, buf);
  if (sscanf(buf, "%d", nspl) != 1 || *nspl <= 0) {
    cmderr("Error reading samples per line (line 10)");
    return(-1);
  }
*/
  *nspl = *nsph;

  /* read scale factors and miss values and names */
  if (set_scale_miss_name(fp, ss, *nv, sf, mv)) return(-1);


  /* if ffi = 1020 read number auxillary variables, scales, misses and names */
  if (ffi == 1020) {
  	read_line(fp, buf);
  	buf[69] = '\0';
  	dr_calc_cmd_line(buf, NULL);
  	if (sscanf(buf, "%d", nauxv) != 1) {
  		cmderr("Error reading number of comments");
  		return(-1);
  	}
	buf[0]='\0';
	if (*nauxv>0 && !no_aux_ask) {
		/* need to know what to do with auxilary vars */
		dr_calc_cmd_io("Create auxillary variable spreadsheet also? [n] ",buf,1);
		if (!strchr(strlwr(buf),'y')) auxing=0;
		else auxing=1;
	}
  }

  if (auxing) {
	if (dr_calc_cmd_io("Auxillary Spreadsheet Name [auxly] ",ass_name,1))
		strcpy(ass_name,"auxly");
  	if (ss_error(ass = ss_create(ass_name, SPT_INCREASING, *nauxv+1, 1))) {
  		cmderr("Cannot open spreadsheet %s", ass_name);
  		return(-1);
  	}
  	if (ss_set_column_title(ass, 0, "Time")) {
  		cmderr("Error %d setting spdsheet title", snafu);
  		ss_close(ass);
  		return(-1);
  	} else if (ss_set_column_format(ass, 0, "%14.11t")) {
  		cmderr("Error %d setting spdsheet formats", snafu);
  		ss_close(ass);
  		return(-1);
  	}


  }

  if (ffi == 1020) {
  	/* read scale factors and miss values and names */
	if (auxing) {
  	   if (set_scale_miss_name(fp, ass, *nauxv, asf, amv)) return(-1);
	}
	else
  	if (*nauxv)
	   for (i=0;i<*nauxv+2;i++)
  	       swlw_ln(fp);
  }

  /* read special and normal comments */
  for (i=0;i<2; i++) {
  	read_line(fp, buf);
  	if (sscanf(buf, "%d", &dy) != 1) {
  		cmderr("Error reading number of comments");
  		return(-1);
  	}
  	for (yr=0; yr<dy; yr++) swlw_ln(fp);
  }

  if (!auxing) *nauxv=0;
  return(ss);
}

int set_scale_miss_name(FILE *fp, sps_ptr ss, int nv, double *sf, double *mv) {
char buf[LN];
int i;

  if (read_vals(fp, nv, sf)) return(-1);
  if (read_vals(fp, nv, mv)) return(-1);

  for (i = 0; i < nv; i++) {
    if (sf[i] == 0.) {
      cmderr("Zero Scale factor for variable %d", i);
      return(-1);
    }
    /* read names */
    read_line(fp, buf);
    if (ss_set_column_title(ss, i+1, buf)) {
      cmderr("Error %d setting column %d title", snafu, i+1);
      return(-1);
    }
    if (ss_set_column_format(ss, i+1, "%9.2e")) {
      cmderr("Error %d setting column %d format", snafu, i+1);
      return(-1);
    }
    buf[69] = '\0';
    dr_calc_cmd_line(buf, NULL);
  }
return(0);
}




/*+ DATA REDUCTION
<sort> read_value
int
read_value(fp, ss, i, T, sf, mv, missed, delta, bd)
FILE *fp;
sps_ptr ss;
int i, *missed;
double T, *sf, *mv, delta, bd;

-*/
int read_value(FILE *fp, sps_ptr ss, int i, double T, double *sf,
               double *mv, int *missed, double delta, double bd) {
  double V;

  if (fscanf(fp, "%lf", &V) != 1) {
    if (!feof(fp)) cmderr("Error in datum %d for time %lf", i, T-bd);
    return(1);
  }
  if (V == mv[i]) {
    if (missed[i]) return(0);
    missed[i] = 1;
    V = non_number;
  } else {
    V = V * sf[i];
    missed[i] = 0;
  }
  if (ss_insert_value(ss, T, delta) && snafu != SFU_VALUE_FOUND) {
      cmderr("Error %d on insert_seq", snafu);
      return(1);
  }
  ss_set(ss, i+1, V);
  return(0);
}

/*+ DATA REDUCTION
<sort> stepin

void
stepin(filename, ss_name)
char *filename, *ss_name;
-*/
void stepin(char *filename, char *ss_name) {
  FILE *fp;
  sps_ptr ss;
  char *fbuf;
  int nv, nsph, nspl, nppl, nauxv;
  int i, j, k, missed[MAX_VARS], amissed[MAX_VARS];
  double bd, dt, sf[MAX_VARS], mv[MAX_VARS], asf[MAX_VARS], amv[MAX_VARS];
  double hack, T, delta;

  if ((fp = snfopen(filename, "r", &fbuf)) == NULL) {
    cmderr("Cannot open input file %s", filename);
    return;
  }
  set_dr_window("STEP Input", 1, 1);
  ss = read_hdr(fp, ss_name, &nv, &nsph, &nspl, &bd, &dt, sf, mv, &nauxv, asf, amv);
/*  dt /= nsph;   WRONG! */
  if (ss_error(ss)) {
    snfclose(fp, fbuf);
    return;
  }
  sdoupdate(0);
  nppl = nspl*nv;
  /* <read in the data> */
  delta = dt/2.;
  for (i = 0; i < nv; i++) missed[i] = 1;
  for (i = 0; i < nauxv; i++) amissed[i] = 1;
  for (;;) {
    if (fscanf(fp, "%lf", &hack) != 1) break;
    hack += bd;
    if (nsph > 1)  {  /* this is where the aux vars are */
	if (nauxv) {
	  for (i = 0; i < nauxv; i++)
		if (read_value(fp, ass, i, hack, asf, amv, amissed, (dt*nsph)/2, bd)) break;
	} else  swlw_ln(fp);
    }
    for (T = hack, j = 0; j < nsph; j++, T += dt) {
      for (i = 0, k = 0; i < nv; i++) {
        if (read_value(fp, ss, i, T, sf, mv, missed, delta, bd)) break;
        if (++k >= nppl) {
          swlw_ln(fp);
          k = 0;
        }
      }
      if (i < nv) break;
    }
    if (j < nsph) break;
    if (kbhit() && term_early(NULL)) break;
  }
  ss_close(ss);
  if (nauxv) ss_close(ass);
  snfclose(fp, fbuf);
  clear_dr_window();
}




