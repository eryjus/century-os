;;===================================================================================================================
;;
;;  kStrLen.s -- calculate the length of a string
;;
;;        Copyright (c)  2017-2018 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Calculate the length of a string, returning the result in ax
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2012-Jun-05                          Initial version
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
global      kStrLen


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section     .text
cpu         586


;;
;; -- Calculate the length of the string
;;    ----------------------------------
kStrLen:
    cld
    push    ecx                                                 ;; save this register
    push    edi                                                 ;; and this one

    mov     edi,[esp+12]                                        ;; get the string
    mov     ecx,-1                                              ;; set max chars to scan (lots)
    xor     eax,eax                                             ;; al holds the char to find (NULL)

    repne   scasb                                               ;; find '\0', decrementing ecx as you go

    not     ecx                                                 ;; where did we start - take complement
    mov     eax,ecx                                             ;; move it to return
    dec     eax                                                 ;; and subtract one

    pop     edi                                                 ;; restore the register
    pop     ecx                                                 ;; and this one
    ret
