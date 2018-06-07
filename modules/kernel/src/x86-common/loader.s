;====================================================================================================================
;
;  kernel/src/x86-common/loader.s -- Common loader for the Intel based-kernels
;
;  Based on the multiboot specification, the multiboot loader will hand off control to the following file at the 
;  loader location.
;
; ------------------------------------------------------------------------------------------------------------------
;                                                                                                                 
;     Date     Tracker  Version  Pgmr  Description                                                                         
;  ----------  -------  -------  ----  ---------------------------------------------------------------------------
;  2012-05-13  Initial           ADCL  Initial version
;  2012-09-15                    ADCL  Copied from Century
;  2018-05-25  Initial   0.1.0   ADCL  Copied from century32 to century-os
;
;====================================================================================================================


;
; -- Expose some labels to other fucntions that the linker can pick up
;    -----------------------------------------------------------------
global		_start					                        ; make _start visible
global		Halt					                        ; make hang visible


;
; -- Now, we need some things from other functions imported
;    ------------------------------------------------------
extern		kInit					                        ; allow reference to external kinit
extern      gdt												; the gdt structure
extern      kMemMove										; the memory move function
extern      BuildIdt                                        ; build the IDT table


;
; -- Define some constants for the boot loader specification
;    -------------------------------------------------------
MODULEALIGN	equ		1<<0			                        ; align loaded on page boundaries
MEMINFO		equ		1<<1			                        ; provide memory map
VIDEOINFO	equ		1<<2			                        ; provide video information

FLAGS		equ		MODULEALIGN | MEMINFO | VIDEOINFO 		; this is the multiboot flag
MAGIC		equ		0x1badb002		                        ; magic number for the header to be found
CHECKSUM	equ		- (MAGIC + FLAGS)	                    ; check number


;
; -- Setup the initial kernel stack
;    ------------------------------
STACKSIZE	equ		0x4000                                  ; that's 16K


;
; -- The __mbheader section is required to be in the first 16?K of the final executable.  This section is organized 
;    by the linker.ld linker script to be at the beginning of the executable.
;    --------------------------------------------------------------------------------------------------------------
section .mbheader
align 4
    dd		MAGIC			                                ; this is the real header block in the seg
    dd		FLAGS			                                ; flags of info to provide
    dd		CHECKSUM		                                ; the proper checksum for the mb spec
    dd		0, 0, 0, 0, 0
    dd		1				                                ; EGA text mode
    dd		80				                                ; 80 columns
    dd		25				                                ; 25 lines
    dd		8				                                ; color depth


;
; -- the structure needed to load the GDT table -- this is temporary and once loaded the data space can go away
;    ----------------------------------------------------------------------------------------------------------
GdtDesc:
    dw      (16*8)-1                        				; size minus one...
    dd      0                               				; this is the addresss


;
; -- the structure needed to load the IDT table -- this is temporary and once loaded the data space can go away
;    ----------------------------------------------------------------------------------------------------------
IdtDesc:
    dw      (256*8)-1                        				; size minus one...  or 0x7ff
    dd      0x800                              				; this is the addresss


;
; -- This is the beginning of the code segment for this file
;    -------------------------------------------------------
section .text
align 4
cpu		586


;--------------------------------------------------------------------------------------------------------------------
; loader() --  Entry point for the kernel which is called implicitly by the boot loader
;--------------------------------------------------------------------------------------------------------------------
_start:
loader:
; load the GDT; copy to address 0x00000000 first
    push	eax												; save the magic number
    push    ebx												; and the mbi address

    push    (16*8)											; the number of bytes to move
    lea     eax,[gdt]										; the gdt to move
    push    eax												; ... the next argument
    push    0												; the target address we will move the GDT to
    call    kMemMove										; move the block of memory to the target location
    add     esp,12											; clean up the stack

    pop     ebx												; restore the mbi address
    pop     eax												; restore the magic number

; load the address into the gdt register
    mov     ecx,GdtDesc                   					; Get the gdt address
    lgdt    [ecx]					                        ; and load the GDT

; set up all the segment selector registers and the new stack
    mov     ecx,0x02<<3			    						; set the value to load -- we will use kernel data
    mov     ds,cx
    mov     es,cx
    mov     fs,cx
    mov     gs,cx
    mov     ss,cx                             				; halts ints (if enabled) for one more instr to set stack
    mov     esp,0x200000    			                  	; set up a stack
    
    push    0x01<<3	    									; The kernel code selector
    push    newGDT   										; the instruction to which to return
    retf                                        			; an almost jump, returning to an addr rather than jump

newGDT:
	cli								                        ; just in case, no interupts please
	mov		esp,stack+STACKSIZE		                        ; setup stack
	push	eax						                        ; push multiboot magic number
	push	ebx						                        ; push multiboot info struct
	
    call    BuildIdt                                        ; go build the IDT table in-place

; load the address into the idt register
    mov     ecx,IdtDesc                   					; Get the idt address
    lidt    [ecx]					                        ; and load the IDT

	call	kInit					                        ; call the C initialization function (passes eax and ebx)

;
; -- if you get back here, then you have a problem and need to hang; fall through to the next function...
;    ----------------------------------------------------------------------------------------------------


;--------------------------------------------------------------------------------------------------------------------
; Halt() -- This is a kernel panic function that will hang the system
;--------------------------------------------------------------------------------------------------------------------
Halt:
	cli								                        ; stop all interrupts

;
; -- The following are all commented out until a TTY terminal is prepared
;    --------------------------------------------------------------------
;	push	ebp						                        ; create a frame pointer for debugging
;	mov		ebp,esp					                        ; ... for debugging!

;	push	0						                        ; we want TTY0
;	call	SetTTY					                        ; make that tty term visible
;	add		esp,4					                        ; clean up the stack

;	push	dword 0x40				                        ; push the color
;	push	dword 0x58				                        ; push the character
;	push	dword 0x00				                        ; push the column
;	call	kStatusChar				                        ; poke the red X on the screen
;	cli								                        ; make sure that ints stay disabled

;
; -- Halt the cpu
;    ------------
	hlt								                        ; force a hard reboot
	jmp		Halt					                        ; loop, just in case


;
; -- This is the begging of the bss segment (0 filled)
;    -------------------------------------------------
section .bss
align 4

stack:
	resb	STACKSIZE				                        ; reserve 16K for a stack
