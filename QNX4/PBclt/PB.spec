cmdbase = PB.cmd
PBreg : PBreg.c dc.o $(OUIDIR)/dc.oui
TGTDIR = /usr/local/bin
%%
LBIN=/usr/local/bin
dist : PBreg PBclt
	@echo "Distribute to $(TGTDIR): \\c"; su -c "ln -fv PBreg PBclt $(TGTDIR)"
