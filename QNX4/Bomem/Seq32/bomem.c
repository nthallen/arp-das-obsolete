#include <stdio.h>
#include <stdlib.h>
#include <windows/Qwindows.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/name.h>
#include "seq32_pc.h"
#include "filegen.h"
#include "bomem.h"
#include "spectrum.h"
#include "nortlib.h"
#include "collect.h"
#include "bo_col.tmc"

const int ioaddr = 0x300;
static char *header = "seq";
int windows = 0; /* set nonzero if windows server is located */
int log_data = 1;
int collect_spec = 0, spec_collected = 0;
BomemTM BomemSeq;
send_id BomemSend;
#define Update_Status(x) { BomemSeq.status = x; Col_send(BomemSend); }

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

#ifdef SAMPLE_STATUS
static void sample_status(void) {
  word scans_0, scans_1, scans_bad;
  long acq_num;
  short resolution, speed;
  short det1[4], det2[4], acq_err;
  short rv;
  
  rv = dsp96_status(&scans_0, &scans_1, &scans_bad, &acq_num,
		&resolution, &speed, det1, det2, &acq_err);
  if (rv == NO_ERROR) {
	fprintf(stderr, "Status:\n  Scans: %u %u %u\n",
			scans_0, scans_1, scans_bad);
	fprintf(stderr, "  Sequence: %ld\n", acq_num);
	fprintf(stderr, "  Resolution: %d\n", resolution);
	fprintf(stderr, "  Speed: %d\n", speed);
	fprintf(stderr, "  Det1 ID: %d\n", det1[0]);
	fprintf(stderr, "  Det2 ID: %d\n", det2[0]);
  } else fprintf(stderr, "Error %d reading status\n");
}
#endif

/* Maybe move these two functions to another file? */
void server_command(pid_t from, const char *cmd) {
}

void server_loop(void) {
  for (;;) {
	from = Receive();
	<if it's for us, send to our processor>
  }
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

  /* Initialize communication with qwindows */
  
  if (GraphicsOpen(getenv("WINSERVER")) != 0) {
	windows = 1;
	SetName("PSR", NULL);
	nl_error = win_err;

	/* Initialize any objects which might require it */
	/* Create the first base window (if necessary) */
	New_Base_Window();
  } else fprintf(stderr, "Running without windows\n");
  
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
  }
  
  Update_Status(1); /* We're running: beginning initializations */

  /* Install the micro code! */
  rv = dsp96_install(1, ioaddr, NULL);
  report_error(rv);
  atexit(dsp96_remove);
  /* sample_status(); */

  Update_Status(2);   /* Initialization was successful */

  if (windows) {
	/* Undim acquire button */
	WindowBarCurrent('B', NULL);
	ChangeOptions("A", "S-d");
	Draw();

	/* Enter Receive Loop */
	Receive_Loop();
  } else if (BomemSend != 0) {
	nl_error(0, "Non-windows Server Mode");
	server_loop();
  } else acquire_data();

  /* Terminate any objects which require it */
  /* Terminate communication with qwindows */
  exit(0);
}

YDATA interf[6];

void acquire_data(void) {
  FILE *fp;
  char buf[80];
  double acq_time;
  short rv, j;
  long int i, acq_num;

  Update_Status(3);   /* Beginning Acquisition Sequence */

  new_plot(NULL, 0); /* make sure the old pointers aren't kept */
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
	Update_Status(6); /* Second data received: writing files */

	int dir, channel, real, r, ino;
	static char *chtext[2] = { ".1", ".2" };
	static char *dirtext[2] = { ".in", ".out" };
	static char *realtext[3] = { "", ".r", ".i" };

	for (real = 0; real < (spec_collected ? 2 : 1); real++) {
	  for (channel = 0; channel < 2; channel ++) {
		for (dir = 0; dir < 2; dir++) {
		  r = (spec_collected ? real+1 : 0 );
		  sprintf(buf, "%s%d%s%s%s", header, sequence, chtext[channel],
			  dirtext[dir], realtext[r]);
		  fp = fopen(buf, "w");
		  if (fp != NULL) {
			ino = channel;
			if (real) ino += 2;
			if (dir)
			  flwrite( (void HPTR *)(interf[ino].buffer + interf[ino].npts),
						interf[ino].npts * sizeof(float), fp);
			else
			  flwrite( (void HPTR *)interf[ino].buffer,
						interf[ino].npts * sizeof(float), fp);
			fclose(fp);
		  } else nl_error(2, "Unable to open file %s for writing", buf);
		}
	  }
	}
	BomemSeq.seq = sequence;
	BomemSeq.n_scans = n_scans;
	Update_Status(7); /* Files have been written. End of Acquisition */
  } else {
	Update_Status(8); /* End of Acquisition: No files written */
  }

  if (windows) plot_opt();
}

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

#ifdef __USAGE
%C	[options]
	-n <n scans>   The initial number of scans for each acquisition
	-s <seq. no.>  The starting sequence number for output files
	-h <hdr>       An alternate header string to use for output files
	-S             Collect Raw Spectrum
	-l             Don't log the data

  bomem will run with or without QNX Windows. If Windows isn't
  found, a single acquisition sequence will occur and the program
  will terminate.
  
  Output files will have names of the form <hdr><seq>.{in|out}.{1|2}
  Ideally, we will generate four files for each sequence, though
  only two are produced at present until we figure out how to
  access the second detector. The default header string is "seq".
  
  The program may output some diagnostic information to stderr
  (fd 2). After each acquisition, the <hdr><seq> combination is
  output to stdout (fd 1). This may be piped into a script for
  subsequent processing. For example:
  
     bomem | sendit

  Such a script should process all files with that prefix.
#endif
