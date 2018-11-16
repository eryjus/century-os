;;===================================================================================================================
;;
;;  OutB.s -- WRite a byte to an I/O port
;;
;;        Copyright (c)  2017-2018 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;    Date      Tracker  Version  Pgmr  Description
;; -----------  -------  -------  ----  ----------------------------------------------------------------------------
;; 2017-Apr-22  Initial   0.0.0   ADCL  Initial version
;; 2018-Jun-06  Initial   0.1.0   ADCL  Copied this file from century to century-os (port-io.s)
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
    global          outb


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .text
cpu		586


;;
;; -- outb() -- send a byte to an I/O port
;;    ------------------------------------
outb:
    push        ebp
    mov         ebp,esp

    mov         edx,[ebp+8]
    mov         eax,[ebp+12]
    out         dx,al

    pop         ebp
    ret
