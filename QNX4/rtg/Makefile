TOOL=Makefile design todo handlers.doc
BINSRC=rtg.ag scan.sps clip.sps limits.sps
BINSRC+=rtgbase.wnd rtgicon.pict
BINSRC+=rtgopt.wnd channel.pict
BINSRC+=axisopt.wnd axisopt.pict
BINSRC+=select.wnd
BINSRC+=chanprop.wnd chanprop.pict
BINSRC+=axisprop.pict
SRC+=axisopt.h
SRC+=rtg.c rtg.h
SRC+=winmgr.c
SRC+=basewin.c
SRC+=channels.c
SRC+=chan_int.c
SRC+=graph.c
SRC+=axis.c
SRC+=dummy.c
SRC+=cdb.c
SRC+=snafu.c
SRC+=select.c
SRC+=clip.c
SRC+=chan_tree.c
SRC+=chanprop.c
SRC+=axisprop.c
SRC+=messages.c messages.h

OBJ=rtg.o winmgr.o basewin.o channels.o chan_int.o graph.o dummy.o axis.o
OBJ+=snafu.o select.o clip.o chan_tree.o chanprop.o axisprop.o
OBJ+=cdb.o messages.o

MODEL=l
MODELARGS=-m$(MODEL) -2
# OPTARGS=-Oatx is good for speed
OPTARGS=-g
CFLAGS=$(MODELARGS) -fo$@ -w4 $(OPTARGS)

MNC=rtg
HOMEDIR=/usr/local/src/das/rtg
RCS=$(TOOL) $(SRC)
SOURCE=$(TOOL) $(BINSRC) $(SRC)
OBJECT=$(OBJ) *.bg *bak *.err
TARGET=rtg

LINC=/usr/local/include

rtg : $(OBJ)
	$(LINK.c) -o rtg $(OBJ) $(LDFLAGS)
rtg.o : rtg.c rtg.h $(LINC)/rtgapi.h
basewin.o : basewin.c rtg.h
winmgr.o : winmgr.c rtg.h messages.h $(LINC)/rtgapi.h
channels.o : channels.c rtg.h
chan_int.o : chan_int.c rtg.h
graph.o : graph.c rtg.h
clip.o : clip.c rtg.h
axis.o : axis.c rtg.h
select.o : select.c rtg.h
snafu.o : snafu.c rtg.h
chan_tree.o : chan_tree.c rtg.h
chanprop.o : chanprop.c rtg.h
axisprop.o : axisprop.c rtg.h
dummy.o : dummy.c rtg.h
cdb.o : cdb.c messages.h $(LINC)/rtgapi.h
messages.o : messages.c rtg.h messages.h $(LINC)/rtgapi.h
