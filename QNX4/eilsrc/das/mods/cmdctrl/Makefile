# include definitions
include $(SRCDIR)/src.mk
SUBDIRS = 
HOMEDIR = $(DASDIR)/mods/cmdctrl
BASE = cmdctrl

# file list macros
TXTS += $(USEDIR)/msg.txt
LIBS = das

# include targets
include $(DASDIR)/tgt.mk

# add perms to all
all : perms

# dependencies
cmdctrl.o : cmdctrl.c cmdctrl.h $(INCDIR)/reboot.h
