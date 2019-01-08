;;===================================================================================================================
;;
;;  loader.s -- Common loader for the Intel based-kernels
;;
;;        Copyright (c)  2017-2019 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Based on the multiboot specification, the multiboot loader will hand off control to the following file at the
;;  loader location.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;    Date      Tracker  Version  Pgmr  Description
;; -----------  -------  -------  ----  ---------------------------------------------------------------------------
;; 2012-May-13  Initial           ADCL  Initial version
;; 2012-Sep-15                    ADCL  Copied from Century
;; 2018-May-25  Initial   0.1.0   ADCL  Copied from century32 to century-os
;; 2018-Jul-01  Initial   0.1.0   ADCL  Remove the multiboot information since the loader is now handling this task.
;;
;;===================================================================================================================


;;
;; -- Expose some labels to other fucntions that the linker can pick up
;;    -----------------------------------------------------------------
    global		_start					                        ;; make _start visible
    global      systemFont
    global		Halt					                        ;; make hang visible


;;
;; -- Now, we need some things from other functions imported
;;    ------------------------------------------------------
    extern		kInit					                        ;; allow reference to external kinit
    extern      kMemSetB
    extern      _stackEnd


CPU_TABLES_BASE     equ     0xff401000


;;
;; -- The data segment
;;    ----------------
section		.data

align 		8
IdtDesc:
dw      (256*8)-1                                           ;; size minus one...
dd      CPU_TABLES_BASE+0x800                               ;; this is the addresss

GdtDesc:
dw      (16*8)-1                                            ;; size minus one...
dd      CPU_TABLES_BASE                                     ;; this is the addresss


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section 	.text
align 		4
cpu			586


;;
;; -- loader() --  Entry point for the kernel which is called implicitly by the boot loader
;;    -------------------------------------------------------------------------------------
_start:
loader:
    push        (256*8)										;; the number of bytes to set
    push        0                                           ;; push the byte to fill
    push		0x800										;; push the destination
    call        kMemSetB
    add         esp,12

    mov         ecx,GdtDesc
    lgdt        [ecx]

    mov         ecx,IdtDesc                                 ;; Get the gdt address
    lidt        [ecx]                                       ;; and load the GDT

    mov         ecx,0x02<<3
    mov         ds,cx
    mov         es,cx
    mov         fs,cx
    mov         gs,cx
    mov         ss,cx
    mov         esp,_stackEnd                               ;; set the final stack

	jmp	    0x08:kInit					                    ;; jump to the C initialization function

;;
;; -- if you get back here, then you have a problem and need to hang; fall through to the next function...
;;    ----------------------------------------------------------------------------------------------------


;;
;; -- This is a kernel panic function that will hang the system
;;    ---------------------------------------------------------
Halt:
	cli								                        ;; stop all interrupts

;;
;; -- Halt the cpu
;;    ------------
	hlt								                        ;; force a hard reboot
	jmp		Halt					                        ;; loop, just in case


;;
;; -- Allocate a stack
;;    ----------------
section         .stack
    times 4096  db 0


;;
;; -- This is where we include the binary data for the system font
;;    ------------------------------------------------------------
section         .data
systemFont:
incbin          "system-font.bin"
