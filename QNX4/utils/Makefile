SRC=namewait.c lister.awk tee.c tabcvt.c ring.c rdoshold.c modtime.c
SRC+=unsu.c parent.c parent.h parent.oui getcon.c getcon.h getcon.oui
SRC+=mount_pt.c ftime.c mtime.c
TOOL=Makefile
OBJ=namewait.o ring.o rdoshold.o modtime.o unsu.o
OBJ+=parent.o parentoui.c parentoui.o
OBJ+=getcon.o getconoui.c getconoui.o
OBJ+=mount_pt.o
OBJ+=ftime.o mtime.o
TEMP=*.err

HOMEDIR=/usr/local/src/utils
MNC=utls
SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ) $(TEMP)
TARGET=namewait tee tabcvt ring rdoshold modtime unsu parent getcon
TARGET+=mount_pt ftime mtime

MODEL=s
LBIN=/usr/local/bin
CFLAGS=-m$(MODEL) -3r -g -w4
LDFLAGS=$(BETALIB) -l eillib
LINK.priv=$(LINK.c) $(LDFLAGS) -T 1 -o $@
LINK.norm=$(LINK.c) $(LDFLAGS) -o $@

all : namewait ring unsu parent getcon mount_pt
namewait : namewait.o
	$(LINK.norm) namewait.o
	usemsg namewait namewait.c
	ln -f namewait $(LBIN)/namewait
ring : ring.o
	$(LINK.priv) ring.o
	ln -f ring $(LBIN)/ring
	promote ring
rdoshold : rdoshold.o
	$(LINK.norm) rdoshold.o
	usemsg rdoshold rdoshold.c
	ln -f rdoshold $(LBIN)/rdoshold
unsu : unsu.o
	$(LINK.norm) unsu.o
	usemsg unsu unsu.c
	ln -f unsu $(LBIN)/unsu
parent : parent.o parentoui.o
	$(LINK.norm) parent.o parentoui.o
	usemsg parent parentoui.c
	ln -f parent $(LBIN)/parent
parent.o : parent.c parent.h
parentoui.o : parentoui.c parent.h
parentoui.c : parent.oui
	oui -o parentoui.c parent.oui

getcon : getcon.o getconoui.o
	$(LINK.norm) getcon.o getconoui.o
	usemsg getcon getconoui.c
	ln -f getcon $(LBIN)/getcon
getconoui.o : getconoui.c
getconoui.c : getcon.oui
	oui -o getconoui.c getcon.oui
mount_pt : mount_pt.c
