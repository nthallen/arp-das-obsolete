# CC=g++
# CFLAGS=-I/usr/local/include -Wall -lang-c++ -Y _gpp -g
#VER=2.95.3
VER=3.3.5
TGT=-V$(VER),gcc_ntox86_cpp -lang-c++
CFLAGS=$(TGT) -I/usr/local/include -Wall -g
LDFLAGS=-L/usr/local/lib -M -Wl,-rpath -Wl,/usr/local/lib
LIBS=-ltm -lnort
DISPLIBS=-ltmph -lph $(LIBS)
OBJS=*.o hsimcmd.c hsimcltoui.c hsimsrvroui.c Collector hsimcol hsimext
OBJS+=hsimsrvr hsimclt *.map

TM=/usr/local/share/huarp

.PHONY : clean all
all : Collector hsimext hsimdisp hsimsrvr hsimclt DCtap
clean :
	rm -f $(OBJS) test_timer

DGOBJS=DG.o DataQueue.o DG_Resmgr.o DG_cmd.o DG_tmr.o
DGINC=DG.h DataQueue.h DG_Resmgr.h DG_cmd.h DG_tmr.h
COLOBJS=hsimcol.o Collector.o DG_data.o $(DGOBJS)
Collector : $(COLOBJS)
	cc $(CFLAGS) $(LDFLAGS) -o Collector $(COLOBJS) $(LIBS)
	/usr/local/sbin/promote Collector
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
hsimclt : hsimclt.o hsimcltoui.o
	cc $(CFLAGS) $(LDFLAGS) -o hsimclt hsimclt.o hsimcltoui.o $(DISPLIBS)
hsimclt.o : hsimcmd.c
	cc $(CFLAGS) -o hsimclt.o -DCLIENT -c hsimcmd.c
hsimcltoui.o : hsimcltoui.c
hsimcltoui.c : /usr/local/share/oui/cic.oui
	oui -o hsimcltoui.c /usr/local/share/oui/cic.oui
hsimsrvr : hsimsrvr.o hsimsrvroui.o
	cc $(CFLAGS) $(LDFLAGS) -o hsimsrvr hsimsrvr.o hsimsrvroui.o $(LIBS)
	/usr/local/sbin/promote hsimsrvr
hsimsrvr.o : hsimcmd.c
	cc $(CFLAGS) -o hsimsrvr.o -DSERVER -c hsimcmd.c
hsimsrvroui.o : hsimsrvroui.c
hsimsrvroui.c : /usr/local/share/oui/cis.oui
	oui -o hsimsrvroui.c /usr/local/share/oui/cis.oui
hsimcmd.c : $(TM)/root.cmd hsim.cmd
	cmdgen -o hsimcmd.c $(TM)/root.cmd hsim.cmd

DCtap : DCtap.o DC.o
	cc $(CFLAGS) $(LDFLAGS) -o DCtap DCtap.o DC.o $(LIBS)
