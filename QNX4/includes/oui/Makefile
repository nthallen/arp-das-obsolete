HOMEDIR=/usr/local/include/oui
MNC=ouilib

TOOL=Makefile
SRC=break.oui
SRC+=client.oui
SRC+=cmdctrl.oui
SRC+=compiler.oui
SRC+=dc.oui
SRC+=collect.oui dg.oui
SRC+=dgdacin.oui kbdclt.oui
SRC+=msg.oui msghdr.oui
SRC+=nl_cons.oui
SRC+=serial.oui
SRC+=server.oui
SRC+=seteuid.oui
SRC+=tma.oui
SRC+=subbus.oui subbus_reqd.oui subbus_opt.oui
SRC+=gpib232.oui
SRC+=tmg.oui
SRC+=runnum.oui drext.oui
SOURCE=$(SRC) $(TOOL)

TGTNODE=//1
DEST=$(TGTNODE)$(HOMEDIR)
distribution :
	cp -vni $(SRC) $(DEST); :
