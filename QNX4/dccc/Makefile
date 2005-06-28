# include definitions
#include $(SRCDIR)/src.mk
SUBDIRS = 
HOMEDIR = $(DASDIR)/mods/dccc
BASE = dccc

# file list macros
TXTS += $(USEDIR)/msg.txt $(USEDIR)/cc.txt
SRCS = $(BASE).c dc_read.c
HDRS += disc_cmd.h
LIBS = das subbus eillib
DOCS = dccc_cmd.txt
LDFLAGS += -T0 -l das -l subbus -l eillib
OBJS = $(SRCS:.c=.o)

LINK.priv=/bin/rm -f $@; $(LINK.c) $(LIBFLAGS) -T 1 -o $@ $(LDFLAGS)
CFLAGS=-ms -2 -w4

# include targets
# include $(DASDIR)/tgt.mk

# add perms to all
#all: perms

# dependencies
$(BASE) : $(OBJS) $(BASE).use
	$(LINK.priv) $(OBJS)
	usemsg $(BASE) $(BASE).use
	promote $(BASE)
dc_read.o: dc_read.c disc_cmd.h
dccc.o: dccc.c $(HDRS)
$(BASE).use : $(BASE).txt $(TXTS)
	cat $(BASE).txt $(TXTS) >$(BASE).use
