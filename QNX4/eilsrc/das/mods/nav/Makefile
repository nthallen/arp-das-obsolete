# include definitions
include $(SRCDIR)/src.mk
SUBDIRS =
HOMEDIR = $(DASDIR)/mods/nav
BASE = nav

# file list macros
LIBS = das
TXTS += $(USEDIR)/das.txt $(USEDIR)/msg.txt $(USEDIR)/serial.txt
DOCS = navsim5.com navtest.c nav.tmc

# include targets
include $(DASDIR)/tgt.mk

# add perms to all
all : perms

nav.o: nav.c nav.tmc
