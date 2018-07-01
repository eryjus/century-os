;;===================================================================================================================
;;
;; kernel/src/x86-common/loader.s -- Common loader for the Intel based-kernels
;;
;; Based on the multiboot specification, the multiboot loader will hand off control to the following file at the
;; loader location.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;    Date     Tracker  Version  Pgmr  Description
;; ----------  -------  -------  ----  ---------------------------------------------------------------------------
;; 2012-05-13  Initial           ADCL  Initial version
;; 2012-09-15                    ADCL  Copied from Century
;; 2018-05-25  Initial   0.1.0   ADCL  Copied from century32 to century-os
;; 2018-07-01  Initial   0.1.0   ADCL  Remove the multiboot information since the loader is now handling this task.
;;
;;===================================================================================================================


;
; -- Expose some labels to other fucntions that the linker can pick up
;    -----------------------------------------------------------------
global		_start					                        ; make _start visible
global		Halt					                        ; make hang visible


;
; -- Now, we need some things from other functions imported
;    ------------------------------------------------------
extern		kInit					                        ; allow reference to external kinit


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
	jmp	    kInit					                        ; jump to the C initialization function

;
; -- if you get back here, then you have a problem and need to hang; fall through to the next function...
;    ----------------------------------------------------------------------------------------------------


;
; -- This is a kernel panic function that will hang the system
;    ---------------------------------------------------------
Halt:
	cli								                        ; stop all interrupts

;
; -- Halt the cpu
;    ------------
	hlt								                        ; force a hard reboot
	jmp		Halt					                        ; loop, just in case
