# make include files for appgen output.
MODEL=s
CPU=-2
# export OPTARGS=g for debugging or make with OPTIM=-g
OPTIM=-$${OPTARGS:-Oatx}
CFLAGS=-m$(MODEL) $(CPU) $(OPTIM) -w4
LIBFLAGS=-l dbr -l eillib -l das $${BETALIB:--b}
LINK.priv=/bin/rm -f $@; $(LINK.c) $(LIBFLAGS) -T 1 -o $@ $(LDFLAGS)
LINK.norm=$(LINK.c) $(LIBFLAGS) -o $@ $(LDFLAGS)
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
DCCC=prt2dccc () { textto -lz $$1; $(AWK)/prt2dccc.awk $$* $(LIBSRC)/dccc.por $$*; }; prt2dccc
PRT2TMC=prt2tmc () { textto -lz $$1; $(AWK)/prt2tmc.awk $$1; }; prt2tmc
PRT2CMD=prt2cmd () { textto -lz $$1; $(AWK)/prt2cmd.awk $$1; }; prt2cmd
PRT2EDF=prt2edf () { textto -lz $$1; $(AWK)/prt2edf.awk $$1; }; prt2edf
EDF2EXT=$(AWK)/edf2ext.awk
SLP2CMD=$(AWK)/slp2cmd.awk
SLP2SOL=$(AWK)/slp2sol.awk
CYCLE=$(AWK)/cycle.awk
DATAATTR=data_attr > $@
SERVER=srvr() { $$1 -vsy & namewait -p $$! cmdinterp || { $$1; return 1; }; }; srvr
TMCALGO=tmcalgo -o $@
SOLFMT=sft () { cat $$* >$@tmp; solfmt -o$@ $@tmp; rm $@tmp; }; sft
.INIT :
	@if [ -z "$$LIBQNX" ]; then echo LIBQNX not defined >&2; exit 1; fi; :
	@if [ -z "$$INCLUDE" ]; then echo INCLUDE not defined >&2; exit 1; fi; :
	@if [ -z "$$SKELETON_PATH" ]; then\
	  echo SKELETON_PATH not defined >&2; exit 1; fi; :
