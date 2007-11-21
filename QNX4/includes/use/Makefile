TOOL=Makefile
SRC=break.txt
SRC+=client.txt
SRC+=cc.txt
SRC+=das.txt
SRC+=dbr.txt
SRC+=dc.txt
SRC+=dg.txt
SRC+=dgdacin.txt
SRC+=eillib.txt
SRC+=msg.txt
SRC+=nl_cons.txt
SRC+=cmdalgo.txt
SRC+=serial.txt
SRC+=dbr_utils.txt
SRC+=tma.txt
OBJ=cmdalgos.txt cmds.txt clients.txt tmas.txt
SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ)
MNC=iuse
HOMEDIR=/usr/local/include/use

all : cmdalgos.txt cmds.txt clients.txt tmas.txt

# cmdalgos.txt is usage for self-service command algorithms (e.g. hoxalgo)
# soon to be obsolete
cmdalgos.txt : cmdalgo.txt msg.txt nl_cons.txt dc.txt
	echo "%C	[options] filename" >cmdalgos.txt
	cat cmdalgo.txt nl_cons.txt msg.txt dc.txt | sort >>cmdalgos.txt

# cmds.txt is usage for self-service command interpreters (e.g. hoxcmd)
cmds.txt : msg.txt nl_cons.txt
	echo "%C	[options]" >cmds.txt
	cat nl_cons.txt msg.txt | sort >>cmds.txt

# clients.txt is usage for interactive command interpreter clients
clients.txt : client.txt msg.txt nl_cons.txt
	echo "%C	[options]" >clients.txt
	cat client.txt msg.txt nl_cons.txt | sort >>clients.txt

# tmas.txt is usage for tmcalgo-generated programs
tmas.txt : nl_cons.txt msg.txt client.txt dc.txt tma.txt
	echo "%C	[options]" >tmas.txt
	cat nl_cons.txt msg.txt client.txt dc.txt tma.txt | sort >>tmas.txt
