HOMEDIR=/usr/local/src/das/hart
MNC=hart
SOURCE=Makefile hart.c doit
OBJECT=hart.o hart.log
TARGET=hart
TGTNODE=//22
TGTDIR=$(TGTNODE)/home/hart

DISTRIB=hart doit
all : $(TARGET)
distribution : $(DISTRIB)
	distribute $(TGTDIR) $(DISTRIB)
