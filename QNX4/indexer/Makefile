# Makefile for indexer
SRC=ixdiag.tmc ixdiag.fld ixdiag.cmd
SRC+=indexer.c ixdrv.h
TOOL=Makefile doit todo
SAVE=$(SRC) $(TOOL)
OBJ=ixdiagcol.c ixdiagcol.o ixdiagcol
OBJ+=ixdiagcmd.c ixdiagcmd.o ixdiagcmd
OBJ+=indexer.o indexer ixdiag.scr ixdiag.disp
OBJ+=ixdiagdisp.c ixdiagdisp.o ixdiagdisp
TEMP=*.bak *.err profile.dbg tm.dac ixdiag.cfg
PURGE=$(OBJ) $(TEMP)
MNC=idxr
CFLAGS=-g -ms -w4 -I /usr/local/include -l nortlib -L /usr/local/lib
CFLAGS+=-l subbus -l das_utils -l dbr_utils
indexer : indexer.o
indexer.o : indexer.c

ixdiagcol : ixdiagcol.o
ixdiagcol.o : ixdiagcol.c
ixdiagcol.c : ixdiag.tmc ixdiag.disp /usr/spade/inst.types
	tmc -c -s -D tm.dac -o ixdiagcol.c /usr/spade/inst.types ixdiag.tmc ixdiag.disp
ixdiag.disp : ixdiag.fld /usr/local/bin/fld2disp.awk
	awk -f /usr/local/bin/fld2disp.awk ixdiag.fld >ixdiag.disp

ixdiagcmd : ixdiagcmd.o
ixdiagcmd.o : ixdiagcmd.c
ixdiagcmd.c : ixdiag.cmd
	cd /usr/local/src/das/cmdgen; cmdgen <$(PWD)/ixdiag.cmd >$(PWD)/ixdiagcmd.c

ixdiagdisp : ixdiagdisp.o
ixdiagdisp.o : ixdiagdisp.c
ixdiagdisp.c : ixdiag.tmc ixdiag.disp /usr/spade/inst.types
	tmc -s -H ohptrs.h -o ixdiagdisp.c /usr/spade/inst.types ixdiag.tmc ixdiag.disp

include /usr/local/include/maint.mk
