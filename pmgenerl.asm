;
;	General routines - FRACTINT for PM version.
;
;	Multiply/Divide from WGENERAL.ASM (WINFRACT)
;	cputype/fputype from GENERAL.ASM (FRACTINT for DOS)
;	toextra/fromextra/cmpextra from GENERAL.ASM (FRACTINT for DOS)
;
;
;	Generic assembler routines that have very little at all
;       to do with fractals.
;
;       (NOTE:  The video routines have been moved over to VIDEO.ASM)
;
;
; ---- 32-bit Multiply/Divide Routines (includes 16-bit emulation)
;
;       multiply()
;       divide()
;
; ---- CPU, FPU Detectors
;
;       cputype()
;	fputype()
;
; ---- Quick-copy to/from Extraseg support
;
;	toextra()
;	fromextra()
;	cmpextra()
;

.MODEL	medium,c

.8086

.DATA


; ************************ Public variables *****************************

public		overflow		; Mul, Div overflow flag: 0 means none

;		arrays declared here, used elsewhere
;		arrays not used simultaneously are deliberately overlapped

public		prefix, suffix, dstack, decoderline ; for the Decoder
public		strlocn, teststring, block	; used by the Encoder
public		boxx, boxy, boxvalues		; zoom-box arrays
public		olddacbox			; temporary DAC saves
public		rlebuf				; Used ty the TARGA En/Decoder

.DATA

; ************************* "Shared" array areas **************************

; Short forms used in subsequent comments:
;   name........duration of use......................modules....................
;   encoder	"s"aving an image                    encoder.c
;   decoder	"r"estoring an image                 decoder.c, gifview.c
;   zoom	zoom box is visible		     zoom.c, video.asm
;   vidswitch	temp during video mode setting	     video.asm
;   vidreset	temp during video reset 	     prompts.c, fractint.c, rotate.c, cmdfiles.c
;   tgaview	restore of tga image		     tgaview.c
;   solidguess	image gen with "g", not to disk      calcfrac.c
;   btm 	image gen with "b", not to disk      calcfrac.c

block		label	byte		; encoder(266)
suffix		dw	2048 dup(0)	; decoder(4k), vidswitch(256)

teststring	label	byte		; encoder(100)
dstack		dw	2048 dup(0)	; decoder(4k), solidguess(4k), btm(2k)
					;   zoom(2k)

strlocn 	label	word		; encoder(10k)
prefix		label	word		; decoder(8k), solidguess(6k)
olddacbox	label	byte		; vidreset(768)
boxx		dw	2048 dup(0)	; zoom(4k), tgaview(4k)
boxy		dw	2048 dup(0)	; zoom(4k)
boxvalues	label	byte		; zoom(2k)
decoderline	db	2050 dup(0)	; decoder(2049), btm(2k)

rlebuf		db	258 dup(0)	; f16.c(258) .tga save/restore?

; Internal Overflow flag

overflow	dw	0		; overflow flag

.CODE


; =======================================================
;
;	32-bit integer multiply routine with an 'n'-bit shift.
;	Overflow condition returns 0x7fffh with overflow = 1;
;
;	long x, y, z, multiply();
;	int n;
;
;	z = multiply(x,y,n)
;
;	requires the presence of an external variable, 'cpu'.
;		'cpu' == 386 if a 386 is present.

;.MODEL  medium,c

.8086

.DATA

temp	dw	5 dup(0)		; temporary 64-bit result goes here
sign	db	0			; sign flag goes here

.CODE

multiply	proc	x:dword, y:dword, n:word

	cmp	cpu,386 		; go-fast time?
	jne	slowmultiply		; no.  yawn...

.386					; 386-specific code starts here

	mov	eax,x			; load X into EAX
	imul	y			; do the multiply
	mov	cx,n			; set up the shift
	cmp	cx,32			; ugly klooge:	check for 32-bit shift
	jb	short fastm1		;  < 32 bits:  no problem
	mov	eax,edx 		;  >= 32 bits:	manual shift
	mov	edx,0			;  ...
	sub	cx,32			;  ...
fastm1: shrd	eax,edx,cl		; shift down 'n' bits
	js	fastm3
	sar	edx,cl
	jne	overmf
	shld	edx,eax,16
	ret
fastm3: sar	edx,cl
	inc	edx
	jne	overmf
	shld	edx,eax,16
	ret
overmf:
	mov	ax,0ffffh		; overflow value
	mov	dx,07fffh		; overflow value
	mov	overflow,1		; flag overflow
	ret

.8086					; 386-specific code ends here

slowmultiply:				; (sigh)  time to do it the hard way...
	push	di
	push	si

	mov	ax,0
	mov	temp+4,ax		; first, zero out the (temporary)
	mov	temp+6,ax		;  result
	mov	temp+8,ax

	mov	bx,word ptr x		; move X to SI:BX
	mov	si,word ptr x+2		;  ...
	mov	cx,word ptr y		; move Y to DI:CX
	mov	di,word ptr y+2		;  ...

	mov	sign,0			; clear out the sign flag
	cmp	si,0			; is X negative?
	jge	mults1			;  nope
	not	sign			;  yup.  flip signs
	not	bx			;   ...
	not	si			;   ...
	stc				;   ...
	adc	bx,ax			;   ...
	adc	si,ax			;   ...
mults1: cmp	di,0			; is DI:CX negative?
	jge	mults2			;  nope
	not	sign			;  yup.  flip signs
	not	cx			;   ...
	not	di			;   ...
	stc				;   ...
	adc	cx,ax			;   ...
	adc	di,ax			;   ...
mults2:

	mov	ax,bx			; perform BX x CX
	mul	cx			;  ...
	mov	temp,ax 		;  results in lowest 32 bits
	mov	temp+2,dx		;  ...

	mov	ax,bx			; perform BX x DI
	mul	di			;  ...
	add	temp+2,ax		;  results in middle 32 bits
	adc	temp+4,dx		;  ...
	jnc	mults3			;  carry bit set?
	inc	word ptr temp+6 	;  yup.  overflow
mults3:

	mov	ax,si			; perform SI * CX
	mul	cx			;  ...
	add	temp+2,ax		;  results in middle 32 bits
	adc	temp+4,dx		;  ...
	jnc	mults4			;  carry bit set?
	inc	word ptr temp+6 	;  yup.  overflow
mults4:

	mov	ax,si			; perform SI * DI
	mul	di			;  ...
	add	temp+4,ax		; results in highest 32 bits
	adc	temp+6,dx		;  ...

	mov	cx,n			; set up for the shift loop
	cmp	cx,24			; shifting by three bytes or more?
	jl	multc1			;  nope.  check for something else
	sub	cx,24			; quick-shift 24 bits
	mov	ax,temp+3		; load up the registers
	mov	dx,temp+5		;  ...
	mov	si,temp+7		;  ...
	mov	bx,0			;  ...
	jmp	short multc4		; branch to common code
multc1: cmp	cx,16			; shifting by two bytes or more?
	jl	multc2			;  nope.  check for something else
	sub	cx,16			; quick-shift 16 bits
	mov	ax,temp+2		; load up the registers
	mov	dx,temp+4		;  ...
	mov	si,temp+6		;  ...
	mov	bx,0			;  ...
	jmp	short multc4		; branch to common code
multc2: cmp	cx,8			; shifting by one byte or more?
	jl	multc3			;  nope.  check for something else
	sub	cx,8			; quick-shift 8 bits
	mov	ax,temp+1		; load up the registers
	mov	dx,temp+3		;  ...
	mov	si,temp+5		;  ...
	mov	bx,temp+7		;  ...
	jmp	short multc4		; branch to common code
multc3: mov	ax,temp 		; load up the regs
	mov	dx,temp+2		;  ...
	mov	si,temp+4		;  ...
	mov	bx,temp+6		;  ...
multc4: cmp	cx,0			; done shifting?
	je	multc5			;  yup.  bail out

multloop:
	shr	bx,1			; shift down 1 bit, cascading
	rcr	si,1			;  ...
	rcr	dx,1			;  ...
	rcr	ax,1			;  ...
	loop	multloop		; try the next bit, if any
multc5:
	cmp	si,0			; overflow time?
	jne	overm1			; yup.	Bail out.
	cmp	bx,0			; overflow time?
	jne	overm1			; yup.	Bail out.
	cmp	dx,0			; overflow time?
	jl	overm1			; yup.	Bail out.

	cmp	sign,0			; should we negate the result?
	je	mults5			;  nope.
	not	ax			;  yup.  flip signs.
	not	dx			;   ...
	mov	bx,0			;   ...
	stc				;   ...
	adc	ax,bx			;   ...
	adc	dx,bx			;   ...
mults5:
	jmp	multiplyreturn

overm1:
	mov	ax,0ffffh		; overflow value
	mov	dx,07fffh		; overflow value
	mov	overflow,1		; flag overflow

multiplyreturn: 			; that's all, folks!
	pop	si
	pop	di
	ret
multiply	endp


; =======================================================
;
;	32-bit integer divide routine with an 'n'-bit shift.
;	Overflow condition returns 0x7fffh with overflow = 1;
;
;	long x, y, z, divide();
;	int n;
;
;	z = divide(x,y,n);	/* z = x / y; */
;
;	requires the presence of an external variable, 'cpu'.
;		'cpu' == 386 if a 386 is present.


.8086

divide		proc	x:dword, y:dword, n:word

	push	di
	push	si

	cmp	cpu,386 		; go-fast time?
	jne	slowdivide		; no.  yawn...

.386					; 386-specific code starts here

	mov	edx,x			; load X into EDX (shifts to EDX:EAX)
	mov	ebx,y			; load Y into EBX

	mov	sign,0			; clear out the sign flag
	cmp	edx,0			; is X negative?
	jge	short divides1		;  nope
	not	sign			;  yup.  flip signs
	neg	edx			;   ...
divides1:
	cmp	ebx,0			; is Y negative?
	jge	short divides2		;  nope
	not	sign			;  yup.  flip signs
	neg	ebx			;   ...
divides2:

	mov	eax,0			; clear out the low-order bits
	mov	cx,32			; set up the shift
	sub	cx,n			; (for large shift counts - faster)
fastd1: cmp	cx,0			; done shifting?
	je	fastd2			; yup.
	shr	edx,1			; shift one bit
	rcr	eax,1			;  ...
	loop	fastd1			; and try again
fastd2:
	cmp	edx,ebx 		; umm, will the divide blow out?
	jae	overd1			;  yup.  better skip it.
	div	ebx			; do the divide
	cmp	eax,0			; did the sign flip?
	jl	overd1			;  then we overflowed
	cmp	sign,0			; is the sign reversed?
	je	short divides3		;  nope
	neg	eax			; flip the sign
divides3:
	push	eax			; save the 64-bit result
	pop	ax			; low-order  16 bits
	pop	dx			; high-order 16 bits
	jmp	dividereturn		; back to common code

.8086					; 386-specific code ends here

slowdivide:				; (sigh)  time to do it the hard way...

	mov	ax,word ptr x		; move X to DX:AX
	mov	dx,word ptr x+2		;  ...

	mov	sign,0			; clear out the sign flag
	cmp	dx,0			; is X negative?
	jge	divides4		;  nope
	not	sign			;  yup.  flip signs
	not	ax			;   ...
	not	dx			;   ...
	stc				;   ...
	adc	ax,0			;   ...
	adc	dx,0			;   ...
divides4:

	mov	cx,32			; get ready to shift the bits
	sub	cx,n			; (shift down rather than up)
	mov	byte ptr temp+4,cl	;  ...

	mov	cx,0			;  clear out low bits of DX:AX:CX:BX
	mov	bx,0			;  ...

	cmp	byte ptr temp+4,16	; >= 16 bits to shift?
	jl	dividex0		;  nope
	mov	bx,cx			;  yup.  Take a short-cut
	mov	cx,ax			;   ...
	mov	ax,dx			;   ...
	mov	dx,0			;   ...
	sub	byte ptr temp+4,16	;   ...
dividex0:
	cmp	byte ptr temp+4,8	; >= 8 bits to shift?
	jl	dividex1		;  nope
	mov	bl,bh			;  yup.  Take a short-cut
	mov	bh,cl			;   ...
	mov	cl,ch			;   ...
	mov	ch,al			;   ...
	mov	al,ah			;   ...
	mov	ah,dl			;   ...
	mov	dl,dh			;   ...
	mov	dh,0			;   ...
	sub	byte ptr temp+4,8	;   ...
dividex1:
	cmp	byte ptr temp+4,0	; are we done yet?
	je	dividex2		;  yup
	shr	dx,1			; shift all 64 bits
	rcr	ax,1			;  ...
	rcr	cx,1			;  ...
	rcr	bx,1			;  ...
	dec	byte ptr temp+4 	; decrement the shift counter
	jmp	short dividex1		;  and try again
dividex2:

	mov	di,word ptr y		; move Y to SI:DI
	mov	si,word ptr y+2		;  ...

	cmp	si,0			; is Y negative?
	jge	divides5		;  nope
	not	sign			;  yup.  flip signs
	not	di			;   ...
	not	si			;   ...
	stc				;   ...
	adc	di,0			;   ...
	adc	si,0			;   ...
divides5:

	mov	byte ptr temp+4,33	; main loop counter
	mov	temp,0			; results in temp
	mov	word ptr temp+2,0	;  ...

dividel1:
	shl	temp,1			; shift the result up 1
	rcl	word ptr temp+2,1	;  ...
	cmp	dx,si			; is DX:AX >= Y?
	jb	dividel3		;  nope
	ja	dividel2		;  yup
	cmp	ax,di			;  maybe
	jb	dividel3		;  nope
dividel2:
	cmp	byte ptr temp+4,32	; overflow city?
	jge	overd1			;  yup.
	sub	ax,di			; subtract Y
	sbb	dx,si			;  ...
	inc	temp			; add 1 to the result
	adc	word ptr temp+2,0	;  ...
dividel3:
	shl	bx,1			; shift all 64 bits
	rcl	cx,1			;  ...
	rcl	ax,1			;  ...
	rcl	dx,1			;  ...
	dec	byte ptr temp+4 	; time to quit?
	jnz	dividel1		;  nope.  try again.

	mov	ax,temp 		; copy the result to DX:AX
	mov	dx,word ptr temp+2	;  ...
	cmp	sign,0			; should we negate the result?
	je	divides6		;  nope.
	not	ax			;  yup.  flip signs.
	not	dx			;   ...
	mov	bx,0			;   ...
	stc				;   ...
	adc	ax,0			;   ...
	adc	dx,0			;   ...
divides6:
	jmp	short dividereturn

overd1:
	mov	ax,0ffffh		; overflow value
	mov	dx,07fffh		; overflow value
	mov	overflow,1		; flag overflow

dividereturn:				; that's all, folks!
	pop	si
	pop	di
	ret
divide		endp

;===============================================================
;
; CPUTYPE.ASM : C-callable functions cputype() and ndptype() adapted
; by Lee Daniel Crocker from code appearing in the late PC Tech Journal,
; August 1987 and November 1987.  PC Tech Journal was a Ziff-Davis
; Publication.  Code herein is copyrighted and used with permission.
;
; The function cputype() returns an integer value based on what kind
; of CPU it found, as follows:
;
;       Value   CPU Type
;       =====   ========
;       86      8086, 8088, V20, or V30
;       186     80186 or 80188
;       286     80286
;       386     80386 or 80386sx
;       -286    80286 in protected mode
;       -386    80386 or 80386sx in protected or 32-bit address mode
;
; The function ndptype() returns an integer based on the type of NDP
; it found, as follows:
;
;       Value   NDP Type
;       =====   ========
;       0       No NDP found
;       87      8087
;       287     80287
;       387     80387
;
; No provisions are made for the 80486 CPU/FPU or Weitek FPA chips.
;
; Neither function takes any arguments or affects any external storage,
; so there should be no memory-model dependencies.

;.model medium, c

.DATA
public		cpu,fpu

		align	2
cpu		dw	0		; cpu type: 86, 186, 286, or 386
fpu		dw	0		; fpu type: 0, 87, 287, 387

.286P
.code

cputype proc
        push    bp

        push    sp                      ; 86/186 will push SP-2;
        pop     ax                      ; 286/386 will push SP.
        cmp     ax, sp
        jz      not86                   ; If equal, SP was pushed
        mov     ax, 186
        mov     cl, 32                  ;   186 uses count mod 32 = 0;
        shl     ax, cl                  ;   86 shifts 32 so ax = 0
        jnz     exit                    ; Non-zero: no shift, so 186
        mov     ax, 86                  ; Zero: shifted out all bits
        jmp     short exit
not86:
        pushf                           ; Test 16 or 32 operand size:
        mov     ax, sp                  ;   Pushed 2 or 4 bytes of flags?
        popf
        inc     ax
        inc     ax
        cmp     ax, sp                  ;   Did pushf change SP by 2?
        jnz     is32bit                 ;   If not, then 4 bytes of flags
is16bit:
        sub     sp, 6                   ; Is it 286 or 386 in 16-bit mode?
        mov     bp, sp                  ; Allocate stack space for GDT pointer
        sgdt    fword ptr [bp]
        add     sp, 4                   ; Discard 2 words of GDT pointer
        pop     ax                      ; Get third word
        inc     ah                      ; 286 stores -1, 386 stores 0 or 1
        jnz     is386
is286:
        mov     ax, 286
        jmp     short testprot          ; Check for protected mode
is32bit:
        db      66h                     ; 16-bit override in 32-bit mode
is386:
        mov     ax, 386
testprot:
        smsw    cx                      ; Protected?  Machine status -> CX
        ror     cx,1                    ; Protection bit -> carry flag
        jnc     exit                    ; Real mode if no carry
        neg     ax                      ; Protected:  return neg value
exit:
        pop     bp
        ret
cputype endp

.data

control dw      0                       ; Temp storage for 8087 control
                                        ;   and status registers
.code

fputype proc
        push    bp

        fninit                          ; Defaults to 64-bit mantissa
        mov     byte ptr control+1, 0
        fnstcw  control                 ; Store control word over 0
;       dw      3ed9h                   ; (klooge to avoid the MASM \e switch)
;       dw      offset control          ; ((equates to the above 'fnstcw' cmd))
        mov     ah, byte ptr control+1  ; Test contents of byte written
        cmp     ah, 03h                 ; Test for 64-bit precision flags
        je      gotone                  ; Got one!  Now let's find which
        xor     ax, ax
        jmp     short fexit             ; No NDP found
gotone:
        and     control, not 0080h      ; IEM = 0 (interrupts on)
        fldcw   control
        fdisi                           ; Disable ints; 287/387 will ignore
        fstcw   control
        test    control, 0080h
        jz      not87                   ; Got 287/387; keep testing
        mov     ax, 87
        jmp     short fexit
not87:
        finit
        fld1
        fldz
        fdiv                            ; Divide 1/0 to create infinity
        fld     st
        fchs                            ; Push -infinity on stack
        fcompp                          ; Compare +-infinity
        fstsw   control
        mov     ax, control
        sahf
        jnz     got387                  ; 387 will compare correctly
        mov     ax, 287
        jmp     short fexit
got387:                                 ; Only one left (until 487/Weitek
        mov     ax, 387                 ;   test is added)
fexit:
        pop     bp
        ret
fputype endp

.DATA
public		extraseg

extraseg	dw	0		; extra 64K segment (allocated by init)

.CODE

; *************** Function toextra(tooffset,fromaddr, fromcount) *********

toextra proc	uses es di si, tooffset:word, fromaddr:word, fromcount:word
	cmp	extraseg,0		; IS there extra memory?
	je	tobad			;  nope.  too bad.
	cld				; move forward
	mov	ax,extraseg		; load ES == extra segment
	mov	es,ax			;  ..
	mov	di,tooffset		; load to here
	mov	si,fromaddr		; load from here
	mov	cx,fromcount		; this many bytes
	rep	movsb			; do it.
tobad:
	ret				; we done.
toextra endp


; *************** Function fromextra(fromoffset, toaddr, tocount) *********

fromextra proc	uses es di si, fromoffset:word, toaddr:word, tocount:word
	push	ds			; save DS for a tad
	pop	es			; restore it to ES
	cmp	extraseg,0		; IS there extra memory?
	je	frombad 		;  nope.  too bad.
	cld				; move forward
	mov	si,fromoffset		; load from here
	mov	di,toaddr		; load to here
	mov	cx,tocount		; this many bytes
	mov	ax,extraseg		; load DS == extra segment
	mov	ds,ax			;  ..
	rep	movsb			; do it.
frombad:
	push	es			; save ES again.
	pop	ds			; restore DS
	ret				; we done.
fromextra endp


; *************** Function cmpextra(cmpoffset,cmpaddr, cmpcount) *********

cmpextra proc	uses es di si, cmpoffset:word, cmpaddr:word, cmpcount:word
	cmp	extraseg,0		; IS there extra memory?
	je	cmpbad			;  nope.  too bad.
	cld				; move forward
	mov	ax,extraseg		; load ES == extra segment
	mov	es,ax			;  ..
	mov	di,cmpoffset		; load to here
	mov	si,cmpaddr		; load from here
	mov	cx,cmpcount		; this many bytes
	repe	cmpsb			; do it.   ****** DPE MASM 6.00 *****
	jnz	cmpbad			; failed.
	mov	ax,0			; 0 == true
	jmp	cmpend
cmpbad:
	mov	ax,1			; 1 == false
cmpend:
	ret				; we done.
cmpextra	endp

	end
