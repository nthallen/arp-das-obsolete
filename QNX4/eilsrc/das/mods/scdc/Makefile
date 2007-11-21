# include definitions
include $(SRCDIR)/src.mk
SUBDIRS = 
HOMEDIR = $(DASDIR)/mods/scdc
BASE = scdc

# file list macros
TXTS += $(USEDIR)/msg.txt $(USEDIR)/cc.txt
LIBS = eillib das

# include targets
include $(DASDIR)/tgt.mk

# dependencies
scdc.o: scdc.c $(HDRS) $(INCDIR)/globmsg.h $(INCDIR)/dccc.h
