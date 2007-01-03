# Other defs: NO_FLOAT NO_ALIGN SMALL_MEM NOMUX

INTFLAGS =  -WC,-s -Wc,-s -zu
MYFLAGS = -Wc,-bm -WC,-bm -3 -mf -D NO_ALIGN -c -zq -WC,-xs -I./include -ot

# MYFLAGS = -Wc,-bm -WC,-bm -3 -mf -D NO_ALIGN -c -zq -WC,-xs -I./include -g

# MYFLAGS = -Wc,-bm -WC,-bm -3 -ms -N32k -ot -D NO_ALIGN -c -zq -WC,-xs -I./include -g -WC,-r

DBGFLAGS = -g -zt -DDEBUG -2 -ml -zq -WC,-xs -I./include

OBJS = bograms.o bo_time.o seq36drv.o bo_intr.o bo_timeisr.o seq36isr.o \
bomemory.o bofifo.o bodmabuf.o bo_bmdma.o spectrum.o frxfm.o CString.o

seq36.lib : ${OBJS}
	cc -A seq36.lib ${OBJS}

seq36drv.o : seq36drv.cpp
	cc  $(MYFLAGS) -o$@ seq36drv.cpp

seq36isr.o : seq36isr.cpp
	cc  $(INTFLAGS) $(MYFLAGS) -o$@ seq36isr.cpp

bo_timeisr.o : bo_timeisr.cpp
	cc  $(INTFLAGS) $(MYFLAGS) -o$@ bo_timeisr.cpp

bograms.o : bograms.cpp
	cc  $(MYFLAGS) -o$@ bograms.cpp

bo_time.o : bo_time.cpp bo_time.h
	cc  $(MYFLAGS) -o$@ bo_time.cpp

bo_intr.o : bo_intr.cpp bo_intr.h
	cc  $(MYFLAGS) -o$@ bo_intr.cpp

bomemory.o : bomemory.cpp bomemory.h
	cc  $(MYFLAGS) -o$@ bomemory.cpp

bofifo.o : bofifo.cpp bofifo.h
	cc  $(MYFLAGS) -o$@ bofifo.cpp

bodmabuf.o : bodmabuf.cpp bodmabuf.h
	cc  $(MYFLAGS) -o$@ bodmabuf.cpp

bo_bmdma.o : bo_bmdma.cpp bo_bmdma.h
	cc  $(MYFLAGS) -o$@ bo_bmdma.cpp

spectrum.o : spectrum.cpp spectrum.h
	cc  $(MYFLAGS) -o$@ spectrum.cpp

frxfm.o : frxfm.cpp
	cc  $(MYFLAGS) -o$@ frxfm.cpp

CString.o : CString.cpp
	cc  $(MYFLAGS) -o$@ CString.cpp

# Test interrupts
bo_intr_test : bo_intr.cpp bo_intr.h
	cc -T1 $(INTFLAGS) $(DBGFLAGS) -o$@ bo_intr.cpp
	promote bo_intr_test

# Test DMA buffer
bodmabuf_test : bodmabuf.cpp bodmabuf.h bomemory.o
	cc -T1 $(DBGFLAGS) -o$@ bodmabuf.cpp bomemory.o
	promote bodmabuf_test

# Test DMA
bo_bmdma_test : bo_bmdma.cpp  bomemory.o bodmabuf.o
	cc -T1 $(DBGFLAGS) -o$@ bo_bmdma.cpp bomemory.o bodmabuf.o 
	promote bo_bmdma_test
