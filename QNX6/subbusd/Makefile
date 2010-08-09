CFLAGS = -g -O0 -Wall
CPPFLAGS = -I/usr/local/include
LDLIBS = -lnort
LDFLAGS = -L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
SCUSB_OBJ = subbusd.o serusb.o

.PHONY: all install

all : subbusd_scusb

subbusd_scusb : $(SCUSB_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o subbusd_scusb $(SCUSB_OBJ) $(LDLIBS)

install : subbusd_scusb subbusd.h
	cp -v subbusd_scusb $(TGTNODE)/usr/local/bin
	cp -v subbusd.h $(TGTNODE)/usr/local/include
