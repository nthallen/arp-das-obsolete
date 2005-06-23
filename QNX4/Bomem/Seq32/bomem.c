#include <stdio.h>
#include <stdlib.h>
#include <windows/Qwindows.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/name.h>
#include "seq32_pc.h"
#include "filegen.h"
#include "bomem.h"
#include "bomemf.h"
#include "spectrum.h"
#include "nortlib.h"
#include "collect.h"
#include "bo_col.tmc"
#ifdef WITH_WINDOWS
  #include "bomemw.h"
#endif

const int ioaddr = 0x300;
static char *header = "seq";
int log_data = 1;
int collect_spec = 0, spec_collected = 0;
int n_scans = 10;
int sequence = 0;
BomemTM BomemSeq;
send_id BomemSend;
#define Update_Status(x) { BomemSeq.status = x; Col_send(BomemSend); }

#ifdef WITH_WINDOWS
  int windows = 0; /* set nonzero if windows server is located */

  int win_err(int level, char *s, ...) {
	char buf[256], *lvlmsg;
	va_list arg;

	va_start(arg, s);
	vsprintf(buf, s, arg);
	va_end(arg);
	switch (level) {
	  case -1: lvlmsg = "Bomem"; break;
	  case 0: lvlmsg = "Bomem"; break;
	  case 1: lvlmsg = "Warning: "; break;
	  case 2: lvlmsg = "Error: "; break;
	  case 3: lvlmsg = "Fatal: "; break;
	  default:
		if (level <= -2) lvlmsg = "Debug: ";
		else lvlmsg = "Internal: ";
		break;
	}
	Tell(lvlmsg, buf);
	if (level > 2 || level == -1) exit(level > 0 ? level : 0);
	return(level);
  }
#endif

void report_error(short rv) {
  char *errtext;

  if (rv != NO_ERROR) {
	switch (rv) {
	  case ERROR: errtext = "ERROR"; break;
	  case INVALID_FILE: errtext = "INVALID_FILE"; break;
	  case INVALID_DIRECTORY: errtext = "INVALID_DIRECTORY"; break;
	  case FILE_IO_ERROR: errtext = "FILE_IO_ERROR"; break;
	  case FILE_NOT_FOUND: errtext = "FILE_NOT_FOUND"; break;
	  case NOT_ENOUGH_MEMORY: errtext = "NOT_ENOUGH_MEMORY"; break;
	  case TIMEOUT: errtext = "TIMEOUT"; break;
	  case TOO_MANY_FILES: errtext = "TOO_MANY_FILES"; break;
	  case UNINITIALIZED: errtext = "UNINITIALIZED"; break;
	  case ERROR_STRUCT_ID: errtext = "ERROR_STRUCT_ID"; break;
	  case ERROR_MISSING_PARM: errtext = "ERROR_MISSING_PARM"; break;
	  case ERR_INVALID_ARGUMENT: errtext = "ERR_INVALID_ARGUMENT"; break;
	  default: errtext = "Unkown error"; break;
	}
	nl_error(3, "%s returned by dsp96_install()", errtext);
  }
}

void Initialize_DSP(void) {
  static int initialized = 0;
  int rv;
  
  if (! initialized) {
	/* Install the micro code! */
	Update_Status(2);   /* Beginning Initialization */

	rv = dsp96_install(1, ioaddr, NULL);
	report_error(rv);
	atexit(dsp96_remove);
	initialized = 1;

	Update_Status(1);   /* Done with Initialization... */
  }
}

void exit_routine(void) {
  Update_Status(0); /* We're Quitting */
}

void main(int argc, const char * const * argv) {
  short rv;
  int c;
  char *server;

  seteuid(getuid());
  optind = 0; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt(argc, argv, "n:s:h:Sl")) != -1) {
	switch (c) {
	  case 'n':
		n_scans = atoi(optarg);
		break;
	  case 's':
		sequence = atoi(optarg);
		if (sequence > 0) sequence--;
		break;
	  case 'h':
		header = optarg;
		break;
	  case 'S':
		collect_spec = 1;
		break;
	  case 'l':
		log_data = 0;
		break;
	  case '?':
		nl_error(3, "Unrecognized Option -%c", optopt);
	  default:
		nl_error(4, "Unsupported Option -%c", c);
	}
  }

  #ifdef WITH_WINDOWS
	/* Initialize communication with qwindows */
	if (GraphicsOpen(getenv("WINSERVER")) != 0) {
	  windows = 1;
	  SetName("PSR", NULL);
	  nl_error = win_err;

	  /* Initialize any objects which might require it */
	  /* Create the first base window (if necessary) */
	  New_Base_Window();
	} else nl_error(0, "Running without windows\n");
  #endif
  
  if (qnx_name_attach(0, nl_make_name("bomem", 0)) == -1) {
	nl_error(3, "Another Process Owns the DSP Board");
  }

  /* Establish communications with TM collection */
  { int old_response;
	old_response = set_response(1);
	BomemSend = Col_send_init("BomemSeq", &BomemSeq, sizeof(BomemSeq));
	set_response(old_response);
	BomemSeq.seq = 0;
	BomemSeq.n_scans = n_scans;
	onexit(exit_routine);
  }
  
  Update_Status(1); /* We're running */

  /* Initialize only if we're not feeding TM */
  if (BomemSend == 0)
	Initialize_DSP();

  #ifdef WITH_WINDOWS
	if (windows) {
	  /* Undim acquire button */
	  dim_acquire(0);

	  /* Enter Receive Loop */
	  Receive_Loop();
	} else if (BomemSend != 0) {
	  nl_error(0, "Non-windows Server Mode");
	  server_loop();
	} else acquire_data();
  #else
	server_loop();
  #endif

  /* Terminate any objects which require it */
  /* Terminate communication with qwindows */
  exit(0);
}

YDATA interf[6];

static void write_file(FILE *fp, bo_file_header *hdr) {
  int l_ino, ino, channel, dir;

  fwrite(hdr, sizeof(bo_file_header), 1, fp);
  l_ino = (hdr->Spectrum ? 2 : 0);
  for (channel = 0; channel < 2; channel ++ ) {
	if ((channel == 0 && hdr->Det_1) || (channel == 1 && hdr->Det_2)) {
	  for (dir = 0; dir < 2; dir++ ) {
		if ((dir == 0 && hdr->In) || (dir == 1 && hdr->Out)) {
		  for (ino = channel; ino <= channel + l_ino; ino += 2) {
			if (dir)
			  flwrite( (void HPTR *)(interf[ino].buffer + interf[ino].npts),
						interf[ino].npts * sizeof(float), fp);
			else
			  flwrite( (void HPTR *)interf[ino].buffer,
						interf[ino].npts * sizeof(float), fp);
		  }
		}
	  }
	}
  }
  fclose(fp);
}

void acquire_data(void) {
  FILE *fp;
  char buf[80];
  double acq_time;
  short rv, j;
  long int i, acq_num;

  #ifdef WITH_WINDOWS
	/* Dim the acquire button */
	dim_acquire(1);
	new_plot(NULL, 0); /* make sure the old pointers aren't kept */
  #endif

  Initialize_DSP();

  Update_Status(3);   /* Beginning Acquisition Sequence */

  for (j = 0; j < 6; j++) {
	if (interf[j].buffer != 0) {
	  bo_free(interf[j].buffer);
	  interf[j].buffer = NULL;
	}
	interf[j].npts = 0;
  }
  
  if (collect_spec)
	rv = dsp96_get_raw_spec(&interf[0], &interf[2], &interf[4], &interf[5],
			n_scans, 1, 0, 0., 3000., 0., 3000., NORTON_B_WEAK, FALSE,
			&acq_time, NULL);
  else
	rv = dsp96_get_int(&interf[0], n_scans, 1, 0, FALSE, &acq_time, NULL);
  report_error(rv);

  Update_Status(4); /* Acquisition has begun, waiting for result */
  
  rv = dsp96_wait_end_coad(&interf[0], &interf[2], &interf[4], &interf[5],
			&acq_time, NULL);
  report_error(rv);

  Update_Status(5); /* Acquisition is completed and first data received */

  rv = dsp96_get_coad(&interf[1], &interf[3], &interf[4], &interf[5],
	&acq_time, &acq_num);
  report_error(rv);
  spec_collected = collect_spec;


  /* Output to the files */
  if (log_data) {
	int dir, channel, ino, l_ino;
	bo_file_header hdr;
	static char *chtext[2] = { ".1", ".2" };
	static char *dirtext[2] = { ".i", ".o" };

	sequence++;
	#ifdef WITH_WINDOWS
	  update_sequence();
	#endif

	Update_Status(6); /* Second data received: writing files */

	hdr.n_bytes = sizeof(hdr);
	hdr.version = BOFH_VERSION;
	hdr.sequence = sequence;
	hdr.Spectrum = spec_collected ? 1 : 0;
	hdr.time = acq_time;
	for (channel = 0; channel < 2; channel ++) {
	  for (dir = 0; dir < 2; dir++) {
		sprintf(buf, "%s%04d%s%s", header, sequence, chtext[channel],
			dirtext[dir]);
		hdr.Det_1 = (channel == 0) ? 1 : 0;
		hdr.Det_2 = (channel == 1) ? 1 : 0;
		hdr.In = (dir == 0) ? 1 : 0;
		hdr.Out = (dir == 1) ? 1 : 0;
		hdr.n_pts = interf[0].npts;
		hdr.firstx = interf[0].firstx;
		hdr.lastx = interf[0].lastx;

		fp = fopen(buf, "w");
		if (fp != NULL)
		  write_file(fp, &hdr);
		else nl_error(2, "Unable to open file %s for writing", buf);
	  }
	}
	BomemSeq.seq = sequence;
	BomemSeq.n_scans = n_scans;
  }

  Update_Status(7); /* End of Acquisition and/or logging */

  #ifdef WITH_WINDOWS
	if (windows) plot_opt();

	/* undim the acquire button */
	dim_acquire(0);
  #endif
  
  Update_Status(1); /* Ready for more commands */
}

#ifdef WITH_WINDOWS
  void plot_opt(void) {
	int i;
	float HPTR *dbuf;
  
	i = (plot_chan != 0) ? 1 : 0;
	ratio_regions(&interf[i], &interf[i+2]);
	if (spec_collected && plot_imag != 0) i += 2;
	if (interf[i].npts != 0) {
	  dbuf = interf[i].buffer;
	  if (plot_dir != 0) dbuf += interf[i].npts;
	  new_plot(dbuf, interf[i].npts);
	}
  }
#endif

#ifdef __USAGE
%C	[options]
	-n <n scans>   The initial number of scans for each acquisition
	-s <seq. no.>  The starting sequence number for output files
	-h <hdr>       An alternate header string to use for output files
	-S             Collect Raw Spectrum
	-l             Don't log the data
#endif
