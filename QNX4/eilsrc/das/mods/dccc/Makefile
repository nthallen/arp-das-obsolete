# include definitions
include $(SRCDIR)/src.mk
SUBDIRS = 
HOMEDIR = $(DASDIR)/mods/dccc
BASE = dccc

# file list macros
TXTS += $(USEDIR)/msg.txt $(USEDIR)/cc.txt
SRCS += dc_read.c
HDRS += disc_cmd.h
LIBS = das subbus eillib
DOCS = dccc_cmd.txt
LDFLAGS += -T0 -l das -l subbus -l eillib

# include targets
include $(DASDIR)/tgt.mk

# add perms to all
all: perms

# dependencies
dc_read.o: dc_read.c disc_cmd.h
dccc.o: dccc.c $(HDRS)
