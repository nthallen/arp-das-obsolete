# General and library targets

all: copyhdr $(LIBS) $(TARGET) liblink $(BASE).use

perms:
	@if test "`find . -pname $(TARGET) -user 0`" -a test -u $(TARGET); then \
	 echo permissions on $(TARGET) already set; \
	else \
	 promote $(TARGET); \
	fi;

#  echo setting effective user id to root;
#  su root -c "chown root $(TARGET); chmod u+s $(TARGET); ls -l $(TARGET)";

copyhdr:
	@if test -f $(BASE).h; then \
	echo checking $(BASE).h with $(INCDIR)/\$(BASE).h; \
	diff $(INCDIR)/$(BASE).h $(BASE).h \
	else \
	echo $(BASE).h does not exist; \
	fi;

$(TARGET): $(OBJS)
	$(LINK.c) -o $@ $(OBJS) $(LDFLAGS)

liblink:
	@for i in $(LIBS); do \
	if test $(LIBDIR)/"$$i"$(MODEL).lib -nt $(TARGET); then \
	echo linking with updated library $$i; \
	$(LINK.c) -o $(TARGET) $(OBJS) $(LDFLAGS); \
	else \
	echo Library $$i up to snuff; \
	fi; \
	done;

$(BASE).use: $(TXTS) $(TARGET)
	@if test $(TXTS); then\
	echo making new $(BASE).use; \
	cat $(TXTS) > $(BASE).use; \
	echo adding new use message to $(TARGET); \
	usemsg $(TARGET) $(BASE).use;\
	touch $(BASE).use; \
	else \
	echo no txt files; \
	fi;

LDFLAGS += -l eillib -l nortlib -l subbus -l curses -l lat -l curses_utils -l termlib

termlib:

eillib:
#	cd $(SRCDIR)/lib; Makelib MODELS=$(MODEL); cd $(HOMEDIR)

nortlib:

curses:

curses_g:

ncurses:

subbus:

lat:

curses_utils:

ncurses_utils:

snfbtch:

ssp:

snfdr:
