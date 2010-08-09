CFLAGS = -g -O0 -Wall
CPPFLAGS = -I/usr/local/include
LDLIBS = -lnort
LDFLAGS = -L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib

.PHONY: all install

all : subbusd

install : subbusd subbusd.h
	cp -v subbusd $(TGTNODE)/usr/local/bin
	cp -v subbusd.h $(TGTNODE)/usr/local/include
