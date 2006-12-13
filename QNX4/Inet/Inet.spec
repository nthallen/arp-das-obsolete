# This is how I started, but I had to edit the Makefile
# to eliminate msg.oui from Inetout, which handles msg
# initialization itself.

SRC = inet.txt InetBridge Inetdoit todo Inet.h
Inetout : Inetout.c Inet.c Inetout.oui
Inetin : Inetin.c Inet.c Inetin.oui
%%
LBIN=/usr/local/bin
all : $(LBIN)/Inetin $(LBIN)/Inetout $(LBIN)/InetBridge
$(LBIN)/Inetin : Inetin
	ln -f Inetin $(LBIN)
$(LBIN)/Inetout : Inetout
	ln -f Inetout $(LBIN)
$(LBIN)/InetBridge : InetBridge
	ln -f InetBridge $(LBIN)
