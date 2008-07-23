if LIBRARY_SUB EQ 1
	name subbus_pi
endif
if LIBRARY_SUB EQ 2
	name subbus_ps
endif
if LIBRARY_SUB EQ 3
	name subbus_sc
endif
if LIBRARY_SUB EQ 4
	name subbus_104
endif
;----------------------------------------------------------------------
; $Log$
; Revision 1.8  1997/07/03 19:49:14  nort
; WAIT_COUNT to 10 for PCICC on P100
;
; Revision 1.7  1997/07/03 19:47:32  nort
; Bug causing stack corruption
;
; Revision 1.6  1995/06/07  19:53:45  nort
; Added cmdstrobe function
;
; Revision 1.5  1995/06/07  19:19:55  nort
; Subbus104 Version
;
; Revision 1.4  1995/04/21  17:54:12  nort
; WAIT_COUNT to 5 for PCICC on 386
;
; Revision 1.3  1992/06/18  15:39:49  nort
; Cleaned up unused functions (including NMI handler).
; Assembled successfully using LASM under DOS
;
; Revision 1.2  1992/06/18  13:27:01  nort
; Major rewrite for QNX4: Initial Entry.
;
; Revision 1.1  1992/06/16  17:24:50  nort
; Initial revision
;----------------------------------------------------------------------
; Modified April 21, 1995 for Syscon/104
;   New Syscon requires redefinition of what SICFUNCs are. Namely
;   we no longer have any NVRAM and no NMI functions, but we do
;   have other functions as yet to be specified (config of interrupts)
;----------------------------------------------------------------------
; Modified October 29, 1991 to add enhanced watchdog functionality:
;   Additional functions are added, so read_switches() is modified
;   to signal the presence of the new functions.
;   Added: subbus_version
;          subbus_name
;          set_tps(*)
;          tick_check(*)
;----------------------------------------------------------------------
; Modified September 16, 1991 to remove timing loops for XT-based
;   systems. It is assumed that PCICC and PCICCSIC are XT for now,
;   but judicious use of the WAIT_COUNT symbol could compile these for
;   faster CPUs.
;----------------------------------------------------------------------
; subbus will be the first resident library.
; Functions included are:
;   int read_subbus(unsigned int address);
;   int read_ack(unsigned int address, int far *data);
;   int write_ack(unsigned int address, int data);
;   void set_cmdenbl(int value);
;   void tick_sic(void);
;   void disarm_sic(void);
;   unsigned char read_novram(unsigned int address);
;   void write_novram(unsigned int address, unsigned char val);
;   unsigned char read_switches(void);
;   void enable_nmi(void (far *func)(void));
;   void disable_nmi(void);
;   void set_failure(int value);
;   unsigned char read_rstcnt(void);
;   unsigned char read_pwrcnt(void);
;   short int set_cmdstrobe(short int value);
; Subbus Library Identification is 1
; The current subfunctions are:
;		1 PC/XT PC/ICC card
;		2 PC/XT PC/ICC with SIC
;		3 PC/AT System Controller
;       4 PC/104 System Controller
;----------------------------------------------------------------------
; SICFUNC Functions are
;	Read/Write NVRam
;	NMI Functions
;----------------------------------------------------------------------
; This used to be a SICFUNC function, but now stands alone
;	Read Switches
;----------------------------------------------------------------------
include sublib.mac

SUBBUS_VERSION equ 310H ; Subbus version 3.10 QNX4

;----------------------------------------------------------------------
; Subbus Features:
;----------------------------------------------------------------------
SBF_SIC			equ	 1	; SIC Functions (SICFUNC)
SBF_LG_RAM		equ	 2	; Large NVRAM (SYSCON)
SBF_HW_CNTS		equ	 4	; Hardware rst & pwr Counters (SIC)
SBF_WD			equ	 8	; Watchdog functions (always)
SBF_SET_FAIL	equ	10H ; Set failure lamp (Comes w/ SICFUNC)
SBF_READ_FAIL	equ	20H ; Read failure lamps (SYSCON)
SBF_READ_SW     equ 40H ; Read Switches (was SICFUNC, now indep.)
SBF_NVRAM       equ 80H ; Any NVRAM at all!
SBF_CMDSTROBE   equ 100H ; Cmdstrobe Function

SUBBUS_FEATURES = SBF_WD

if LIBRARY_SUB EQ 1
	PCICC	equ	1
	SIC		equ	0
	SYSCON	equ	0
	SICFUNC	equ	0
	SC104   equ 0
	READSWITCH equ 0
	NVRAM   equ 0
endif

if LIBRARY_SUB EQ 2
	PCICC	equ	1
	SIC		equ	1
	SYSCON	equ	0
	SICFUNC	equ	1
	SC104   equ 0
	READSWITCH equ 1
	NVRAM   equ 1
endif

if LIBRARY_SUB EQ 3
	PCICC	equ	0
	SIC		equ	0
	SYSCON	equ	1
	SICFUNC	equ	1
	SC104   equ 0
	READSWITCH equ 1
	NVRAM   equ 1
endif

if LIBRARY_SUB EQ 4
	PCICC	equ	0
	SIC		equ	0
	SYSCON	equ	1
	SICFUNC	equ	0
	SC104   equ 1
	READSWITCH equ 1
	NVRAM   equ 0
endif

if PCICC
	SC_SB_RESET		equ		310H
	SC_SB_LOWA		equ		308H
	SC_SB_HIGHA		equ		30CH
	SC_SB_LOWB		equ		309H
	SC_SB_HIGHB		equ		30DH
	SC_SB_LOWC		equ		30AH
	SC_SB_HIGHC		equ		30EH
	SC_SB_LOWCTRL 	equ		30BH
	SC_SB_HIGHCTRL	equ		30FH
	SC_SB_CONFIG	equ   0C1C0H
	SC_CMDENBL		equ		311H
	SC_DISARM		equ		318H
	SC_TICK			equ		319H
	WAIT_COUNT		equ		10
endif

if SIC
	SC_RES_CNT		equ		31AH
	SC_PWR_CNT		equ		31BH
	SC_RAMADDR		equ		31CH
	SC_NMI_ENABLE	equ		31CH
	SC_NMIE_VAL		equ		 20H
	SC_RAMDATA		equ		31DH
	SC_SWITCHES		equ		31EH
	SC_LAMP			equ		31FH
	SUBBUS_FEATURES = SUBBUS_FEATURES OR SBF_HW_CNTS
endif

if SYSCON
	SC_SB_RESET		equ		310H
	SC_SB_LOWA		equ		308H
	SC_SB_LOWB		equ		30AH
	SC_SB_LOWC		equ		30CH
	SC_SB_LOWCTRL	equ		30EH
	SC_SB_HIGHA		equ		309H
	SC_SB_HIGHB		equ		30BH
	SC_SB_HIGHC		equ		30DH
	SC_SB_HIGHCTRL	equ		30FH
	SC_SB_CONFIG	equ   0C1C0H
	SC_CMDENBL		equ		318H
	SC_DISARM		equ		311H
	SC_TICK			equ		319H
	SC_LAMP			equ		317H
	if SC104
		SC_SWITCHES		equ		316H
		WAIT_COUNT		equ		5
		SUBBUS_FEATURES = SUBBUS_FEATURES OR SBF_SET_FAIL
	else
		SC_RAMADDR		equ		31AH
		SC_RAMDATA		equ		31DH
		SC_NMI_ENABLE	equ		31CH
		SC_NMIE_VAL		equ		1 
		WAIT_COUNT		equ		1
		SC_SWITCHES		equ		31CH
		SUBBUS_FEATURES = SUBBUS_FEATURES OR SBF_LG_RAM
	endif
	SUBBUS_FEATURES = SUBBUS_FEATURES OR SBF_READ_FAIL OR SBF_CMDSTROBE
endif

if SICFUNC
	TICKFAIL	equ	6 ; novram addr for tick fail info
	SUBBUS_FEATURES = SUBBUS_FEATURES OR SBF_SIC
	SUBBUS_FEATURES = SUBBUS_FEATURES OR SBF_SET_FAIL
endif

if READSWITCH
	SUBBUS_FEATURES = SUBBUS_FEATURES OR SBF_READ_SW
endif

;----------------------------------------------------------------
; SC104 is the first unit without any NVRAM
;----------------------------------------------------------------
if NVRAM
	SUBBUS_FEATURES = SUBBUS_FEATURES OR SBF_NVRAM
endif

DSEG
public _sbfs
_sbfs	dw SUBBUS_VERSION
		dw SUBBUS_FEATURES
		dw LIBRARY_SUB
		dd read_subbus_
		dd write_ack_
		dd read_ack_
		dd set_cmdenbl_
		dd read_novram_
		dd write_novram_
		dd read_switches_
		dd set_failure_
		dd read_rstcnt_
		dd read_pwrcnt_
		dd read_failure_
		dd cmdstrobe_

public _subbus_name
_subbus_name db 'Subbus Resident Library V3.01', 0
ENDDS

PSEG

;----------------------------------------------------------------------
; cleanup: make sure NMI is disabled.
;----------------------------------------------------------------------
INITFUNC cleanup_
	if SYSCON
		call	far ptr ldisarm_sic_
		call	far ptr ldisable_nmi_
	endif
		ret
cleanup_ endp

; These are the arguments (I'm ignoring the subbus number)

;----------------------------------------------------------------------
; rsub is a local function which does the reading for both
; read_subbus and read_ack.
;  Inputs:  ax subbus address
;  Outputs: ax data, zero flag clear (non-zero)
;           if transfer was acknowledged
;	    cx wait count left
;  dx are trashed.
;----------------------------------------------------------------------
if SC104 EQ 0
rsub proc near
		mov		cx, WAIT_COUNT
		mov		dx, SC_SB_LOWB		; Write the address
		cli							;;; Disable
      if SYSCON
		out		dx, ax				;;;
      else
		out		dx, al				;;;
		mov		dx, SC_SB_HIGHB		;;;
		mov		al, ah				;;;
		out		dx, al				;;;
      endif
		mov		dx, SC_SB_LOWCTRL	;;; Set Read
		mov		al, 1				;;;
		out		dx, al				;;;
      if WAIT_COUNT
		mov		dx, SC_SB_HIGHC		;;;
1$:		in		al, dx				;;; check for EXPACK/
		test	al, 1				;;;
		jz		2$					;;;
		loop	1$					;;;
2$:		mov		dx, SC_SB_LOWCTRL	;;;
      endif
		xor		al, al				;;; Reset the read bit
		out		dx, al				;;;
		mov		dx, SC_SB_LOWC		;;; Get the status
		in		al, dx				;;;
		test	al, 20H				;;; Set flag
      if SYSCON
		mov		dx, SC_SB_LOWA		;;;
		in		ax, dx				;;;
      else
		mov		dx, SC_SB_HIGHA		;;; Get the data
		in		al, dx				;;;
		mov		ah, al				;;;
		mov		dx, SC_SB_LOWA		;;;
		in		al, dx				;;;
      endif
		sti
		ret
rsub endp

else

;----------------------------------------------------------------
; Let me try an SC104 version
;----------------------------------------------------------------
rsub proc near
		mov		cx, WAIT_COUNT
		mov		dx, SC_SB_LOWB		; Write the address
		cli							;;; Disable
		out		dx, ax				;;;
		mov		dx, SC_SB_LOWC   	;;; Set Read
		mov		al, 1				;;;
		out		dx, al				;;;
1$:		in		ax, dx				;;; check for !(RD+WR)
		test	ax, 800H			;;;
		jz		2$					;;;
		loop	1$					;;;
2$:		test	al, 40H				;;; Set Z flag (EXPACK at timeout)
		mov		dx, SC_SB_LOWA		;;;
		in		ax, dx				;;;
		sti
		ret
rsub endp

endif

;----------------------------------------------------------------------
; read_subbus(int subbus_id, unsigned int address);
;----------------------------------------------------------------------
;  Actual invocation is read_subbus(unsigned int address);
;  Under WATCOM, address is passed in AX
;  Must save all registers used.
;----------------------------------------------------------------------
LIBFUNC read_subbus_
		push	cx
		push	dx
		call	rsub
		pop		dx
		pop		cx
		ret
read_subbus_ endp

;----------------------------------------------------------------------
; success = read_ack(int subbus_id, unsigned int address, int far *data);
;   The success code has the following values
;        1 to WAIT_COUNT+1  How far we got in the wait loop.
;			    1 actually indicates we timed out (never
;			    actually saw EXPACK/) but the data was
;			    latched by the time we checked.
;	 0 timed out and saw no latching.
;       -1 Saw EXPACK but didn't latch the data.  Indicative of
;          subbus controller malfunction or perhaps continuous
;          EXPACK/
;----------------------------------------------------------------------
; Actual invocation: read_ack(uint address, int far *data);
; Under WATCOM
;   address is in AX, data is in CX:BX
;----------------------------------------------------------------------
LIBFUNC read_ack_
		push	ds
		push	dx
		push	bx
		push	cx
		call	rsub
		jz		2$		; Jump if we didn't get an acknowledge
		inc		cx
		jmp		short 1$
2$:
if SC104 EQ 0
		jcxz	1$
		mov		cx, -1		; -1 means saw EXPACK/ but no latch	
else
		xor		cx, cx
endif
1$:		pop		ds
		pop		bx
		mov		[bx], ax
		mov		ax, cx
		pop		dx
		pop		ds
		ret
read_ack_ endp

;----------------------------------------------------------------------
; Write_ack returns a value between 0 and WAIT_COUNT.  Zero means
; we timed out.  Non-zero value indicates how far we got through the
; loop.
;----------------------------------------------------------------------
; Actual invocation: write_ack(uint address, uint data)
;    address is AX
;    data is DX
;----------------------------------------------------------------------
if SC104 EQ 0

LIBFUNC write_ack_
		push	cx
		push	dx					; Save data: address is in AX
		push	dx
		mov		dx, SC_SB_LOWB
      if WAIT_COUNT
		mov		cx, WAIT_COUNT
      endif
		cli							;;;
      if SYSCON						;;; Write out the address
		out		dx, ax				;;;
      else							;;;
		out		dx, al				;;;
		mov		dx, SC_SB_HIGHB		;;;
		mov		al, ah				;;;
		out		dx, al				;;;
      endif							;;;
		pop		ax					;;; Get the data
		mov		dx, SC_SB_LOWA		;;;
      if SYSCON						;;; Write out the data
		out		dx, ax				;;;
      else							;;;
		out		dx, al				;;;
		mov		dx, SC_SB_HIGHA		;;;
		mov		al, ah				;;;
		out		dx, al				;;;
      endif							;;;
		mov		al, 5				;;; command to set C2: EXPWR
		mov		dx, SC_SB_LOWCTRL	;;;
		out		dx, al				;;;
		mov		dx, SC_SB_HIGHC		;;; High byte
1$:		in		al, dx				;;; check for EXPACK/
      if WAIT_COUNT
		and		al, 1				;;;
		jz		2$					;;;
		loop	1$					;;;
2$:
      else
		mov		cl, al
      endif
		mov		al, 4				;;; command to reset C2: EXPWR
		mov		dx, SC_SB_LOWCTRL	;;;
		out		dx, al				;;;
		sti
      if WAIT_COUNT
		mov		ax, cx				; Non-zero count means saw ACK
      else
		mov		al, cl
		not		al
		and		ax, 1
      endif
		pop		dx
		pop		cx					; Restore CX
		ret
write_ack_ endp

else

;----------------------------------------------------------------
; SC104 version
;----------------------------------------------------------------
LIBFUNC write_ack_
		push	cx
		push	dx					; Save data: address is in AX
		push	dx
		mov		dx, SC_SB_LOWB
		mov		cx, WAIT_COUNT
		cli							;;;
		out		dx, ax				;;; Write out the address
		pop		ax					;;; Get the data
		mov		dx, SC_SB_LOWA		;;;
		out		dx, ax				;;; Write out the data
		mov		dx, SC_SB_LOWC		;;; Control/Status Port
1$:		in		ax, dx				;;; check for EXPACK/
		test	ax, 800H			;;; (EXPRD+EXPWR) bit
		jz		2$					;;;
		loop	1$					;;;
2$:		sti
		test	ax, 40H				; Check EXPACK bit
		jz		3$		; Jump if we didn't get an acknowledge
		inc		cx
		jmp		short 4$
3$:		; If we get here, we didn't get an acknowledge, so
		; we should simply return 0
		; We shouldn't have timed out, though (cx==0).
		; That would indicate a problem with the SC104.
		xor		cx, cx		; That's a zero
4$:		mov		ax, cx
		pop		dx
		pop		cx					; Restore CX
		ret
write_ack_ endp

endif

;----------------------------------------------------------------------
; set_cmdenbl(int val);
;   The LSB is the value
;----------------------------------------------------------------------
; Under WATCOM, val is passed in AX
;----------------------------------------------------------------------
LIBFUNC set_cmdenbl_
		push	dx
		mov		dx, SC_CMDENBL
		out		dx, al
		pop		dx
		ret
set_cmdenbl_ endp

;----------------------------------------------------------------------
; read_novram(unsigned int address)
;   I take some pains to make sure interrupts are disabled during the
;   read or write operation without blindly enabling them afterwards.
;   This is because I wish to call these routines during NMI shutdown
;   and I don't want any interrupts enabled during that time.
;----------------------------------------------------------------------
; Under WATCOM address is in AX
;----------------------------------------------------------------------
LIBFUNC read_novram_
    if NVRAM
		push	dx
		mov		dx, SC_RAMADDR
		pushf
		cli						;;;
      if SYSCON					;;;
		out		dx, ax			;;;
      else						;;;
		out		dx, al			;;;
      endif						;;;
		mov		dx, SC_RAMDATA	;;;
		in		al, dx			;;;
		popf
		xor		ah, ah
		pop		dx
    else
		mov		ax, 0FFH
    endif
		ret
read_novram_ endp

;----------------------------------------------------------------------
; write_novram(unsigned int address, unsigned char data)
;----------------------------------------------------------------------
; Under WATCOM, address is in AX, data is in DX
;----------------------------------------------------------------------
LIBFUNC write_novram_
    if NVRAM
		push	dx				; Save Data
		mov		dx, SC_RAMADDR
		pushf
		cli						;;;
      if SYSCON					;;;
		out		dx, ax			;;;
      else						;;;
		out		dx, al			;;;
      endif						;;;
		pop		ax 				;;;
		mov		dx, SC_RAMDATA	;;;
		out		dx, al			;;;
		popf
    endif
		ret
write_novram_ endp

;----------------------------------------------------------------------
; unsigned char read_switches()
;----------------------------------------------------------------------
LIBFUNC read_switches_
		mov		ax, 8000H	; Signals new functionality
    if READSWITCH
		push	dx
		mov		dx, SC_SWITCHES
		in		al, dx
		pop		dx
    endif
		ret
read_switches_ endp

;---------------------------------------------------------------------------
; lenable_nmi(void (far *shutdown)(void))
;---------------------------------------------------------------------------
INITFUNC lenable_nmi_
		; Later on, I'll figger out how to deal with the NMI.
	if SICFUNC
		; Write the enable out
		push	dx
		push	ax
		mov		dx, SC_NMI_ENABLE
		mov		al, SC_NMIE_VAL
		out		dx, al
		pop		ax
		pop		dx
    endif
	 	ret
lenable_nmi_ endp

;---------------------------------------------------------------------------
; disable_nmi(void);
;---------------------------------------------------------------------------
INITFUNC ldisable_nmi_
    if SICFUNC
		; Write the disable out.
		push	dx
		push	ax
		mov		dx, SC_NMI_ENABLE
		xor		al, al
		out		dx, al
		pop		ax
		pop		dx
		; Do something regarding vectors?
    endif
		ret
ldisable_nmi_ endp

;----------------------------------------------------------------------
; set_failure(int val)
;   val == 0 turns the lamp off.
;   val == 1 turns it on.
; With new SYSCONS, there are 8 bits of I/O.
;----------------------------------------------------------------------
; Under WATCOM val is in AX
;----------------------------------------------------------------------
LIBFUNC set_failure_
      if SIC
		push	dx
		mov		dx, SC_LAMP
		or		ax, ax
		jz		1$
		in		al, dx		; Read to turn if on
		jmp		short 2$
1$:		out		dx, al		; Write to turn it off
2$:		pop		dx
      endif
	  if SYSCON
		push	dx
		mov		dx, SC_LAMP
		out		dx, al
		pop		dx
	  endif
		ret
set_failure_ endp

;----------------------------------------------------------------------
; read_failure()
;----------------------------------------------------------------------
LIBFUNC read_failure_
	  if SYSCON
		push	dx
		mov		dx, SC_LAMP
		in		al, dx
		pop		dx
	  endif
		ret
read_failure_ endp

;----------------------------------------------------------------------
; read_rstcnt()  Return the hardware reset count and clear it.
;----------------------------------------------------------------------
LIBFUNC read_rstcnt_
      if SIC
		push	dx
		mov		dx, SC_RES_CNT
		in		al, dx
		and		ax, 0FH
		out		dx, al	; clear it
		pop		dx
      else
		xor		ax, ax
      endif
		ret
read_rstcnt_ endp

;----------------------------------------------------------------------
; read_pwrcnt()
;----------------------------------------------------------------------
LIBFUNC read_pwrcnt_
      if SIC
		push	dx
		mov		dx, SC_PWR_CNT
		in		al, dx
		and		ax, 0FH
		out		dx, al	; clear it
		pop		dx
      else
		xor		ax, ax
      endif
		ret
read_pwrcnt_ endp

;----------------------------------------------------------------------
; short int cmdstrobe(short int val)
;   val == 0 turns the cmdstrobe.
;   val == 1 turns on cmdstrobe.
;  Returns non-zero on success, zero if operation isn't supported.
;  Function did not exist at all before version 3.10, so
;  programs intending to use this function should verify that
;  the resident library version is at least 3.10. The feature
;  word can also be checked for support, and that is consistent
;  back to previous versions.
;----------------------------------------------------------------------
; Under WATCOM val is in AX
;----------------------------------------------------------------------
LIBFUNC cmdstrobe_
	if SYSCON
		push	dx
		or		ax, ax
	  if SC104
		mov		dx, SC_SB_LOWC
		mov		al, 8
		jnz		1$
		or		al, 2
	  else
		mov		dx, SC_SB_LOWCTRL
		mov		al, 2
		jz		1$
		or		al, 1
	  endif
1$:		out		dx, al
		pop		dx
	else
		xor		ax, ax
	endif
		ret
cmdstrobe_ endp

;----------------------------------------------------------------------
; No args. Called via message.
;----------------------------------------------------------------------
INITFUNC lltick_sic_
		push	dx
		mov		dx, SC_TICK
		out		dx, al
		pop		dx
		ret
lltick_sic_ endp

;----------------------------------------------------------------------
; No args. Called via message.
;----------------------------------------------------------------------
INITFUNC ldisarm_sic_
		push	dx
		mov		dx, SC_DISARM
		out		dx, al
		pop		dx
		ret
ldisarm_sic_ endp

;----------------------------------------------------------------------
; Critical status is in AX
;----------------------------------------------------------------------
INITFUNC lreboot_
		cli						;;;
		mov		dx, SC_DISARM	;;; Disarm Reset long enough to shutdown
		out		dx, al			;;;
		mov		dx, ax			;;; Critical Status
		xor		ax, ax			;;;
		call	write_novram_	;;;
		mov		dx, SC_TICK		;;; Tick to trigger a reset just in case
		out		dx, al			;;; power didn't really fail
1$:		hlt						;;; There is no recovery.
		jmp		1$				;;; Really.
lreboot_ endp

INITFUNC init_subbus_
		;-----------------------------------------------------------------
		; <initialize the subbus>
		;-----------------------------------------------------------------
		push	ax
		push	dx
		mov		dx, SC_SB_RESET
		out		dx, ax
	  if SC104 EQ 0
		mov		dx, SC_SB_LOWCTRL
		mov		ax, SC_SB_CONFIG
      if SYSCON
		out		dx, ax
      else
		out		dx, al
		mov		dx, SC_SB_HIGHCTRL
		mov		al, ah
		out		dx, al
      endif
	  endif
		pop		dx
		pop		ax
		ret
init_subbus_ endp
ENDPS
end
