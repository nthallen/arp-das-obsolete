# Makefile for nortlib
# Compiling options
MODEL=l
LINC=$(LINCNODE)/usr/local/include
LIB=$(LIBNODE)/usr/local/lib
CFLAGS=-m$(MODEL) -2 -w4 -g -I $(LINC)

.c.o:
	$(COMPILE.c) -Wc,-fo$@ $<

#  Maintenance Information
SRC=skeleton.c nl_error.c nl_verr.c nldbg.c
SRC+=colsptr.c colrptr.c colsprx.c colrprx.c find_dg.c send_dg.c
SRC+=nlresp.c find_cc.c send_cc.c senddasc.c repbyte.c
SRC+=ixdrive.c ixscan.c
SRC+=find_name.c find_tmr.c send_tmr.c make_proxy.c
SRC+=eir_proxy.c eir_reset.c tmr_set.c tmr_proxy.c tmr_reset.c
SRC+=ix_cmd.c ix_stop.c ix_mvin.c ix_mvout.c ix_offline.c
SRC+=ix_online.c ix_seton.c ix_sofdel.c ix_sondel.c solprox.c
SRC+=coninit.c congetch.c cgrecv.c
SRC+=cmdalgo.c cmdlex.c cmdlex.h
SRC+=cic.c cis.c cictrans.c
SRC+=tma.c rational.c
TOOL=Makefile todo
OBJ=$(MODEL)/skeleton.o
OBJ+=$(MODEL)/nl_error.o $(MODEL)/nl_verr.o $(MODEL)/nldbg.o
OBJ+=$(MODEL)/colsptr.o $(MODEL)/find_dg.o $(MODEL)/colrptr.o
OBJ+=$(MODEL)/send_dg.o $(MODEL)/colsprx.o $(MODEL)/colrprx.o
OBJ+=$(MODEL)/nlresp.o $(MODEL)/find_cc.o $(MODEL)/send_cc.o
OBJ+=$(MODEL)/senddasc.o $(MODEL)/repbyte.o
OBJ+=$(MODEL)/ixdrive.o $(MODEL)/ixscan.o
OBJ+=$(MODEL)/find_name.o $(MODEL)/find_tmr.o
OBJ+=$(MODEL)/send_tmr.o $(MODEL)/make_proxy.o
OBJ+=$(MODEL)/eir_proxy.o $(MODEL)/eir_reset.o
OBJ+=$(MODEL)/tmr_set.o $(MODEL)/tmr_proxy.o $(MODEL)/tmr_reset.o
OBJ+=$(MODEL)/ix_cmd.o $(MODEL)/ix_stop.o $(MODEL)/ix_mvin.o
OBJ+=$(MODEL)/ix_mvout.o $(MODEL)/ix_offline.o $(MODEL)/ix_online.o
OBJ+=$(MODEL)/ix_seton.o $(MODEL)/ix_sofdel.o $(MODEL)/ix_sondel.o
OBJ+=$(MODEL)/solprox.o $(MODEL)/coninit.o $(MODEL)/congetch.o
OBJ+=$(MODEL)/cgrecv.o $(MODEL)/cmdalgo.o $(MODEL)/cmdlex.o
OBJ+=$(MODEL)/cic.o $(MODEL)/cis.o $(MODEL)/cictrans.o
OBJ+=$(MODEL)/tma.o $(MODEL)/rational.o
TEMP=*.err *.bak
TGT=$(LIB)/nortlib$(MODEL).lib

SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ) $(TEMP)
TARGET=
MODELS=l s c
MNC=nrtl
HOMEDIR=/usr/local/src/nortlib

$(TGT) : $(OBJ)
	@echo $(OBJ) | xargs -i echo +{} | xargs -t wlib -n $(TGT)
$(MODEL)/skeleton.o : skeleton.c
$(MODEL)/nl_error.o : nl_error.c $(LINC)/nortlib.h
$(MODEL)/nl_verr.o : nl_verr.c $(LINC)/nortlib.h
$(MODEL)/nldbg.o : nldbg.c $(LINC)/nortlib.h
$(MODEL)/colsptr.o : colsptr.c
$(MODEL)/find_dg.o : find_dg.c
$(MODEL)/colrptr.o : colrptr.c
$(MODEL)/send_dg.o : send_dg.c
$(MODEL)/colsprx.o : colsprx.c
$(MODEL)/colrprx.o : colrprx.c
$(MODEL)/nlresp.o : nlresp.c
$(MODEL)/find_cc.o : find_cc.c
$(MODEL)/send_cc.o : send_cc.c
$(MODEL)/senddasc.o : senddasc.c
$(MODEL)/repbyte.o : repbyte.c
$(MODEL)/ixdrive.o : ixdrive.c
$(MODEL)/ixscan.o : ixscan.c
$(MODEL)/find_name.o : find_name.c
$(MODEL)/find_tmr.o : find_tmr.c
$(MODEL)/send_tmr.o : send_tmr.c
$(MODEL)/make_proxy.o : make_proxy.c
$(MODEL)/eir_proxy.o : eir_proxy.c $(LINC)/timerbd.h
$(MODEL)/eir_reset.o : eir_reset.c $(LINC)/timerbd.h
$(MODEL)/tmr_set.o : tmr_set.c $(LINC)/timerbd.h
$(MODEL)/tmr_proxy.o : tmr_proxy.c $(LINC)/timerbd.h
$(MODEL)/tmr_reset.o : tmr_reset.c $(LINC)/timerbd.h
$(MODEL)/ix_cmd.o : ix_cmd.c $(LINC)/indexer.h
$(MODEL)/ix_stop.o : ix_stop.c $(LINC)/indexer.h
$(MODEL)/ix_mvin.o : ix_mvin.c $(LINC)/indexer.h
$(MODEL)/ix_mvout.o : ix_mvout.c $(LINC)/indexer.h
$(MODEL)/ix_offline.o : ix_offline.c $(LINC)/indexer.h
$(MODEL)/ix_online.o : ix_online.c $(LINC)/indexer.h
$(MODEL)/ix_seton.o : ix_seton.c $(LINC)/indexer.h
$(MODEL)/ix_sofdel.o : ix_sofdel.c $(LINC)/indexer.h
$(MODEL)/ix_sondel.o : ix_sondel.c $(LINC)/indexer.h
$(MODEL)/solprox.o : solprox.c $(LINC)/nortlib.h
$(MODEL)/coninit.o : coninit.c $(LINC)/nortlib.h $(LINC)/nl_cons.h
$(MODEL)/congetch.o : congetch.c $(LINC)/nortlib.h $(LINC)/nl_cons.h
$(MODEL)/cgrecv.o : cgrecv.c $(LINC)/nortlib.h $(LINC)/nl_cons.h
$(MODEL)/cmdalgo.o : cmdalgo.c cmdlex.h $(LINC)/nortlib.h $(LINC)/nl_cons.h
$(MODEL)/cmdlex.o : cmdlex.c cmdlex.h $(LINC)/nortlib.h $(LINC)/nl_cons.h
$(MODEL)/cic.o : cic.c $(LINC)/nortlib.h $(LINC)/cmdalgo.h
$(MODEL)/cis.o : cis.c $(LINC)/nortlib.h $(LINC)/cmdalgo.h
$(MODEL)/cictrans.o : cictrans.c $(LINC)/nortlib.h $(LINC)/cmdalgo.h
$(MODEL)/tma.o : tma.c $(LINC)/nortlib.h
$(MODEL)/rational.o : rational.c $(LINC)/rational.h
