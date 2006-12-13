# include definitions
include $(SRCDIR)/src.mk
SUBDIRS = 
HOMEDIR = $(DASDIR)/dfs/mods/dbs/pbfr
BASE = pbfr

# file list macros
TXTS += $(USEDIR)/dc.txt $(USEDIR)/msg.txt $(USEDIR)/cc.txt
SRCS += pbfr_dc.c pbfr_dg.c
LIBS = dfs_mod dfs das

# include targets
include $(DASDIR)/tgt.mk

# dependencies
pbfr_dc.o: pbfr_dc.c $(HDRS)
pbfr_dg.o: pbfr_dg.c $(HDRS)
pbfr.o : pbfr.c $(HDRS)
