CFLAGS = -g

all: scicmd scicmd.use

scicmd : scicmd.o encodcrd.o 
	cc -g -o scicmd scicmd.o encodcrd.o -l das_utils

TXTS = scicmd.txt ../../utils/msg.txt
scicmd.use: $(TXTS)
	cat $(TXTS) >scicmd.use
	usemsg scicmd scicmd.use
