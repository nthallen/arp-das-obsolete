# Makefile Generated Fri Oct 22 14:33:04 edt 1999
# Then editted by HAND! to remove msg.oui from Inetout
MNC=Inet
HOMEDIR=/usr/local/src/das/Inet
SPECFILE=Inet.spec
SRC=todo Inet.h Inetout.c Inet.c
SRC+=Inetout.oui Inetin.c Inetin.oui
TOOL=Inet.spec Makefile
SCRIPT=
OBJ=Inetoutoui.c Inetout.o Inet.o Inetoutoui.o Inetinoui.c Inetin.o
OBJ+=Inetinoui.o
TGT=Inetout Inetin
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

Inetout : Inetout.o Inet.o Inetoutoui.o
	$(LINK.norm) Inetout.o Inet.o Inetoutoui.o
	$(OUIUSE) Inetoutoui.c
Inetin : Inetin.o Inet.o Inetinoui.o
	$(LINK.norm) Inetin.o Inet.o Inetinoui.o
	$(OUIUSE) Inetinoui.c
Inetoutoui.c : Inetout.oui
	$(OUI) Inetout.oui
Inetout.o : Inetout.c
Inet.o : Inet.c
Inetoutoui.o : Inetoutoui.c
Inetinoui.c : Inetin.oui $(OUIDIR)/msg.oui
	$(OUI) Inetin.oui $(OUIDIR)/msg.oui
Inetin.o : Inetin.c
Inetinoui.o : Inetinoui.c
LBIN=/usr/local/bin
all : $(LBIN)/Inetin $(LBIN)/Inetout
$(LBIN)/Inetin : Inetin
	ln -f Inetin $(LBIN)
$(LBIN)/Inetout : Inetout
	ln -f Inetout $(LBIN)
