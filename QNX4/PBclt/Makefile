# Makefile Generated Mon Oct 18 11:43:06 edt 1999
MNC=PB
HOMEDIR=/usr/local/src/das/PBclt
SPECFILE=PB.spec
SRC=PBreg.c PB.cmd
TOOL=PB.spec Makefile
SCRIPT=
OBJ=PBregoui.c PBreg.o PBregoui.o PBcmd.c PBcmd.dat PBclt.o
OBJ+=PBcltoui.c PBcltoui.o PBsrvr.o PBsrvroui.c PBsrvroui.o
TGT=PBreg PBclt PBsrvr
DISTRIB=
NONRCS=
TMCBASE=
CMDBASE=PB.cmd
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
TGTDIR= /usr/local/bin
include /usr/local/lib/src/appgen.mk

all : $(TGT)
distribution : $(DSTRB)
	distribute $(TGTDIR) $(DSTRB); :

PBreg : PBreg.o PBregoui.o
	$(LINK.norm) PBreg.o PBregoui.o
	$(OUIUSE) PBregoui.c
PBclt : PBclt.o PBcltoui.o
	$(LINK.norm) PBclt.o PBcltoui.o
	$(OUIUSE) PBcltoui.c
PBsrvr : PBsrvr.o PBsrvroui.o
	$(LINK.priv) PBsrvr.o PBsrvroui.o
	$(OUIUSE) PBsrvroui.c
	promote PBsrvr
PBregoui.c : $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
PBreg.o : PBreg.c
PBregoui.o : PBregoui.c
PBcmd.c : PB.cmd
	$(CMDGEN) -d PBcmd.dat PB.cmd
PBclt.o : PBcmd.c
	$(COMPILE.clt) PBcmd.c
PBcltoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
PBcltoui.o : PBcltoui.c
PBsrvr.o : PBcmd.c
	$(COMPILE.srvr) PBcmd.c
PBsrvroui.c : $(OUIDIR)/server.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus_opt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/server.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus_opt.oui $(OUIDIR)/msg.oui
PBsrvroui.o : PBsrvroui.c
LBIN=/usr/local/bin
dist : PBreg PBclt
	@echo "Distribute to $(TGTDIR): "; su -c "ln -fv PBreg PBclt $(TGTDIR)"