# makefile for /usr/local/src/utils
# $Log$
# Revision 1.2  1996/03/21  14:25:59  nort
# *** empty log message ***
#
# Revision 1.1  1993/09/15  19:29:30  nort
# Initial revision
#
SRC=ringtap.c startdbr.c pick_file.c pick_file.oui
TOOL=Makefile
OBJ=ringtap.o startdbr.o sdbr.o pick_file.o ringtap.use
OBJ+=pfoui.c pfoui.o
TEMP=*.err

HOMEDIR=/usr/local/src/das/ringtap
MNC=rgtp
SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ) $(TEMP)
TARGET=ringtap startdbr pick_file
USE=/usr/local/include/use

MODEL=s
CFLAGS=-m$(MODEL) -2 -g -w4
LDFLAGS=-l dbr -l das -l eillib
LINK.priv=$(LINK.c) $(LDFLAGS) -T 1 -o $@
LINK.norm=$(LINK.c) $(LDFLAGS) -o $@

all : $(TARGET)
ringtap : ringtap.o ringtap.use
	$(LINK.norm) ringtap.o
	usemsg ringtap ringtap.use
ringtap.o : ringtap.c
ringtap.use : $(USE)/msg.txt $(USE)/dc.txt
	{ echo "%C	[options] &"; \
	cat $(USE)/msg.txt $(USE)/dc.txt | sort; } >ringtap.use
startdbr : startdbr.o
	$(LINK.norm) startdbr.o
	usemsg startdbr startdbr.c
startdbr.o : startdbr.c
pick_file : pick_file.o pfoui.o
	$(LINK.priv) pick_file.o pfoui.o
	usemsg pick_file pfoui.c
	ln -f pick_file /usr/local/bin/pick_file
	promote pick_file
pick_file.o : pick_file.c
pfoui.o : pfoui.c
pfoui.c : pick_file.oui
	oui -o pfoui.c pick_file.oui
