;;===================================================================================================================
;;
;;  entryAp.s -- Entry point for x86 Application Processors
;;
;;        Copyright (c)  2017-2019 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2019-Jun-17  Initial   0.4.6   ADCL  Initial version
;;
;;===================================================================================================================


;;
;; -- take care of making sure that the conditional assemble flags are defined
;;    ------------------------------------------------------------------------
%ifndef ENABLE_CACHE
%define ENABLE_CACHE 0
%endif


;;
;; -- Expose some labels to other fucntions that the linker can pick up
;;    -----------------------------------------------------------------
    global      entryAp


;;
;; -- Need some symbols from other source files
;;    -----------------------------------------
    extern      intTableAddr                                ;; the location of the gdt, idt, and tss structures


;;
;; -- This is the entry point for the APs
;;    -----------------------------------
section         .smpboot


    bits        16                      ;; we start in 16-bit real mode!!
entryAp:
    cli                                 ;; disable interrupts immediately
    cld

    jmp         spin                    ;; Skip over the data elements

    align       4
lck:
    dw          0                       ;; this is the lock
stck:
    dw          0x2000                  ;; this is the first stack
ldtSize:
    dw          0
ldtLoc:
    dd          0



;;
;; -- Set up the data segment registers (real mode!)
;;    ----------------------------------------------
spin:
    xor         ax,ax                   ;; clear ax; needed for the variable locations above
    mov         ds,ax
    mov         es,ax


;;
;; -- This sets up a race condition to this point.  Only 1 core will get past this point at a
;;    time, ensuring that the stack can be set up properly for each core
;;    ---------------------------------------------------------------------------------------
    mov         ebx,lck                 ;; the lock address
    mov         cx,1                    ;; this is the value to load

loop:
    mov         ax,0                    ;; this is the value it should be to load
LOCK cmpxchg    [ebx],cx                ;; do the cmpxchg -- notice the LOCK prefix
    jnz         loop                    ;; if the lock was not unlocked, loop


;;
;; -- set up the stack segment register (real mode!)
;;    ----------------------------------------------
segs:
    xor         eax,eax                 ;; clear ax
    mov         ecx,stck                ;; the stack address
    mov         ss,ax
    mov         esp,[ecx]               ;; set the stack


;;
;; -- increment to the next stack
;;    ---------------------------
    add         dword [ecx],0x800       ;; next stack
    mov         dword [ebx],0           ;; unlock the lock and let the remaining cores race for the lock


;;
;; -- at this point, we can load the gdt and idt registers; hoping to get the tss as well
;;    -----------------------------------------------------------------------------------
    mov     eax,0x7ff                   ;; 0x800 bytes - 1
    mov     ecx,ldtSize
    mov     [ecx],ax                    ;; load that into the size
    mov     eax,intTableAddr            ;; this is the locaiton of the idt variable
    mov     eax,[eax]                   ;; this is the location of the idt
    add     eax,0x800
    mov     ecx,ldtLoc                  ;; load the address of the structure
    mov     [ecx],eax                   ;; load that into the struture
    mov     eax,ldtSize                 ;; get the address of the structure
    lidt    [eax]                       ;; load the idt

    mov     eax,0x7f                    ;; 0x80 bytes - 1
    mov     ecx,ldtSize
    mov     [ecx],ax                    ;; load that into the size
    mov     eax,intTableAddr            ;; this is the locaiton of the idt variable
    mov     eax,[eax]                   ;; this is the location of the gdt
    mov     ecx,ldtLoc                  ;; load the address of the structure
    mov     [ecx],eax                   ;; load that into the struture
    mov     eax,ldtSize                 ;; get the address of the structure
    lgdt    [eax]                       ;; load the gdt





