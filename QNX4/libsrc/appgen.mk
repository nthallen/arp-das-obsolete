# make include files for appgen output.
MODEL=s
# Change OPTARGS to -g for debugging
OPTARGS=-Oatx
CFLAGS=-m$(MODEL) -2 $(OPTARGS) -w4
LIBFLAGS=-l dbr -l eillib -l das $(BETALIB)
LINK.priv=/bin/rm -f $@; $(LINK.c) $(LIBFLAGS) -T 1 -o $@
LINK.norm=$(LINK.c) $(LIBFLAGS) -o $@
TMC=tmc -s -o $@ $(TMCFLAGS)
TMC.col=name=$@; $(TMC) -p -V $${name%col.c}.pcm -c -D tm.dac
USE=/usr/local/include/use
USAGE=usage () {\
  image=$$1; shift;\
  ( echo "%C\t[options]";\
	cat $$* | sort; ) > usage.tmp;\
  usemsg $$image usage.tmp && /bin/rm usage.tmp; }; usage $@
LIBSRC=/usr/local/lib/src
CMDGEN=cmdgen -o $@
COMPILE.clt=$(COMPILE.c) -fo$@ -D CLIENT
COMPILE.srvr=$(COMPILE.c) -fo$@ -D SERVER
AWK=awk > $@ -f $(LIBSRC)
FLD2DISP=$(AWK)/fld2disp.awk
DCCC=$(AWK)/prt2dccc.awk
PRT2EDF=prt2edf () { textto -lz $$1; $(AWK)/prt2edf.awk $$1; }; prt2edf
PRT2TMC=$(AWK)/prt2tmc.awk
PRT2CMD=$(AWK)/prt2cmd.awk
# PRT2EDF=$(AWK)/prt2edf.awk
EDF2EXT=$(AWK)/edf2ext.awk
DATAATTR=data_attr > $@
TMCALGO=tmcalgo -o $@
.INIT :
	@if [ -z "$$LIBQNX" ]; then echo LIBQNX not defined >&2; exit 1; fi; :
	@if [ -z "$$INCLUDE" ]; then echo INCLUDE not defined >&2; exit 1; fi; :
	@if [ -z "$$SKELETON_PATH" ]; then\
	  echo SKELETON_PATH not defined >&2; exit 1; fi; :
	@if [ -z "$$BETALIB" ]; then echo BETALIB not defined >&2; exit 1; fi; :
