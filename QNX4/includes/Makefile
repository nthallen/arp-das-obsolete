# makefile for /usr/local/include
# $Id$
# $Log$
# Revision 1.4  1992/07/15  18:11:58  nort
# Changed Maintenance target to use 'include'
#
# Revision 1.3  1992/06/18  19:27:41  nort
# Updated standard maintenance targets.
#
# Revision 1.2  1992/05/19  21:13:51  nort
# Got rid of "tmc"
#
# Revision 1.1  1992/05/19  18:30:10  nort
# Initial revision
#
SRC=das.h dbr_utils.h dac.h dbr.h das_utils.h globmsg.h file.h mod_utils.h
SRC+=msg.h sights.h sounds.h symname.h timing.h usage.h curses.h company.h
SRC+=lat.h cmdctrl.h subbus.h maint.mk
TOOL=Makefile
DOC=
OBJ=
INTERMED=
TEMPORARY=
MNC=linc
SAVE=$(SRC) $(TOOL) $(DOC)
PURGE=$(OBJ) $(INTERMED) $(TEMPORARY)

all : archive

include /usr/local/include/maint.mk
