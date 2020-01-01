;;===================================================================================================================
;;
;;  entry.s -- Entry point for x86 architecture
;;
;;        Copyright (c)  2017-2020 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Based on the multiboot specification, the multiboot loader will hand off control to the following file at the
;;  loader location.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2019-Feb-18  Initial   0.3.0   ADCL  Initial version
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
    global        entry
    global      JumpKernel
    global      NextEarlyFrame

;;
;; -- Now, we need some things from other functions imported
;;    ------------------------------------------------------
    extern      LoaderMain                                  ;; allow reference to external LoaderMain
    extern      mb1Data                                     ;; the address of the MB1 MBI
    extern      mb2Data                                     ;; the address of the MB2 MBI
    extern      pmmEarlyFrame                               ;; the next frame to be allocated
    extern      mmuLvl1Count                                ;; the number of frames for this level 1 mmu table
    extern      mmuLvl1Table                                ;; the frame for the level 1 mmu table
    extern      earlyStackFrame                             ;; the frame the stack is allocated to
    extern      intTableAddr                                ;; the location of the gdt, idt, and tss structures
    extern      _bssStart                                   ;; the bss starting address
    extern      _bssEnd                                     ;; the bss ending address
    extern      phys_loc                                    ;; this is the physical location in memory
    extern      kern_loc                                    ;; this is the vitual address space for the kernel
    extern      _loaderStart                                ;; this is the location of the loader in memory
    extern      _loaderEnd                                  ;; this is the end of the loader in memory


;;
;; -- This is the multiboot header.  During the link process, it will be relocated to the beginning of the
;;    resulting ELF file.  This is required since the multiboot header needs to be in the first 4/8K (I think).
;;    ---------------------------------------------------------------------------------------------------------
section         .mboot


;;
;; -- The following are used to populate the multiboot v1 header
;;    ----------------------------------------------------------
MAGIC           equ     0x1badb002
FLAGS           equ     (1<<1 | 1<<2)                       ;; get mem & set video

;;
;; -- The following are used to populate the multiboot v2 header
;;    ----------------------------------------------------------
MAGIC2          equ     0xe85250d6
LEN             equ     MultibootHeader2End - MultibootHeader2
CHECK2          equ     (-(MAGIC2 + LEN) & 0xffffffff)


;;
;; -- Standard Video resolution
;;    -------------------------
MODE_TYPE       equ     0
WIDTH           equ     1024
HEIGHT          equ     768
DEPTH           equ     16


;;
;; -- This is the stack size
;;    ----------------------
STACK_SIZE      equ     4096


;;
;; -- This is the multiboot 1 header
;;    ------------------------------
align           4
multiboot_header:
;; -- magic fields
dd      MAGIC
dd      FLAGS
dd      -MAGIC-FLAGS
;; -- address fields (unused placeholders)
dd      0
dd      0
dd      0
dd      0
dd      0
;; -- video fields
dd      MODE_TYPE
dd      WIDTH
dd      HEIGHT
dd      DEPTH


;;
;; -- This is the multiboot 2 header
;;    ------------------------------
align           8
MultibootHeader2:
dd      MAGIC2
dd      0                                                   ;; architecture: 0=32-bit protected mode
dd      LEN                                                 ;; total length of the mb2 header
dd      CHECK2                                              ;; mb2 checksum

align           8
Type1Start:
dw      1                                                   ;; type=1
dw      0                                                   ;; not optional
dd      Type1End-Type1Start                                 ;; size = 40
;dd      1                                                   ;; provide boot command line
;dd      2                                                   ;; provide boot loader name
;dd      3                                                   ;; provide module info
;dd      4                                                   ;; provide basic memory info
;dd      5                                                   ;; provide boot device
dd      6                                                   ;; provide memory map
;dd      8                                                   ;; provide frame buffer info
;dd      9                                                   ;; provide elf symbol table
Type1End:

align           8
Type4Start:
dw      4                                                   ;; type=4
dw      0                                                   ;; not optional
dd      Type4End-Type4Start                                 ;; size = 12
dd      1                                                   ;; graphics
Type4End:

align           8
Type5Start:
dw      5                                                   ;; graphic mode
dw      1                                                   ;; not optional
dd      Type5End-Type5Start                                 ;; size = 20
dd      WIDTH                                               ;; 1024
dd      HEIGHT                                              ;; 768
dd      DEPTH                                               ;; 16
Type5End:

align           8
Type6Start:
dw      6                                                   ;; Type=6
dw      1                                                   ;; Not optional
dd      Type6End-Type6Start                                 ;; size = 8 bytes even tho the doc says 12
Type6End:

align           8
Type8Start:
dw      0                                                   ;; Type=0
dw      0                                                   ;; flags=0
dd      8                                                   ;; size=8
MultibootHeader2End:


;;
;; -- This is a short loader data section for the GDT, IDT and TSS.  Templates are stored here for these
;;    structures at compile time to make the rest of the work less fussy.
;;    --------------------------------------------------------------------------------------------------
    section     .ldrdata
    align       4

gdtStart:
    dd          0x00000000,0x00000000                   ;; this is the null segment descriptor
    dd          0x0000ffff,0x00cf9a00                   ;; this is the kernel code segment
    dd          0x0000ffff,0x00cf9200                   ;; this is the kernel stack (and data) segment
    dd          0x0000ffff,0x00cffa00                   ;; this is the user code segment
    dd          0x0000ffff,0x00cff200                   ;; this is the user stack (and data) segment
    dd          0x00000000,0x00000000                   ;; reserved for kernel data segment
    dd          0x00000000,0x00000000                   ;; reserved for user data segment
    dd          0x00000000,0x00000000                   ;; reserved for loader code segment
    dd          0x00000000,0x00000000                   ;; reserved for loader data segment
    dd          0x1080ffff,0xff0fe940                   ;; this is the 32-bit tss
    dd          0x00000000,0x00000000                   ;; reserved rest of 64-bit tss
    dd          0x00000000,0x00000000                   ;; Future use
    dd          0x00000000,0x00000000                   ;; Future use
    dd          0x00000000,0x00000000                   ;; Future use
    dd          0x00000000,0x00000000                   ;; Future use
    dd          0x00000000,0x00000000                   ;; Future use


idtTemplate:
    dd          0x00000008,0x00ee0000


ldtSize:
    dw          0
ldtLoc:
    dd          0


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
    section     .ldrtext
    align       4
    cpu         586


;;
;; -- This is the entry point where we will get control -- first which MBI do we have?
;;    --------------------------------------------------------------------------------
entry:
    cmp         eax,0x2badb002
    jne         chkMB2
    mov         [mb1Data],ebx
    jmp         initialize

chkMB2:
    cmp         eax,0x36d76289
    jne         initialize
    mov         [mb2Data],ebx


;;
;; -- The next order is business is to get a stack set up.  This will be done through `NextEarlyFrame()`,
;;    but a stack needs to be set up to call the function.  So just perform the same tasks here rather than
;;    calling a function.
;;    -----------------------------------------------------------------------------------------------------
initialize:
    mov         eax,[pmmEarlyFrame]                         ;; get the next frame to allocate
    mov         ebx,eax                                     ;; save that on the ebx register
    dec         eax                                         ;; decrement to the next frame to allocate
    mov         [pmmEarlyFrame],eax                         ;; save the resulting next frame back in the var
    mov         [earlyStackFrame],ebx                       ;; also save the location for later cleanup

    shl         ebx,12                                      ;; turn the frame into an address
    add         ebx,4096                                    ;; and adjust to the top of the stack
    mov         esp,ebx                                     ;; set the stack


;;
;; -- Now, we need to clear the bss.  This is easily done by storing 0 for the entire section.  Good thing
;;    the linker will provide addressed to the start and end of the sections.  The trick here is that these
;;    addresses are located in the upper virtual address space, not the physical space, so they need to be
;;    adjusted properly.
;;    -----------------------------------------------------------------------------------------------------
clearBss:
    mov         ecx,_bssEnd                                 ;; get the ending address
    mov         eax,_bssStart                               ;; get the starting address
    sub         ecx,eax                                     ;; calculate the difference in bytes
    shr         ecx,2                                       ;; operate in dwords, or 4 bytes

    mov         ebx,[kern_loc]                              ;; get the kernel location
    sub         eax,ebx                                     ;; adjust to an offset from the start of the kernel

    mov         ebx,[phys_loc]                              ;; get the physical location
    add         eax,ebx                                     ;; adjust to the start of the physical location

    mov         ebx,_loaderEnd                              ;; this is the loader in memory, which we adjust for
    add         eax,ebx
    mov         ebx,_loaderStart                            ;; and then the start to create an offset
    sub         eax,ebx                                     ;; eax now has the physical start of the bss

    mov         edi,eax                                     ;; move to the edi reg
    xor         eax,eax                                     ;; clear eax
    cld                                                     ;; clear the direction (think decrement) flag
    rep         stosd                                       ;; clear the memory for ecx bytes


;;
;; -- The next order of business here is to set up the initial paging table with a 4MB page.  This will require
;;    a carefully crafted 32-bit entry in the page directory and a bit set in the cr4 register.
;;
;;    IMPORTANT NOTE HERE: I am not going to fully initialize this structure -- just enough for the first 4MB
;;    to work properly.  Any data access outside this first 4MB will result in an exception -- or maybe not --
;;    it will be the decision of the unitialized memory bit gods!
;;    ---------------------------------------------------------------------------------------------------------
    mov     eax,1                       ;; this is the number of pages we allocated for the level 1 table
    mov     [mmuLvl1Count],eax          ;; set the frame count

    call    NextEarlyFrame              ;; get the next frame we can use in eax
    shl     eax,12                      ;; convert the frame to an address
    mov     [mmuLvl1Table],eax          ;; set the address of the frame variable
    mov     edi,eax                     ;; and move that into the edi register
    mov     cr3,eax                     ;; populate the cr3 register while we have the data


;;
;; -- We need to map 1 X 4MB sections to the physical addressed; start by building the value we will store.
;;                             3322222222221111111111
;;                             10987654321098765432109876543210
;;                             --------------------------------
;;    This value needs to be 0b00000000000000000000000010000111
;;    -----------------------------------------------------------------------------------------------------
    mov     eax,0x00000087              ;; set the value we will store
    mov     [edi+0],eax                 ;; and then place that into the PDT

    mov     eax,cr4                     ;; get the cr4 register contents
    or      eax,1<<4                    ;; set the pse bit
    mov     cr4,eax                     ;; and write the value back


;;
;; -- Now we need a GDT and an IDT built.  The GDT will need to be perfectly constructed as it will persist
;;    for the rest of the OS; the IDT does not need to be and can actually fit in the same frame as the
;;    GDT.  However, we need to load the GDTR and IDTR properly for the CPU.  At the same time, get the TSS
;;    ready to go.
;;    -----------------------------------------------------------------------------------------------------
    call    NextEarlyFrame              ;; go get a frame for the GDT/TSS/IDT stuff
    shl     eax,12                      ;; adjust to an address
    mov     [intTableAddr],eax          ;; save this frame for later
    mov     edi,eax                     ;; save the frame to the edi
    mov     ecx,0x400                   ;; we going to move 0x400 dwords
    xor     eax,eax                     ;; and we are going to clear this frame
    cld                                 ;; make sure we increment
    mov     edx,edi                     ;; save the starting location; want it again
    rep     stosd                       ;; clear the frame
    mov     edi,edx                     ;; get the starting address back

;; -- now, load the gdt
    mov     esi,gdtStart                ;; get the start of the gdt
    mov     ecx,32                      ;; the number of dwords to move
    cld                                 ;; increment please
    rep     movsd                       ;; copy the data

;; -- load the idt
    mov     edi,edx                     ;; get our starting point again
    add     edi,0x800                   ;; adjust for the start of the IDT

    mov     ecx,255                     ;; we will load 256 entries
    mov     esi,idtTemplate             ;; get the address of the template to load
    mov     eax,[esi]                   ;; get the first dword
    mov     ebx,[esi+4]                 ;; and the second dword

loop:
    mov     [edi+ecx*8],eax             ;; write the first dword
    mov     [edi+ecx*8+4],ebx           ;; write the second dword
    loopne  loop                        ;; decrement ecx and check for 0; break the loop if 0

;; -- when loopne decremented, it will not execute when ecx == 0; so clean that up
    mov     [edi],eax                   ;; write the last value
    mov     [edi+4],ebx                 ;; write the last value


;;
;; -- finally at this point, we can load the gdt and idt registers; hoping to get the tss as well
;;    -------------------------------------------------------------------------------------------
    mov     eax,0x7ff                   ;; 0x800 bytes - 1
    mov     [ldtSize],ax                ;; load that into the size
    mov     eax,[intTableAddr]          ;; this is the location of the idt
    add     eax,0x800
    mov     [ldtLoc],eax                ;; load that into the struture
    mov     eax,ldtSize                 ;; get the address of the structure
    lidt    [eax]                       ;; load the idt

    mov     eax,0x7f                    ;; 0x80 bytes - 1
    mov     [ldtSize],ax                ;; load that into the size
    mov     eax,[intTableAddr]          ;; this is the location of the gdt
    mov     [ldtLoc],eax                ;; load that into the struture
    mov     eax,ldtSize                 ;; get the address of the structure
    lgdt    [eax]                       ;; load the gdt

    jmp     0x08:newGdt                 ;; jump to make the new GDT take effect

newGdt:
    mov     eax,0x10                    ;; the segment selector for data
    mov     ds,ax
    mov     es,ax
    mov     fs,ax
    mov     gs,ax
    mov     ss,ax

;; -- the last step here is to load the tss
    mov     eax,0x4b                    ;; this is the segment selector for the tss
    ltr     ax                          ;; load this up


;; -- the final task is to enable paging
    mov     eax,cr0                     ;; get that control register
    or      eax,1<<31                   ;; set the PG bit
    mov     cr0,eax                     ;; all hell can break loose from here!


;;
;; -- enable caches if defined
;;    ------------------------
%if ENABLE_CACHE == 1
    mov     eax,cr0                     ;; get the control register
    or      eax,(1<<29|1<<30)           ;; enable the CD and NW bits
    mov     cr0,eax                     ;; save the cr0 register
%endif


;;
;; -- If we manage to get here, we now can jump to LoaderMain -- not a call
;;    ---------------------------------------------------------------------
    jmp     LoaderMain


;;
;; -- this function will simple get the next frame and decrement the value
;;    Clobbers: eax -- carefully constructed to only update eax
;;    --------------------------------------------------------------------
NextEarlyFrame:         ;; called from C
    mov         eax,[pmmEarlyFrame]                         ;; get the next frame to allocate
    dec         eax                                         ;; update the next frame to allocate
    mov         [pmmEarlyFrame],eax                         ;; and store that back in the var
    inc         eax                                         ;; we actually wanted the previous val; reset it
    ret


;;
;; -- Set the stack up and the jump to the kernel itself
;;    --------------------------------------------------
JumpKernel:
    mov         eax,[esp+4]
    mov         esp,[esp+8]
    jmp         eax


