include $(SRCDIR)/src.mk
BASE = scrdes
MODEL = l
SUBDIRS =
HOMEDIR = $(SRCDIR)/graphics/$(BASE)
SRCS = scrdes.c draw_lin.c draw.c help.c
HDRS = bufglobs.h draw_lin.h etc.h fldglobs.h globals.h help.h mnuglobs.h \
	 pattr.h txtglobs.h version.h
LIBS = curses curses_utils lat
DOCS = ascii.dos ascii.qnx scrdes0.hlp scrdes1.hlp scrdes2.hlp \
	scrdes3.hlp scrdes4.hlp scrdes5.hlp scrdes6.hlp \
	mono.cfg color.cfg scrdes.cfg

LDFLAGS += -ltermlib
include $(SRCDIR)/tgt.mk

scrdes.o : scrdes.c globals.h etc.h pattr.h help.h version.h draw_lin.h
help.o : help.c help.h mnuglobs.h
draw_lin.o : draw_lin.c draw_lin.h globals.h
draw.o : draw.c globals.h etc.h fldglobs.h bufglobs.h txtglobs.h

