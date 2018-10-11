;====================================================================================================================
;
; kernel/src/i686/IsrCommonStub.s -- Common code for handling all interrupts
;
; Provides the common code for handling all interrupts
;
; ------------------------------------------------------------------------------------------------------------------
;
;     Date     Tracker  Version  Pgmr  Description
;  ----------  -------  -------  ----  ---------------------------------------------------------------------------
;  2012-05-28                          Initial Version
;  2012-06-09    #35                   Fix sflags, cr0, and cr3 in core dump
;  2012-09-16                          Leveraged from Century
;  2018-05-29  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
;
;====================================================================================================================


;
; -- Now, we need some things from other functions imported
;    ------------------------------------------------------
extern	IsrHandler


;
; -- This is the beginning of the code segment for this file
;    -------------------------------------------------------
section .text
cpu		586


;
; -- This is a macro to for an interrupt that does not contain an error code
;    -----------------------------------------------------------------------
%macro ISR_NOERRORCODE 1
global		isr%1			                                ; make the label available to other modules

isr%1:
    cli                                                     ; clear interrupts just in case
	push	dword 0				                            ; since no error was pushed, align the stack
	push	dword 0				                            ; no need to ack IRQ
	push	dword %1			                            ; push the interrupt number
	jmp		IsrCommonStub		                            ; jump to the common handler (below)
%endmacro


;
; -- This is a macro to for an interrupt that contains an error code
;    ---------------------------------------------------------------
%macro ISR_ERRORCODE 1
global		isr%1			                                ; make the label available to other modules

isr%1:
	cli							                            ; clear interrupts (just in case)
								                            ; remember, error code was pushed...
	push	dword 0				                            ; no need to ack IRQ
	push	dword %1			                            ; push the interrupt number
	jmp		IsrCommonStub		                            ; jump to the common handler (below)
%endmacro


;
; -- This is a macro to for an IRQ
;    -----------------------------
%macro IRQ_HANDLER 2
								                            ;    parm 1 is the IRQ number
								                            ;    parm 2 is the ISR number
global		irq%1			                                ; make the label available to other modules

irq%1:
	cli							                            ; clear interrupts
	push	dword %1			                            ; since no error was pushed, use it for IRQ#
	push	dword 1				                            ; assume we need to ackIRQ
	push	dword %2			                            ; push the ISR number to use for this IRQ
	jmp		IsrCommonStub		                            ; jump to the common handler (below)
%endmacro


;
; -- Now, we put the macros to use to define the interrupt entry points
;    ------------------------------------------------------------------
ISR_NOERRORCODE 0				                    		; define isr0
ISR_NOERRORCODE 1				                    		; define isr1
ISR_NOERRORCODE 2				                    		; define isr2
ISR_NOERRORCODE 3				                    		; define isr3
ISR_NOERRORCODE 4				                    		; define isr4
ISR_NOERRORCODE 5				                    		; define isr5
ISR_NOERRORCODE 6				                    		; define isr6
ISR_NOERRORCODE 7				                    		; define isr7
ISR_ERRORCODE 8					                    		; define isr8
ISR_NOERRORCODE 9				                    		; define isr9
ISR_ERRORCODE 10				                    		; define isr10
ISR_ERRORCODE 11				                    		; define isr11
ISR_ERRORCODE 12				                    		; define isr12
ISR_ERRORCODE 13				                    		; define isr13
ISR_ERRORCODE 14				                    		; define isr14
ISR_NOERRORCODE 15				                    		; define isr15
ISR_NOERRORCODE 16				                    		; define isr16
ISR_NOERRORCODE 17				                    		; define isr17
ISR_NOERRORCODE 18				                    		; define isr18
ISR_NOERRORCODE 19				                    		; define isr19
ISR_NOERRORCODE 20				                    		; define isr20
ISR_NOERRORCODE 21				                    		; define isr21
ISR_NOERRORCODE 22				                    		; define isr22
ISR_NOERRORCODE 23				                    		; define isr23
ISR_NOERRORCODE 24				                    		; define isr24
ISR_NOERRORCODE 25				                    		; define isr25
ISR_NOERRORCODE 26				                    		; define isr26
ISR_NOERRORCODE 27				                    		; define isr27
ISR_NOERRORCODE 28				                    		; define isr28
ISR_NOERRORCODE 29				                    		; define isr29
ISR_NOERRORCODE 30				                    		; define isr30
ISR_NOERRORCODE 31				                    		; define isr31

IRQ_HANDLER 0,32				                    		; define irq0
IRQ_HANDLER 1,33				                    		; define irq1
IRQ_HANDLER 2,34				                    		; define irq2
IRQ_HANDLER 3,35				                    		; define irq3
IRQ_HANDLER 4,36				                    		; define irq4
IRQ_HANDLER 5,37				                    		; define irq5
IRQ_HANDLER 6,38				                    		; define irq6
IRQ_HANDLER 7,39				                    		; define irq7
IRQ_HANDLER 8,40				                    		; define irq8
IRQ_HANDLER 9,41				                    		; define irq9
IRQ_HANDLER 10,42				                    		; define irq10
IRQ_HANDLER 11,43				                    		; define irq11
IRQ_HANDLER 12,44				                    		; define irq12
IRQ_HANDLER 13,45				                    		; define irq13
IRQ_HANDLER 14,46				                    		; define irq14
IRQ_HANDLER 15,47		                            		; define irq15


;
; -- build the stack in the following order
;     interrupt frame
;     error code and parms
;     all regs
;     ds
;     es
;     fs
;     gs
;    --------------------------------------
IsrCommonStub:
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	esp
	push	ebp
	push	esi
	push	edi

	mov		eax,cr0				                            ; get cr0
	push	eax					                            ; and push it
	mov		eax,cr2				                            ; get cr2
	push	eax					                            ; and push it
	mov		eax,cr3				                            ; get cr3
	push	eax					                            ; and push it

	xor		eax,eax				                            ; clear eax for work on other seg regs
	mov		ax,ds				                            ; get ds
	push	eax					                            ; and push it
	mov		ax,es				                            ; get es
	push	eax					                            ; and push it
	mov		ax,fs				                            ; get fs
	push	eax					                            ; and push it
	mov		ax,gs				                            ; get gs
	push	eax					                            ; and push it
	mov		ax,ss				                            ; get ss
	push	eax					                            ; and push it

	mov		ax,0x10				                            ; now make sure that data is setup for ring0
	mov		ds,ax				                            ; move it to ds, ...
	mov		es,ax				                            ; ... es, ...
	mov		fs,ax				                            ; ... fs, and ...
	mov		gs,ax				                            ; ... gs

	call	IsrHandler			                            ; call the C ISR haandler

	pop		eax					                            ; pop ss and discard
	pop		eax					                            ; pop gs
	mov		gs,ax				                            ; and set it
	pop		eax					                            ; pop fs
	mov		fs,ax				                            ; and set it
	pop		eax					                            ; pop es
	mov		es,ax				                            ; and set it
	pop		eax					                            ; pop ds
	mov		ds,ax				                            ; and set it

	add		esp,12				                            ; skip past cr0, cr2, and cr3

	pop		edi
	pop		esi
	pop		ebp
	pop		eax					                            ; throw away this value it was esp
	pop		ebx
	pop		edx
	pop		ecx
	pop		eax

	add		esp,12				                            ; skip past the parameters
	iret						                            ; pop flags and return
