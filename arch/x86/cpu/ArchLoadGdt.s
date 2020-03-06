;;===================================================================================================================
;;
;;  ArchLoadGdt.s -- Load the final GDT and set the proper section selectors
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2020-Jan-06  Initial  v0.5.0e  ADCL  Initial version
;;
;;===================================================================================================================


%include "constants.inc"


;;
;; -- Expose some labels to other fucntions that the linker can pick up
;;    -----------------------------------------------------------------
    global      ArchLoadGdt



;;
;; -- Load the GDT and set the the segment selectors
;;    ----------------------------------------------
ArchLoadGdt:
    mov         eax,[esp+4]                     ;; get the gdtr values
    lgdt        [eax]                           ;; load the gdt register

    jmp         0x08:.newgdt

.newgdt:
    mov         eax,0x10
    mov         ss,ax

    mov         eax,0x28
    mov         ds,ax
    mov         es,ax
    mov         fs,ax

    ret

