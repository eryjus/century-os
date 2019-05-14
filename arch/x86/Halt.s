;;===================================================================================================================
;;
;;  Halt.s -- Halt the CPU
;;
;;        Copyright (c)  2017-2019 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ----------------------------------------------------------------------------
;;  2019-Feb-22  Initial   0.3.0   ADCL  Initial version
;;
;;===================================================================================================================


    global      Halt
    global      systemFont


;;
;; -- This is the code for CPU 0 to execute
;;    -------------------------------------
    section     .text


;;
;; -- Halt the CPU
;;    ------------
Halt:
    cli
    xchg bx,bx
    hlt
    jmp         Halt


;;
;; -- This is where we include the binary data for the system font
;;    ------------------------------------------------------------
    section     .rodata
systemFont:
    incbin      "system-font.bin"


