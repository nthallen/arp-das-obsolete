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
SRC+=colsend.c
SRC+=nlresp.c find_cc.c send_cc.c senddasc.c repbyte.c
SRC+=ixdrive.c ixscan.c
SRC+=find_name.c find_tmr.c send_tmr.c make_proxy.c make_name.c
SRC+=eir_proxy.c eir_reset.c tmr_set.c tmr_proxy.c tmr_reset.c
SRC+=ix_cmd.c
SRC+=solprox.c cmdprox.c
SRC+=coninit.c congetch.c cgrecv.c
SRC+=cmdalgo.c cmdlex.c cmdlex.h
SRC+=cic.c cicf.c cis.c cictrans.c
SRC+=tma.c rational.c compiler.c medfilt.c newmem.c digdelay.c
SRC+=digdlyul.c digdelay.h seq_ss.c strdup.c ll_of_str.c
SRC+=opnifile.c

OBJ=$(OBJMDL)/skeleton.o
OBJ+=$(OBJMDL)/nl_error.o $(OBJMDL)/nl_verr.o $(OBJMDL)/nldbg.o
OBJ+=$(OBJMDL)/colsptr.o $(OBJMDL)/find_dg.o $(OBJMDL)/colrptr.o
OBJ+=$(OBJMDL)/colsend.o
OBJ+=$(OBJMDL)/send_dg.o $(OBJMDL)/colsprx.o $(OBJMDL)/colrprx.o
OBJ+=$(OBJMDL)/nlresp.o $(OBJMDL)/find_cc.o $(OBJMDL)/send_cc.o
OBJ+=$(OBJMDL)/senddasc.o $(OBJMDL)/repbyte.o
OBJ+=$(OBJMDL)/ixdrive.o $(OBJMDL)/ixscan.o
OBJ+=$(OBJMDL)/find_name.o $(OBJMDL)/find_tmr.o $(OBJMDL)/make_name.o
OBJ+=$(OBJMDL)/send_tmr.o $(OBJMDL)/make_proxy.o
OBJ+=$(OBJMDL)/eir_proxy.o $(OBJMDL)/eir_reset.o
OBJ+=$(OBJMDL)/tmr_set.o $(OBJMDL)/tmr_proxy.o $(OBJMDL)/tmr_reset.o
OBJ+=$(OBJMDL)/ix_cmd.o
OBJ+=$(OBJMDL)/solprox.o $(OBJMDL)/coninit.o $(OBJMDL)/congetch.o
OBJ+=$(OBJMDL)/cgrecv.o
OBJ+=$(OBJMDL)/cic.o $(OBJMDL)/cicf.o $(OBJMDL)/cis.o $(OBJMDL)/cictrans.o
OBJ+=$(OBJMDL)/tma.o $(OBJMDL)/rational.o $(OBJMDL)/compiler.o
OBJ+=$(OBJMDL)/medfilt.o $(OBJMDL)/newmem.o $(OBJMDL)/digdelay.o
OBJ+=$(OBJMDL)/digdlyul.o $(OBJMDL)/seq_ss.o
OBJ+=$(OBJMDL)/strdup.o $(OBJMDL)/ll_of_str.o $(OBJMDL)/opnifile.o
OBJ+=$(OBJMDL)/cmdprox.o

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
$(OBJMDL)/colsend.o : colsend.c $(LINC)/collect.h $(LINC)/nortlib.h
$(OBJMDL)/nlresp.o : nlresp.c
$(OBJMDL)/find_cc.o : find_cc.c
$(OBJMDL)/send_cc.o : send_cc.c
$(OBJMDL)/senddasc.o : senddasc.c
$(OBJMDL)/repbyte.o : repbyte.c
$(OBJMDL)/ixdrive.o : ixdrive.c $(LINC)/indexer.h
$(OBJMDL)/ixscan.o : ixscan.c $(LINC)/indexer.h
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
$(OBJMDL)/solprox.o : solprox.c $(LINC)/nortlib.h
$(OBJMDL)/coninit.o : coninit.c $(LINC)/nortlib.h $(LINC)/nl_cons.h
$(OBJMDL)/congetch.o : congetch.c $(LINC)/nortlib.h $(LINC)/nl_cons.h
$(OBJMDL)/cgrecv.o : cgrecv.c $(LINC)/nortlib.h $(LINC)/nl_cons.h
$(OBJMDL)/cic.o : cic.c $(LINC)/nortlib.h $(LINC)/cmdalgo.h
$(OBJMDL)/cicf.o : cicf.c $(LINC)/nortlib.h $(LINC)/cmdalgo.h
$(OBJMDL)/cis.o : cis.c $(LINC)/nortlib.h $(LINC)/cmdalgo.h
$(OBJMDL)/cictrans.o : cictrans.c $(LINC)/nortlib.h $(LINC)/cmdalgo.h
$(OBJMDL)/tma.o : tma.c $(LINC)/nortlib.h
$(OBJMDL)/rational.o : rational.c $(LINC)/rational.h
$(OBJMDL)/compiler.o : compiler.c $(LINC)/compiler.h
$(OBJMDL)/newmem.o : newmem.c $(LINC)/nortlib.h
$(OBJMDL)/medfilt.o : medfilt.c $(LINC)/nl_dsp.h $(LINC)/nortlib.h
$(OBJMDL)/digdelay.o : digdelay.c $(LINC)/nl_dsp.h $(LINC)/nortlib.h
$(OBJMDL)/digdlyul.o : digdlyul.c digdelay.h $(LINC)/nl_dsp.h $(LINC)/nortlib.h
$(OBJMDL)/seq_ss.o : seq_ss.c $(LINC)/seq_ss.h $(LINC)/nortlib.h $(LINC)/ssp.h
$(OBJMDL)/strdup.o : strdup.c $(LINC)/nortlib.h
$(OBJMDL)/ll_of_str.o : ll_of_str.c $(LINC)/nortlib.h $(LINC)/ll_of_str.h
$(OBJMDL)/opnifile.o : opnifile.c $(LINC)/nortlib.h $(LINC)/compiler.h
$(OBJMDL)/cmdprox.o : cmdprox.c $(LINC)/nortlib.h
