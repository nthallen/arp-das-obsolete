TOOL=Makefile design todo handlers.doc
BINSRC=rtg.ag scan.sps clip.sps limits.sps
BINSRC+=rtgbase.wnd rtgicon.pict
BINSRC+=rtgopt.wnd channel.pict
BINSRC+=chanprop.wnd chanprop.pict
BINSRC+=axisprop.pict graphprop.pict windprop.pict
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
SRC+=clip.c
SRC+=chan_tree.c
SRC+=chanprop.c
SRC+=axisprop.c axprop.c
SRC+=graphprop.c windprop.c
SRC+=messages.c messages.h
SRC+=props.c

OBJ=rtg.o winmgr.o basewin.o channels.o chan_int.o graph.o dummy.o axis.o
OBJ+=snafu.o clip.o chan_tree.o chanprop.o axprop.o
OBJ+=cdb.o messages.o props.o graphprop.o windprop.o

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
TARGET=rtg rtg.save

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
snafu.o : snafu.c rtg.h
chan_tree.o : chan_tree.c rtg.h
chanprop.o : chanprop.c rtg.h
axprop.o : axprop.c rtg.h
props.o : props.c rtg.h $(LINC)/nortlib.h
graphprop.o : graphprop.c rtg.h $(LINC)/nortlib.h
windprop.o : windprop.c rtg.h $(LINC)/nortlib.h
dummy.o : dummy.c rtg.h
cdb.o : cdb.c messages.h $(LINC)/rtgapi.h
messages.o : messages.c rtg.h messages.h $(LINC)/rtgapi.h
