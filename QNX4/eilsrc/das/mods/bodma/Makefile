# include definitions
# include $(SRCDIR)/src.mk
SUBDIRS = 
HOMEDIR = $(DASDIR)/mods/bodma
BASE = bodma

# file list macros
# TXTS += $(USEDIR)/msg.txt $(USEDIR)/cc.txt
# LIBS = das eillib
# DOCS = 
# LDFLAGS += -T1 -l lib/seq36.lib -Wl,"op nocache" -Wl,"op heapsize=2m"
LDFLAGS += -T1 -l lib/seq36.lib

# The SMALL_MEM pre-compiler flag if for the Seq36 NDMA board and 1cm MIR.
# Otherwise, it is Seq36 NDMA board and 1cm NIR ( which we don't use )

MYFLAGS = -Wc,-bm -WC,-bm -3 -mf -zq -WC,-xs -ot -D SMALL_MEM
# MYFLAGS = -Wc,-bm -WC,-bm -3 -mf -zq -WC,-xs -g
# MYFLAGS = -Wc,-bm -WC,-bm -ot -3 -ms -zq -WC,-xs -g -WC,-r

# include targets
# include $(DASDIR)/tgt.mk

# add perms to all
# all : perms
#	usemsg bodma bodma.use

# dependencies
bodma: bodma.cpp lib/seq36.lib
	cc $(MYFLAGS) $(LDFLAGS) -o$@ bodma.cpp -l eillib -l das -l dfs_mod
	usemsg bodma bodma.use
	promote bodma
