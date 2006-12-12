include $(SRCDIR)/src.mk
SUBDIRS =
HOMEDIR = $(DASDIR)/dfs/lib/plib
BASE = dfs
MODELS = s
MNC = dfslib

# file list macros
TXTS = dc.txt dg.txt dgdacin.txt
USES = 
HDRS = dfs.h dc.h dg.h gw.h
SRCS = dg.c dc.c dg_options.c dc_options.c dfs_info.c dgdacin.c gw.c dfs.c
OBJS = $(MODEL)/dg.o $(MODEL)/dc.o $(MODEL)/dg_options.o $(MODEL)/dc_options.o $(MODEL)/dfs_info.o $(MODEL)/dgdacin.o $(MODEL)/gw.o $(MODEL)/dfs.o

include $(SRCDIR)/lib.mk

$(MODEL)/dg.o : dg.c dg.h $(INCDIR)/globmsg.h dfs.h
$(MODEL)/dg_options.o : dg_options.c dfs.h
$(MODEL)/dc.o : dc.c dc.h $(INCDIR)/globmsg.h dfs.h
$(MODEL)/gw.o : gw.c gw.h $(INCDIR)/globmsg.h dfs.h
$(MODEL)/dc_options.o : dc_options.c dfs.h
$(MODEL)/dfs_info.o : dfs_info.c dfs.h
$(MODEL)/dfs.o : dfs.c dfs.h
$(MODEL)/dgdacin.o : dgdacin.c dfs.h
