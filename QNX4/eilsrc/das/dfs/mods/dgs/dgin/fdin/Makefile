# include definitions
include $(SRCDIR)/src.mk
SUBDIRS =
HOMEDIR = $(DASDIR)/dbr/mods/dgs/dgin/fdin/fdin
BASE = fdin
CFLAGS += -g

# file list macros
TXTS += $(USEDIR)/dg.txt $(USEDIR)/das.txt $(USEDIR)/msg.txt
LIBS = dbr das
SRCS += fdin_dg.c

# include targets
include $(DASDIR)/tgt.mk

fdin.o: fdin.c
fdin_dg.o : fdin_dg.c

