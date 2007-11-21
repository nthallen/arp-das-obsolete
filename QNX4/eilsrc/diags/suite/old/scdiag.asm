;---------------------------------------------------------------------------
; scdiag.asm assembly routines for diagnosing the system controller board.
; Written January 18, 1990
;---------------------------------------------------------------------------
; int sb_word(int n_words, int *list);
;    Writes the list of words out to port B of the system controller,
;    verifying each word after writing.  The number of words successfully
;    verified is returned.
;---------------------------------------------------------------------------
name scdiag
include dos.mac

DSEG
public nmi_seen
nmi_seen dw ?
NMI_VECTOR equ 2
SUBBUS_PORTC equ 30CH
NMI_ENABLE equ 31CH
ENDDS

PSEG scdiag
SETX
;---------------------------------------------------------------------------
; void outpw(unsigned int, unsigned int);
;---------------------------------------------------------------------------
BEGIN outpw
	push	bp
	mov	bp, sp
	mov	ax, [bp+X+2]
	mov	dx, [bp+X]
	out	dx, ax
	pop	bp
	ret
outpw endp

;---------------------------------------------------------------------------
; unsigned inpw(unsigned int);
;---------------------------------------------------------------------------
BEGIN inpw
	push	bp
	mov	bp, sp
	mov	dx, [bp+X]
	in	ax, dx
	pop	bp
	ret
inpw endp

next_nmisr dd ?

BEGIN set_nmi
	mov	nmi_seen, 0
	push	es
	mov	ah, 35h
	mov	al, NMI_VECTOR
	int	21h
	mov	word ptr next_nmisr, bx
	mov	ax, es
	mov	word ptr next_nmisr+2, ax
	pop	es
	push	ds
	mov	ax, cs
	mov	ds, ax
	mov	dx, offset cs:nmisr
	mov	ah, 25h
	mov	al, NMI_VECTOR
	int	21h
	pop	ds
	ret
set_nmi endp

BEGIN reset_nmi
	push	ds
	mov	dx, word ptr next_nmisr[2]
	mov	ds, word ptr next_nmisr
	mov	ah, 25h
	mov	al, NMI_VECTOR
	int	21h
	pop	ds
	ret
reset_nmi endp

nmisr:
	push	ax
	push	dx
	mov	dx, SUBBUS_PORTC+1
	in	al, dx
	and	al, 4
	jnz	lowpower
cannot_solve:
	pop	dx
	pop	ax
	jmp	dword ptr next_nmisr
lowpower:
	mov	al, 8DH		; Disable NMI on the mother board
	out	70H, al		; THIS IS PC/AT SPECIFIC
	mov	dx, NMI_ENABLE	; clear nmi_enable on syscon
	xor	al, al
	out	dx, al
	in	al, 61H		; Clear I/O chk by pulsing Enable I/O Chk
	or	al, 8H
	out	61H, al
	and	al, not 8H
	out	61H, al
	mov	al, 0DH		; Re-enable NMI on the mother board
	out	70H, al
	push	ds		; Set NMI-SEEN.
	mov	ax, dgroup
	mov	ds, ax
	mov	nmi_seen, 1
	pop	ds
	pop	dx
	pop	ax
	iret

ENDPS scdiag
end
