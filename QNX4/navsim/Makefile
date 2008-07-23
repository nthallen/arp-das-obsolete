# Makefile Generated Fri Mar 15 17:15:51 est 2002
MNC=navsim
HOMEDIR=/usr/local/src/das/navsim
SPECFILE=navsim.spec
SRC=navout.h navutil.h navout.c navutil.c navout.oui
TOOL=navsim.spec Makefile todo navlist
SCRIPT=
OBJ=navoutoui.c navout.o navutil.o navoutoui.o
TGT=navout
DISTRIB=
NONRCS=
TMCBASE=
CMDBASE=
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
include /usr/local/lib/src/appgen.mk

all : $(TGT)
distribution : $(DSTRB)
	distribute $(TGTDIR) $(DSTRB); :

navout : navout.o navutil.o navoutoui.o
	$(LINK.norm) navout.o navutil.o navoutoui.o
	$(OUIUSE) navoutoui.c
navoutoui.c : navout.oui $(OUIDIR)/msg.oui
	$(OUI) navout.oui $(OUIDIR)/msg.oui
navout.o : navout.c
navutil.o : navutil.c
navoutoui.o : navoutoui.c
