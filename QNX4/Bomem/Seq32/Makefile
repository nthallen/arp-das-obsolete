MNC=bom
HOMEDIR=/usr/local/src/das/exp/psr/bomem

CFLAGS=-2 -ml
COMPILE.c=wpp -d2 -fo=$@ $(CFLAGS)

SRC=acq_96.c
SRC+=acq_sim.c
SRC+=basewin.c
SRC+=bo_time.c
SRC+=boalloc.c
SRC+=bofclose.c
SRC+=bomem.c
SRC+=flaccess.c
SRC+=other.c
SRC+=plot.c
SRC+=ratio.c
SRC+=server.c
SRC+=time2.c
SRC+=timer_c.c
SRC+=timerdos.c
SRC+=v_add.c
SRC+=v_scale.c
SRC+=winmgr.c
SRC+=acq_mike.h
SRC+=bo_timer.h
SRC+=bomem.h
SRC+=display.h
SRC+=filegen.h
SRC+=harvard.h
SRC+=seq32_pc.h
SRC+=spectrum.h
SRC+=useful.h
SRC+=vector.h
SRC+=bo_col.tmc
SRC+=asminc.asm
SRC+=seq32_pc.asm
BINSRC=coamb100.bin
BINSRC+=sin_cos.bin
BINSRC+=bomem.wnd

TOOL=Makefile setup setup.dbg

OBJ=bomem.o acq_96.o seq32_pc.o v_add.o v_scale.o boalloc.o bofclose.o
OBJ+=flaccess.o timer_c.o timerdos.o bo_time.o time2.o other.o
OBJ+=server.o
WOBJ=bomemw.o acq_96.o seq32_pc.o v_add.o v_scale.o boalloc.o bofclose.o
WOBJ+=flaccess.o timer_c.o timerdos.o bo_time.o time2.o other.o
WOBJ+=winmgr.o basewin.o plot.o ratio.o serverw.o
SOURCE=$(SRC) $(BINSRC) $(TOOL)
RCS=$(SRC) $(TOOL)
OBJECT=$(OBJ) *.err
TARGET=bomemw
DISTRIB=bomem coamb100.bin sin_cos.bin bomem.wnd
bomem : $(WOBJ)
	rm -f bomem; :
	$(LINK.c) -g -T1 -o bomem $(WOBJ) $(LDFLAGS) | tee link.err
	chmod g-w bomem
	usemsg bomem bomem.c
	promote bomem
bomem_nw : $(OBJ)
	rm -f bomem_nw; :
	$(LINK.c) -g -T1 -o bomem_nw $(OBJ) $(LDFLAGS) | tee link_nw.err
	chmod g-w bomem_nw
	usemsg bomem_nw bomem.c
	promote bomem_nw
acq_96.o : acq_96.c seq32_pc.h
seq32_pc.o : seq32_pc.asm asminc.asm
	rm -f seq32_pc.obj
	rundos -h tasm /dDOSX286 seq32_pc
	[ ! -f seq32_pc.obj ] && exit 1 || mv seq32_pc.obj seq32_pc.o
bomem.o : bomem.c bomem.h seq32_pc.h
server.o : server.c bomem.h
bomemw.o : bomem.c bomem.h seq32_pc.h
	$(COMPILE.c) -dWITH_WINDOWS=1 bomem.c
serverw.o : server.c bomem.h
	$(COMPILE.c) -dWITH_WINDOWS=1 server.c
winmgr.o : winmgr.c bomem.h
	$(COMPILE.c) -dWITH_WINDOWS=1 winmgr.c
basewin.o : basewin.c bomem.h
	$(COMPILE.c) -dWITH_WINDOWS=1 basewin.c
plot.o : plot.c bomem.h
	$(COMPILE.c) -dWITH_WINDOWS=1 plot.c
ratio.o : ratio.c bomem.h seq32_pc.h
	$(COMPILE.c) -dWITH_WINDOWS=1 ratio.c
distribution : $(DISTRIB)
	@[ -z "$(TGTDIR)" ] && echo TGTDIR not defined && exit 1; :
	[ -n "$(TGTDIR)" ] && distribute $(TGTDIR) $(DISTRIB); :
