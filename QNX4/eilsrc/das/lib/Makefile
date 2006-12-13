include $(SRCDIR)/src.mk
BASE = das
HOMEDIR = $(DASDIR)/lib
SUBDIRS = 
MNC = daslib

# file list macros
DOCS =
TXTS += cc.txt
USES = 
HDRS += cc.h
SRCS = cc.c cc_options.c
OBJS = $(MODEL)/cc.o $(MODEL)/cc_options.o

include $(SRCDIR)/lib.mk

$(MODEL)/cc.o: cc.c cc.h $(INCDIR)/symname.h $(INCDIR)/msg.h $(INCDIR)/cmdctrl.h $(INCDIR)/globmsg.h
$(MODEL)/cc_options.o : cc_options.c cc.h $(INCDIR)/msg.h $(INCDIR)/cmdctrl.h
