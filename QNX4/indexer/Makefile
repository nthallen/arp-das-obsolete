# Makefile for indexer
# $Log$
SRC+=indexer.c ixdrv.h indexer.txt
TOOL=Makefile doit todo
OBJ+=indexer.o indexer.use
TEMP=*.bak *.err profile.dbg

SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ) $(TEMP)
TARGET=indexer
HOMEDIR=/usr/local/src/das/indexer
MNC=idxr

CFLAGS=-g -ms -2 -w4 -I /usr/local/include
LDFLAGS=-l nortlib -l subbus -l das -l dbr -l eillib -L /usr/local/lib
LINK.priv=$(LINK.c) $(LDFLAGS) -T 1 -o $@
LINK.norm=$(LINK.c) $(LDFLAGS) -o $@
LINK.ext=$(LINK.c) $(LDFLAGS) -o $@ -l ssp -l memlib
USE=/usr/local/include/use

indexer : indexer.o indexer.use
	$(LINK.priv) indexer.o
	usemsg indexer indexer.use
	ln -f indexer /usr/local/bin/indexer
	promote indexer
indexer.o : indexer.c ixdrv.h
indexer.use : $(USE)/msg.txt $(USE)/cc.txt indexer.txt
	{ echo "%C	[options] &"; \
	cat $(USE)/msg.txt $(USE)/cc.txt indexer.txt | sort; } >indexer.use
