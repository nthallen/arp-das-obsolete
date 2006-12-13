# include definitions
include $(SRCDIR)/src.mk
SUBDIRS = 
HOMEDIR = $(DASDIR)/dfs/mods/dbs/bfr/pbfr/gw
BASE = gw

# file list macros
TXTS += $(USEDIR)/dg.txt $(USEDIR)/dc.txt $(USEDIR)/msg.txt $(USEDIR)/cc.txt
SRCS += gw_dc_dg.c
LIBS =  dfs das eillib

# include targets
include $(DASDIR)/tgt.mk

# dependencies
gw_dc_dg.o: gw_dc_dg.c
gw.o : gw.c
