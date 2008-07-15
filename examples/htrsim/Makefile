# Makefile Generated Wed Jan 24 14:01:22 est 2001
MNC=hsim
HOMEDIR=/usr/local/src/das/exp/htrsim
SPECFILE=hsim.spec
SRC=hsim.pl box.pl fld.pl htrsim.h hsim.tmas hsim.tmas.holdv
SRC+=hsim.tmas.large hsim.tmas.lincur hsim.tmas.linpwr hsim.tmas.nop
SRC+=hsim.rtg hsim.tmc hsim.tbl hsim.tmg hsim.doit hsim.edf hsim.tma
SRC+=scan.tmg play.doit hsim.cmd
TOOL=hsim.spec Makefile
SCRIPT=interact Experiment.config Inetdoit
OBJ=hsim.log hsim.gdt hsimcol.c hsim.pcm tm.dac hsimcoloui.c
OBJ+=hsimcol.o hsimcoloui.o hsim.fld hsimdisp.tmc hsimdisptmg.tmc
OBJ+=hsimdisp.c hsimdispoui.c hsimdisp.o hsimdispoui.o hsimextedf.oui
OBJ+=hsimext.tmc hsimext.c hsimextoui.c hsimext.o hsimextoui.o
OBJ+=hsimalgotmg.tmc hsimalgoa.tmc hsimalgo.c hsimalgooui.c
OBJ+=hsimalgo.o hsimalgooui.o hsimcmd.c hsimcmd.dat hsimclt.o
OBJ+=hsimcltoui.c hsimcltoui.o hsimsrvr.o hsimsrvroui.c hsimsrvroui.o
TGT=hsimcol hsimdisp hsimdoit hsimext hsimalgo playdoit hsimclt
TGT+=hsimsrvr
DISTRIB=hsim.tma hsim.tmc hsim.pcm tm.dac hsim.fld
NONRCS=hsim.cfg fields.cfg
TMCBASE=hsim.tmc
CMDBASE=/usr/local/lib/src/root.cmd hsim.cmd
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
include /usr/local/lib/src/appgen.mk

all : $(TGT)
distribution : $(DSTRB)
	distribute $(TGTDIR) $(DSTRB); :

hsimcol : hsimcol.o hsimcoloui.o
	$(LINK.priv) hsimcol.o hsimcoloui.o
	$(OUIUSE) hsimcoloui.c
	promote hsimcol
hsimdisp : hsimdisp.o hsimdispoui.o
	$(LINK.norm) hsimdisp.o hsimdispoui.o
	$(OUIUSE) hsimdispoui.c
hsimdoit : hsim.doit
	mkdoit2 -o hsimdoit  hsim.doit && chmod a+x hsimdoit
hsimext : hsimext.o hsimextoui.o
	$(LINK.norm) hsimext.o hsimextoui.o
	$(OUIUSE) hsimextoui.c
hsimalgo : hsimalgo.o hsimalgooui.o
	$(LINK.norm) hsimalgo.o hsimalgooui.o
	$(OUIUSE) hsimalgooui.c
playdoit : play.doit
	mkdoit2 -o playdoit  play.doit && chmod a+x playdoit
hsimclt : hsimclt.o hsimcltoui.o
	$(LINK.norm) hsimclt.o hsimcltoui.o
	$(OUIUSE) hsimcltoui.c
hsimsrvr : hsimsrvr.o hsimsrvroui.o
	$(LINK.priv) hsimsrvr.o hsimsrvroui.o
	$(OUIUSE) hsimsrvroui.c
	promote hsimsrvr
hsimcol.c : hsim.tmc
	$(TMC.col) hsim.tmc
hsimcoloui.c : $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
hsimcol.o : hsimcol.c
hsimcoloui.o : hsimcoloui.c
hsim.fld : hsim.tbl
	$(TABLE) hsim.tbl
hsimdisp.tmc : hsim.fld
	$(FLD2DISP) hsim.fld
hsimdisptmg.tmc : hsim.tmg
	$(TMG2TMC) hsim.tmg
hsimdisp.c : hsim.tmc /usr/local/lib/src/flttime.tmc hsimdisp.tmc hsimdisptmg.tmc
	$(TMC) hsim.tmc /usr/local/lib/src/flttime.tmc hsimdisp.tmc hsimdisptmg.tmc
hsimdispoui.c : $(OUIDIR)/nl_cons.oui $(OUIDIR)/tmg.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/nl_cons.oui $(OUIDIR)/tmg.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
hsimdisp.o : hsimdisp.c
hsimdispoui.o : hsimdispoui.c
hsimextedf.oui : hsim.edf
	$(EDF2OUI) hsim.edf
hsimext.tmc : hsim.edf
	$(EDF2EXT) hsim.edf
hsimext.c : hsim.tmc hsimext.tmc
	$(TMC) hsim.tmc hsimext.tmc
hsimextoui.c : hsimextedf.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) hsimextedf.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
hsimext.o : hsimext.c
hsimextoui.o : hsimextoui.c
hsimalgotmg.tmc : scan.tmg
	$(TMG2TMC) scan.tmg
hsimalgoa.tmc : hsim.tma hsimsrvr
	$(SERVER) hsimsrvr
	$(TMCALGO) hsim.tma
hsimalgo.c : hsim.tmc hsimalgoa.tmc hsimalgotmg.tmc
	$(TMC) hsim.tmc hsimalgoa.tmc hsimalgotmg.tmc
hsimalgooui.c : $(OUIDIR)/tmg.oui $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/tmg.oui $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
hsimalgo.o : hsimalgo.c
hsimalgooui.o : hsimalgooui.c
hsimcmd.c : /usr/local/lib/src/root.cmd hsim.cmd
	$(CMDGEN) -d hsimcmd.dat /usr/local/lib/src/root.cmd hsim.cmd
hsimclt.o : hsimcmd.c
	$(COMPILE.clt) hsimcmd.c
hsimcltoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
hsimcltoui.o : hsimcltoui.c
hsimsrvr.o : hsimcmd.c
	$(COMPILE.srvr) hsimcmd.c
hsimsrvroui.c : $(OUIDIR)/server.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/server.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
hsimsrvroui.o : hsimsrvroui.c
hsimdoit : hsim.fld
TMAREV=tmcalgoV2R1 -D $(<:tma=gdt)
