# makefile for /usr/local/include
# $Id$
# $Log$
# Revision 1.8  1992/09/24  20:25:06  nort
# An update
# .,
#
# Revision 1.7  1992/09/09  18:45:23  nort
# Latest version
#
# Revision 1.6  1992/08/20  20:59:54  nort
# More targets
#
# Revision 1.5  1992/07/16  19:13:54  nort
# Changes relative to maintenance.
#
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
SRC=dbr_utils.h dac.h dbr.h das_utils.h globmsg.h file.h mod_utils.h
SRC+=msg.h sounds.h symname.h timing.h usage.h curses.h company.h
SRC+=lat.h cmdctrl.h subbus.h maint.mk break.h cc.h diaglib.h
SRC+=memlib.h ssp.h snafuerr.h nortlib.h collect.h indexer.h
SRC+=colmain.skel extmain.skel timerbd.h sol.h reboot.h
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
