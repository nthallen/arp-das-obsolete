# to compile card:
#	make BASE=card
#

include $(SRCDIR)/src.mk
BASE = syscon
SUBDIRS =
HOMEDIR = $(SRCDIR)/diags/suite
TXTS =
SRCS = diagutil.c suite.c syscon.c
LIBS = curses subbus lat curses_utils
HDRS = attribut.h define.h diagmenu.h scdiag.h sic.h syscon.h
DOCS = diags.bak syscon.hlp scdiag.cfg
CFLAGS += -D$(BASE)

include $(SRCDIR)/tgt.mk
suite.o : suite.c syscon.h attribut.h scdiag.h 
diagutil.o: diagutil.c define.h
