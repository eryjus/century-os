;;===================================================================================================================
;;
;;  entry.s -- Entry point for x86 architecture
;;
;;        Copyright (c)  2017-2019 -- Adam Clark
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


%include "constants.inc"


;;
;; -- take care of making sure that the conditional assemble flags are defined
;;    ------------------------------------------------------------------------
%ifndef ENABLE_CACHE
%define ENABLE_CACHE 0
%endif


;;
;; -- Expose some labels to other fucntions that the linker can pick up
;;    -----------------------------------------------------------------
    global      entry
    global      JumpKernel
    global      NextEarlyFrame
    global      earlyFrame
    global      mmuLvl1Count                    ;; the number of frames for this level 1 mmu table
    global      mmuLvl1Table                    ;; the frame for the level 1 mmu table
    global      mb1Data
    global      mb2Data
    global      intTableAddr                    ;; interrupt table address


;;
;; -- Now, we need some things from other functions imported
;;    ------------------------------------------------------
    extern      LoaderMain                      ;; allow reference to external LoaderMain
    extern      earlyStackFrame                 ;; the frame the stack is allocated to
    extern      bssPhys                         ;; the bss starting address (linker provided)
    extern      bssSize                         ;; the bss size (linker provided)
    extern      mbStart                         ;; mboot section starting address (linker provided)
    extern      mbPhys                          ;; mboot section (linker provided)
    extern      mbSize                          ;; mboot size (linker provided)
    extern      ldrPhys                         ;; loader section virtual address (linker provided)
    extern      ldrSize                         ;; loader size (linker provided)
    extern      ldrVirt                         ;; loader section starting address (linker provided)
    extern      sysPhys                         ;; syscall section virtual address (linker provided)
    extern      sysSize                         ;; syscall size (linker provided)
    extern      sysVirt                         ;; syscall section starting address (linker provided)
    extern      txtPhys                         ;; kernel code section virtual address (linker provided)
    extern      txtSize                         ;; kernel code size (linker provided)
    extern      txtVirt                         ;; kernel code section starting address (linker provided)
    extern      dataPhys                        ;; kernel data section virtual address (linker provided)
    extern      dataSize                        ;; kernel data size (linker provided)
    extern      dataVirt                        ;; kernel data section starting address (linker provided)

    extern      isr0                            ;; interrupt handler 0x00
    extern      isr1                            ;; interrupt handler 0x01
    extern      isr2                            ;; interrupt handler 0x02
    extern      isr3                            ;; interrupt handler 0x03
    extern      isr4                            ;; interrupt handler 0x04
    extern      isr5                            ;; interrupt handler 0x05
    extern      isr6                            ;; interrupt handler 0x06
    extern      isr7                            ;; interrupt handler 0x07
    extern      isr8                            ;; interrupt handler 0x08
    extern      isr9                            ;; interrupt handler 0x09
    extern      isr10                           ;; interrupt handler 0x0a
    extern      isr11                           ;; interrupt handler 0x0b
    extern      isr12                           ;; interrupt handler 0x0c
    extern      isr13                           ;; interrupt handler 0x0d
    extern      isr14                           ;; interrupt handler 0x0e
    extern      isr15                           ;; interrupt handler 0x0f
    extern      isr16                           ;; interrupt handler 0x10
    extern      isr17                           ;; interrupt handler 0x11
    extern      isr18                           ;; interrupt handler 0x12
    extern      isr19                           ;; interrupt handler 0x13
    extern      isr20                           ;; interrupt handler 0x14
    extern      isr21                           ;; interrupt handler 0x15
    extern      isr22                           ;; interrupt handler 0x16
    extern      isr23                           ;; interrupt handler 0x17
    extern      isr24                           ;; interrupt handler 0x18
    extern      isr25                           ;; interrupt handler 0x19
    extern      isr26                           ;; interrupt handler 0x1a
    extern      isr27                           ;; interrupt handler 0x1b
    extern      isr28                           ;; interrupt handler 0x1c
    extern      isr29                           ;; interrupt handler 0x1d
    extern      isr30                           ;; interrupt handler 0x1e
    extern      isr31                           ;; interrupt handler 0x1f

;;
;; -- This is the multiboot header.  During the link process, it will be relocated to the beginning of the
;;    resulting ELF file.  This is required since the multiboot header needs to be in the first 4/8K (I think).
;;    ---------------------------------------------------------------------------------------------------------
    section     .mboot


;;
;; -- The following are used to populate the multiboot v2 header
;;    ----------------------------------------------------------
LEN             equ     MultibootHeader2End - MultibootHeader2
CHECK2          equ     (-(MAGIC2 + LEN) & 0xffffffff)


;;
;; -- This is the multiboot 1 header
;;    ------------------------------
    align       4
multiboot_header:
    ;; -- magic fields
    dd          MAGIC1
    dd          MBFLAGS
    dd          -MAGIC1-MBFLAGS
;; -- address fields (unused placeholders)
    dd          0
    dd          0
    dd          0
    dd          0
    dd          0
;; -- video fields
    dd          MODE_TYPE
    dd          WIDTH
    dd          HEIGHT
    dd          DEPTH


;;
;; -- This is the multiboot 2 header
;;    ------------------------------
    align       8
MultibootHeader2:
    dd          MAGIC2
    dd          0                               ;; architecture: 0=32-bit protected mode
    dd          LEN                             ;; total length of the mb2 header
    dd          CHECK2                          ;; mb2 checksum

    align       8
Type1Start:
    dw          1                               ;; type=1
    dw          0                               ;; not optional
    dd          Type1End-Type1Start             ;; size = 40
    dd          1                               ;; provide boot command line
    dd          2                               ;; provide boot loader name
    dd          3                               ;; provide module info
    dd          4                               ;; provide basic memory info
    dd          5                               ;; provide boot device
    dd          6                               ;; provide memory map
    dd          8                               ;; provide frame buffer info
    dd          9                               ;; provide elf symbol table
Type1End:

    align       8
Type4Start:
    dw          4                               ;; type=4
    dw          0                               ;; not optional
    dd          Type4End-Type4Start             ;; size = 12
    dd          1                               ;; graphics
Type4End:

    align       8
Type5Start:
    dw          5                               ;; graphic mode
    dw          1                               ;; not optional
    dd          Type5End-Type5Start             ;; size = 20
    dd          WIDTH                           ;; 1024
    dd          HEIGHT                          ;; 768
    dd          DEPTH                           ;; 16
Type5End:

    align       8
Type6Start:
    dw          6                               ;; Type=6
    dw          1                               ;; Not optional
    dd          Type6End-Type6Start             ;; size = 8 bytes even tho the doc says 12
Type6End:

    align       8
Type8Start:
    dw          0                               ;; Type=0
    dw          0                               ;; flags=0
    dd          8                               ;; size=8
MultibootHeader2End:


;;
;; -- OK, I need a temporary GDT.  I cannot use the low memory yet because it has not been scraped for
;;    data.  So, to make this work I am going to set up a temporary GDT for getting into high memory.
;;    This data will live in the `.data.entry` section, and will be just the minimum to get started.
;;    --------------------------------------------------------------------------------------------------
    align       4

gdtStart:
    dd          0x00000000,0x00000000           ;; this is the null segment descriptor
    dd          0x0000ffff,0x00cf9a00           ;; this is the kernel code segment
    dd          0x0000ffff,0x00cf9200           ;; this is the kernel stack (and data) segment

earlyFrame:
    dd          (4 * 1024)                      ;; start allocating at 4MB

mmuLvl1Count:
    dd          0

mmuLvl1Table:
    dd          0

stackBase:
    dd          0

idtTemplate:
    dd          0x00000008,0x00ee0000


ldtSize:
    dw          0
ldtLoc:
    dd          0


;;
;; -- some additional variables for use later
;;    ---------------------------------------
mb1Data:
    dd          0
mb2Data:
    dd          0

intTableAddr:
    dd          0



;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
    section     .text.entry
    align       4
    cpu         586


;;
;; -- This is the entry point where we will get control -- first which MBI do we have?
;;    --------------------------------------------------------------------------------
entry:
    cmp         eax,MB1SIG
    jne         chkMB2
    mov         [mb1Data],ebx
    jmp         initialize

chkMB2:
    cmp         eax,MB2SIG
    jne         initialize
    mov         [mb2Data],ebx


;;
;; -- The next order is business is to get a stack set up.  This will be done through `NextEarlyFrame()`.
;;    ---------------------------------------------------------------------------------------------------
initialize:
    mov         esp,stack_top                   ;; This will set up an early stack so we can call a function
    call        NextEarlyFrame                  ;; get a frame back in eax
    add         eax,STACK_SIZE                  ;; go to the end of the stack
    mov         esp,eax                         ;; set the stack
    mov         [stackBase],eax                 ;; save that for later


;;
;; -- Now, we need to clear the bss.  This is easily done by storing 0 for the entire section.  Good thing
;;    the linker will provide addressed to the start and end of the sections.  The trick here is that these
;;    addresses are located in the upper virtual address space, not the physical space, so they need to be
;;    adjusted properly.
;;    -----------------------------------------------------------------------------------------------------
clearBss:
    mov         edi,[bssPhys]                   ;; get the starting physical address
    mov         ecx,[bssSize]                   ;; get the size
    shr         ecx,2                           ;; operate in dwords, or 4 bytes
    xor         eax,eax                         ;; clear eax
    cld                                         ;; clear the direction (think decrement) flag
    rep         stosd                           ;; clear the memory for ecx bytes


;;
;; -- The next order of business here is to set up the paging tables.  The goal here is to get them fully
;;    populated before handing control to the loader space in upper memory.
;;    ---------------------------------------------------------------------------------------------------
    mov     eax,1                               ;; this is the number of pages we allocated for the level 1 table
    mov     [mmuLvl1Count],eax                  ;; set the frame count

    call    MakePageTable                       ;; go get a level 1 paging table
    mov     [mmuLvl1Table],eax                  ;; set the address of the frame variable
    mov     cr3,eax                             ;; populate the cr3 register while we have the data
    mov     ebx,eax                             ;; keep the table safe in ebx

;; -- take care of the recursive mapping
    or      eax,X86_MMU_BASE                    ;; convert the value in eax into an Entry
    mov     [ebx + 4092],eax                    ;; create the recursive mapping (1024 * 4)


;;
;; -- now we start to take care of the Page Tables at level 2.  There are several we will need.
;;    start with one for 0x00100000
;;    -----------------------------------------------------------------------------------------
    call    MakePageTable                       ;; get a page table
    mov     ebp,eax                             ;; save the location for later
    or      eax,X86_MMU_BASE                    ;; fix up the bits
    mov     [ebx + (0 * 4)],eax                 ;; this will be at entry 0 (0 / 0x400000)
    mov     eax,ebp                             ;; get the saved address

;; -- init to populate the table
    mov     ecx,[mbSize]                        ;; get the size of the multiboot section
    shr     ecx,12                              ;; convert that to pages/frames (size is page aligned)
    inc     ecx                                 ;; we need one extra page for loopne
    mov     esi,[mbPhys]                        ;; get the address
    mov     edi,[mbStart]                       ;; the index into the table (0x100000 >> 12)
    shr     edi,12

;; -- loop to populate the table
.loop1:
    mov     edx,esi                             ;; get the address (page aligned)
    or      edx,X86_MMU_BASE                    ;; get the other bits
    mov     [eax + (edi * 4)],edx               ;; set the page table entry
    inc     edi                                 ;; next index
    add     esi,0x1000                          ;; next page
    loopne  .loop1


;;
;; -- Identity map the page/frame that contains the current GDT and IDT
;;    -----------------------------------------------------------------
    call    MakePageTable                       ;; get the address of the GDT/IDT for later
    mov     [intTableAddr],eax

    call    MakePageTable                       ;; get a page table
    mov     ebp,eax                             ;; save for later
    or      eax,X86_MMU_BASE                    ;; fix up the bits
    mov     [ebx + (4 * 4)],eax                 ;; this will be at entry 4 (0x100a000 / 0x400000)
    mov     eax,ebp                             ;; it's later

    mov     edx,[intTableAddr]                  ;; get the address
    mov     edi,edx                             ;; the index into the table (0x100000 >> 12)
    shr     edi,12
    and     edi,0x3ff
    or      edx,X86_MMU_BASE                    ;; get the other bits
    mov     [eax + (edi * 4)],edx               ;; set the page table entry


;;
;; -- 0x80000000 for loader code
;;    --------------------------
    call    MakePageTable                       ;; get a page table
    mov     ebp,eax                             ;; save the location for later
    or      eax,X86_MMU_BASE|X86_MMU_KERNEL     ;; fix up the other bits
    mov     [ebx + (512 * 4)],eax               ;; 0x80000000 / 0x400000 = 0x200 (512)
    mov     eax,ebp                             ;; get the saved address

;; -- init to populate the table
    mov     ecx,[ldrSize]                       ;; get the size of the multiboot section
    shr     ecx,12                              ;; convert that to pages/frames (size is page aligned)
    inc     ecx                                 ;; we need one extra page
    mov     esi,[ldrPhys]                       ;; get the address
    mov     edi,[ldrVirt]                       ;; the index into the table
    shr     edi,12
    and     edi,0x3ff

;; -- loop to populate the table
.loop2:
    mov     edx,esi                             ;; get the address (page aligned)
    or      edx,X86_MMU_BASE|X86_MMU_KERNEL     ;; get the other bits
    mov     [eax + (edi * 4)],edx               ;; set the page table entry
    inc     edi                                 ;; next index
    add     esi,0x1000                          ;; next page
    loopne  .loop2


;;
;; -- 0x80400000 for pergatory
;;    ------------------------
    call    MakePageTable                       ;; get a page table
    mov     ebp,eax                             ;; save the location for later
    or      eax,X86_MMU_BASE|X86_MMU_KERNEL     ;; fix up the other bits
    mov     [ebx + (513 * 4)],eax               ;; 0x80400000 / 0x400000 = 0x201 (513)
    mov     eax,ebp                             ;; get the saved address

;; -- init to populate the table
    mov     ecx,[sysSize]                       ;; get the size of the multiboot section
    shr     ecx,12                              ;; convert that to pages/frames (size is page aligned)
    inc     ecx                                 ;; we need one extra page
    mov     esi,[sysPhys]                       ;; get the address
    mov     edi,[sysVirt]                       ;; the index into the table
    shr     edi,12
    and     edi,0x3ff

;; -- loop to populate the table
.loop3:
    mov     edx,esi                             ;; get the address (page aligned)
    or      edx,X86_MMU_BASE|X86_MMU_KERNEL     ;; get the other bits
    mov     [eax + (edi * 4)],edx               ;; set the page table entry
    inc     edi                                 ;; next index
    add     esi,PAGE_SIZE                       ;; next page
    loopne  .loop3


;;
;; -- 0x80800000 for kernel code
;;    --------------------------
    call    MakePageTable                       ;; get a page table
    mov     ebp,eax                             ;; save the location for later
    or      eax,X86_MMU_BASE|X86_MMU_KERNEL     ;; fix up the other bits
    mov     [ebx + (514 * 4)],eax               ;; 0x80800000 / 0x400000 = 0x202 (514)
    mov     eax,ebp                             ;; get the saved address

;; -- init to populate the table
    mov     ecx,[txtSize]                       ;; get the size of the multiboot section
    shr     ecx,12                              ;; convert that to pages/frames (size is page aligned)
    inc     ecx                                 ;; we need one extra page
    mov     esi,[txtPhys]                       ;; get the address
    mov     edi,[txtVirt]                       ;; the index into the table
    shr     edi,12
    and     edi,0x3ff

;; -- loop to populate the table
.loop4:
    mov     edx,esi                             ;; get the address (page aligned)
    or      edx,X86_MMU_BASE|X86_MMU_KERNEL     ;; get the other bits
    mov     [eax + (edi * 4)],edx               ;; set the page table entry
    inc     edi                                 ;; next index
    add     esi,PAGE_SIZE                       ;; next page
    loopne  .loop4

;; -- see if we have more than 4MB kernel code (See Redmine #431 and complete it here)


;;
;; -- 0x81000000 for kernel data
;;    --------------------------
    call    MakePageTable                       ;; get a page table
    mov     ebp,eax                             ;; save the location for later
    or      eax,X86_MMU_BASE|X86_MMU_KERNEL     ;; fix up the other bits
    mov     [ebx + (516 * 4)],eax               ;; 0x8100000 / 0x400000 = 0x204 (516)
    mov     eax,ebp                             ;; get the saved address

;; -- init to populate the table
    mov     ecx,[dataSize]                      ;; get the size of the multiboot section
    shr     ecx,12                              ;; convert that to pages/frames (size is page aligned)
    inc     ecx                                 ;; we need one extra page
    mov     esi,[dataPhys]                      ;; get the address
    mov     edi,[dataVirt]                      ;; the index into the table
    shr     edi,12
    and     edi,0x3ff

;; -- loop to populate the table
.loop5:
    mov     edx,esi                             ;; get the address (page aligned)
    or      edx,X86_MMU_BASE|X86_MMU_KERNEL     ;; get the other bits
    mov     [eax + (edi * 4)],edx               ;; set the page table entry
    inc     edi                                 ;; next index
    add     esi,PAGE_SIZE                       ;; next page
    loopne  .loop5

;; -- see if we have more than 4MB kernel code (See Redmine #432 and complete it here)


;;
;; -- 0xff800000 for our stack
;;    ------------------------
    call    MakePageTable                       ;; get a page table
    mov     ebp,eax                             ;; save the location for later
    or      eax,X86_MMU_BASE|X86_MMU_KERNEL     ;; fix up the other bits
    mov     [ebx + (1022 * 4)],eax              ;; 0xff800000 / 0x400000 = 0x3fe (1022)
    mov     eax,ebp                             ;; get the saved address

;; -- init to populate the table
    mov     esi,[stackBase]                     ;; get the stack physical address
    mov     edi,0                               ;; the index into the table

;; -- loop to populate the table
    mov     edx,esi                             ;; get the address (page aligned)
    or      edx,X86_MMU_BASE|X86_MMU_KERNEL     ;; get the other bits
    mov     [eax + (edi * 4)],edx               ;; set the page table entry


;;
;; -- Finally, we need a page table for 0xff400000 for small arch stuff)
;;    ------------------------------------------------------------------
    call    MakePageTable                       ;; get a page table
    or      eax,X86_MMU_BASE|X86_MMU_KERNEL     ;; fix up the other bits
    mov     [ebx + (1021 * 4)],eax              ;; 0xff400000 / 0x400000 = 0x3fe (1021)


;;
;; -- Now we need a GDT and an IDT built.  The GDT will need to be perfectly constructed as it will persist
;;    for the rest of the OS; the IDT does not need to be and can actually fit in the same frame as the
;;    GDT.  However, we need to load the GDTR and IDTR properly for the CPU.  At the same time, get the TSS
;;    ready to go.
;;    -----------------------------------------------------------------------------------------------------
    mov     eax,[intTableAddr]                  ;; get this from earlier
    mov     edi,eax                             ;; save the frame to the edi
    mov     ecx,0x400                           ;; we going to move 0x400 dwords
    xor     eax,eax                             ;; and we are going to clear this frame
    cld                                         ;; make sure we increment
    mov     edx,edi                             ;; save the starting location; want it again
    rep     stosd                               ;; clear the frame
    mov     edi,edx                             ;; get the starting address back

;; -- now, load the gdt
    mov     esi,gdtStart                        ;; get the start of the gdt
    mov     ecx,32                              ;; the number of dwords to move
    cld                                         ;; increment please
    rep     movsd                               ;; copy the data

;; -- load the idt
    mov     edi,edx                             ;; get our starting point again
    add     edi,0x800                           ;; adjust for the start of the IDT

    mov     ecx,255                             ;; we will load 256 entries
    mov     esi,idtTemplate                     ;; get the address of the template to load
    mov     eax,[esi]                           ;; get the first dword
    mov     ebx,[esi+4]                         ;; and the second dword

.loop9:
    mov     [edi+ecx*8],eax                     ;; write the first dword
    mov     [edi+ecx*8+4],ebx                   ;; write the second dword
    loopne  .loop9                              ;; decrement ecx and check for 0; break the loop if 0

;; -- when loopne decremented, it will not execute when ecx == 0; so clean that up
    mov     [edi],eax                           ;; write the last value
    mov     [edi+4],ebx                         ;; write the last value


;;
;; -- from here, we need to map the address of the IDT
;;    ------------------------------------------------
    mov     ebx,[intTableAddr]                  ;; get the address of the table
    add     ebx,0x800                           ;; adjust to the IDT

    mov     eax,0x08                            ;; exception 0x00
    mov     edx,isr0
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x00)],eax
    mov     [ebx + (8 * 0x00) + 4],edx

    mov     eax,0x08                            ;; exception 0x01
    mov     edx,isr1
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x01)],eax
    mov     [ebx + (8 * 0x01) + 4],edx

    mov     eax,0x08                            ;; exception 0x02
    mov     edx,isr2
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x02)],eax
    mov     [ebx + (8 * 0x02) + 4],edx

    mov     eax,0x08                            ;; exception 0x03
    mov     edx,isr3
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x03)],eax
    mov     [ebx + (8 * 0x03) + 4],edx

    mov     eax,0x08                            ;; exception 0x04
    mov     edx,isr4
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x04)],eax
    mov     [ebx + (8 * 0x04) + 4],edx

    mov     eax,0x08                            ;; exception 0x05
    mov     edx,isr5
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x05)],eax
    mov     [ebx + (8 * 0x05) + 4],edx

    mov     eax,0x08                            ;; exception 0x06
    mov     edx,isr6
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x06)],eax
    mov     [ebx + (8 * 0x06) + 4],edx

    mov     eax,0x08                            ;; exception 0x07
    mov     edx,isr7
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x07)],eax
    mov     [ebx + (8 * 0x07) + 4],edx

    mov     eax,0x08                            ;; exception 0x08
    mov     edx,isr8
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x08)],eax
    mov     [ebx + (8 * 0x08) + 4],edx

    mov     eax,0x08                            ;; exception 0x09
    mov     edx,isr9
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x09)],eax
    mov     [ebx + (8 * 0x09) + 4],edx

    mov     eax,0x08                            ;; exception 0x0a
    mov     edx,isr10
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x0a)],eax
    mov     [ebx + (8 * 0x0a) + 4],edx

    mov     eax,0x08                            ;; exception 0x0b
    mov     edx,isr11
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x0b)],eax
    mov     [ebx + (8 * 0x0b) + 4],edx

    mov     eax,0x08                            ;; exception 0x0c
    mov     edx,isr12
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x0c)],eax
    mov     [ebx + (8 * 0x0c) + 4],edx

    mov     eax,0x08                            ;; exception 0x0d
    mov     edx,isr13
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x0d)],eax
    mov     [ebx + (8 * 0x0d) + 4],edx

    mov     eax,0x08                            ;; exception 0x0e
    mov     edx,isr14
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x0e)],eax
    mov     [ebx + (8 * 0x0e) + 4],edx

    mov     eax,0x08                            ;; exception 0x0f
    mov     edx,isr15
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x0f)],eax
    mov     [ebx + (8 * 0x0f) + 4],edx

    mov     eax,0x08                            ;; exception 0x10
    mov     edx,isr16
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x10)],eax
    mov     [ebx + (8 * 0x10) + 4],edx

    mov     eax,0x08                            ;; exception 0x11
    mov     edx,isr17
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x11)],eax
    mov     [ebx + (8 * 0x11) + 4],edx

    mov     eax,0x08                            ;; exception 0x12
    mov     edx,isr18
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x12)],eax
    mov     [ebx + (8 * 0x12) + 4],edx

    mov     eax,0x08                            ;; exception 0x13
    mov     edx,isr19
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x13)],eax
    mov     [ebx + (8 * 0x13) + 4],edx

    mov     eax,0x08                            ;; exception 0x14
    mov     edx,isr20
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x14)],eax
    mov     [ebx + (8 * 0x14) + 4],edx

    mov     eax,0x08                            ;; exception 0x15
    mov     edx,isr21
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x15)],eax
    mov     [ebx + (8 * 0x15) + 4],edx

    mov     eax,0x08                            ;; exception 0x16
    mov     edx,isr22
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x16)],eax
    mov     [ebx + (8 * 0x16) + 4],edx

    mov     eax,0x08                            ;; exception 0x17
    mov     edx,isr23
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x17)],eax
    mov     [ebx + (8 * 0x17) + 4],edx

    mov     eax,0x08                            ;; exception 0x18
    mov     edx,isr24
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x18)],eax
    mov     [ebx + (8 * 0x18) + 4],edx

    mov     eax,0x08                            ;; exception 0x19
    mov     edx,isr25
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x19)],eax
    mov     [ebx + (8 * 0x19) + 4],edx

    mov     eax,0x08                            ;; exception 0x1a
    mov     edx,isr26
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x1a)],eax
    mov     [ebx + (8 * 0x1a) + 4],edx

    mov     eax,0x08                            ;; exception 0x1b
    mov     edx,isr27
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x1b)],eax
    mov     [ebx + (8 * 0x1b) + 4],edx

    mov     eax,0x08                            ;; exception 0x1c
    mov     edx,isr28
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x1c)],eax
    mov     [ebx + (8 * 0x1c) + 4],edx

    mov     eax,0x08                            ;; exception 0x1d
    mov     edx,isr29
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x1d)],eax
    mov     [ebx + (8 * 0x1d) + 4],edx

    mov     eax,0x08                            ;; exception 0x1e
    mov     edx,isr30
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x1e)],eax
    mov     [ebx + (8 * 0x1e) + 4],edx

    mov     eax,0x08                            ;; exception 0x1f
    mov     edx,isr31
    call    MakeIdtEntry
    mov     [ebx + (8 * 0x1f)],eax
    mov     [ebx + (8 * 0x1f) + 4],edx


;;
;; -- finally at this point, we can load the gdt and idt registers; hoping to get the tss as well
;;    -------------------------------------------------------------------------------------------
    mov     eax,0x7ff                           ;; 0x800 bytes - 1
    mov     [ldtSize],ax                        ;; load that into the size
    mov     eax,[intTableAddr]                  ;; this is the location of the idt
    add     eax,0x800
    mov     [ldtLoc],eax                        ;; load that into the struture
    mov     eax,ldtSize                         ;; get the address of the structure
    lidt    [eax]                               ;; load the idt

    mov     eax,0x17                            ;; 0x18 bytes - 1 (3 entries)
    mov     [ldtSize],ax                        ;; load that into the size
    mov     eax,[intTableAddr]                  ;; this is the location of the gdt
    mov     [ldtLoc],eax                        ;; load that into the struture
    mov     eax,ldtSize                         ;; get the address of the structure
    lgdt    [eax]                               ;; load the gdt

    jmp     0x08:newGdt                         ;; jump to make the new GDT take effect

newGdt:
    mov     eax,0x10                            ;; the segment selector for data
    mov     ds,ax
    mov     es,ax
    mov     fs,ax
    mov     gs,ax
    mov     ss,ax
    mov     esp,STACK_LOCATION                  ;; set the stack for the loader


;; -- the final task is to enable paging
    mov     eax,cr0                             ;; get that control register
    or      eax,1<<31                           ;; set the PG bit
%if ENABLE_CACHE == 1                           ;; enable caches if defined
    or      eax,(1<<29|1<<30)                   ;; enable the CD and NW bits
%endif
    mov     cr0,eax                             ;; all hell can break loose from here!


;;
;; -- If we manage to get here, we now can jump to LoaderMain -- not a call
;;    ---------------------------------------------------------------------
    jmp     LoaderMain


;;
;; -- Make a paging table -- cleared out to be sure
;;    ---------------------------------------------
MakePageTable:
    push    ecx
    push    edi

    call    NextEarlyFrame                      ;; go get a new frame

    mov     edi,eax                             ;; move it to the edi register
    push    eax                                 ;; save eax on the stack
    xor     eax,eax                             ;; clear eax
    mov     ecx,PAGE_SIZE                       ;; the number of bytes to clear
    cld                                         ;; increment
    rep     stosd                               ;; clear the block
    pop     eax                                 ;; pop the address

    pop     edi                                 ;; pop saved registers
    pop     ecx
    ret


;;
;; -- Create an IDT entry to call an ISR where eax holds the section selector and edx holds the
;;    handler address.  The value of the IDT entry will be held in edx:eax.
;;    -----------------------------------------------------------------------------------------
MakeIdtEntry:
    push    esi
    push    edi

    mov     esi,eax                             ;; save the selector
    mov     edi,edx                             ;; save the address

    xor     eax,eax                             ;; zero out the eax register
    xor     edx,edx                             ;; zero out the edx register

    shl     esi,16                              ;; adjust the selector
    mov     eax,esi                             ;; move it to the destination; esi is now free
    mov     esi,edi                             ;; get the address
    and     esi,0xffff                          ;; and mask out the low part
    or      eax,esi                             ;; move that part into the return register

    and     edi,0xffff0000                      ;; get the high section
    mov     edx,edi                             ;; put it in the return register
    or      edx,0x8e00                          ;; set the flags

    pop     edi
    pop     esi
    ret


;;
;; -- this function will simple get the next frame and decrement the value
;;    --------------------------------------------------------------------
NextEarlyFrame:                                 ;; called from C
    mov     eax,[earlyFrame]                    ;; get the next frame to allocate
    inc     dword [earlyFrame]                  ;; we actually wanted the previous val; reset it
    shl     eax,12                              ;; make the frame an address
    ret


;;
;; -- Set the stack up and the jump to the kernel itself
;;    --------------------------------------------------
JumpKernel:
    mov     eax,[esp+4]
    mov     esp,[esp+8]
    jmp     eax


;;
;; -- we need a small stack for out first calls to get a stack
;;    --------------------------------------------------------
    align       4
stack:
    dd          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    dd          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
stack_top:
