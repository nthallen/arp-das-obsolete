# Makefile for subbus/lib
# $Log$
#
HOMEDIR=/usr/local/src/das/subbus/lib
MNC=sblib
LIBNAME=subbus
# Default model: cannot use "3r" without setting MODELARGS=-ms -3
MODEL=l
# Fill in the models used.
MODELS=l s

#  Maintenance Information: Define TOOL, SRC, DOC, OBJ and TEMP
TOOL=Makefile

SRC=nmi.c reboot.c sb_name.c sublib.c tick_chk.c tick_sic.c sbb.c
SRC+=sbsnload.c sbba.c

DOC=todo

OBJ=$(OBJMDL)/nmi.o
OBJ+=$(OBJMDL)/reboot.o
OBJ+=$(OBJMDL)/sb_name.o
OBJ+=$(OBJMDL)/sublib.o
OBJ+=$(OBJMDL)/tick_chk.o
OBJ+=$(OBJMDL)/tick_sic.o
OBJ+=$(OBJMDL)/sbb.o
OBJ+=$(OBJMDL)/sbsnload.o
OBJ+=$(OBJMDL)/sbba.o

TEMP=*.err

# Include standard library rules
include /usr/local/lib/src/library.mk

# Define dependencies explicitly
$(OBJMDL)/nmi.o : nmi.c $(LINC)/subbus.h
$(OBJMDL)/reboot.o : reboot.c $(LINC)/subbus.h
$(OBJMDL)/sb_name.o : sb_name.c $(LINC)/subbus.h
$(OBJMDL)/sublib.o : sublib.c $(LINC)/subbus.h
$(OBJMDL)/tick_chk.o : tick_chk.c $(LINC)/subbus.h
$(OBJMDL)/tick_sic.o : tick_sic.c $(LINC)/subbus.h
$(OBJMDL)/sbb.o : sbb.c $(LINC)/subbus.h
$(OBJMDL)/sbsnload.o : sbsnload.c $(LINC)/subbus.h
$(OBJMDL)/sbba.o : sbba.c $(LINC)/subbus.h
