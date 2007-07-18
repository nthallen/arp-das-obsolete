# include definitions
include $(SRCDIR)/src.mk
SUBDIRS =
HOMEDIR = $(DASDIR)/dfs/mods/dgs/rdr
BASE = rdr

# file list macros
TXTS += $(USEDIR)/dg.txt $(USEDIR)/msg.txt $(USEDIR)/dgdacin.txt
SRCS += rdr_dg.c
LIBS = dbr eillib dfs_mod

# include targets
include $(DASDIR)/tgt.mk
CFLAGS += -g
LDFLAGS += -ldbr -ldfs_mod
