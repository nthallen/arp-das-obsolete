# Diagnostic Library
# $Log$
HOMEDIR=/usr/local/src/das/diagnost/lib
MNC=diagl
LIBNAME=diaglib
# Default model: cannot use "3r" without setting MODELARGS=-ms -3
MODEL=s
# Fill in the models used.
MODELS=s

#  Maintenance Information: Define TOOL, SRC, DOC, OBJ and TEMP
TOOL=Makefile

SRC=checkack.c checkrw.c skipchks.c diagerr.c

OBJ=$(OBJMDL)/checkack.o $(OBJMDL)/checkrw.o $(OBJMDL)/skipchks.o
OBJ+=$(OBJMDL)/diagerr.o

TEMP=*.err *.bak

# Include standard library rules
include /usr/local/lib/src/library.mk

# Define dependencies explicitly
$(OBJ) : $(LINC)/diaglib.h
$(OBJMDL)/checkack.o : checkack.c $(LINC)/subbus.h
$(OBJMDL)/checkrw.o : checkrw.c $(LINC)/subbus.h
$(OBJMDL)/skipchks.o : skipchks.c
$(OBJMDL)/diagerr.o : diagerr.c
