MNC=isrv
HOMEDIR=/usr/local/src/das/intserv
TOOL=todo Makefile
SRC=intserv.c expint.c hint.c handler.c isrv.oui
SRC+=internal.h
SOURCE=$(TOOL) $(SRC)
OBJ=intserv.o expint.o hint.o handler.o isrvoui.o
TMP=*.err isrvoui.c
OBJECT=$(OBJ) $(TMP)
TARGET=intserv

LINC=/usr/local/include

CFLAGS=-ms -2 -g -w4
LDFLAGS=-b -l eillib

intserv : $(OBJ)
	$(LINK.c) -o intserv -T1 $(OBJ) $(LDFLAGS)
	usemsg intserv isrvoui.c
	promote intserv
intserv.o : intserv.c $(LINC)/intserv.h internal.h
expint.o : expint.c $(LINC)/intserv.h internal.h
hint.o : hint.c $(LINC)/intserv.h internal.h
handler.o : handler.c $(LINC)/intserv.h internal.h
	$(COMPILE.c) -zu -Wc,-s handler.c
isrvoui.o : isrvoui.c $(LINC)/intserv.h internal.h
isrvoui.c : isrv.oui
	oui -o isrvoui.c isrv.oui
