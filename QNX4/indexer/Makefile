# Makefile for indexer
CFLAGS=-g -ms -w4 -I /usr/local/include -l nortlib -L /usr/local/lib
CFLAGS+=-l subbus -l das_utils
indexer : indexer.o
indexer.o : indexer.c
	$(COMPILE.c) -D INDXR_SPEC_FILE=\"ohindxr.h\" indexer.c
