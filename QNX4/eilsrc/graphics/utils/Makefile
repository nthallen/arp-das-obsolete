include $(SRCDIR)/src.mk
BASE = curses_utils
MNC = grutils
SUBDIRS =
# CFLAGS += -g
HOMEDIR = $(SRCDIR)/graphics/utils
TXTS =
HDRS += menu.h attr.h fld.h scr.h popup.h tabs.h cfg.h hdr.h txt.h
DOCS = menu.doc
USES =
SRCS = menu.c attr.c scr.c fld.c cfg.c popup.c tabs.c txt.c
OBJS =$(MODEL)/menu.o $(MODEL)/attr.o $(MODEL)/scr.o $(MODEL)/fld.o \
	$(MODEL)/cfg.o $(MODEL)/popup.o $(MODEL)/tabs.o $(MODEL)/txt.o

include $(SRCDIR)/lib.mk

$(MODEL)/menu.o : menu.c menu.h
$(MODEL)/attr.o : attr.c attr.h
$(MODEL)/scr.o  : scr.c scr.h hdr.h
$(MODEL)/fld.o  : fld.c fld.h hdr.h
$(MODEL)/cfg.o  : cfg.c cfg.h
$(MODEL)/popup.o : popup.c popup.h
$(MODEL)/tabs.o : tabs.c tabs.h
$(MODEL)/txt.o : txt.c txt.h
