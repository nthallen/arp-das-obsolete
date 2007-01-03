#include <conio.h>
#include <stdlib.h>
#include <i86.h>

main(int argc, char **argv) {
short dma_ioadr;

dma_ioadr = atoh(argv[1]);

_enable();
	// reset card

	outpw (dma_ioadr+4, 0);
	outp (dma_ioadr+4, 0);

	// reset instrument
	outp (dma_ioadr+5, 0xc0);

}