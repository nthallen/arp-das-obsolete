TOOL=Makefile design todo handlers.doc
TOOL+=announce
BINSRC=rtg.ag scan.sps clip.sps limits.sps
BINSRC+=rtgicon.pict
BINSRC+=chanprop.pict
BINSRC+=graphprop.pict windprop.pict axprop2.pict 
BINSRC+=globprop.pict
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
SRC+=axisprop.c
SRC+=graphprop.c windprop.c
SRC+=messages.c messages.h
SRC+=proper.c elttype.c script.c rtgscript.c
SRC+=inits.oui globprop.c

OBJ=rtg.o winmgr.o basewin.o channels.o chan_int.o graph.o dummy.o axis.o
OBJ+=snafu.o clip.o chan_tree.o chanprop.o axisprop.o script.o
OBJ+=cdb.o messages.o graphprop.o windprop.o proper.o elttype.o rtgscript.o
OBJ+=inits.o globprop.o

MODEL=l
MODELARGS=-m$(MODEL) -2
# OPTARGS=-Oatx is good for speed
OPTARGS=-g
CFLAGS=$(MODELARGS) -fo$@ -w4 $(OPTARGS)

MNC=rtg
HOMEDIR=/usr/local/src/das/rtg
RCS=$(TOOL) $(SRC)
SOURCE=$(TOOL) $(BINSRC) $(SRC)
OBJECT=$(OBJ) inits.c *.bg *bak *.err
OBJECT+=source.html source.txt sourcel.html
TARGET=rtg rtg.save
DISTRIB=rtg axprop2.pict rtgicon.pict chanprop.pict 
DISTRIB+=graphprop.pict windprop.pict globprop.pict
DISTRIB+=scan.sps

LINC=/usr/local/include

rtg : $(OBJ)
	$(LINK.c) -o rtg $(OBJ) $(LDFLAGS)
	usemsg rtg inits.c
rtg.o : rtg.c rtg.h $(LINC)/rtgapi.h $(LINC)/nortlib.h $(LINC)/oui.h
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
axisprop.o : axisprop.c rtg.h
props.o : props.c rtg.h $(LINC)/nortlib.h
graphprop.o : graphprop.c rtg.h $(LINC)/nortlib.h
dummy.o : dummy.c rtg.h
cdb.o : cdb.c messages.h $(LINC)/rtgapi.h
messages.o : messages.c rtg.h messages.h $(LINC)/rtgapi.h
proper.o : proper.c rtg.h $(LINC)/nortlib.h
elttype.o : elttype.c rtg.h
windprop.o : windprop.c rtg.h
script.o : script.c rtg.h $(LINC)/nortlib.h
rtgscript.o : rtgscript.c rtg.h $(LINC)/nortlib.h
inits.o : inits.c rtg.h $(LINC)/nortlib.h $(LINC)/oui.h
inits.c : inits.oui
	oui inits
globprop.o : globprop.c rtg.h

distribution : $(DISTRIB)
	cp -nv $(DISTRIB) $(TGTNODE)/usr/lib/windows/apps/rtg; :
