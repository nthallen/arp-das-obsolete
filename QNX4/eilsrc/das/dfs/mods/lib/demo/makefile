CFLAGS = -g
MODEL = s

all: mod_utils.h mod_utils$(MODEL).lib fctr

fctr: fctr.c
	cc -o fctr -l das_utils fctr.c
	usemsg fctr fctr.use

mod_utils.h: file.h frame.h filename.h
	cp file.h ${HOME}/include
	cp frame.h ${HOME}/include
	cp filename.h ${HOME}/include
	cp mod_utils.h ${HOME}/include
	touch mod_utils.h

.c.o:
	$(COMPILE.c) $<
	wlib mod_utils$(MODEL).lib +-$@

mod_utils$(MODEL).lib : file.o frame.o filename.o
	cp mod_utils$(MODEL).lib ${HOME}/lib

file.o : file.c file.h

frame.o : frame.c frame.h

filename.o : filename.c filename.h

testfile :
	cc -g -o testfile -l mod_utils -l das_utils testfile.c

testdg : testdg.c
	cc -g -o testdg -l dbr_utils -l das_utils testdg.c

startdbr : startdbr.c
	cc -g -o startdbr -l dbr_utils -l das_utils startdbr.c

starttime : starttime.c
	cc -g -o starttime -l dbr_utils -l das_utils starttime.c

makedac :
	cc -g -o makedac -l dbr_utils makedac.c
