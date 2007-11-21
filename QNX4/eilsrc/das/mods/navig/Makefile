# Makefile Generated Thu Apr 04 12:21:10 est 2002
MNC=navig
HOMEDIR=/usr/local/src/das/mods/navig
SPECFILE=navig.spec
SRC=navigutil.c navvars.h navframe.h navig.tmc swstat.tmc navig.cmd
SRC+=navigfile.cmd swstat.cmd navig.tbl navig.doit navig.tma
TOOL=navig.spec Makefile
SCRIPT=interact navout
OBJ=navigcol.c navig.pcm tm.dac navigcoloui.c navigcol.o navigutil.o
OBJ+=navigcoloui.o navigcmd.c navigcmd.dat navigclt.o navigcltoui.c
OBJ+=navigcltoui.o navigsrvr.o navigsrvroui.c navigsrvroui.o
OBJ+=navig.fld navigdisp.tmc navigdisp.c navigdispoui.c navigdisp.o
OBJ+=navigdispoui.o navigalgoa.tmc navigalgo.c navigalgooui.c
OBJ+=navigalgo.o navigalgooui.o
TGT=navigcol navigclt navigsrvr navigdisp navigdoit navigalgo
DISTRIB=Experiment.config navig.cfg navattr.tmc navig.tma navig.tmc
DISTRIB+=swstat.tmc navig.pcm tm.dac navig.fld
NONRCS=
TMCBASE=navig.tmc swstat.tmc
CMDBASE=//2/usr/local/lib/src/root.cmd navig.cmd navigfile.cmd
CMDBASE+=swstat.cmd navigutil.c
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
include /usr/local/lib/src/appgen.mk

all : $(TGT)
distribution : $(DSTRB)
	distribute $(TGTDIR) $(DSTRB); :

navigcol : navigcol.o navigutil.o navigcoloui.o
	$(LINK.priv) navigcol.o navigutil.o navigcoloui.o
	$(OUIUSE) navigcoloui.c
	promote navigcol
navigclt : navigclt.o navigutil.o navigutil.o navigcltoui.o
	$(LINK.norm) navigclt.o navigutil.o navigutil.o navigcltoui.o
	$(OUIUSE) navigcltoui.c
navigsrvr : navigsrvr.o navigutil.o navigutil.o navigsrvroui.o
	$(LINK.priv) navigsrvr.o navigutil.o navigutil.o navigsrvroui.o
	$(OUIUSE) navigsrvroui.c
	promote navigsrvr
navigdisp : navigdisp.o navigutil.o navigdispoui.o
	$(LINK.norm) navigdisp.o navigutil.o navigdispoui.o
	$(OUIUSE) navigdispoui.c
navigdoit : navig.doit
	mkdoit2 -o navigdoit  navig.doit && chmod a+x navigdoit
navigalgo : navigalgo.o navigalgooui.o
	$(LINK.norm) navigalgo.o navigalgooui.o
	$(OUIUSE) navigalgooui.c
navigcol.c : navig.tmc swstat.tmc
	$(TMC.col) navig.tmc swstat.tmc
navigcoloui.c : $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
navigcol.o : navigcol.c
navigutil.o : navigutil.c
navigcoloui.o : navigcoloui.c
navigcmd.c : //2/usr/local/lib/src/root.cmd navig.cmd navigfile.cmd swstat.cmd
	$(CMDGEN) -d navigcmd.dat //2/usr/local/lib/src/root.cmd navig.cmd navigfile.cmd swstat.cmd
navigclt.o : navigcmd.c
	$(COMPILE.clt) navigcmd.c
navigcltoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
navigcltoui.o : navigcltoui.c
navigsrvr.o : navigcmd.c
	$(COMPILE.srvr) navigcmd.c
navigsrvroui.c : $(OUIDIR)/server.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/server.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
navigsrvroui.o : navigsrvroui.c
navig.fld : navig.tbl
	$(TABLE) navig.tbl
navigdisp.tmc : navig.fld
	$(FLD2DISP) navig.fld
navigdisp.c : navig.tmc swstat.tmc navigdisp.tmc
	$(TMC) navig.tmc swstat.tmc navigdisp.tmc
navigdispoui.c : $(OUIDIR)/nl_cons.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/nl_cons.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
navigdisp.o : navigdisp.c
navigdispoui.o : navigdispoui.c
navigalgoa.tmc : navig.tma navigsrvr
	$(SERVER) navigsrvr
	$(TMCALGO) navig.tma
navigalgo.c : navig.tmc swstat.tmc navigalgoa.tmc
	$(TMC) navig.tmc swstat.tmc navigalgoa.tmc
navigalgooui.c : $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
navigalgo.o : navigalgo.c
navigalgooui.o : navigalgooui.c
