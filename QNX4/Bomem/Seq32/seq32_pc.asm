	include	asminc.asm
DUMP_X		equ	00763be0h	; Dump X (movep x:(r0)+,y:HST_REG)
DUMP_Y		equ	00763fe0h	; Dump Y (movep y:(r0)+,y:HST_REG)
DUMP_P		equ	007637e0h	; Dump P (movep p:(r0)+,y:HST_REG)

LOAD_X		equ	00763ae0h	; Load X (movep y:HST_REG,x:(r0)+)
LOAD_Y		equ	00763ee0h	; Load Y (movep y:HST_REG,y:(r0)+)
LOAD_P		equ	007636e0h	; Load P (movep y:HST_REG,p:(r0)+)

BOOT_X		equ	30862000h	; Load X (movep d0.l,x:(r0)+)
BOOT_Y		equ	34862000h	; Load Y (movep d0.l,y:(r0)+)
BOOT_P		equ	00662008h	; Load P (movep d0.l,p:(r0)+)

TIMEOUT_LIMIT	equ	20000		; # retries on H_CVR acknownledge

HST_LEN		equ	1024		; HST_FIFO length

HST_FLG		equ	0		; RW16 Host flags register
HST_RST_LH	equ	0		; R8   Reset low/high logic
HST_INT		equ	0		; W8   Signal interrupt to SEQ32
HST_REG		equ	2		; RW16 Host register
HST_RESET	equ	2		; R8   Reset SEQ32 board
HST_FIFO	equ	4		; R16  Host fifo
;					
;	Flag register bit definitions
;
HST_PC0		equ	1		; RW Written to DSP
HST_PC1		equ	2		; RW Written to DSP
HST_PC2		equ	4		; RW Written to DSP
HST_PC3		equ	8		; RW Written to DSP
HST_PC4		equ	10h		; RW Written to DSP
HST_M0		equ	20h		; R  Written by DSP
HST_M1		equ	40h		; R  Written by DSP
HST_M2		equ	80h		; R  Written by DSP
HST_M3		equ	100h		; R  Written by DSP
HST_M4		equ	200h		; R  Written by DSP
HST_M5		equ	400h		; R  Written by DSP
HST_TXF		equ	800h		; R  Host transmit register full flag
HST_RXF		equ	1000h		; R  Host receive register full flag
HST_FIF_FULL	equ	2000h		; R  /Host fifo full flag
HST_FIF_HALF	equ	4000h		; R  /Host fifo half full flag
HST_FIF_EMPTY	equ	8000h		; R  /Host fifo empty flag


set_flg	macro	flg
	cli
	in	ax, dx
	or	ax, flg
	out	dx, ax
	sti
	endm

clr_flg	macro	flg
	cli
	in	ax, dx
	and	ax, not flg
	out	dx, ax
	sti
	endm

	begmod

	.fardata
seq32_base	dw	0		; SEQ32 i/o base address
hst_len1	dw	HST_LEN-1
tbmove		dd	DUMP_X, DUMP_Y, DUMP_P, LOAD_X, LOAD_Y, LOAD_P
tbboot		dd	BOOT_X, BOOT_Y, BOOT_P

	.code
;------------------------------------------------------------------------------
;Synopsis
;	call    sdump
;	Input:
;               es:di	Point output buffer
;               ecx	Nbr of points to be read from SEQ32
;		dx	seq32_base
;
;	Output:
;		es:di	di + 4 * ecx
;		ecx	0
;
;Description
;	Internal routine use to get data from SEQ32 and put it in ram
;
;Destroyed registers
;	eax, ecx, di, es
;..............................................................................
sdump	proc	near

dump1:	in	ax, dx			; Wait until DSP word ready
	test	ax, HST_RXF
	je	dump1

	add	dx, HST_REG		; Point HST_REG
	in	ax, dx			; Get word high part
	shl	eax, 16
	in	ax, dx			; Get word low part
	mov	es:[di], eax
	sub	dx, HST_REG		; Point HST_FLG
	add	di, 4			; Take care of wrap around

ifdef	DOSX286
	jne	dump2
	mov	ax, es
	add	ax, AHINCR
	mov	es, ax
else
	jns	dump2
	mov	ax, es
	add	ax, SEG_STEP
	mov	es, ax
	sub	di, OFF_STEP
endif

dump2:	loopd	dump1
dump3:	in	ax, dx			; Avoid lost of last transfer
	test	ax, HST_RXF
	je	dump3
	ret

sdump	endp
;------------------------------------------------------------------------------
;Synopsis
;	call    sload
;	Input:
;		es:di	Point input buffer
;		ecx	Nbr of points to be loaded on SEQ32
;		dx	seq32_base
;
;	Output:
;		es:di	di + 4 * ecx
;		ecx	0
;
;Description
;	Internal routine use to load data on SEQ32 from ram
;
;Destroyed registers
;	eax, ecx, di, es
;..............................................................................
sload	proc	near

load1:	in	ax, dx	    		; Wait until DSP ready to receive word
	test	ax, HST_TXF
	jne	load1

	mov	eax, es:[di]
	add	dx, HST_REG		; Point HST_REG
	out	dx, ax			; Put word low part
	shr	eax, 16
	out	dx, ax			; Put word high part

	sub	dx, HST_REG		; Point HST_FLG
	add	di, 4 			; Take care of wrap around
ifdef	DOSX286
	jne	load2
	mov	ax, es
	add	ax, AHINCR
	mov	es, ax
else
	jns	load2
	mov	ax, es
	add	ax, SEG_STEP
	mov	es, ax
	sub	di, OFF_STEP
endif

load2:	loopd	load1
load3:	in	ax, dx	    		; Avoid lost of last transfer
	test	ax, HST_TXF
	jne	load3
	ret

sload	endp
;#$%!--------------------------------------------------------------------------
;                     COPYRIGHT (C) BOMEM INC, 1994
;
;Name:   Set DSP board base address
;File:   SEQ32_PC.ASM
;Author: Claude Lafond
;Date:   Jan 14, 1994
;
;Synopsis
;	#include "seq32_pc.h"
;
;	void seq32_set_base (unsigned short seq32_base);
;
;	seq32_base	SEQ32 DSP board i/o base address
;
;Description
;	Use when ucode is load before program, in development only
;
;Destroyed registers
;	ax
;#$%!..........................................................................
	oproc	seq32_set_base
	arg	_seq32_base:word
	uses	ds

	mov	ax, @fardata		; Setup data segment
	mov	ds, ax

	mov	ax, [_seq32_base] 	; Save DSP base address
	mov	seq32_base, ax

	ret

	cproc	seq32_set_base
;#$%!--------------------------------------------------------------------------
;                     COPYRIGHT (C) BOMEM INC, 1994
;
;Name:   Reset DSP board
;File:   SEQ32_PC.ASM
;Author: Claude Lafond
;Date:   Jan 14, 1994
;
;Synopsis
;	#include "seq32_pc.h"
;
;	void seq32_reset (unsigned short seq32_base, short flags);
;
;	seq32_base	SEQ32 DSP board i/o base address
;	flags		Should be 0 for the moment
;
;Description
;	Reset SEQ32 board.  Must be done to be sure that the DSP is properly
;	reset.
;
;Destroyed registers
;	ax, dx
;#$%!..........................................................................
	oproc	seq32_reset
	arg	_seq32_base:word, flags:word
	uses	ds

	mov	ax, @fardata		; Setup data segment
	mov	ds, ax

	mov	dx, [_seq32_base] 	; Save DSP base address
	mov	seq32_base, dx
	mov	ax, [flags]		; Setup flags
	out	dx, ax
	in	al, dx			; Reset low/high logic

	add	dx, HST_RESET		; Point HST_RESET
	in	ax, dx			; Flush HST_REG (input)
	in	ax, dx
	in	al, dx			; Reset board

	ret

	cproc	seq32_reset
;#$%!--------------------------------------------------------------------------
;                     COPYRIGHT (C) BOMEM INC, 1994
;
;Name:	 Receive data from SEQ32
;File:	 SEQ32_PC.ASM
;Author: Claude Lafond
;Date:	 Jan 14, 1994
;
;Synopsis
;	#include "seq32_pc.h"
;
;	short seq32_rx_data (long HPTR *buffer, unsigned long len,
;			     short ram_type, unsigned long address);
;
;	buffer		Output buffer
;	len		Buffer length
;	ram_type	DSP target ram  X:0  Y:1  P:2
;	address		DSP ram address
;
;	Returns		NO_ERROR	Everything is ok
;			ERROR		DSP hangs up
;
;Description
;	Read data directly in SEQ32 ram
;
;Destroyed registers
;	ax, bx, cx, dx
;#$%!..........................................................................
	oproc	seq32_rx_data
	arg	buffer:dword, len:dword, ram_type:word, address:dword
	uses	ds, di

	mov	ax, @fardata		; Setup data segment
	mov	ds, ax

	call	sstart
	call	sdump
	jmp	short tx0

	cproc	seq32_rx_data
;#$%!--------------------------------------------------------------------------
;                     COPYRIGHT (C) BOMEM INC, 1994
;
;Name:	 Transmit data to SEQ32
;File:	 SEQ32_PC.ASM
;Author: Claude Lafond
;Date:	 Jan 14, 1994
;
;Synopsis
;	#include "seq32_pc.h"
;
;	short seq32_tx_data (long HPTR *buffer, unsigned long len,
;			     short ram_type, unsigned long address);
;
;	buffer		Input buffer
;	len		Buffer length
;	ram_type	DSP target ram  X:3   Y:4   P:5
;	address		DSP ram address
;
;	Returns		NO_ERROR	Everything is ok
;			ERROR		DSP hangs up
;
;Description
;	Transmit data directly in SEQ32 ram
;
;Destroyed registers
;	ax, bx, cx, dx
;#$%!..........................................................................
	oproc	seq32_tx_data
	arg	buffer:dword, len:dword, ram_type:word, address:dword
	uses	ds, di

	mov	ax, @fardata		; Setup data segment
	mov	ds, ax

	call	sstart
	call	sload

tx0:	set_flg	HST_PC0

tx1:	in	ax, dx			; Wait until DSP signal end of transfer
	test	al, HST_M0
	je	tx1

tx2:	mov	ax, NO_ERROR		; Everything is ok

tx3:	ret

	cproc	seq32_tx_data
;------------------------------------------------------------------------------
;Synopsis
;	call    sstart
;	Input:
;
;	Output:
;
;Description
;	Internal routine use to start data transfer on SEQ32 rx_data and tx_data
;
;Destroyed registers
;	eax, ecx, dx, di, es
;..............................................................................
sstart	proc	near

	cmp	dword ptr [len], 0	; If transfer == 0 ==> no transfer
	jne	sstart0
	pop	ax	 		; Flush near return address
	jmp	tx2

sstart0:mov	dx, seq32_base		; Point HST_FLG
	in	al, dx			; Reset low/high logic
	clr_flg	HST_PC0			; Reset communication flag HST_PC0

	mov	bx, [ram_type]		; Ram type
	shl	bx, 2			; Translate ram type to movep instr
	mov	eax, tbmove[bx]

	add	dx, HST_REG		; Point HST_REG
	out	dx, ax			; Send word low part
	shr	eax, 16
	out	dx, ax			; Send word high part
	in	ax, dx			; Flush receive register
	in	ax, dx
	sub	dx, HST_REG		; Point HST_INT

	out	dx, al			; Signal interrupt to SEQ32

	mov	ecx, 1000000
sstart1:in	ax, dx
	test	ax, HST_TXF
	je	sstart2
	loopd	sstart1
	pop	ax	 		; Flush near return address
	mov	ax, ERROR
	jmp	short tx3

sstart2:mov	eax, [address]		; Send transfer address
	add	dx, HST_REG		; Point HST_REG
	out	dx, ax			; Send word low part
	shr	eax, 16
	out	dx, ax			; Send word high part
	sub	dx, HST_REG		; Point HST_FLG

	les	di, [buffer]		; Buffer address
	mov	ecx, [len]		; Buffer length

	ret

sstart	endp
;#$%!--------------------------------------------------------------------------
;                     COPYRIGHT (C) BOMEM INC, 1994
;
;Name:   Transmit bootstrap code
;File:   SEQ32_PC.ASM
;Author: Claude Lafond
;Date:   Jan 14, 1994
;
;Synopsis
;       #include "seq32_pc.h"
;
;       short seq32_bootstrap (long HPTR *buffer, unsigned long len);
;       buffer          Input buffer
;       len		Buffer length
;
;       Returns          NO_ERROR        Everything is ok
;
;Description
;      Transmit bootstrap to DSP
;
;Destroyed registers
;      ax, cx, dx
;
;#$%!..........................................................................
	oproc	seq32_bootstrap
	arg	buffer:dword, len:dword
	uses	ds, di

	mov	ax, @fardata		; Setup data segment
	mov	ds, ax

	mov	dx, seq32_base 		; Reset communication flags
	xor	ax, ax
	out	dx, ax
	in	al, dx			; Reset low/high logic

	les	di, [buffer]		; Buffer address
	mov	ecx, [len]		; Buffer length

boot1:	in	ax, dx	    		; Wait until DSP ready to receive word
	test	ax, HST_TXF
	jne	boot1

	mov	eax, es:[di]
	add	dx, HST_REG		; Point HST_REG
	out	dx, ax			; Put word low part
	shr	eax, 16
	out	dx, ax			; Put word high part
	sub	dx, HST_REG		; Point HST_FLG

boot2:	in	ax, dx	    		; Wait until DSP ready to receive word
	test	ax, HST_RXF
	je	boot2

	add	dx, HST_REG		; Point HST_REG
	in	ax, dx
	shl	eax, 16
	in	ax, dx
	sub	dx, HST_REG		; Point HST_FLG

	cmp	eax, es:[di]
	je	boot3
	mov	ax, ERROR
	jmp	short boot5

boot3:	add	di, 4 			; Take care of wrap around
ifdef	DOSX286
	jne	boot4
	mov	ax, es
	add	ax, AHINCR
	mov	es, ax
else
	jns	boot4
	mov	ax, es
	add	ax, SEG_STEP
	mov	es, ax
	sub	di, OFF_STEP
endif

boot4:	loopd	boot1

	mov	ax, NO_ERROR
boot5:	ret

	cproc	seq32_bootstrap
;#$%!--------------------------------------------------------------------------
;                     COPYRIGHT (C) BOMEM INC, 1994
;
;Name:   Get data from DSP
;File:   SEQ32_PC.ASM
;Author: Claude Lafond
;Date:   Jan 14, 1994
;
;Synopsis
;       #include "seq32_pc.h"
;
;       short seq32_get_data (long HPTR *buffer, long buf_len, long answer_len);
;
;       buffer		Output buffer
;       buf_len		Buffer length
;	answer_len	Answer real length <= buf_len
;
;       Returns		NO_ERROR	Everything is ok
;			ERROR		No data avaliable
;			-2		buffer too small
;
;Description
;       Receive data from DSP when the DSP initiates the transfer
;
;Destroyed registers
;       ax, bx, cx, dx
;
;#$%!..........................................................................
	oproc	seq32_get_data
	arg	buffer:dword, buf_len:dword, answer_len:dword
	uses	ds, di

	mov	ax, @fardata		; Setup data segment
	mov	ds, ax

	cld
	mov	dx, seq32_base

get0:	in	ax, dx			; Do we really have data?
	test	al, HST_M1
	jne	get1
	mov	ax, ERROR		; Error:  No data avaliable
	jmp	get6

get1:	set_flg	HST_PC1
	add	dx, HST_FIFO		; Point HST_FIFO
	in	ax, dx			; Get transfer length
	mov	bx, ax
	add	dx, 2			; DEBUG
	in	ax, dx
	sub	dx, 2			; DEBUG
	shl	eax, 16
	mov	ax, bx
	mov	ebx, eax
	les	di, [answer_len]	; Save transfer length
	mov	es:[di], eax	
	sub	dx, HST_FIFO		; Point HST_FLG

	cmp	ebx, [buf_len]		; Is buffer big enough?
	jle	get2
	mov	ax, -2			; Error:  Output buffer too small
	jmp	short get6

get2:	les	di, [buffer]

get3:	in	ax, dx			; Is DSP ready for transfer?
	test	al, HST_M1
	je	get3

	test	ax, HST_FIF_EMPTY
	je	get3


get3a:	movzx	ecx, hst_len1
	cmp	ebx, ecx
	jge	get4
	mov	ecx, ebx

get4:	sub	ebx, ecx
    	add	dx, HST_FIFO		; Point HST_FIFO
	jmp	cl0

;----------------------------------------------------------------
; Code here to handle wrapping
; We skip this the first time, but loop here if wrapping is
; really required. This avoids a bug where we wrap at the very
; end of transfer even though we don't need to access that next
; byte
;----------------------------------------------------------------
cl0a:
ifdef	DOSX286
	mov	ax, es
	add	ax, AHINCR
	mov	es, ax
else
	mov	ax, es
	add	ax, SEG_STEP
	mov	es, ax
	sub	di, OFF_STEP
endif

cl0:	add	dx, 2			; DEBUG
	in	ax, dx
	mov	es:[di+2], ax
	sub	dx, 2
	in	ax, dx
	mov	es:[di], ax
	add	di, 4
ifdef	DOSX286
	je	cl1a
else
	js	cl1a
endif
cl1:	loop	cl0			; DEBUG
	jmp	cl1b
cl1a:	loop	cl0a
cl1b:
;;;	rep	insw
    	sub	dx, HST_FIFO		; Point HST_FLG

	or	ebx, ebx		; Is transfer completed?
	jne	get3

get5:	in	ax, dx			; Be sure that DSP set M1 == 0
	test	al, HST_M1
	jne	get5

	clr_flg	HST_PC1

	mov	ax, NO_ERROR

get6:	ret

	cproc	seq32_get_data

	end


