include $(SRCDIR)/src.mk
SNAFUDIR=//2/usr/local/src/das/snafu
MODEL=l
MODELDIR=OBJ.$(MODEL)
#MODELDIR=$(MODEL)9.5
CFLAGS += -I //2/usr/local/src/das/snafu -I //2/usr/local/src/das/snafu/dr -N8k
#LDFLAGS = -l snfdr -l snfbtch -l ssp
LDFLAGS = -l snfdr -l ssp
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
	 $(SNAFUDIR)/$(MODELDIR)/mem_sal.o \
	 $(SNAFUDIR)/$(MODELDIR)/exitsnfu.o
LIBS =

include $(SRCDIR)/tgt.mk
