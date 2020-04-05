;;===================================================================================================================
;;
;;  kStrCmp.s -- compare 2 strings
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  compare 2 strings to each other by performing the subtraction str1[n] - *str2[n] until we have result other
;;  than 0 and neither character is null.
;;
;;  Prototype:
;;  void kStrCmp(char *str1, char *Str2);
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;    Date      Tracker  Version  Pgmr  Description
;; -----------  -------  -------  ----  ---------------------------------------------------------------------------
;; 2017-Oct-12                          Initial version
;; 2018-Jun-23  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
;;
;;===================================================================================================================



;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
    global kStrCmp


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .text
cpu        586


;;
;; -- Compare 2 strings
;;    -----------------
kStrCmp:
    push    esi
    push    edi

    mov     edi,[esp+12]                ;; str1
    mov     esi,[esp+16]                ;; str2

.loop:
    xor     eax,eax
    mov     al,[edi]                    ;; get the character
    sub     al,[esi]                    ;; perform the subtraction
    cmp     al,0                        ;; is there a difference?
    jne     .out                        ;; if there is a difference, exit

    cmp     byte [edi],0                ;; end of the string?
    je      .out                        ;; if null, we're done

    cmp     byte [esi],0                ;; end of this string?
    je      .out                        ;; if null, we're done

    inc     esi                         ;; next char
    inc     edi
    jmp     .loop

.out:
    pop     edi
    pop     esi

    ret



