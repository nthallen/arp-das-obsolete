include $(SRCDIR)/src.mk
SNAFUDIR=//2/usr/local/src/das/snafu
MODELDIR=OBJ.$(MODEL)
CFLAGS += -I //2/usr/local/src/das/snafu -I //2/usr/local/src/das/snafu/dr
#LDFLAGS = -l snfdr -l snfbtch -l ssp
LDFLAGS = -l snfdr  -l ssp
BASE = sps2step
HOMEDIR = $(SRCDIR)/drs/$(BASE)
DOCS = sps2step.exe
SRCS = sps2step.c stepout.c ind_wind.c
OBJS = sps2step.o stepout.o ind_wind.o \
	$(SNAFUDIR)/$(MODELDIR)/attribut.o \
	 $(SNAFUDIR)/$(MODELDIR)/check_ss.o \
	 $(SNAFUDIR)/$(MODELDIR)/curses.o \
	 $(SNAFUDIR)/$(MODELDIR)/refresh.o \
	 $(SNAFUDIR)/$(MODELDIR)/snmemory.o \
	 $(SNAFUDIR)/$(MODELDIR)/input_li.o \
	 $(SNAFUDIR)/$(MODELDIR)/mem_sal.o \
	 $(SNAFUDIR)/$(MODELDIR)/exitsnfu.o
LIBS =

include $(SRCDIR)/tgt.mk
