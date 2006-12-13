# include definitions
include $(SRCDIR)/src.mk
SUBDIRS = 
HOMEDIR = $(DASDIR)/dfs/mods/dbs/bfr/pbfr
BASE = bfr

# file list macros
TXTS += $(USEDIR)/dg.txt $(USEDIR)/dc.txt $(USEDIR)/msg.txt $(USEDIR)/cc.txt
SRCS += bfr_dc.c bfr_dg.c
LIBS = dfs_mod dfs das eillib

# include targets
include $(DASDIR)/tgt.mk

# dependencies
bfr_dc.o: bfr_dc.c $(HDRS)
bfr_dg.o: bfr_dg.c $(HDRS)
bfr.o : bfr.c $(HDRS)
