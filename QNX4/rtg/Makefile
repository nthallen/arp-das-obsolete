TOOL=Makefile design todo rtg.ag rtg.prt scan.sps clip.sps
SRC=rtgbase.wnd rtgicon.pict
SRC+=rtgopt.wnd channel.pict
SRC+=axisopt.wnd axisopt.pict
SRC+=select.wnd
SRC+=axisopt.h
SRC+=rtg.c rtg.h
SRC+=winmgr.c
SRC+=basewin.c
SRC+=channels.c
SRC+=chan_int.c
SRC+=graph.c
SRC+=axis.c
SRC+=dummy.c
SRC+=cdb.c cdb.h
SRC+=snafu.c
SRC+=select.c
SRC+=clip.c

OBJ=rtg.o winmgr.o basewin.o channels.o chan_int.o graph.o dummy.o axis.o
OBJ+=snafu.o select.o clip.o

MODEL=s
MODELARGS=-m$(MODEL) -2
# OPTARGS=-Oatx is good for speed
OPTARGS=-g
CFLAGS=$(MODELARGS) -fo$@ -w4 $(OPTARGS)

MNC=rtg
HOMEDIR=/usr/local/src/das/rtg
SOURCE=$(TOOL) $(SRC)
OBJECT=$(OBJ) *.bg *bak *.err
TARGET=rtg

rtg : $(OBJ)
	$(LINK.c) -o rtg $(OBJ) $(LDFLAGS)
rtg.o : rtg.c rtg.h
basewin.o : basewin.c rtg.h
winmgr.o : winmgr.c rtg.h
channels.o : channels.c rtg.h
chan_int.o : chan_int.c rtg.h
graph.o : graph.c rtg.h
clip.o : clip.c rtg.h
axis.o : axis.c rtg.h
select.o : select.c rtg.h
snafu.o : snafu.c rtg.h
dummy.o : dummy.c rtg.h
cdb.o : cdb.c cdb.h
