%{
  #include <limits.h>
  #include <string.h>
  #include "da_cache.h"
  #include "globmsg.h"
  #include "nortlib.h"
  #include "msg.h"
  #include "navigutil.h"
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/uio.h>
  #include "navframe.h"
  char navfile[PATH_MAX+1] = { '\000' };
  int fd = 0;
  int ret = 0;
  Nav_frame nf;
%}

&navfilecmd
	: Set NavFilename to &navfile_form * {
		strncpy(navfile,$4,PATH_MAX);
		if (fd > 0) close(fd);
		cache_write(0x2001,0); /* None */
		/* open file, set nav frame stat */
		if ( (fd = open(navfile,O_RDONLY)) < 0 )
			cache_write(0x2001,4); /* Error */
		else {
			/* read header */
			if (lseek(fd,5*(sizeof(Nav_frame)+1),SEEK_SET) < 0)
				cache_write(0x2001,4); /* Error */
			else cache_write(0x2001,1);
		}
	}
	: Read Frame from NavFile * {
		/* read into a frame data structure */
		if (fd > 0) {
			ret=read(fd,&nf,sizeof(nf));
			if (ret < 0) cache_write(0x2001,4); /* Error */
			else if (ret == 0) cache_write(0x2001,3);
			else {
				cache_write(0x2001,2);
				/* cache_lwrite's */
				cache_lwrite(vars[0].addr, ascii_navig
					(nf.static_presr, vars[0].precision));
				cache_lwrite(vars[1].addr, ascii_navig
					(nf.total_presr, vars[1].precision));
				cache_lwrite(vars[2].addr, ascii_navig
					(nf.static_temp, vars[2].precision));
				cache_lwrite(vars[3].addr, ascii_navig
					(nf.tru_air_speed, vars[3].precision));
				cache_lwrite(vars[8].addr, ascii_navig
					(nf.gps_alt, vars[8].precision));
				cache_lwrite(vars[9].addr, ascii_navig
					(nf.tru_heading, vars[9].precision));
				cache_lwrite(vars[10].addr, ascii_navig
					(nf.pitch, vars[10].precision));
				cache_lwrite(vars[11].addr, ascii_navig
					(nf.roll, vars[11].precision));
				cache_lwrite(vars[12].addr, ascii_navig
					(nf.ephemeris_elevatn, vars[12].precision));
				cache_lwrite(vars[13].addr, ascii_navig
					(nf.ephemeris_azimuth, vars[13].precision));
			}
		}
	}
	;
&navfile_form <char *>
	: %s (Enter Filename) { $0 = $1; }
	;

