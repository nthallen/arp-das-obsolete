; SEQ32_PC.ASM
; ********************** COPYRIGHT (C) BOMEM INC, 1994 *******************
; This software is the property of Bomem and should be considered and
; treated as proprietary information.  Refer to the "CAAP Source Code
; License Agreement"
; ************************************************************************

;      !!!!!! TLIB Revision history ( Do not remove ) !!!!!!
; 1 SEQ32_PC.ASM 29-Apr-94,20:52:48,`THOMAS' File creation
; 1:1 SEQ32_PC.ASM 11-Jul-94,14:49:50,`THOMAS'
;      Added correct key sequence in the comment teplates so that the documentation
;      for the driver can be extracted automatically with PT.exe when the liobrary
;      documentation is generated.
; 1:2 SEQ32_PC.ASM 2-Nov-94,14:29:44,`CLAUDE'
;      Add very small delay between HST_REG low/high read to avoid problem
;      with bad PC.
;      Add infinite wait detection
; 1:3 SEQ32_PC.ASM 14-Nov-94,15:41:18,`CLAUDE' Add code to avoid dead lock
;      Remove model dependent segment wrap around code
;      Add small delay between in or out that use toggle mode on host interface
;      Add and redefine error code
; 1:4 SEQ32_PC.ASM 14-Nov-94,16:04:58,`CLAUDE' Add Tlib support
; 1:5 SEQ32_PC.ASM 13-Feb-95,12:00:32,`CLAUDE'
;      Add seq32_data_ready function
; 1:6 SEQ32_PC.ASM 13-Feb-95,13:56:44,`CLAUDE' Fix stupid bug jeq --> je
;      !!!!!! TLIB Revision history ( Do not remove ) !!!!!!

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

TIMEOUT_LIMIT	equ	3000000		; # retries on H_CVR acknownledge

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


TIMEOUT_ERROR		equ	-1	; No answer from DSP before TIME_OUT
NO_DATA			equ	-2	; DSP did not request data transfer
BUFFER_TOO_SMALL	equ	-3	; Reception buffer is too small
TRANSFER_ERROR		equ	-4	; Communication error

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
;#$%!i------------------------------------------------------------------------------
;Synopsis
;	call    sdump
;	Input:
;               es:di	Point output buffer
;               ebx	Nbr of points to be read from SEQ32
;		dx	seq32_base
;
;	Output:
;		es:di	di + 4 * ebx
;		ebx	0
;
;Description
;	Internal routine use to get data from SEQ32 and put it in ram
;
;Destroyed registers
;	eax, ebx, ecx, di, es
;#$%!i..............................................................................
sdump	proc	near

dump1:	mov	ecx, TIMEOUT_LIMIT	; Avoid dead lock
dump1a:	in	ax, dx			; Wait until DSP word ready
	test	ax, HST_RXF
	jne	dump1b
	loopd	dump1a
	jmp	short dmp_err

dump1b:	add	dx, HST_REG		; Point HST_REG
	in	ax, dx			; Get word high part
	shl	eax, 8			; Small delay for chip select
	shl	eax, 8
	in	ax, dx			; Get word low part

dump1c:	mov	es:[di], eax		; Store 32bits word
	sub	dx, HST_REG		; Point HST_FLG
	add	di, 4			; Point next data point

	jne	dump2			; Handle segment wrap around
	mov	ax, es
	add	ax, offset __AHINCR
	mov	es, ax

dump2:	dec	ebx
	jne	dump1

	mov	ecx, TIMEOUT_LIMIT	; Avoid dead lock
dump3:	in	ax, dx			; Avoid lost of last transfer
	test	ax, HST_RXF
	jne	dump3a
	loopd	dump3
dmp_err:stc				; Timeout error
	ret

dump3a:	clc				; No error
	ret

sdump	endp
;#$%!i------------------------------------------------------------------------------
;Synopsis
;	call    sload
;	Input:
;		es:di	Point input buffer
;		ebx	Nbr of points to be loaded on SEQ32
;		dx	seq32_base
;
;	Output:
;		es:di	di + 4 * ebx
;		ecx	0
;
;Description
;	Internal routine use to load data on SEQ32 from ram
;
;Destroyed registers
;	eax, ebx, ecx, di, es
;#$%!i..............................................................................
sload	proc	near

load1:	mov	ecx, TIMEOUT_LIMIT	; Avoid dead lock
load1a:	in	ax, dx	    		; Wait until DSP ready to receive word
	test	ax, HST_TXF
	je	load1b
	loopd	load1a
	jmp	short ld_err

load1b:	mov	eax, es:[di]		; Read 32bits word from PC ram
	add	dx, HST_REG		; Point HST_REG
	out	dx, ax			; Put word low part
	shr	eax, 8			; Small delay for chip select
	shr	eax, 8
	out	dx, ax			; Put word high part

	sub	dx, HST_REG		; Point HST_FLG
	add	di, 4 			; Point next data point

	jne	load2			; Handle segment wrap around
	mov	ax, es
	add	ax, offset __AHINCR
	mov	es, ax

load2:	dec	ebx
	jne	load1

	mov	ecx, TIMEOUT_LIMIT	; Avoid dead lock
load3:	in	ax, dx	    		; Avoid lost of last transfer
	test	ax, HST_TXF
	je	load3a
	loopd	load3
ld_err:	stc
	ret

load3a:	clc
	ret

sload	endp
;#$%!i--------------------------------------------------------------------------
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
;	Use when ucode is loaded before program, in development only
;
;Destroyed registers
;	ax
;#$%!i..........................................................................
	oproc	seq32_set_base
	arg	_seq32_base:word
	uses	ds

	mov	ax, @fardata		; Setup data segment
	mov	ds, ax

	mov	ax, [_seq32_base] 	; Save DSP base address
	mov	seq32_base, ax

	ret

	cproc	seq32_set_base
;#$%!i--------------------------------------------------------------------------
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
;	reset.  You MUST wait around 0.1s bfore trying to talk with SEQ32 to
;	leave tie to DSP to complete its initialization.
;
;Destroyed registers
;	ax, dx
;#$%!i..........................................................................
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
;#$%!i--------------------------------------------------------------------------
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
;			TIMEOUT_ERROR	DSP did not answer
;
;Description
;	Read data directly in SEQ32 ram
;
;Destroyed registers
;	eax, ebx, ecx, dx, es
;#$%!i..........................................................................
	oproc	seq32_rx_data
	arg	buffer:dword, len:dword, ram_type:word, address:dword
	uses	ds, di

	mov	ax, @fardata		; Setup data segment
	mov	ds, ax

	cmp	[len], 0  		; Detect len == 0 transfer
	je	tx_no

	call	sstart
	jc	tx_err
	call	sdump
	jmp	short tx0

	cproc	seq32_rx_data
;#$%!i--------------------------------------------------------------------------
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
;			TIMEOUT_ERROR	DSP did not answer
;
;Description
;	Transmit data directly in SEQ32 ram
;
;Destroyed registers
;	eax, ebx, ecx, dx, es
;#$%!i..........................................................................
	oproc	seq32_tx_data
	arg	buffer:dword, len:dword, ram_type:word, address:dword
	uses	ds, di

	mov	ax, @fardata		; Setup data segment
	mov	ds, ax

	cmp	[len], 0		; Detect len == 0 transfer
	jne	tx00
tx_no:	mov	ax, NO_DATA
	jmp	short tx3

tx00:	call	sstart
	jc	tx_err
	call	sload

tx0:	jc	tx_err

tx0a:	set_flg	HST_PC0

tx1:	mov	ecx, TIMEOUT_LIMIT	; Avoid dead lock
tx1a:	in	ax, dx			; Wait until DSP signal end of transfer
	test	al, HST_M0
	jne	tx2
	loopd	tx1a
tx_err:	mov	ax, TIMEOUT_ERROR
	jmp	short tx3

tx2:	mov	ax, NO_ERROR		; Everything is ok

tx3:	ret

	cproc	seq32_tx_data
;#$%!i------------------------------------------------------------------------------
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
;	eax, ebx, dx, di, es
;#$%!i..............................................................................
sstart	proc	near

sstart0:mov	dx, seq32_base		; Point HST_FLG
	in	al, dx			; Reset low/high logic
	clr_flg	HST_PC0			; Reset communication flag HST_PC0

	mov	bx, [ram_type]		; Ram type
	shl	bx, 2			; Translate ram type to movep instr
	mov	eax, tbmove[bx]

	add	dx, HST_REG		; Point HST_REG
	out	dx, ax			; Send word low part
	shr	eax, 8			; Small delay for chip select
	shr	eax, 8
	out	dx, ax			; Send word high part
	in	ax, dx			; Flush receive register
	in	ax, dx
	sub	dx, HST_REG		; Point HST_INT

	out	dx, al			; Signal interrupt to SEQ32

	mov	ecx, TIMEOUT_LIMIT	; Avoid dead lock
sstart1:in	ax, dx
	test	ax, HST_TXF
	je	sstart2
	loopd	sstart1
	stc				; TIMEOUT_ERROR
	ret

sstart2:mov	eax, [address]		; Send transfer address
	add	dx, HST_REG		; Point HST_REG
	out	dx, ax			; Send word low part
	shr	eax, 8			; Small delay for chip select
	shr	eax, 8
	out	dx, ax			; Send word high part
	sub	dx, HST_REG		; Point HST_FLG

	les	di, [buffer]		; Buffer address
	mov	ebx, [len]		; Buffer length

	clc
	ret

sstart	endp
;#$%!i--------------------------------------------------------------------------
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
;       Returns         NO_ERROR        Everything is ok
;			TIMEOUT_ERROR	DSP did not answer
;			TRANSFER_ERROR	Communication error
;
;Description
;      Transmit bootstrap to DSP
;
;Destroyed registers
;      eax, ebx, ecx, dx, es
;#$%!i..........................................................................
	oproc	seq32_bootstrap
	arg	buffer:dword, len:dword
	uses	ds, di

	mov	ax, @fardata		; Setup data segment
	mov	ds, ax

	cmp	[len], 0		; Detect len == 0 transfer
	jne	boot0
	mov	ax, NO_DATA
	jmp	short boot5

boot0:	mov	dx, seq32_base 		; Reset communication flags
	xor	ax, ax
	out	dx, ax
	in	al, dx			; Reset low/high logic

	les	di, [buffer]		; Buffer address
	mov	ebx, [len]		; Buffer length

boot1:	mov	ecx, TIMEOUT_LIMIT
boot1a:	in	ax, dx	    		; Wait until DSP ready to receive word
	test	ax, HST_TXF
	je	boot1b
	loopd	boot1a
	jmp	short booterr

boot1b:	mov	eax, es:[di]
	add	dx, HST_REG		; Point HST_REG
	out	dx, ax			; Put word low part
	shr	eax, 8			; Small delay for HOST interface
	shr	eax, 8
	out	dx, ax			; Put word high part
	sub	dx, HST_REG		; Point HST_FLG

	mov	ecx, TIMEOUT_LIMIT
boot2:	in	ax, dx	    		; Wait until DSP ready to receive word
	test	ax, HST_RXF
	jne	boot2a
	loopd	boot2
booterr:mov	ax, TIMEOUT_ERROR
	jmp	short boot5

boot2a:	add	dx, HST_REG		; Point HST_REG
	in	ax, dx
	shl	eax, 8			; Small delay for HOST interface
	shl	eax, 8
	in	ax, dx
	sub	dx, HST_REG		; Point HST_FLG

	cmp	eax, es:[di]
	je	boot3
	mov	ax, TRANSFER_ERROR
	jmp	short boot5

boot3:	add	di, 4 			; Take care of wrap around

	jne	boot4			; Handle segment wrap around
	mov	ax, es
	add	ax, offset __AHINCR
	mov	es, ax

boot4:	dec	ebx
	jne	short boot1

	mov	ax, NO_ERROR
boot5:	ret

	cproc	seq32_bootstrap
;#$%!i--------------------------------------------------------------------------
;                     COPYRIGHT (C) BOMEM INC, 1995
;
;Name:   Check to see if SEQ32 is ready for seq32_get_data
;File:   SEQ32_PC.ASM
;Author: Claude Lafond
;Date:   Feb 13, 1995
;
;Synopsis
;       #include "seq32_pc.h"
;
;       short seq32_data_ready (void);
;
;       Returns
;			1	Data ready
;			0	No data avaliable
;
;Description
;       Check to see if SEQ32 is ready for seq32_get_data
;
;Destroyed registers
;       ax, dx
;#$%!i..........................................................................
	oproc	seq32_data_ready
	uses	ds

	mov	ax, @fardata		; Setup data segment
	mov	ds, ax

	mov	dx, seq32_base		; Get SEQ32 base address

	in	ax, dx			; Do we really have data?
	and	ax, HST_M1
	je	dat_rdy			; No

	mov	ax, 1			; Yes
dat_rdy:ret
	cproc	seq32_data_ready
;#$%!i--------------------------------------------------------------------------
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
;       Returns		NO_ERROR	 Everything is ok
;			TIMEOUT_ERROR	 DSP did not answer
;			NO_DATA		 No data avaliable
;			BUFFER_TOO_SMALL Reception buffer is too small
;
;Description
;       Receive data from DSP when the DSP initiates the transfer
;
;Destroyed registers
;       eax, ebx, ecx, dx
;#$%!i..........................................................................
	oproc	seq32_get_data
	arg	buffer:dword, buf_len:dword, answer_len:dword
	uses	ds, di

	mov	ax, @fardata		; Setup data segment
	mov	ds, ax

	cld
	mov	dx, seq32_base

	in	ax, dx			; Do we really have data?
	test	al, HST_M1
	jne	get1
	mov	ax, NO_DATA		; Error:  No data avaliable
	jmp	get6

get1:	set_flg	HST_PC1
	add	dx, HST_FIFO+2		; Point HST_FIFO high word part
	in	ax, dx			; Get transfer length
	shl	eax, 16
	sub	dx, 2			; Point HST_FIFO high word part
	in	ax, dx
	mov	ebx, eax

	les	di, [answer_len]	; Save transfer length
	mov	es:[di], eax	
	sub	dx, HST_FIFO		; Point HST_FLG

	cmp	ebx, [buf_len]		; Is buffer big enough?
	jle	get2
	mov	ax, BUFFER_TOO_SMALL	; Error:  Output buffer too small
	jmp	short get6

get2:	les	di, [buffer]

get3:	mov	ecx, TIMEOUT_LIMIT
get3a:	in	ax, dx			; Is DSP ready for next block transfer?
	test	al, HST_M1
	je	get3b

	test	ax, HST_FIF_EMPTY	; Avoid problem if DSP is interrupted
	jne	get3c			; before clearing HST_M1
get3b:	loopd	get3a
	jmp	short get_err

get3c:	movzx	ecx, hst_len1		; Is it a full fifo transfer?
	cmp	ebx, ecx
	jge	get4
	mov	ecx, ebx

get4:	sub	ebx, ecx
    	add	dx, HST_FIFO		; Point HST_FIFO

get4a:	add	dx, 2
	in	ax, dx			; Get word high part
	shl	eax, 16
	sub	dx, 2
	in	ax, dx			; Get word low part
	mov	es:[di], eax
	add	di, 4

	jne	get4b	  		; Handle segment wrap around
	mov	ax, es
	add	ax, offset __AHINCR
	mov	es, ax
get4b:	loop	get4a

    	sub	dx, HST_FIFO		; Point HST_FLG

	or	ebx, ebx		; Is transfer completed?
	jne	get3

get5:	mov	ecx, TIMEOUT_LIMIT	; Avoid dead lock
get5a:	in	ax, dx			; Be sure that DSP set M1 == 0
	test	al, HST_M1
	je	get5b
	loopd	get5a
get_err:mov	ax, TIMEOUT_ERROR
	jmp	short get6

get5b:	clr_flg	HST_PC1

	mov	ax, NO_ERROR

get6:	ret

	cproc	seq32_get_data

	end

