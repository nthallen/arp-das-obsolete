CFLAGS=-I/usr/local/include -g
# LIBS=-Bstatic -ltmph -ltm -lnort -Bdynamic -lph
CLTLIBS=-ltmph -ltm -lnort -Bdynamic -lph
SRVRLIBS=-ltm -lnort -Bdynamic -lph
LDFLAGS=-L/usr/local/lib
cmdsrvr_LDFLAGS=-Wl,-rpath -Wl,/usr/local/lib
OUIDIR=/usr/local/share/oui

all : cmdclient cmdsrvr
cmdclient : clt.o cltoui.o
	cc $(LDFLAGS) -o cmdclient clt.o cltoui.o $(CLTLIBS)
cmdsrvr : srvr.o srvroui.o
	cc $(LDFLAGS) $(cmdsrvr_LDFLAGS) -o cmdsrvr srvr.o srvroui.o $(CLTLIBS)
	sudo sh -c "chown root:root cmdsrvr; chmod u+s cmdsrvr; ls -l cmdsrvr"
clt.o : trial.c
	cc -c -o clt.o -DCLIENT $(CFLAGS) trial.c
srvr.o : trial.c
	cc -c -o srvr.o -DSERVER $(CFLAGS) trial.c
trial.c : trial.cmd
	cmdgen trial.cmd
cltoui.c : $(OUIDIR)/cic.oui
	oui -o cltoui.c $(OUIDIR)/cic.oui
srvroui.c : $(OUIDIR)/cis.oui
	oui -o srvroui.c $(OUIDIR)/cis.oui

.PHONY: all clean
clean :
	rm -f *.o trial.c cmdclient cmdsrvr *oui.c
