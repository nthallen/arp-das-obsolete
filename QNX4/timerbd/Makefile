# Makefile for timerbd
# $Log$
# Revision 1.2  1995/06/01  01:16:33  nort
# *** empty log message ***
#
#
SRC+=tmrdrvr.h timerbd.c picinit.c tmrhw.c handler.c timerbd.txt
SRC+=timerbd.oui
TOOL=Makefile todo
OBJ=timerbd.o tmrhw.o picinit.o handler.o timerbdoui.o
TEMP=*.err timerbdoui.c

LINC=/usr/local/include
CFLAGS=-g -ms -2 -w4
LDFLAGS=-b -l das -l eillib -T 1

SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ) $(QOBJ) $(TEMP)
TARGET=timerbd
MNC=trbd
HOMEDIR=/usr/local/src/das/timerbd

timerbd : $(OBJ)
	$(LINK.c) -o timerbd $(OBJ) $(LDFLAGS)
	usemsg timerbd timerbdoui.c
	ln -f timerbd /usr/local/bin/timerbd
	promote timerbd
timerbd.o : timerbd.c $(LINC)/timerbd.h tmrdrvr.h
tmrhw.o : tmrhw.c $(LINC)/timerbd.h tmrdrvr.h
picinit.o : picinit.c tmrdrvr.h
handler.o : handler.c $(LINC)/timerbd.h tmrdrvr.h
	$(COMPILE.c) -Wc,-s handler.c
timerbdoui.o : timerbdoui.c tmrdrvr.h
timerbdoui.c : timerbd.oui
	oui -o timerbdoui.c timerbd.oui
