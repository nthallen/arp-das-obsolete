/* stepout.c produces STEP output files according to the undated "Format
   Specification for STEP Data Interchange."
   Written August 12, 1987
   Modified by MBM, Dec. 16, 1987 to use standard dr I/O functions and
   to observe windowing and new_line conventions.
   Made ANSI on or after October 17, 1989
   Modified by Eil, July 1991 for Ascii File Format Standards Document
   Version 1.
*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "ssp.h"
#include "memlib.h"
#include "snafucmd.h"
#include "snafuerr.h"
#include "snf_dr.h"
#include "curses.h"
#include "scurses.h"
#include "lat.h"


/*+step
<sort> out_default

void out_default(FILE *fp, char *prmpt, char *dflt, char *follow)

out_default prompts for a given input and reads in the answer.  If the
response is simply a carriage return, the default is used.  The default
is shown in brackets following the prompt.  If the default is NULL, no
brackets are shown.  After the value is read in, it is written out to
the output file.
     The follow text, if not NULL, is appended to the end of the output
line.
-*/
void out_default(FILE *fp, char *prmpt, char *dflt, char *follow) {
  char buf[60], all[80];

  if (dflt != NULL) {
    if (dflt[strlen(dflt)-1]=='\n') dflt[strlen(dflt)-1]='\0';
    sprintf(all, "%s[%s] ", prmpt, dflt);
    if (dr_calc_cmd_io(all, buf, 1)) strcpy(buf, dflt);
  } else if (dr_calc_cmd_io(prmpt, buf, 1)) buf[0] = '\0';
  fprintf(fp, "%s", buf);
  if (follow != NULL) fprintf(fp, "%s", follow);
  fprintf(fp, "\n");
}

#define LN 133

char ONAME[LN], ORG[LN], SNAME[LN], MNAME[LN],
XNAME[LN], NCOM[LN], DATAHEAD[LN];

int read_defaults(char *filename) {
FILE *fp;
if (!(fp=fopene(filename,"r",NULL))) return(0);
if (!(fgets(ONAME,LN,fp))) return(0);
if (!(fgets(ORG,LN,fp))) return(0);
if (!(fgets(SNAME,LN,fp))) return(0);
if (!(fgets(MNAME,LN,fp))) return(0);
if (!(fgets(XNAME,LN,fp))) return(0);
if (!(fgets(NCOM,LN,fp))) return(0);
if (!(fgets(DATAHEAD,LN,fp))) return(0);
fclose(fp);
return(1);
}


/*+ DATA REDUCTION
<sort> stepout

void
stepout(ss_name, file_name)
char *ss_name, *file_name;

This function is called with the name of a spreadsheet and the name of the
STEP output file to create with the data from this spreadsheet.  Step output
can output up to ten columns in addition to column 0.

We will always use the single-data-point-per-time-hack method for output
since we can't be absolutely certain of the periodicity of our data.  In
particular, the ClO mixing ratio data points will probably occur every
32 seconds, but this may depend on the loading of the flight computer.
Of course, we don't expect to be driving the valves during critical data
taking, but nonetheless, I leave that caveat.

NV=ss->n_columns - 1

Header information will be as follows: (Line numbers for reference only)
     1  <Number of Lines in header> 1001   NLHEAD=12+NV+NSCOML+NNCOML+2
     2  Anderson, Jim           Experimenter (with possible option to change?)
     3  Harvard University Atmospheric Research Project
     4  <Source of Measurements name>
     5  <Mission Name>          This is an input line
     6  <volume numer> <total number of volumes>
     7  YYYY MM DD YYYY MM DD	Date at which the data in file begins.
				Date of data reduction.
	                        For now, this is an input line.  We could use
                                the time of the first datum, but that would be
                                work!  This should match the file name.
     8	DX(1) interval between values of independent variable X(i,s),i=1,NX(s)
     9  XNAME(1)
     10 <Number of variables NV>
     11 variable scale(n) n=1,NV
     12 variable missing data indicator(n) n=1,NV
     variable name(n) n=1,NV
     <Number of special comment lines NSCOML>
     Special comment(n) n=1,NSCOML
     <Number of normal comment line NNCOML>
     Normal Comment(n) n=1,NNCOML
     [X(m,1) (V(m,n) n=1,NV)]

-*/
void stepout(char *ss_name, char *file_name) {

int i;
int cols[10], n_cols;
FILE *fp;
char *fbuf, ibuf[LN], ibuf2[LN];
struct tm *ut;
time_t t;
sps_ptr ss;
double scales[10], base_time, v;
long int start, end;

  if (ss_error(ss = ss_open(ss_name))) {
    cmderr("Error opening spreadsheet %s", ss_name);
    return;
  }
  if (ss_width(ss) < 2) {
    ss_close(ss);
    cmderr("Not enough columns for STEP output");
    return;
  }
  if ((fp = snfopen(file_name, "w", &fbuf)) == NULL) {
    cmderr("Error opening output file %s", file_name);
    ss_close(ss);
    return;
  }

  if (!(read_defaults("step.def"))) {
    cmderr("Error opening defaults file step.def");
    ONAME[0] = ORG[0] = SNAME[0] = MNAME[0] =
    XNAME[0] = NCOM[0] = DATAHEAD[0] = '\0';
  }

  set_dr_window("STEP Output", 2, 3);
  dr_hdr_cmd_str(ss_name, 3, -1);

  for (n_cols = 0; n_cols < 10; n_cols++)
    if (enter_col("Column: ", cols, n_cols, &cols[n_cols], ss_width(ss)))
      break;

  fprintf(fp, "%d  %d\n", n_cols + 12 + 1 + 2 + 2, 1001);
  out_default(fp, "Experimenter: ", ONAME, NULL);
  out_default(fp, "From: ", ORG, NULL);
  out_default(fp, "System Name: ", SNAME, NULL);
  out_default(fp, "Mission Name: ", MNAME, NULL);
  fprintf(fp, "1  1\n");
  /* data date */
  if (ss_jump_row(ss, 0L) || ss_get(ss, 0, &base_time))
     cmderr("Error %d getting first time", snafu);
  /* process base_time */
  base_time = base_date(base_time);
  t = (time_t)base_time;
  ut = localtime(&t);
  sprintf(ibuf2, "%04d %02d %02d", ut->tm_year+1900, ut->tm_mon+1, ut->tm_mday+1);
  /* reduction date */
  t = time(NULL);
  ut = localtime(&t);
  sprintf(ibuf, " %04d %02d %02d", ut->tm_year+1900, ut->tm_mon+1, ut->tm_mday);
  strcat(ibuf2,ibuf);  
  out_default(fp, "Data & Reduction YYYY MM DD: ", ibuf2, NULL);
  out_default(fp, "Time Interval: ", "0", NULL);
  out_default(fp, "Independent Variable: ", XNAME, NULL);
  fprintf(fp, "%d\n", n_cols);

  /* scale factors for NV variables */
  for (i = 0; i < n_cols; i++) {
    if (ss_get_column_title(ss, cols[i], ibuf2) || ibuf2[0] == '\0')
      sprintf(ibuf2, "%d", cols[i]);
    sprintf(ibuf, "Scale factor for %s: ", ibuf2);
    while (1) {
      if ((!get_in_dub(ibuf, &scales[i], 0)) && scales[i] != 0.) break;
      cmderr("Invalid scale factor");
      dr_calc_clr_row();
    }
    fprintf(fp, "%lg ", scales[i]);
  }
  fprintf(fp, "\n");

  /* missing data values */
  for (i = 0; i < n_cols; i++) fprintf(fp, "999999 ");
  fprintf(fp, "\n");

  /* variable names and units */
  for (i = 0; i < n_cols; i++) {
    /* <enter names and units> */
    if (ss_get_column_title(ss, cols[i], ibuf) || ibuf[0] == '\0')
      sprintf(ibuf, "%d", cols[i]);
    out_default(fp, "Name and Units: ", ibuf, NULL);
  }

  fputs("1\n",fp);
  out_default(fp, "Special Comment: ", NULL, NULL);
  fputs("2\n",fp);
  out_default(fp, "Normal Comment: ", NCOM, NULL);
  out_default(fp, "Data Header: ", DATAHEAD, NULL);

  if ((!get_range(ss, &start, &end)) && (n_cols != 0) && (end >= start)) {
     if (ss_jump_row(ss, start))  cmderr("Error %d on first_row", snafu);
     else {
     	sdoupdate(0);
     	do {
     		if (kbhit() &&
     		term_early("Currently at row %ld of %ld.  Terminate early? [n/y]",
     		start, end)) break;
     		ss_get(ss, 0, &v);
     		fprintf(fp, "%.4lf", v - base_time);
     		for (i = 0; i < n_cols; i++) {
     			ss_get(ss, cols[i], &v);
     			if (is_number(v))
     			fprintf(fp, " %ld", (long int) floor(v/scales[i] + .5));
     			else fprintf(fp, " 999999");
     		}
     		fprintf(fp, "\n");
     	} while ((ss_next_row(ss) == 0) && (++start <= end));
     }
  }

  /* cleanup */
  snfclose(fp, fbuf);
  ss_close(ss);
  clear_dr_window();
}
