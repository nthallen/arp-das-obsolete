# include definitions
include $(SRCDIR)/src.mk
SUBDIRS =
HOMEDIR = $(DASDIR)/dfs/mods/lib
BASE = dfs_mod
MODELS = s
MNC = dfsmodlib

# file list macros
TXTS = 
DOCS =
USES = 
HDRS = file.h frame.h filename.h dfs_mod.h rollover.h
SRCS = file.c frame.c filename.c

# include targets
include $(SRCDIR)/lib.mk

file.o : file.c file.h rollover.h
frame.o : frame.c frame.h
filename.o : filename.c filename.h
stream.o : stream.c stream.h
