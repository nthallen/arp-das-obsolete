include $(SRCDIR)/src.mk
SUBDIRS =
HOMEDIR = $(DASDIR)/dbr/lib/nlib
BASE = dbr
MODELS = s
MNC = dbrlib

# file list macros
TXTS = dc.txt dg.txt dgdacin.txt
USES = 
HDRS = dbr.h
SRCS = dg.c dc.c dg_options.c dc_options.c db.c dbr_info.c dgdacin.c
OBJS = $(MODEL)/dg.o $(MODEL)/dc.o $(MODEL)/dg_options.o $(MODEL)/dc_options.o $(MODEL)/db.o $(MODEL)/dbr_info.o $(MODEL)/dgdacin.o

include $(SRCDIR)/lib.mk

$(MODEL)/dg.o : dg.c dbr.h $(INCDIR)/globmsg.h
$(MODEL)/dg_options.o : dg_options.c dbr.h
$(MODEL)/dc.o : dc.c dbr.h $(INCDIR)/globmsg.h
$(MODEL)/dc_options.o : dc_options.c dbr.h
$(MODEL)/db.o : db.c dbr.h $(INCDIR)/globmsg.h
$(MODEL)/dbr_info.o : dbr_info.c dbr.h
$(MODEL)/dgdacin.o : dgdacin.c dbr.h
