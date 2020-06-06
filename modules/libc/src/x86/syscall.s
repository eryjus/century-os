;;===================================================================================================================
;;
;;  syscall.s -- execute a system call
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  This function will execute a system call to the kernel.  To accomplish this, the following ABI (which is
;;  supposed to follow the Linux ABI) will be used:
;;  eax: the system function number
;;  ebx: parameter 1
;;  ecx: parameter 2
;;  edx: parameter 3
;;  esi: parameter 4
;;  edi: parameter 5
;;  ebp: parameter 6
;;
;;  Now, the prototype for this call is the following:
;;      int syscall(int func, int parmCnt, ...);
;;
;;  All register contents (including eflags) will be preserved across this function call, with the return result
;;  being passed back in eax.
;;
;;  When the system call returns a value < 0, the variable errno will be populated with the -eax and eax will be
;;  set to -1.
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2020-Apr-18  Initial  v0.7.0a  ADCL  Initial version
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
    global  syscall
    global  errno


;;
;; -- this is the error number from the last call
;;    -------------------------------------------
    section .data
errno:
    dd      0               ;; this is the error number global variable


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
    section .text
    cpu     586


;;
;; -- perform the system call
;;    -----------------------
syscall:                    ;; ret == esp + 28
    pushf                   ;; esp + 24
    push    ebx             ;; esp + 20
    push    ecx             ;; esp + 16
    push    edx             ;; esp + 12
    push    esi             ;; esp + 8
    push    edi             ;; esp + 4
    push    ebp             ;; esp + 0

;; -- clear the registers for the system call
    xor     eax,eax         ;; clear eax
    mov     ebx,eax         ;; clear ebx
    mov     ecx,eax         ;; clear ecx
    mov     edx,eax         ;; clear edx
    mov     esi,eax         ;; clear esi
    mov     edi,eax         ;; clear edi
    mov     ebp,eax         ;; clear ebp

;; -- set the system call parameters
    mov     eax,[esp+36]    ;; get the number of parameters

    cmp     eax,1           ;; do we have 1 parameter?
    jl      .doCall         ;; ready to do the call

;; -- populate parameter 1
    mov     ebx,[esp+40]    ;; parameter 1
    cmp     eax,2           ;; do we have 2 parameters?
    jl      .doCall         ;; ready to do the call

;; -- populate parameter 2
    mov     ebx,[esp+44]    ;; parameter 2
    cmp     eax,3           ;; do we have 3 parameters?
    jl      .doCall         ;; ready to do the call

;; -- populate parameter 3
    mov     ebx,[esp+48]    ;; parameter 3
    cmp     eax,4           ;; do we have 4 parameters?
    jl      .doCall         ;; ready to do the call

;; -- populate parameter 4
    mov     ebx,[esp+52]    ;; parameter 4
    cmp     eax,5           ;; do we have 5 parameters?
    jl      .doCall         ;; ready to do the call

;; -- populate parameter 5
    mov     ebx,[esp+56]    ;; parameter 5
    cmp     eax,6           ;; do we have 6 parameters?
    jl      .doCall         ;; ready to do the call

;; -- populate parameter 6
    mov     ebx,[esp+60]    ;; parameter 6

;; -- do the system call
.doCall:
    mov     eax,[esp+32]    ;; finally, get the system call function number
    int     100             ;; execute the system call


;; -- did we get an error back?
    cmp     eax,0           ;; did we have an error
    jge     .noError

    neg     eax             ;; make it an error number
    mov     [errno],eax     ;; store the result
    mov     eax,-1          ;; and set the return value
    jmp     .out            ;; now, we can leave

;; -- no error
.noError:
    xor     ebx,ebx         ;; clear ebx
    mov     [errno],ebx     ;; set errno to no error

;; -- restore state and return
.out:
    pop     ebp             ;; restore ebp
    pop     edi             ;; restore edi
    pop     esi             ;; restore esi
    pop     edx             ;; restore edx
    pop     ecx             ;; restore ecx
    pop     ebx             ;; restore ebx
    popf                    ;; restore flags
    ret


