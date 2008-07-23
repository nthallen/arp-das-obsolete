# CC=g++
# CFLAGS=-I/usr/local/include -Wall -lang-c++ -Y _gpp -g
#VER=2.95.3
VER=3.3.5
TGT=-V$(VER),gcc_ntox86_cpp -lang-c++
CFLAGS=$(TGT) -I/usr/local/include -Wall -g
LDFLAGS=-L/usr/local/lib -M -Wl,-rpath -Wl,/usr/local/lib
LIBS=-ltm -lnort
DISPLIBS=-ltmph -lph $(LIBS)
OBJS=$(COLOBJS)

.PHONY : clean all
all : Collector hsimext hsimdisp
clean :
	rm -f $(OBJS) test_timer

DGOBJS=DG.o DataQueue.o DG_Resmgr.o DG_cmd.o DG_tmr.o
DGINC=DG.h DataQueue.h DG_Resmgr.h DG_cmd.h DG_tmr.h
COLOBJS=hsimcol.o Collector.o $(DGOBJS)
Collector : $(COLOBJS)
	cc $(CFLAGS) $(LDFLAGS) -o Collector $(COLOBJS) $(LIBS)
hsimcol.o : hsimcol.c Collector.h $(DGINC)
Collector.o : Collector.c Collector.h $(DGINC)
DG_tmr.o : DG_tmr.c DG_tmr.h DG_Resmgr.h DG.h DataQueue.h
DG_cmd.o : DG_cmd.c DG_cmd.h DG_Resmgr.h DG.h DataQueue.h
DG_Resmgr.o : DG_Resmgr.c DG_Resmgr.h
EXTOBJS=hsimext.o DC.o
hsimext : $(EXTOBJS)
	cc $(CFLAGS) $(LDFLAGS) -o hsimext $(EXTOBJS) $(LIBS)
DISPOBJS=hsimdisp.o DC.o DCph.o
hsimdisp : $(DISPOBJS)
	cc $(CFLAGS) $(LDFLAGS) -o hsimdisp $(DISPOBJS) $(DISPLIBS)
