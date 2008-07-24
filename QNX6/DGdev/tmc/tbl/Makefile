flttime=/usr/local/share/huarp/flttime.tmc
OBJ=hsimdisp.c hsimdisp.tmc try.c
.PHONY : clean

hsimdisp.c : hsim.tmc $(flttime) hsimdisp.tmc
	tmc -o hsimdisp.c hsim.tmc $(flttime) hsimdisp.tmc
hsimdisp.tmc : hsim.tbl
	phtable hsim.tbl >hsimdisp.tmc
try.c : hsim.tmc try.tmc
	tmc -o try.c hsim.tmc try.tmc
clean :
	rm $(OBJ)
