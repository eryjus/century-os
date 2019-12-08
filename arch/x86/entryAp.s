;;===================================================================================================================
;;
;;  entryAp.s -- Entry point for x86 Application Processors
;;
;;        Copyright (c)  2017-2019 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  This startup file is a little tricky.  All APs start in 16-bit real mode and they need to be put into the
;;  target mode as quickly as possible.  However, we also need a stack for each AP and we have a race condition
;;  that is set up when more than 1 core is started at the same time.
;;
;;  We will not want to do that, but just in case we will halt each core by a simple spinlock.
;;
;;  Now, once we have the segment registers initialized, we will want to get into 32-bit protected mode with paging
;;  enabled as quick as we can.  Once that is accomplished, we can jump to the C code initializer.
;;
;;  Before the AP can leave this file, the following need to be completed:
;;  * Clear the segment registers
;;  * Wait for its turn
;;  * Allocate a stack and set it up
;;  * Load the GDTR
;;  * Load the IDTR
;;  * Enable Protection
;;  * Perform an internal jump to kick in protection
;;  * Load the segment registers properly
;;  * Load the `CR3` register
;;  * Enable Paging
;;  * Jump to the kernel
;;
;;  Now, the portion of memory where this is located is identity mapped, so that should not be a problem when
;;  Paging is enabled.
;;
;;  What might be a problem is that this code really needs to be relocated to low memory (it will live in memory
;;  > 1MB when loaded and the CPU cannot go that high in 16-bit mode).  Also, `cs` is expected to be some segment
;;  and `ip` is expected to be `0`.  My reference tof that statement is:
;;  https://github.com/doug65536/dgos/blob/af27c99d75f54c28f27420f92dd6c8a68efebed6/boot/mpentry_bios.S#L7.
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
    extern      intTableAddr            ;; the location of the gdt, idt, and tss structures
    extern      kInitAp                 ;; the location of the C initializer
    extern      mmuLvl1Table            ;; the value to stuff into cr3


;;
;; -- This is the entry point for the APs
;;    -----------------------------------
    section     .smptext


    bits        16                      ;; we start in 16-bit real mode!!
entryAp:
    cli                                 ;; disable interrupts immediately
    cld

    jmp         setup                   ;; jump past these variables


lck:                                    ;; offset = 4
    dd          0

stck:                                   ;; offset = 8
    dd          0x2000                  ;; this is the top of the first stack

gdtSize:                                ;; offset = 12 or 0xc
    dw          0x7f                    ;; the size of the global descriptor table to load

gdtLoc:                                 ;; offset = 14 or 0xe
    dd          0                       ;; the location of the global descriptor table

idtSize:                                ;; offset = 18 or 0x12
    dw          0x7ff                   ;; the size of the interrupt descriptor table

idtLoc:                                 ;; offset = 20 or 0x14
    dd          0                       ;; the location of the interrupt descriptor table

addr:                                   ;; offset = 24 or 0x18
    dd          0x8000+(tgt_32_bit-entryAp)    ;; this is the absolute address of the 32-bit entry

sel:                                    ;; offset = 28 or 0x1c
    dw          0x08                    ;; CS segment selector


;;
;; -- Set up the data segment registers (real mode!)
;;    ----------------------------------------------
setup:
    mov         ax,cs                   ;; get the cs segment
    mov         ds,ax
    mov         es,ax


;;
;; -- This is a safety net to keep only one core from moving on at a time.  There is a race to get here
;;    when more than 1 core is started at a time.  The spinlock will work as a boundary to keep the others
;;    at bay until it is safe to continue.
;;    ----------------------------------------------------------------------------------------------------
    mov         ecx,1                   ;; this is the value to load
    mov         ebx,4                   ;; This is the address of LCK -- an offset from the seg start!

loop:
    mov         eax,0                   ;; this is the value it should be to load
LOCK cmpxchg    [ebx],ecx               ;; do the cmpxchg -- notice the LOCK prefix
    jnz         loop                    ;; if the lock was not unlocked, loop


;;
;; -- only 1 core at a time gets here -- set up the stack segment register (real mode!)
;;    ---------------------------------------------------------------------------------
segs:
    xor         eax,eax                 ;; clear ax
    mov         ecx,8                   ;; the stack address; offset = 8
    mov         ss,ax
    mov         esp,[ecx]               ;; set the stack


;;
;; -- increment to the next stack (a small 2K stack)
;;    ----------------------------------------------
    add         dword [ecx],0x800       ;; next stack
    mov         dword [ebx],0           ;; unlock the lock and let the remaining cores race for the lock


;;
;; -- Load the IDT
;;    ------------
;;    mov     eax,ds                      ;; construct the physical address of the idtr contents
;;    shl     eax,4
    mov     eax,18                      ;; get the address of the structure
    cs lidt    [eax]                       ;; load the idt


;;
;; -- Load the GDT
;;    ------------
    sub     eax,6                       ;; go to the previous strutures
    cs lgdt    [eax]                       ;; load the gdt


;;
;; -- Now, we need to set the PE bit and perform a long jump to get into protected mode
;;    ---------------------------------------------------------------------------------
    mov     eax,cr0                     ;; get CR0
    or      eax,1                       ;; set the PE bit (Protection Enable)
    mov     cr0,eax                     ;; store CR0


;;
;; -- the problem with the long jump is that it is an absolute address and we must be position independent.
;;    so, to that end, we need to calculate our position in memory.  This should not be too hard to do.
;;    The basic calculation is: (tgt_32_bit - entryAp) + (`cs` << 4)
;;    -----------------------------------------------------------------------------------------------------
;    jmp     long 0x08:0x80000+(tgt_32_bit-entryAp)  ;; jmp to 32-bit code
    mov     edi,24                      ;; the offset of the target address
    pushfd
    push    0x08
    push    dword [edi]
    iretd
;    cs jmp long [edi]


;;
;; -- 32-bit code starts here; load the data segment selectors properly
;;    -----------------------------------------------------------------
    cpu         586
    bits        32                      ;; we start in 32-bit real mode!!
tgt_32_bit:
    mov     eax,0x10                    ;; this is the segment selector for the data/stack
    mov     ds,ax
    mov     es,ax
    mov     fs,ax
    mov     gs,ax
    mov     ss,ax

;; -- the last step here is to load the tss
    mov     eax,0x4b                    ;; this is the segment selector for the tss
;    ltr     ax                          ;; load this up

;;
;; -- load the CR3 register
;;    ---------------------
    mov     eax,[mmuLvl1Table]          ;; get the address of the frame variable
    mov     cr3,eax                     ;; populate the cr3 register while we have the data


;;
;; -- set up for large pages
;;    ----------------------
    mov     eax,cr4                     ;; get the cr4 register contents
    or      eax,1<<4                    ;; set the pse bit
    mov     cr4,eax                     ;; and write the value back

;;
;; -- and enable paging
;;    -----------------
    mov     eax,cr0                     ;; get that control register
    or      eax,1<<31                   ;; set the PG bit
    mov     cr0,eax                     ;; all hell can break loose from here!

;;
;; -- finally, jump to the kernel
;;    ---------------------------
    mov     eax,kInitAp
    jmp     eax

apHalt:
    hlt
    jmp     apHalt


