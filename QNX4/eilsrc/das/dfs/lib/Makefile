# makefile for das/dbr/utils
# $Id$
# $Log$
# Revision 1.6  1992/07/16  21:12:08  nort
# Changed maintenance to use include
# New maintenance removes need for check and .DEFAULT targets
#
# Revision 1.5  1992/06/10  14:31:11  nort
# Added check target to compare working file against RCS
#
# Revision 1.4  1992/06/09  15:08:48  nort
# Modified .DEFAULT rule; Removed "all" target
#
# Revision 1.3  1992/05/20  19:53:01  nort
# Added .DEFAULT rule for automatic RCS check-out
#
# Revision 1.2  1992/05/20  17:27:39  nort
# Added maintenance targets.
# Added dgdacin.
#
# Revision 1.1  1992/05/19  14:20:47  nort
# Initial revision
#
MODEL=s
CFLAGS = -g -m$(MODEL) -I /usr/local/include

SRC=dacin.c dacoff.c db.c dbr_info.c dc.c dg.c dr.c dgdacin.c
TOOL=Makefile
DOC=dc.txt dg.txt report todo
OBJ=dacin.o dacoff.o db.o dbr_info.o dc.o dg.o dr.o dgdacin.o
INTERMED=dbr_utils$(MODEL).lib
TEMPORARY=dbr_utils$(MODEL).bak *.err
INCLUDED=dac.h dbr.h dbr_utils.h globmsg.h das_utils.h msg.h sounds.h
INCLUDED+=usage.h symname.h cmdctrl.h
MNC=dbru
SAVE=$(SRC) $(TOOL) $(DOC)
PURGE=$(OBJ) $(INTERMED) $(TEMPORARY)

dbr_utils$(MODEL).lib : $(OBJ)
	ln dbr_utils$(MODEL).lib /usr/local/lib

.c.o:
	$(COMPILE.c) $<
	wlib dbr_utils$(MODEL).lib +-$@

dg.o : dg.c

dc.o : dc.c

db.o : db.c

dr.o : dr.c

dacin.o : dacin.c /usr/local/include/dac.h

dacoff.o : dacoff.c /usr/local/include/dac.h

dbr_info.o : dbr_info.c

dgdacin.o : dgdacin.c

include /usr/local/include/maint.mk

