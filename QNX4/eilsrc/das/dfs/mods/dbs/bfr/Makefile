# include definitions
include $(SRCDIR)/src.mk
SUBDIRS = 
HOMEDIR = $(DASDIR)/dfs/mods/dbs/bfr
BASE = bfr

# file list macros
TXTS += $(USEDIR)/dc.txt $(USEDIR)/msg.txt $(USEDIR)/cc.txt
SRCS += bfr_dc.c bfr_db.c
HDRS += bfr_vars.h
LIBS = dbr das

# include targets
include $(DASDIR)/tgt.mk

# dependencies
bfr_db.o: bfr_db.c $(HDRS)
bfr_dc.o: bfr_dc.c $(HDRS)
bfr.o : bfr.c $(HDRS)
