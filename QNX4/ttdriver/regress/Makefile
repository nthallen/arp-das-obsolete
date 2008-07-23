MNC=ttdrgs
HOMEDIR=/usr/local/src/das/ttdriver/regress
SRC=testatod.c
SRC+=testgc.c
SRC+=readgc.c
SRC+=readgc1.c
SRC+=benchatod.c

TOOL=Makefile

OBJ=testatod.o
OBJ+=testgc.o
OBJ+=readgc.o
OBJ+=readgc1.o
OBJ+=benchatod.o

TGT=testatod testgc readgc readgc1 benchatod

SOURCE=$(TOOL) $(SRC)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)

LLIB=/usr/local/lib
LINC=/usr/local/include

MODEL=s
CFLAGS=-m$(MODEL) -2 -g -w4
LDFLAGS=-b -l eillib

all : $(TGT)

testgc : testgc.o
	$(LINK.c) -o testgc testgc.o $(LDFLAGS)
testgc.o : testgc.c $(LINC)/ttdriver.h $(LINC)/nortlib.h
readgc : readgc.o
	$(LINK.c) -o readgc readgc.o $(LDFLAGS)
readgc.o : readgc.c $(LINC)/ttdriver.h $(LINC)/nortlib.h
readgc1 : readgc1.o
	$(LINK.c) -o readgc1 readgc1.o $(LDFLAGS)
readgc1.o : readgc1.c $(LINC)/ttdriver.h $(LINC)/nortlib.h
benchatod : benchatod.o
	$(LINK.c) -o benchatod benchatod.o $(LDFLAGS)
benchatod.o : benchatod.c $(LINC)/ttdriver.h $(LINC)/nortlib.h
testatod : testatod.o
	$(LINK.c) -o testatod testatod.o $(LDFLAGS)
testatod.o : testatod.c $(LINC)/ttdriver.h $(LINC)/nortlib.h
