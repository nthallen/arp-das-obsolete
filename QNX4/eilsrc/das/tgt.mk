include $(SRCDIR)/tgt.mk

#LDFLAGS += -l das -l dfs -l dfs_mod -l subbus
LDFLAGS += -l das -l dbr -l dfs_mod -l subbus

das : $(LIBDIR)/das$(MODEL).lib
#	cd $(DASDIR)/lib; Makelib MODELS=$(MODEL); cd $(HOMEDIR);

$(LIBDIR)/das$(MODEL).lib:
#	cd $(DASDIR)/lib; Makelib MODELS=$(MODEL); cd $(HOMEDIR);

dbr : $(LIBDIR)/dbr$(MODEL).lib
#	cd $(DASDIR)/dfs/lib; Makelib MODELS=$(MODEL); cd $(HOMEDIR);

dfs : $(LIBDIR)/dfs$(MODEL).lib
	cd $(DASDIR)/dfs/lib/plib; Makelib MODELS=$(MODEL); cd $(HOMEDIR);

$(LIBDIR)/dbr$(MODEL).lib:
#	cd $(DASDIR)/dfs/lib; Makelib MODELS=$(MODEL); cd $(HOMEDIR);

dbr_mod : $(LIBDIR)/dbr_mod$(MODEL).lib
	cd $(DASDIR)/dfs/mods/lib; Makelib MODELS=$(MODEL); cd $(HOMEDIR);

dfs_mod : $(LIBDIR)/dfs_mod$(MODEL).lib
	cd $(DASDIR)/dfs/mods/lib; Makelib MODELS=$(MODEL); cd $(HOMEDIR);

$(LIBDIR)/dbr_mod$(MODEL).lib:
	cd $(DASDIR)/dfs/mods/lib; Makelib MODELS=$(MODEL); cd $(HOMEDIR);

subbus :
