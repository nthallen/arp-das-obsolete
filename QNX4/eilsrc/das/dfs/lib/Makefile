# makefile for das/dbr/utils
# $Id$
# $Log$
CFLAGS = -g
MODEL = s

all : dbr_utils.h dbr_utils$(MODEL).lib

dbr_utils.h: dbr.h dac.h
	cp dbr.h ${HOME}include
	cp dac.h ${HOME}include
	cp dbr_utils.h ${HOME}include
	touch dbr_utils.h

dbr_utils$(MODEL).lib : dg.o dc.o db.o dr.o dacin.o dacoff.o dbr_info.o
	cp dbr_utils$(MODEL).lib ${HOME}lib

.c.o:
	$(COMPILE.c) $<
	wlib dbr_utils$(MODEL).lib +-$@

dg.o : dg.c

dc.o : dc.c

db.o : db.c

dr.o : dr.c

dacin.o : dacin.c dac.h

dacoff.o : dacoff.c dac.h

dbr_info.o : dbr_info.c
