include $(SRCDIR)/src.mk
SNAFUDIR=//2/usr/local/src/das/snafu
MODEL=3r
MODELDIR=OBJ.$(MODEL)
# CFLAGS += -I //2/usr/local/src/das/snafu -I //2/usr/local/src/das/snafu/dr -N8k
CFLAGS += -g -I //2/usr/local/src/das/snafu -I //2/usr/local/src/das/snafu/dr
#LDFLAGS = -l snfdr -l snfbtch -l ssp
LDFLAGS = -l snfdr -l ssp -l termlib
BASE = step2sps
HOMEDIR = $(SRCDIR)/drs/$(BASE)
DOCS = step2sps.exe
SRCS = step2sps.c stepin.c ind_wind.c
OBJS = step2sps.o stepin.o ind_wind.o \
	$(SNAFUDIR)/$(MODELDIR)/attribut.o \
	 $(SNAFUDIR)/$(MODELDIR)/check_ss.o \
	 $(SNAFUDIR)/$(MODELDIR)/curses.o \
	 $(SNAFUDIR)/$(MODELDIR)/refresh.o \
	 $(SNAFUDIR)/$(MODELDIR)/snmemory.o \
	 $(SNAFUDIR)/$(MODELDIR)/input_li.o \
	 $(SNAFUDIR)/$(MODELDIR)/mem_sal.o
#	 $(SNAFUDIR)/$(MODELDIR)/exitsnfu.o
LIBS =

include $(SRCDIR)/tgt.mk
