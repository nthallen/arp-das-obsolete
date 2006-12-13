# include definitions
include $(SRCDIR)/src.mk
SUBDIRS =
HOMEDIR = $(DASDIR)/dbr/mods/dgs/msgin
BASE = msgin

# file list macros
TXTS += $(USEDIR)/dg.txt $(USEDIR)/das.txt $(USEDIR)/msg.txt
LIBS = dbr das
SRCS += msgin_dg.c

# include targets
include $(DASDIR)/tgt.mk

msgin.o: msgin.c
msgin_dg.o : msgin_dg.c

