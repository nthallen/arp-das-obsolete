# Makefile for nortlib
HOMEDIR=/usr/local/src/nortlib
MNC=nrtl
LIBNAME=nortlib
# Default model: cannot use "3r" without setting MODELARGS=-ms -3
MODEL=l
# Fill in the models used.
MODELS=3r l s c

#  Maintenance Information: Define TOOL, SRC, OBJ and TEMP
TOOL=Makefile todo

SRC=skeleton.c nl_error.c nl_verr.c nldbg.c
SRC+=colsptr.c colrptr.c colsprx.c colrprx.c find_dg.c send_dg.c
SRC+=nlresp.c find_cc.c send_cc.c senddasc.c repbyte.c
SRC+=ixdrive.c ixscan.c
SRC+=find_name.c find_tmr.c send_tmr.c make_proxy.c make_name.c
SRC+=eir_proxy.c eir_reset.c tmr_set.c tmr_proxy.c tmr_reset.c
SRC+=ix_cmd.c ix_stop.c ix_mvin.c ix_mvout.c ix_offline.c
SRC+=ix_online.c ix_seton.c ix_sofdel.c ix_sondel.c solprox.c
SRC+=coninit.c congetch.c cgrecv.c
SRC+=cmdalgo.c cmdlex.c cmdlex.h
SRC+=cic.c cis.c cictrans.c
SRC+=tma.c rational.c compiler.c

OBJ=$(OBJMDL)/skeleton.o
OBJ+=$(OBJMDL)/nl_error.o $(OBJMDL)/nl_verr.o $(OBJMDL)/nldbg.o
OBJ+=$(OBJMDL)/colsptr.o $(OBJMDL)/find_dg.o $(OBJMDL)/colrptr.o
OBJ+=$(OBJMDL)/send_dg.o $(OBJMDL)/colsprx.o $(OBJMDL)/colrprx.o
OBJ+=$(OBJMDL)/nlresp.o $(OBJMDL)/find_cc.o $(OBJMDL)/send_cc.o
OBJ+=$(OBJMDL)/senddasc.o $(OBJMDL)/repbyte.o
OBJ+=$(OBJMDL)/ixdrive.o $(OBJMDL)/ixscan.o
OBJ+=$(OBJMDL)/find_name.o $(OBJMDL)/find_tmr.o $(OBJMDL)/make_name.o
OBJ+=$(OBJMDL)/send_tmr.o $(OBJMDL)/make_proxy.o
OBJ+=$(OBJMDL)/eir_proxy.o $(OBJMDL)/eir_reset.o
OBJ+=$(OBJMDL)/tmr_set.o $(OBJMDL)/tmr_proxy.o $(OBJMDL)/tmr_reset.o
OBJ+=$(OBJMDL)/ix_cmd.o $(OBJMDL)/ix_stop.o $(OBJMDL)/ix_mvin.o
OBJ+=$(OBJMDL)/ix_mvout.o $(OBJMDL)/ix_offline.o $(OBJMDL)/ix_online.o
OBJ+=$(OBJMDL)/ix_seton.o $(OBJMDL)/ix_sofdel.o $(OBJMDL)/ix_sondel.o
OBJ+=$(OBJMDL)/solprox.o $(OBJMDL)/coninit.o $(OBJMDL)/congetch.o
OBJ+=$(OBJMDL)/cgrecv.o
OBJ+=$(OBJMDL)/cic.o $(OBJMDL)/cis.o $(OBJMDL)/cictrans.o
OBJ+=$(OBJMDL)/tma.o $(OBJMDL)/rational.o $(OBJMDL)/compiler.o

TEMP=*.err *.bak

# include standard library definition
include /usr/local/lib/src/library.mk

# Define dependencies explicitly
$(OBJMDL)/skeleton.o : skeleton.c
$(OBJMDL)/nl_error.o : nl_error.c $(LINC)/nortlib.h
$(OBJMDL)/nl_verr.o : nl_verr.c $(LINC)/nortlib.h
$(OBJMDL)/nldbg.o : nldbg.c $(LINC)/nortlib.h
$(OBJMDL)/colsptr.o : colsptr.c
$(OBJMDL)/find_dg.o : find_dg.c
$(OBJMDL)/colrptr.o : colrptr.c
$(OBJMDL)/send_dg.o : send_dg.c
$(OBJMDL)/colsprx.o : colsprx.c
$(OBJMDL)/colrprx.o : colrprx.c
$(OBJMDL)/nlresp.o : nlresp.c
$(OBJMDL)/find_cc.o : find_cc.c
$(OBJMDL)/send_cc.o : send_cc.c
$(OBJMDL)/senddasc.o : senddasc.c
$(OBJMDL)/repbyte.o : repbyte.c
$(OBJMDL)/ixdrive.o : ixdrive.c
$(OBJMDL)/ixscan.o : ixscan.c
$(OBJMDL)/find_name.o : find_name.c
$(OBJMDL)/make_name.o : make_name.c
$(OBJMDL)/find_tmr.o : find_tmr.c
$(OBJMDL)/send_tmr.o : send_tmr.c
$(OBJMDL)/make_proxy.o : make_proxy.c
$(OBJMDL)/eir_proxy.o : eir_proxy.c $(LINC)/timerbd.h
$(OBJMDL)/eir_reset.o : eir_reset.c $(LINC)/timerbd.h
$(OBJMDL)/tmr_set.o : tmr_set.c $(LINC)/timerbd.h
$(OBJMDL)/tmr_proxy.o : tmr_proxy.c $(LINC)/timerbd.h
$(OBJMDL)/tmr_reset.o : tmr_reset.c $(LINC)/timerbd.h
$(OBJMDL)/ix_cmd.o : ix_cmd.c $(LINC)/indexer.h
$(OBJMDL)/ix_stop.o : ix_stop.c $(LINC)/indexer.h
$(OBJMDL)/ix_mvin.o : ix_mvin.c $(LINC)/indexer.h
$(OBJMDL)/ix_mvout.o : ix_mvout.c $(LINC)/indexer.h
$(OBJMDL)/ix_offline.o : ix_offline.c $(LINC)/indexer.h
$(OBJMDL)/ix_online.o : ix_online.c $(LINC)/indexer.h
$(OBJMDL)/ix_seton.o : ix_seton.c $(LINC)/indexer.h
$(OBJMDL)/ix_sofdel.o : ix_sofdel.c $(LINC)/indexer.h
$(OBJMDL)/ix_sondel.o : ix_sondel.c $(LINC)/indexer.h
$(OBJMDL)/solprox.o : solprox.c $(LINC)/nortlib.h
$(OBJMDL)/coninit.o : coninit.c $(LINC)/nortlib.h $(LINC)/nl_cons.h
$(OBJMDL)/congetch.o : congetch.c $(LINC)/nortlib.h $(LINC)/nl_cons.h
$(OBJMDL)/cgrecv.o : cgrecv.c $(LINC)/nortlib.h $(LINC)/nl_cons.h
$(OBJMDL)/cic.o : cic.c $(LINC)/nortlib.h $(LINC)/cmdalgo.h
$(OBJMDL)/cis.o : cis.c $(LINC)/nortlib.h $(LINC)/cmdalgo.h
$(OBJMDL)/cictrans.o : cictrans.c $(LINC)/nortlib.h $(LINC)/cmdalgo.h
$(OBJMDL)/tma.o : tma.c $(LINC)/nortlib.h
$(OBJMDL)/rational.o : rational.c $(LINC)/rational.h
$(OBJMDL)/compiler.o : compiler.c $(LINC)/compiler.h
