# include definitions
include $(SRCDIR)/src.mk
SUBDIRS =
HOMEDIR = $(DASDIR)/dbr/mods/dgs/serin
BASE = serin

# file list macros
TXTS += $(USEDIR)/dg.txt $(USEDIR)/das.txt $(USEDIR)/eillib.txt
SRCS += serin_dg.c rational.c
LIBS = dbr

# include targets
include $(DASDIR)/tgt.mk

serin_dg.o: serin_dg.c serin.h
serin.o: serin.c serin.h

