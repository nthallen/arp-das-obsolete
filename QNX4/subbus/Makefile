# Makefile for subbus
# $Log$
#
MODEL=s
LINC=/usr/local/include
CFLAGS=-g -m$(MODEL) -2 -M -w4 -I $(LINC)
LDFLAGS=-b -L /usr/local/lib
LINK.priv=$(LINK.c) $(LDFLAGS) -T 1 -o $@

SRC=submain.c subbus.asm sb_internal.h sublib.mac subbus_sc.o
SRC+=subbus_ps.o subbus_pi.o sb_ram.c nvram.c nvram.h
SRC+=subbus_104.o
OBJ=submain.o sb_ram.o
TEMP=*.err *.map
TOOL=Makefile lmkfile
DOC=todo

HOMEDIR=/usr/local/src/das/subbus
MNC=sbus
SOURCE=$(SRC) $(TOOL) $(DOC)
OBJECT=$(OBJ) $(TEMP)
TARGET=subbus_104 subbus_sc subbus_ps subbus_pi subbus*.[124]0
SUBDIRS=lib sblib

DOSFILES=subbus.asm sublib.mac
DOSDIR=/dos/c/reslib/qnx4

all : $(TARGET)

subbus_104 : submain.o subbus_104.o sb_ram.o
	$(LINK.priv) submain.o subbus_104.o sb_ram.o
	usemsg subbus_104 submain.c
	promote subbus_104
subbus_sc : submain.o subbus_sc.o sb_ram.o
	$(LINK.priv) submain.o subbus_sc.o sb_ram.o
	usemsg subbus_sc submain.c
	promote subbus_sc
subbus_ps : submain.o subbus_ps.o sb_ram.o
	$(LINK.priv) submain.o subbus_ps.o sb_ram.o
	usemsg subbus_ps submain.c
	promote subbus_ps
subbus_pi : submain.o subbus_pi.o sb_ram.o
	$(LINK.priv) submain.o subbus_pi.o sb_ram.o
	usemsg subbus_pi submain.c
	promote subbus_pi
submain.o : submain.c $(LINC)/subbus.h sb_internal.h
sb_ram.o : sb_ram.c $(LINC)/subbus.h sb_internal.h nvram.h
subbus_104.o : subbus.asm sublib.mac
	rundos -h lasm -a -DLIBRARY_SUB=4 -osbus_104.obj subbus.asm
	mv sbus_104.obj subbus_104.o
subbus_sc.o : subbus.asm sublib.mac
	rundos -h lasm -a -DLIBRARY_SUB=3 -osbus_sc.obj subbus.asm
	mv sbus_sc.obj subbus_sc.o
subbus_ps.o : subbus.asm sublib.mac
	rundos -h lasm -a -DLIBRARY_SUB=2 -osbus_ps.obj subbus.asm
	mv sbus_ps.obj subbus_ps.o
subbus_pi.o : subbus.asm sublib.mac
	rundos -h lasm -a -DLIBRARY_SUB=1 -osbus_pi.obj subbus.asm
	mv sbus_pi.obj subbus_pi.o

nvram : nvram.o
	$(LINK.priv) nvram.o -l subbus -l nortlib
	usemsg nvram nvram.c
	promote nvram
nvram.o : nvram.c $(LINC)/subbus.h nvram.h
