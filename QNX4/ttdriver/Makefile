MNC=ttdrv
HOMEDIR=/usr/local/src/das/ttdriver
SUBDIRS=regress ttdrgs

SRC=atod.c digital.c gc.c dac.c ttdriver.c timeout.c
SRC+=ttdriver.oui
SRC+=ttintern.h
TOOL=Makefile todo
OBJ=atod.o digital.o gc.o dac.o ttdriver.o ttoui.o timeout.o
INT=ttoui.c
TGT=ttdriver

SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ) $(INT) *.err
TARGET=$(TGT)

LLIB=/usr/local/lib
LINC=/usr/local/include

MODEL=s
CFLAGS=-m$(MODEL) -2 -g -w4
LDFLAGS=-b -l eillib

ttdriver : $(OBJ)
	$(LINK.c) -o ttdriver -T1 $(OBJ) $(LDFLAGS)
	usemsg ttdriver ttoui.c
	promote ttdriver
atod.o : atod.c $(LINC)/ttdriver.h ttintern.h
dac.o : dac.c $(LINC)/ttdriver.h ttintern.h
digital.o : digital.c $(LINC)/ttdriver.h ttintern.h
gc.o : gc.c $(LINC)/ttdriver.h ttintern.h $(LINC)/nortlib.h
ttdriver.o : ttdriver.c $(LINC)/ttdriver.h ttintern.h $(LINC)/nortlib.h
ttdriver.o : $(LINC)/oui.h $(LINC)/globmsg.h
ttoui.o : ttoui.c $(LINC)/oui.h $(LINC)/msg.h
ttoui.c : ttdriver.oui
	oui -o ttoui.c ttdriver.oui
timeout.o : timeout.c $(LINC)/ttdriver.h ttintern.h $(LINC)/nortlib.h
