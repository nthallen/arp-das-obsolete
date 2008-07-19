# Temporary Makefile while bringing the QNX6 DAS infrastructure together
hsimcol.c : hsim.tmc
	tmc -s -o hsimcol.c -p -V hsim.pcm -c hsim.tmc
hsimext.c : hsim.tmc hsimext.tmc
	tmc -o hsimext.c hsim.tmc hsimext.tmc
lcmmn.c : lcmmn.tmc
	tmc -s -o lcmmn.c -p -V lcmmn.pcm -c lcmmn.tmc
