# makefile for das/dbr/utils
# $Id$
# $Log$
# Revision 1.9  1992/09/28  13:29:59  nort
# An update
#
# Revision 1.8  1992/08/20  21:03:52  nort
# New approach to building libraries.
#
# Revision 1.7  1992/08/10  19:30:46  nort
# With common maintenance tools?
#
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
LINC=$(LINCNODE)/usr/local/include
LIB=$(LIBNODE)/usr/local/lib
CFLAGS=-g -Wc,-fo$@ -m$(MODEL) -w4 -I $(LINC)

SRC=dacin.c dacoff.c db.c dbr_info.c dc.c dg.c dr.c dgdacin.c
SRC+=dc_options.c dg_options.c
TOOL=Makefile
DOC=dc.txt dg.txt report todo
OBJ=$(MODEL)/dacin.o
OBJ+=$(MODEL)/dacoff.o
OBJ+=$(MODEL)/db.o
OBJ+=$(MODEL)/dbr_info.o
OBJ+=$(MODEL)/dc.o
OBJ+=$(MODEL)/dg.o
OBJ+=$(MODEL)/dr.o
OBJ+=$(MODEL)/dgdacin.o
OBJ+=$(MODEL)/dc_options.o
OBJ+=$(MODEL)/dg_options.o
TGT=$(LIB)/dbr_utils$(MODEL).lib
TEMPORARY=*.bak *.err

HOMEDIR=/usr/local/src/das/dbr/utils
MNC=dbru
SOURCE=$(SRC) $(TOOL) $(DOC)
OBJECT=$(OBJ) $(INTERMED) $(TEMPORARY)
TARGET=
MODELS=s

$(TGT) : $(OBJ)
	@echo $(OBJ) | xargs -i echo +{} | xargs -t wlib -n $(TGT)

$(MODEL)/dg.o : dg.c
$(MODEL)/dc.o : dc.c
$(MODEL)/db.o : db.c
$(MODEL)/dr.o : dr.c
$(MODEL)/dacin.o : dacin.c /usr/local/include/dac.h
$(MODEL)/dacoff.o : dacoff.c /usr/local/include/dac.h
$(MODEL)/dbr_info.o : dbr_info.c
$(MODEL)/dgdacin.o : dgdacin.c
$(MODEL)/dc_options.o : dc_options.c
$(MODEL)/dg_options.o : dg_options.c
