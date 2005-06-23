/* bomemf.h defines the file structure written and read by the
   bomem utilities.
 * $Log$
 */

/* This is the header at the beginning of each outupt file from
   the bomem program. Much of the data in the header is somewhat
   redundant, as it may be included in the filename or the file
   creation time, or the TM data stream, but a little redundancy
   is cheap and can prove very useful.
   
   n_bytes is the number of bytes in the header. If you chose to
   use another means to glean the header data, you can skip over
   the header by reading the first word of the file and seeking
   to that offset in the file to begin reading "real data".
   
   version is simply a hook against future developments. If we
   add new features to the log file, future readers can be
   sensitive to this version number and use a different header
   structure. "dumb" readers will still be able to skip the
   header as described above, assuming the "real data" format
   hasn't changed.
   
   sequence is the sequence number shown on the screen and in
   the TM format. It is almost cerainly also part of the filename,
   but it might be handy to get it here without having to parse
   the filename, especially if the filename changes.
   
   Spectrum, Det_1, Det_2, In and Out are flags indicating the
   nature of the data. Spectrum is true if the data is a spectrum,
   false if it is an interferogram. If true, both real and
   imaginary data is included. Det_1 and Det_2 are independent
   flags documenting the presenece of data for each of the detectors.
   This allows files to include data for one or both detectors.
   In practice, files will usually be either one or both rather
   than randomly mixed, but why limit the structure now? In and Out
   are similarly independent flags indicating whether the file
   contains data from an inward or outward scan or both.
   
   time is the acquisition time as reported by the bomem software.
   This should be approximately the same as the time reported by
   TM, although I'm not sure what timezone they use... I suspect
   I will ignore this field in most applications.
   
   n_pts is the number of data points in each scan. This must be
   multiplied by the number of detectors, the number of directions,
   and by 2 if a spectrum is included.
   
   firstx/lastx are the X values associated with the first/last data
   point of each scan. These are valid for spectra and probably
   invalid for interferograms until further notice.
*/
typedef struct {
  unsigned short n_bytes; /* Number of bytes in the header */
  unsigned short version; /* The file structure version */
  unsigned short sequence; /* File Sequence Number */
  unsigned short Spectrum:1; /* True for spectra, false for interferograms */
  unsigned short Det_1:1; /* True if Det. 1 data is present */
  unsigned short Det_2:1; /* True if Det. 2 data is present */
  unsigned short In:1; /* True if In direction data is present */
  unsigned short Out:1; /* True if out direction data is present */
  unsigned long time; /* seconds since 1970 */
  unsigned long n_pts;
  double firstx;
  double lastx;
} bo_file_header;

#define BOFH_VERSION 0

/* Following the header comes the "real data". This is organized as
   follows:
   
   for each detector in (1, 2) {
     for each direction in (In, Out) {
	   Real data vector [ n_pts ]
	   Imaginary data vector [ n_pts ] (if spectrum)
	 }
   }
   
   Each vector consist of single-precision IEEE floating-point
   numbers as returned by the bomem soft/hardware.
   
   What combination of detectors and directions get included in
   a file remains to be decided. The "natural" options I can
   think of are:

     Write one file for each (detector,direction) combination.
     Write one file for each detector including both in and out.
	 Write one file for each scan including both detectors and directions.

  Since the data from both directions is ultimately combined, it
  makes some sense to combine it in a file, whereas it wouldn't
  make sense to combine the "In" data of both detectors.
*/
