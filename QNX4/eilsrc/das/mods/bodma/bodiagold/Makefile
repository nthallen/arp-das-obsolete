# include definitions
# include $(SRCDIR)/src.mk
SUBDIRS = 
HOMEDIR = $(DASDIR)/mods/bodma/bodiag
BASE = bodiag

# file list macros
# TXTS += $(USEDIR)/msg.txt
# LIBS = das eillib
DOCS = 
LDFLAGS += -T1 -l ../lib/seq36.lib

MYFLAGS = -zt -g -2 -ml -zq -WC,-xs

# include targets
# include $(DASDIR)/tgt.mk

# add perms to all
# all : perms

# dependencies
bodiag: bodiag.cpp
	cc $(MYFLAGS) $(LDFLAGS) -o$@ bodiag.cpp -l eillib
	usemsg bodiag bodiag.use
	promote bodiag










