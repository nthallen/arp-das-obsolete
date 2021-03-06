# make include files for appgen output.
MODEL=s
CPU=-2
# export OPTARGS=g for debugging or make with OPTIM=-g
# export LIBQNX=/usr/lib:/usr/local/libg:/usr/local/lib
# For 3r model:
# make MODEL=s CPU=-3
OPTIM=-$${OPTARGS:-Oatx}
CFLAGS=-m$(MODEL) $(CPU) $(OPTIM) -w4
LIBFLAGS=-l dbr -l eillib -l das $${BETALIB:--b}
LINK.priv=/bin/rm -f $@; $(LINK.c) $(LIBFLAGS) -T 1 -o $@ $(LDFLAGS)
LINK.norm=$(LINK.c) $(LIBFLAGS) -o $@ $(LDFLAGS)
TMCREV=tmc
TMC=$(TMCREV) -s -o $@ $(TMCFLAGS)
TMC.col=name=$@; $(TMC) -p -V $${name%col.c}.pcm -c -D tm.dac $(COLFLAGS)
OUIDIR=/usr/local/include/oui
OUI=oui -o $@
OUIUSE=usemsg $@
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
DCCC=prt2dccc () { echo prt2dccc $$*; textto -lz $$*; sort -k4.1n -o dccc.tmp $$*; $(AWK)/prt2dccc.awk dccc.tmp $(LIBSRC)/dccc.por dccc.tmp; /bin/rm dccc.tmp;}; prt2dccc
PRT2TMC=prt2tmc () { echo prt2tmc $$1; textto -lz $$1; $(AWK)/prt2tmc.awk $$1; }; prt2tmc
PRT2CMD=prt2cmd () { echo prt2cmd $$1; textto -lz $$1; $(AWK)/prt2cmd.awk $$1; }; prt2cmd
PRT2EDF=prt2edf () { echo prt2edf $$1; textto -lz $$1; $(AWK)/prt2edf.awk $$1; }; prt2edf
EDF2EXT=$(AWK)/edf2ext.awk
SLP2CMD=$(AWK)/slp2cmd.awk
SLP2SOL=$(AWK)/slp2sol.awk
EDF2OUI=$(AWK)/edf2oui.awk
TMG2TMC=tmg2tmc > $@
CYCLE=$(AWK)/cycle.awk
TABLE=table > $@
DATAATTR=data_attr > $@
SERVER=srvr() { $$1 -vsy & namewait -p $$! cmdinterp || { $$1; return 1; }; }; srvr
TMAREV=tmcalgo
TMCALGO=$(TMAREV) -o $@
# SOLFMT=sft () { cat $$* >$@tmp; solfmt -o$@ $@tmp; rm $@tmp; }; sft
SOLFMT=solfmt -o$@
.INIT :
	@if [ -z "$$LIBQNX" ]; then echo LIBQNX not defined >&2; exit 1; fi; :
	@if [ -z "$$INCLUDE" ]; then echo INCLUDE not defined >&2; exit 1; fi; :
	@[ -n "$(SPECFILE)" ] && [ "$(SPECFILE)" -nt Makefile ] &&\
	  echo Must rerun appgen >&2 && exit 1; :
