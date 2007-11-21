;#$%!--------------------------------------------------------------------------
;                     COPYRIGHT (C) BOMEM INC, 1990
;
;Name:   Assembler definitions
;File:   ASMINC.asm
;Author: Claude Lafond
;Date:   march 22, 1990
;
;Description
;	Defines a standard model for all assembler functions, the module
;	should be organized as follows:
;
;	begmod
;
;	extrn ...
;
;	.code
;	...
;
;	.data
;	...
;
;	Procedures that are callable from C or C++ should be defined using
;	the oproc and cproc macros. Also any arguments should be
;	defined with an arg directive, local variables with a local directive
;	and registers to preserve with a uses directive
;
;#$%!..........................................................................
;
	.386p				; Accept full 386 instruction set

;
;	Beginning of a module
;

ifdef __HUGE__
begmod	macro
	.model use16 farstack tchuge

;	extrn	__AHSHIFT:far
;	extrn	__AHINCR:far
	assume	ds:@fardata
	.fardata
@curseg	ends
	endm
else
begmod	macro
;	.model use16 farstack large
	.model large

;	extrn	__AHSHIFT:far
;	extrn	__AHINCR:far
	assume	ds:@fardata
	.fardata
;@curseg	ends
	endm
endif

;	Equates
;
AHINCR      equ 8
SEG_STEP	equ	800h
OFF_STEP	equ	8000h
NO_ERROR	equ	0
;ERROR		equ	-1
TIMEOUT		equ	-6
;
;	Macros
;
;open or start function definition
;
oproc	macro	nom
	public	_&nom
_&nom	proc	C
	endm

;
;close or end function definition
;
cproc	macro	nom
_&nom	endp
	endm

;------------------------------------------------------------------------------
;	Make sure that the offset part of a long pointer is less than 15.
;	When this previous statement is true, we may work with the offset
;	for 64kb-15 bytes without offset wrap around problem.
;	This function does nothing in protected mode
;------------------------------------------------------------------------------
correct_seg	macro	seg_reg, off_reg, tmp1
ifndef	DOSX286
	push	off_reg			; Take a copy of the offset
	shr	off_reg, 4		; Convert the rest of the offset
					; in page of 16 bytes as segment
	mov	tmp1, seg_reg		; Update the segment register
	add	tmp1, off_reg
	mov	seg_reg, tmp1		; New segment
	pop	off_reg
	and	off_reg, 0fh		; New offset
endif
	endm
;------------------------------------------------------------------------------
;	Force delay between in/out at the same port (bug on AT)
;------------------------------------------------------------------------------
at_bug	macro
	local	fin
	jmp	fin
fin:
	endm

