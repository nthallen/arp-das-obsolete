# makefile for /usr/local/include
# $Id$
# $Log$
# Revision 1.12  1993/02/15  18:42:40  nort
# After Eil's library reorganization
#
NORTS+=subbus.h diaglib.h snfbatch.h
NORTS+=memlib.h ssp.h snafuerr.h nortlib.h collect.h indexer.h
NORTS+=timerbd.h nl_dsp.h seq_ss.h oui.h
NORTS+=cmdalgo.h nl_cons.h tmctime.h
NORTS+=solfmt.h rational.h compiler.h ll_of_str.h
EILS=dac.h dbr.h globmsg.h file.h
EILS+=eillib.h das.h dbr_mod.h
EILS+=msg.h sounds.h symname.h timing.h usage.h curses.h company.h
EILS+=lat.h cmdctrl.h break.h cc.h diaglib.h
EILS+=soldrv.h reboot.h nav.h
EILS+=serial.h spawn.h dccc.h memo.h scdc.h
EILS+=filename.h frame.h sights.h vector.h
EILS+=beeps.h reply.h get_priv.h rollover.h
EILS+=stream.h pif.h
TOOL=Makefile

SOURCE=$(NORTS) $(EILS) $(TOOL)
RCS=$(NORTS) $(TOOL)
HOMEDIR=/usr/local/include
MNC=linc
SUBDIRS=use    iuse
SUBDIRS+=oui ouilib

TGTNODE=//1
DEST=$(TGTNODE)$(HOMEDIR)
distribution :
	cp -ni $(NORTS) $(DEST); :
#	cd $(DEST); cp -ni $(EILS) $(HOMEDIR); :
