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
SRC+=ix_cmd.c kbdclt.c
SRC+=solprox.c cmdprox.c
SRC+=coninit.c congetch.c cgrecv.c condisp.c
SRC+=cmdalgo.c cmdlex.c cmdlex.h
SRC+=cic.c cicf.c cis.c cictrans.c
SRC+=tma.c tmaR1.c tmaR2.c tma_file.c
SRC+=rational.c compiler.c newmem.c
SRC+=seq_ss.c strdup.c ll_of_str.c
SRC+=opnifile.c intserv.c cltsrvr.c ccquit.c idx64.c linefind.c
SRC+=average.c PBdef.c
SRC+=cache.c dccc.c mlf.c analogic.c
SRC+=timetext.c memo.c getcon.c
SRC+=md5c.c md5.h
SRC+=nav_a2n.c nav_n2a.c navutil.h
SRC+=qcliclt.c qcliclt.h
#--------------------
# The HPF family:
#--------------------
SRC+=runnum.c runcat.c runmol.c runwrite.c drinit.c
#----------------
# Here's the DSP family:
#----------------
SRC+=digdly.c  medfilt.c medfiltg.c digdelay.c runavg.c
#----------------
# Here's the RTG family:
#----------------
SRC+=rtgapi.c
#----------------
# GPIB232
#----------------
SRC+=gpib232.c
SRC+=dtoe.c
#-----------------------------------------
# Thompson Stack, DMM32 Client, OMS Client
#-----------------------------------------
SRC+=ttclient.c
SRC+=dmmclient.c omsclient.c

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
OBJ+=$(OBJMDL)/solprox.o
OBJ+=$(OBJMDL)/coninit.o $(OBJMDL)/congetch.o $(OBJMDL)/condisp.o
OBJ+=$(OBJMDL)/cgrecv.o
OBJ+=$(OBJMDL)/cic.o $(OBJMDL)/cicf.o $(OBJMDL)/cis.o $(OBJMDL)/cictrans.o
OBJ+=$(OBJMDL)/tma.o $(OBJMDL)/tmaR1.o $(OBJMDL)/tmaR2.o 
OBJ+=$(OBJMDL)/tma_file.o
OBJ+=$(OBJMDL)/rational.o $(OBJMDL)/compiler.o
OBJ+=$(OBJMDL)/seq_ss.o $(OBJMDL)/newmem.o
OBJ+=$(OBJMDL)/strdup.o $(OBJMDL)/ll_of_str.o $(OBJMDL)/opnifile.o
OBJ+=$(OBJMDL)/cmdprox.o
OBJ+=$(OBJMDL)/medfilt.o $(OBJMDL)/medfiltus.o
OBJ+=$(OBJMDL)/medfiltss.o
OBJ+=$(OBJMDL)/digdelay.o
OBJ+=$(OBJMDL)/digdlyul.o $(OBJMDL)/digdlydbl.o $(OBJMDL)/runavg.o
OBJ+=$(OBJMDL)/rtgapi.o
OBJ+=$(OBJMDL)/kbdclt.o $(OBJMDL)/gpib232.o $(OBJMDL)/dtoe.o
OBJ+=$(OBJMDL)/ttclient.o $(OBJMDL)/dmmclient.o
OBJ+=$(OBJMDL)/omsclient.o
OBJ+=$(OBJMDL)/cltsrvr.o $(OBJMDL)/intserv.o $(OBJMDL)/ccquit.o
OBJ+=$(OBJMDL)/idx64.o $(OBJMDL)/linefind.o
OBJ+=$(OBJMDL)/average.o $(OBJMDL)/PBdef.o $(OBJMDL)/cache.o
OBJ+=$(OBJMDL)/dccc.o $(OBJMDL)/timetext.o $(OBJMDL)/memo.o
OBJ+=$(OBJMDL)/getcon.o
OBJ+=$(OBJMDL)/runnum.o $(OBJMDL)/runcat.o $(OBJMDL)/runmol.o
OBJ+=$(OBJMDL)/runwrite.o $(OBJMDL)/drinit.o
OBJ+=$(OBJMDL)/mlf.o $(OBJMDL)/analogic.o
OBJ+=$(OBJMDL)/md5c.o
OBJ+=$(OBJMDL)/nav_a2n.o $(OBJMDL)/nav_n2a.o
OBJ+=$(OBJMDL)/qcliclt.o

TEMP=*.err *.bak source.html sourcel.html source.txt

# include standard library definition
include /usr/local/lib/src/library.mk

NLH=$(LINC)/nortlib.h

# Define dependencies explicitly
$(OBJMDL)/skeleton.o : skeleton.c
$(OBJMDL)/nl_error.o : nl_error.c $(NLH)
$(OBJMDL)/nl_verr.o : nl_verr.c $(NLH)
$(OBJMDL)/nldbg.o : nldbg.c $(NLH)
$(OBJMDL)/colsptr.o : colsptr.c
$(OBJMDL)/find_dg.o : find_dg.c
$(OBJMDL)/colrptr.o : colrptr.c
$(OBJMDL)/send_dg.o : send_dg.c
$(OBJMDL)/colsprx.o : colsprx.c
$(OBJMDL)/colrprx.o : colrprx.c
$(OBJMDL)/colsend.o : colsend.c $(LINC)/collect.h $(NLH)
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
$(OBJMDL)/solprox.o : solprox.c $(NLH)
$(OBJMDL)/coninit.o : coninit.c $(NLH) $(LINC)/nl_cons.h
$(OBJMDL)/congetch.o : congetch.c $(NLH) $(LINC)/nl_cons.h
$(OBJMDL)/condisp.o : condisp.c $(NLH) $(LINC)/nl_cons.h
$(OBJMDL)/cgrecv.o : cgrecv.c $(NLH) $(LINC)/nl_cons.h
$(OBJMDL)/cic.o : cic.c $(NLH) $(LINC)/cmdalgo.h
$(OBJMDL)/cicf.o : cicf.c $(NLH) $(LINC)/cmdalgo.h
$(OBJMDL)/cis.o : cis.c $(NLH) $(LINC)/cmdalgo.h
$(OBJMDL)/cictrans.o : cictrans.c $(NLH) $(LINC)/cmdalgo.h
$(OBJMDL)/tma.o : tma.c $(NLH) $(LINC)/tma.h
$(OBJMDL)/tmaR1.o : tmaR1.c $(NLH) $(LINC)/tma.h
$(OBJMDL)/tmaR2.o : tmaR2.c $(NLH) $(LINC)/tma.h
$(OBJMDL)/tma_file.o : tma_file.c $(NLH) $(LINC)/tma.h
$(OBJMDL)/rational.o : rational.c $(LINC)/rational.h
$(OBJMDL)/compiler.o : compiler.c $(LINC)/compiler.h
$(OBJMDL)/newmem.o : newmem.c $(NLH)
$(OBJMDL)/seq_ss.o : seq_ss.c $(LINC)/seq_ss.h $(NLH) $(LINC)/ssp.h
$(OBJMDL)/strdup.o : strdup.c $(NLH)
$(OBJMDL)/ll_of_str.o : ll_of_str.c $(NLH) $(LINC)/ll_of_str.h
$(OBJMDL)/opnifile.o : opnifile.c $(NLH) $(LINC)/compiler.h
$(OBJMDL)/cmdprox.o : cmdprox.c $(NLH)
$(OBJMDL)/medfilt.o : medfilt.c $(LINC)/nl_dsp.h $(NLH)
$(OBJMDL)/medfiltus.o : medfiltg.c $(LINC)/nl_dsp.h $(NLH)
	$(COMPILE.c) -Wc,-nm=medfiltus -D MF_USHORT medfiltg.c 2>medfiltus.err
$(OBJMDL)/medfiltss.o : medfiltg.c $(LINC)/nl_dsp.h $(NLH)
	$(COMPILE.c) -Wc,-nm=medfiltss -D MF_SSHORT medfiltg.c 2>medfiltss.err
$(OBJMDL)/digdelay.o : digdelay.c $(LINC)/nl_dsp.h $(NLH)
$(OBJMDL)/digdlyul.o : digdly.c $(LINC)/nl_dsp_ul.h $(LINC)/nl_dsp.h $(NLH)
	$(COMPILE.c) -Wc,-nm=digdlyul -fi=nl_dsp_ul.h digdly.c 2>digdlyul.err
$(OBJMDL)/digdlydbl.o : digdly.c $(LINC)/nl_dsp_dbl.h $(LINC)/nl_dsp.h $(NLH)
	$(COMPILE.c) -Wc,-nm=digdlydbl -fi=nl_dsp_dbl.h digdly.c 2>digdlydbl.err
$(OBJMDL)/runavg.o : runavg.c $(LINC)/nl_dsp_dbl.h $(LINC)/nl_dsp.h $(NLH)
$(OBJMDL)/rtgapi.o : rtgapi.c $(LINC)/rtgapi.h $(NLH)
$(OBJMDL)/kbdclt.o : kbdclt.c $(LINC)/oui.h $(LINC)/cmdctrl.h $(NLH)
$(OBJMDL)/gpib232.o : gpib232.c $(LINC)/gpib232.h $(NLH)
$(OBJMDL)/dtoe.o : dtoe.c $(LINC)/dtoe.h
$(OBJMDL)/ttclient.o : ttclient.c $(LINC)/ttdriver.h $(NLH)
$(OBJMDL)/cltsrvr.o : cltsrvr.c $(LINC)/cltsrvr.h $(NLH)
$(OBJMDL)/intserv.o : intserv.c $(LINC)/cltsrvr.h $(LINC)/intserv.h $(NLH)
$(OBJMDL)/ccquit.o : ccquit.c $(LINC)/cltsrvr.h $(LINC)/cmdctrl.h $(NLH)
$(OBJMDL)/idx64.o : idx64.c $(LINC)/cltsrvr.h $(LINC)/idx64.h $(NLH)
$(OBJMDL)/linefind.o : linefind.c $(LINC)/linefind.h $(NLH)
$(OBJMDL)/average.o : average.c $(LINC)/average.h
$(OBJMDL)/PBdef.o : PBdef.c $(LINC)/cltsrvr.h
$(OBJMDL)/dmmclient.o : dmmclient.c $(LINC)/dmmdrv.h $(NLH)
$(OBJMDL)/omsclient.o : omsclient.c $(LINC)/omsdrv.h $(LINC)/cltsrvr.h $(NLH)
$(OBJMDL)/cache.o : cache.c $(LINC)/da_cache.h $(NLH)
$(OBJMDL)/dccc.o : dccc.c $(LINC)/dccc.h $(NLH)
$(OBJMDL)/timetext.o : timetext.c $(LINC)/tmctime.h
$(OBJMDL)/memo.o : memo.c $(NLH)
$(OBJMDL)/getcon.o : getcon.c $(NLH)
$(OBJMDL)/runnum.o : runnum.c $(LINC)/runnum.h
$(OBJMDL)/runcat.o : runcat.c $(LINC)/runnum.h
$(OBJMDL)/runmol.o : runmol.c $(LINC)/runnum.h
$(OBJMDL)/runwrite.o : runwrite.c $(LINC)/runnum.h
$(OBJMDL)/drinit.o : drinit.c $(LINC)/runnum.h
$(OBJMDL)/mlf.o : mlf.c $(LINC)/mlf.h
$(OBJMDL)/analogic.o : analogic.c $(LINC)/analogic.h $(LINC)/cltsrvr.h
$(OBJMDL)/md5c.o : md5c.c md5.h
$(OBJMDL)/nav_a2n.o : nav_a2n.c navutil.h
$(OBJMDL)/nav_n2a.o : nav_n2a.c navutil.h
$(OBJMDL)/qcliclt.o : qcliclt.c qcliclt.h

RCSFILES=RCS/*
source.html : $(SRC) $(TOOL) $(RCSFILES)
	rcs2html nortlib
