;;===================================================================================================================
;;
;;  InB.s -- Read a byte from an I/O port
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
global          inb


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .text
cpu		586


;;
;; -- inb() -- get a byte from an I/O port
;;    ------------------------------------
inb:
    push        ebp
    mov         ebp,esp

    mov         edx,[ebp+8]
    xor         eax,eax
    in          al,dx

    pop         ebp
    ret

