# library general targets and default definitions

MODELS = s l c 3r
TARGET = $(LIBDIR)/$(BASE)$(MODEL).lib

all:  $(INCDIR)/$(BASE).h $(USEDIR)/$(BASE).txt $(TARGET)

$(INCDIR)/$(BASE).h : $(HDRS)
	@if test $(HDRS); then\
		echo Copying $(HDRS) to $(INCDIR); \
		cp -f $(HDRS) $(INCDIR); \
		touch $@; \
	fi

$(USEDIR)/$(BASE).txt : $(TXTS)
	@if test $(TXTS); then\
		echo Making new $(BASE).txt; \
		cat $(TXTS) > $(BASE).txt; \
		echo Copying $(TXTS) $(BASE).txt to $(USEDIR); \
		cp -f $(TXTS) $(BASE).txt $(USEDIR); \
	else \
		echo No txt files; \
	fi

.c.o:
	$(COMPILE.c) -Wc,-fo$@ $<

$(TARGET) : $(OBJS)
	if test -f $(TARGET); then \
	for i in $?; do\
		wlib -b $@ +-$$i;\
	done; \
	else \
		$(CC) -A $(TARGET) $(OBJS); \
	fi

