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
global		hang					                        ; make hang visible


;
; -- Now, we need some things from other functions imported
;    ------------------------------------------------------
extern		kInit					                        ; allow reference to external kinit


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
	cli								                        ; just in case, no interupts please
	mov		esp,stack+STACKSIZE		                        ; setup stack
	push	eax						                        ; push multiboot magic number
	push	ebx						                        ; push multiboot info struct
	
	call	kInit					                        ; call the C initialization function

;
; -- if you get back here, then you have a problem and need to hang; fall through to the next function...
;    ----------------------------------------------------------------------------------------------------


;--------------------------------------------------------------------------------------------------------------------
; hang() -- This is a kernel panic function that will hang the system
;--------------------------------------------------------------------------------------------------------------------
hang:
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
	jmp		hang					                        ; loop, just in case


;
; -- This is the begging of the bss segment (0 filled)
;    -------------------------------------------------
section .bss
align 4

stack:
	resb	STACKSIZE				                        ; reserve 16K for a stack
