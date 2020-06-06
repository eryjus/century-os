;;===================================================================================================================
;;
;;  CpuPanicPushRegs.s -- Panic halt all the cpus, pushing the registers
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2020-Mar-04  Initial  v0.5.0h  ADCL  Initial Version
;;
;;===================================================================================================================


;;
;; -- Now, we need some things from other functions imported
;;    ------------------------------------------------------
extern    CpuPanic


;;
;; -- Expose some addresses for other things to use
;;    ---------------------------------------------
global      CpuPanicPushRegs


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .text
cpu        586


;;
;; -- Push all the registers and then Panic all CPUs
;;    ----------------------------------------------
CpuPanicPushRegs:
    pushf                               ;; flags
    push        cs                      ;; cs
    push        dword [esp+(2*4)]       ;; eip
    push        dword 0                 ;; error code
    push        dword 0                 ;; ack
    push        dword 0                 ;; interrupt number

    push        eax
    push        ecx
    push        edx
    push        ebx
    push        esp
    push        ebp
    push        esi
    push        edi

    mov         eax,cr0                 ;; get cr0
    push        eax                     ;; and push it
    mov         eax,cr2                 ;; get cr2
    push        eax                     ;; and push it
    mov         eax,cr3                 ;; get cr3
    push        eax                     ;; and push it

    xor         eax,eax                 ;; clear eax for work on other seg regs
    mov         ax,ds                   ;; get ds
    push        eax                     ;; and push it
    mov         ax,es                   ;; get es
    push        eax                     ;; and push it
    mov         ax,fs                   ;; get fs
    push        eax                     ;; and push it
    mov         ax,gs                   ;; get gs
    push        eax                     ;; and push it
    mov         ax,ss                   ;; get ss
    push        eax                     ;; and push it

    mov         ax,0x28                 ;; now make sure that data is setup for ring0
    mov         ds,ax                   ;; move it to ds, ...
    mov         es,ax                   ;; ... es, ...
    mov         fs,ax                   ;; ... fs, and ...
    mov         ax,0x10                 ;; Stack is seg sel 0x10...
    mov         ss,ax                   ;; ... ss

    push        esp                     ;; the structure pointer
    push        dword [esp+(24*4)]      ;; the message to print
    call        CpuPanic

