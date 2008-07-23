# Makefile Generated Tue Oct 11 11:08:19 edt 1994
MNC=idx
HOMEDIR=/usr/local/src/das/diagnost/indexer
SRC=idx.dccc types.tmc idxtmc.prt idxcol.tmc idx.fld idx.a.cmd
SRC+=ix.cmd
TOOL=idx.spec Makefile
SCRIPT=interact
OBJ=idx.cfg idx.tmc idxcol.c idx.pcm tm.dac idxcoloui.c idxcol.o
OBJ+=idxcoloui.o idxdisp.tmc idxdisp.c idxdispoui.c idxdisp.o
OBJ+=idxdispoui.o idxcmd.c idxcmd.dat idxclt.o idxcltoui.c
OBJ+=idxcltoui.o idxsrvr.o idxsrvroui.c idxsrvroui.o
OBJ+=idx.log
TGT=idxcol idxdisp idxclt idxsrvr
DISTRIB=idx.dccc idx.fld types.tmc idxtmc.prt idx.pcm tm.dac
NONRCS=indexer.ag
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
include /usr/local/lib/src/appgen.mk

all : $(TGT)
distribution : $(DSTRB)
	distribute $(TGTDIR) $(DSTRB); :

idxcol : idxcol.o idxcoloui.o
	$(LINK.priv) idxcol.o idxcoloui.o
	$(OUIUSE) idxcoloui.c
	promote idxcol
idxdisp : idxdisp.o idxdispoui.o
	$(LINK.norm) idxdisp.o idxdispoui.o
	$(OUIUSE) idxdispoui.c
idxclt : idxclt.o idxcltoui.o
	$(LINK.norm) idxclt.o idxcltoui.o
	$(OUIUSE) idxcltoui.c
idxsrvr : idxsrvr.o idxsrvroui.o
	$(LINK.priv) idxsrvr.o idxsrvroui.o
	$(OUIUSE) idxsrvroui.c
	promote idxsrvr
idx.tmc : idxtmc.prt
	$(PRT2TMC) idxtmc.prt
idxcol.c : types.tmc idx.tmc idxcol.tmc
	$(TMC.col) types.tmc idx.tmc idxcol.tmc
idxcoloui.c : $(OUIDIR)/collect.oui $(OUIDIR)/subbus_reqd.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/collect.oui $(OUIDIR)/subbus_reqd.oui $(OUIDIR)/msg.oui
idxcol.o : idxcol.c
idxcoloui.o : idxcoloui.c
idxdisp.tmc : idx.fld
	$(FLD2DISP) idx.fld
idxdisp.c : types.tmc idx.tmc idxdisp.tmc
	$(TMC) types.tmc idx.tmc idxdisp.tmc
idxdispoui.c : $(OUIDIR)/nl_cons.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/nl_cons.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
idxdisp.o : idxdisp.c
idxdispoui.o : idxdispoui.c
idxcmd.c : idx.a.cmd ix.cmd
	$(CMDGEN) -d idxcmd.dat idx.a.cmd ix.cmd
idxclt.o : idxcmd.c
	$(COMPILE.clt) idxcmd.c
idxcltoui.c : $(OUIDIR)/client.oui $(OUIDIR)/nl_cons.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/client.oui $(OUIDIR)/nl_cons.oui $(OUIDIR)/msg.oui
idxcltoui.o : idxcltoui.c
idxsrvr.o : idxcmd.c
	$(COMPILE.srvr) idxcmd.c
idxsrvroui.c : $(OUIDIR)/subbus_opt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/subbus_opt.oui $(OUIDIR)/msg.oui
idxsrvroui.o : idxsrvroui.c
