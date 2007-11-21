# makefile for /usr/local/include
# $Id$
# $Log$
# Revision 1.13  1994/11/22  14:56:18  nort
# *** empty log message ***
#
# Revision 1.12  1993/02/15  18:42:40  nort
# After Eil's library reorganization
#
NORTS+=subbus.h diaglib.h snfbatch.h
NORTS+=memlib.h ssp.h snafuerr.h nortlib.h collect.h indexer.h
NORTS+=timerbd.h seq_ss.h oui.h rtgapi.h
NORTS+=nl_dsp.h nl_dsp_ul.h nl_dsp_dbl.h
NORTS+=cmdalgo.h average.h nl_cons.h tmctime.h
NORTS+=solfmt.h rational.h compiler.h ll_of_str.h
NORTS+=gpib232.h dtoe.h ttdriver.h idx64.h cltsrvr.h
NORTS+=intserv.h tma.h linefind.h da_cache.h dmmdrv.h
NORTS+=omsdrv.h runnum.h mlf.h analogic.h md5.h
NORTSX=gdbm.h dbm.h ndbm.h sdbm.h navutil.h
# From gd-1.7.3.tar.gz, libpng-1.0.5.tar.gz, zlib-1.1.3.tar.gz
GDX=gd.h gd_io.h gdcache.h gdfontg.h gdfontl.h gdfontmb.h
GDX+=gdfonts.h gdfontt.h
GDX+=pngconf.h png.h
GDX+=zlib.h zconf.h
EILS=dac.h dbr.h globmsg.h file.h
EILS+=eillib.h das.h dbr_mod.h
EILS+=msg.h sounds.h symname.h timing.h usage.h curses.h company.h
EILS+=lat.h cmdctrl.h break.h cc.h diaglib.h
EILS+=soldrv.h reboot.h nav.h
EILS+=serial.h spawn.h dccc.h memo.h scdc.h
EILS+=filename.h frame.h sights.h vector.h
EILS+=beeps.h reply.h get_priv.h rollover.h
EILS+=stream.h pif.h sound_esc.h serin.h port_types.h topaz.h
EILS+=sigs.h bodma.h idler.h
EILS+=fd.h mx.h qnx_ipc.h
EILSX=syscon.h curses_utils.h menu.h attr.h fld.h scr.h
EILSX+=popup.h tabs.h cfg.h hdr.h txt.h

TOOL=Makefile

SOURCE=$(NORTS) $(NORTSX) $(EILS) $(TOOL)
STANDARD=$(EILSX) $(GDX) eliport.h ltdl.h
RCS=$(NORTS) $(NORTSX) $(TOOL)
HOMEDIR=/usr/local/include
MNC=linc
SUBDIRS=use    iuse
SUBDIRS+=oui ouilib

TGTNODE=//1
DEST=$(TGTNODE)$(HOMEDIR)
distribution :
	cp -vni $(NORTS) $(DEST); :
