@@===================================================================================================================
@@
@@  entry.s -- This will be the entry point for the combined loader/kernel.
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@  All of the early initialization will take place here.  For ARM, this will be to set up the Interrupt Vector
@@  Table, the Paging Tables for TTBR0 (no split tables yet), and then enable paging.  Once those are done, we can
@@  begin initialization.
@@
@@  One additional note here, the rpi2b at least will start the CPU in 'hyp' mode.  If this is the case, we need
@@  to thunk it back down into 'svc' mode where this kernel is intended to run.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@  2019-Feb-09  Initial   0.3.0   ADCL  Initial version
@@
@@===================================================================================================================


.include "constants.inc"


@@
@@ -- make sure that if the required symbols are defined; Branch Predictor
@@    --------------------------------------------------------------------
.ifndef ENABLE_BRANCH_PREDICTOR
    .equ        ENABLE_BRANCH_PREDICTOR,0
.endif


@@
@@ -- Cache (instruction and data)
@@    ----------------------------
.ifndef ENABLE_CACHE
    .equ        ENABLE_CACHE,0
.endif


@@
@@ -- explose some global symbols
    .global     entry
    .global     NextEarlyFrame
    .global     JumpKernel
    .global     earlyFrame
    .global     mmuLvl1Count                    @@ the number of frames for this level 1 mmu table
    .global     mmuLvl1Table                    @@ the frame for the level 1 mmu table
    .global     mb1Data
    .global     mb2Data
    .global     intTableAddr                    @@ IVT location
    .global     ldrStackFrame
    .global     OutputChar
    .global     stack_top                       @@ each core will use this scratch space


@@
@@ -- This is the multiboot header.  During the link process, it will be relocated to the beginning of the
@@    resulting ELF file.  This is required since the multiboot header needs to be in the first 4/8K (I think).
@@    ---------------------------------------------------------------------------------------------------------
    .section    .mboot,"ax"


@@
@@ -- This is the multiboot 1 header
@@    ------------------------------
multiboot_header:
@@ -- magic fields
    .long       MAGIC1
    .long       MBFLAGS
    .long       0-MAGIC1-MBFLAGS

@@
@@ -- The PMM frame early allocation mechanism
@@    ----------------------------------------
earlyFrame:
    .long       (4 * 1024)                      @@ start allocating at 4MB


@@
@@ -- these variables hold the physical addresses of the respective table
@@    -------------------------------------------------------------------
mmuLvl1Count:
    .long       0
mmuLvl1Table:
    .long       0
ttl2Mgmt:
    .long       0


@@
@@ -- some additional variables for use later
@@    ---------------------------------------
mb1Data:
    .long       0
mb2Data:
    .long       0

intTableAddr:
    .long       0

ldrStackFrame:
    .long       0


@@
@@ -- This is the entry point.  Multiboot will load this kernel image at 0x100000.  The first order of
@@    business is to dump us out of 'hyp' mode if we are there and to make sure we are in 'svc' mode
@@    before we go on.
@@
@@    On entry r0 holds the proper multiboot signature for MB1, r1 holds the address of the MBI,
@@    and r2 holds the value 0.
@@    ------------------------------------------------------------------------------------------------
    .section    .text.entry,"ax"
    .align      4

entry:
@@
@@ -- figure out which CPU we are on; only CPU 0 continues after this
@@
@@    Note that this code is expected to be _ALWAYS_ executing on Core0.  This check is there just in case
@@    something happens later down the road with a change/bug in `bootcode.bin`.  If this were to happen,
@@    we will emulate the expected behavior of the firmware by holding the core in a holding pen until we
@@    are ready to release it.
@@    ----------------------------------------------------------------------------------------------------
    mrc     p15,0,r3,c0,c0,5                    @@ Read Multiprocessor Affinity Register
    and     r3,r3,#0x3                          @@ Extract CPU ID bits
    cmp     r3,#0
    bne     entryApHold                         @@ if we’re not on CPU0, go to the holding pen

@@ -- Save off the MBI structure
save:
    ldr     r2,=mb1Data                         @@ get the address to put it in
    str     r1,[r2]                             @@ and save the address

    mrs     r0,cpsr                             @@ get the current program status register
    and     r0,#0x1f                            @@ and mask out the mode bits
    cmp     r0,#0x1a                            @@ are we in hyp mode?
    beq     hyp                                 @@ if we are in hyp mode, go to that section
    cpsid   iaf,#0x13                           @@ if not switch to svc mode, ensure we have a stack for the kernel; no ints
    b       cont                                @@ and then jump to set up the stack

@@ -- from here we are in hyp mode so we need to exception return to the svc mode
hyp:
    mrs     r0,cpsr                             @@ get the cpsr again
    and     r0,#~0x1f                           @@ clear the mode bits
    orr     r0,#0x013                           @@ set the mode for svc
    orr     r0,#1<<6|1<<7|1<<8                  @@ disable interrupts as well
    msr     spsr_cxsf,r0                        @@ and save that in the spsr

    ldr     r0,=cont                            @@ get the address where we continue
    msr     elr_hyp,r0                          @@ store that in the elr register

    eret                                        @@ this is an exception return

@@ -- everyone continues from here
cont:


@@
@@ -- some early CPU initialization
@@    -----------------------------
    mrc     p15,0,r3,c1,c0,0                    @@ get the SCTLR
.if ENABLE_BRANCH_PREDICTOR
    orr     r3,#(1<<11)                         @@ set the Z bit for branch prediction (may be forced on by HW!)
.endif

.if ENABLE_CACHE
    orr     r3,#(1<<2)                          @@ set the data cache enabled
    orr     r3,#(1<<12)                         @@ set the instruction cache enabled
.endif
    mcr     p15,0,r3,c1,c0,0                    @@ write the SCTLR back with the caches enabled


@@ -- Clear out kernel bss -- this needs to be adjusted for the kernel physical load address
initialize:
    ldr     sp,=stack_top                       @@ set the stack

    ldr     r4,=bssPhys
    ldr     r4,[r4]
    ldr     r9,=bssSize
    ldr     r9,[r9]
    add     r9,r9,r4

@@ -- values to store
    mov     r5,#0
    mov     r6,#0
    mov     r7,#0
    mov     r8,#0

bssLoop:
@@ -- store multiple at r4
    stmia   r4!,{r5-r8}

@@ -- If we're still below _bssEnd, loop
    cmp     r4,r9
    blo     bssLoop

@@ -- now, we can get the stack
    bl      NextEarlyFrame                      @@ -- get a stack frame
    add     sp,r0,#STACK_SIZE                   @@ -- and set the new stack
    ldr     r1,=ldrStackFrame                   @@ -- get the address to store the stack
    str     r0,[r1]                             @@ -- and save that for later

@@ -- get a frame and initialize the Vector Table
    bl      NextEarlyFrame
    ldr     r1,=intTableAddr                    @@ get the location we will share the vbar frame
    str     r0,[r1]                             @@ save that value

@@ -- prepare the bit-coded jump instruction
    movw    r1,#0xf018                          @@ and fill in the bottom 2 bytes
    movt    r1,#0xe59f                          @@ this is the bitcode of the jump instruction

@@ -- fill in the jump instructions
    str     r1,[r0,#0x00]                       @@ These are the jump instructions (not used -- reset)
    str     r1,[r0,#0x04]                       @@ These are the jump instructions (UNDEF)
    str     r1,[r0,#0x08]                       @@ These are the jump instructions (SVC CALL)
    str     r1,[r0,#0x0c]                       @@ These are the jump instructions (PREFETCH ABORT)
    str     r1,[r0,#0x10]                       @@ These are the jump instructions (DATA ABORT)
    str     r1,[r0,#0x14]                       @@ These are the jump instructions (not used -- hyp mode)
    str     r1,[r0,#0x18]                       @@ These are the jump instructions (IRQ INT)
    str     r1,[r0,#0x1c]                       @@ These are the jump instructions (FIQ INT)

@@ -- fill in the jump targets
    ldr     r1,=ResetTarget
    str     r1,[r0,#0x20]                       @@ The target for a reset (never used but filled in anyway)

    ldr     r1,=UndefinedTarget
    str     r1,[r0,#0x24]                       @@ The target for an UNDEF

    ldr     r1,=SuperTarget
    str     r1,[r0,#0x28]                       @@ The target for SUPER

    ldr     r1,=PrefetchTarget
    str     r1,[r0,#0x2c]                       @@ The target for PREFETCH ABORT

    ldr     r1,=DataAbortTarget
    str     r1,[r0,#0x30]                       @@ The target for DATA ABORT

    mov     r1,#0
    str     r1,[r0,#0x34]                       @@ The target for not used (hyp)

    ldr     r1,=IRQTarget
    str     r1,[r0,#0x38]                       @@ The target for IRQ

    ldr     r1,=FIQTarget
    str     r1,[r0,#0x3c]                       @@ The target for FIQ

@@ -- Set up the VBAR to use an absolute address
    mrc     p15,0,r1,c1,c0,0
    and     r1,r1,#(~(1<<13))
    mcr     p15,0,r1,c1,c0,0

    ldr     r0,=EXCEPT_VECTOR_TABLE             @@ load the address of the actual table location in virtual memory
    mcr     p15,0,r0,c12,c0,0                   @@ and set the VBAR


@@===================================================================================================================


@@
@@ -- Now, we can start to build out the MMU tables (now the long-descriptor format); get the top level table
@@    -------------------------------------------------------------------------------------------------------
    bl      MakePageTable                       @@ get a 4K aligned level 1 table (only 4 64bit entries are used)
    ldr     r1,=mmuLvl1Table                    @@ get the location of the top level table addres (used elsewhere)
    str     r0,[r1]                             @@ save that value for use later

@@
@@ -- Now, get 4 table tables for each of the level 1 entries that are possible (VA address bits 31:30)
@@    Note that this is a 64-bit value and the major element (the table address) is split between 2 words
@@    ---------------------------------------------------------------------------------------------------
    mov     r5,r0                               @@ save off the root of the tree to r8

@@ -- get a new table for entry 0
    bl      MakePageTable                       @@ we have a new table
    mov     r6,r0                               @@ r6 will contain the table 0 for mapping
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA     @@ set the low bits to be a table (upper bits will still be 0)
    orr     r0,r1
    str     r0,[r5,#0]                          @@ populate the entry, little endian

    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r5,#4]

@@ -- get a new table for entry 1
    bl      MakePageTable                       @@ we have a new table
    mov     r7,r0                               @@ r7 will contain the table 1 for mapping
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA     @@ set the low bits to be a table (upper bits will still be 0)
    orr     r0,r1
    str     r0,[r5,#0x08]                       @@ populate the entry, little endian

    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r5,#0x0c]

@@ -- fix up some recursive mappings
    ldr     r0,[r5,#8]                          @@ get the lower half of the table
    str     r0,[r7,#0xff8]                      @@ recursively map this table
    ldr     r0,[r5,#0]                          @@ get the lower half of the table
    str     r0,[r7,#0xff0]                      @@ and recursively map that as well

    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r7,#0xff4]                      @@ and recursively map that as well
    str     r0,[r7,#0xffc]                      @@ and recursively map that as well

@@ -- get a new table for entry 2
    bl      MakePageTable                       @@ we have a new table
    mov     r8,r0                               @@ r8 will contain the table 2 for mapping
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA     @@ set the low bits to be a table (upper bits will still be 0)
    orr     r0,r1
    str     r0,[r5,#0x10]                       @@ populate the entry, little endian

    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r5,#0x14]


@@ -- get a new table for entry 3
    bl      MakePageTable                       @@ we have a new table
    mov     r9,r0                               @@ r9 will contain the table 3 for mapping
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA     @@ set the low bits to be a table (upper bits will still be 0)
    orr     r0,r1
    str     r0,[r5,#0x18]                       @@ populate the entry, little endian

    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r5,#0x1c]

@@ -- fix up some recursive mappings
    ldr     r0,[r5,#0x18]                       @@ get the lower half of the table
    str     r0,[r9,#0xff8]                      @@ recursively map this table
    ldr     r0,[r5,#0x10]                       @@ get the lower half of the table
    str     r0,[r9,#0xff0]                      @@ and recursively map that as well

    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r9,#0xff4]                      @@ and recursively map that as well
    str     r0,[r9,#0xffc]                      @@ and recursively map that as well


@@
@@ -- now, we have the level 1 table, and the level 2 tables.  The level 3 tables will be created on demand
@@    start by creating the level 3 table for exception vectors, and clearing frames, which is at `0xff401000`.
@@    This can be statically calculated:
@@    level 1 index: 0xff401000 >> 30 or 0x03 (so we will use the table in r9)
@@    level 2 index: 0xff401000 >> 21 & 0x1ff or index 0x1da or offset 0xfd0 (so we want to update address [r9,#fd0])
@@    level 3 index: 0xff401000 >> 12 & 0x1ff or index 1 (update [r5,#8])
@@    ---------------------------------------------------------------------------------------------------------------
    bl      MakePageTable                       @@ get a new frame
    mov     r5,r0                               @@ save this location in to r5 for use below
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA
    orr     r0,r1                               @@ this is a table record
    str     r0,[r9,#0xfd0]                      @@ store this table entry

    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r9,#0xfd4]


@@ -- we need a starting point
    ldr     r0,=intTableAddr                    @@ get address of the exception vector table
    ldr     r0,[r0]
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_EXEC
    orr     r0,r1                               @@ convert that the the low 32-bits

    mov     r1,#ARMV7_PAGE_UPPER_ATTRS_EXEC     @@ get the upper 32 bits

@@ -- Complete the mapping
    str     r0,[r5,#8]                          @@ load the upper record bits
    str     r1,[r5,#0x0c]                       @@ load the lower record bits


@@
@@ -- now we can get into the process of mapping the kernel; starting with the multiboot code/data at 1MB...
@@    Again, this can be statically calculated:
@@    level 1 index: 0x00100000 >> 30 or 0x00 (so use the table in r6)
@@    level 2 index: 0x00100000 >> 21 & 0x1ff or index 0x000 or offset 0x000 (so we want to update address [r6,#0])
@@    level 3 index: 0x00100000 >> 12 & 0x1ff or index 0x100 or offset 0x800 (so we want to update addr [r5,#0x800])
@@    --------------------------------------------------------------------------------------------------------------
    bl      MakePageTable                       @@ get a new frame
    mov     r5,r0                               @@ save this location in to r5 for use below
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA
    orr     r0,r1                               @@ this is a table record
    str     r0,[r6,#0]                          @@ store this table entry

    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r6,#4]


@@ -- we need a starting point
    ldr     r0,=mbPhys                          @@ get the mboot physical address starting point
    ldr     r0,[r0]
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_EXEC
    orr     r0,r1                               @@ convert that the the low 32-bits

    mov     r1,#ARMV7_PAGE_UPPER_ATTRS_EXEC     @@ get the upper 32 bits

@@ -- and we need a number of frames
    ldr     r2,=mbSize                          @@ get the size of the mboot
    ldr     r2,[r2]
    lsr     r2,#12                              @@ number of pages to write into the table

    mov     r3,#0x800                           @@ this is the offset we are loading


@@ -- now perform the mappings
.mbLoop:
    str     r0,[r5,r3]                          @@ load the upper record bits
    add     r3,#4                               @@ move to the next word
    str     r1,[r5,r3]                          @@ load the lower record bits

    add     r0,#PAGE_SIZE                       @@ move to the next page
    add     r3,#4                               @@ move to the next entry
    sub     r2,#1                               @@ one fewer mapping to do
    cmp     r2,#0                               @@ are we done?
    bhi     .mbLoop                             @@ loop if we have more to do


@@
@@ -- now we can get into the process of mapping the loader; starting with the loader code/data at 2GB...
@@    Again, this can be statically calculated:
@@    level 1 index: 0x80000000 >> 30 or 0x02 (so use the table in r8)
@@    level 2 index: 0x80000000 >> 21 & 0x1ff or index 0x000 or offset 0x0000 (so we want to upd addr [r8,#0x0000])
@@    level 3 index: 0x80000000 >> 12 & 0x1ff or index 0x000 or oddset 0x0000 (so we want to update addr [r5,#0x000])
@@    ---------------------------------------------------------------------------------------------------------------
    bl      MakePageTable                       @@ get a new frame
    mov     r5,r0                               @@ save this location in to r5 for use below
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA
    orr     r0,r1                               @@ this is a table record
    str     r0,[r8,#0]                          @@ store this table entry

    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r8,#4]


@@ -- we need a starting point
    ldr     r0,=ldrPhys                         @@ get the loader physical address starting point
    ldr     r0,[r0]
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_EXEC
    orr     r0,r1                               @@ convert that the the low 32-bits

    mov     r1,#ARMV7_PAGE_UPPER_ATTRS_EXEC     @@ get the upper 32 bits

@@ -- and we need a number of frames
    ldr     r2,=ldrSize                         @@ get the size of the loader
    ldr     r2,[r2]
    lsr     r2,#12                              @@ number of pages to write into the table

    mov     r3,#0x000                           @@ this is the offset we are loading


@@ -- now perform the mappings
.ldrLoop:
    str     r0,[r5,r3]                          @@ load the upper record bits
    add     r3,#4                               @@ move to the next word
    str     r1,[r5,r3]                          @@ load the lower record bits

    add     r0,#PAGE_SIZE                       @@ move to the next page
    add     r3,#4                               @@ move to the next entry
    sub     r2,#1                               @@ one fewer mapping to do
    cmp     r2,#0                               @@ are there any left?
    bhi     .ldrLoop                            @@ loop if we have more to do


@@
@@ -- now we can get into the process of mapping the system call pages
@@    Again, this can be statically calculated:
@@    level 1 index: 0x80400000 >> 30 or 0x02 (so use the table in r8)
@@    level 2 index: 0x80400000 >> 21 & 0x1ff or index 0x002 or offset 0x0010 (so we want to upd addr [r8,#0x0010])
@@    level 3 index: 0x80400000 >> 12 & 0x1ff or index 0x000 or oddset 0x0000 (so we want to update addr [r5,#0x000])
@@    ---------------------------------------------------------------------------------------------------------------
    bl      MakePageTable                       @@ get a new frame
    mov     r5,r0                               @@ save this location in to r5 for use below
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA
    orr     r0,r1                               @@ this is a table record
    str     r0,[r8,#0x10]                       @@ store this table entry

    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r8,#0x14]


@@ -- we need a starting point
    ldr     r0,=sysPhys                         @@ get the syscall physical address starting point
    ldr     r0,[r0]
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_EXEC
    orr     r0,r1                               @@ convert that the the low 32-bits

    mov     r1,#ARMV7_PAGE_UPPER_ATTRS_EXEC     @@ get the upper 32 bits

@@ -- and we need a number of frames
    ldr     r2,=sysSize                         @@ get the size of the syscall section
    ldr     r2,[r2]
    lsr     r2,#12                              @@ number of pages to write into the table

    mov     r3,#0x000                           @@ this is the offset we are loading


@@ -- now perform the mappings
.sysLoop:
    str     r0,[r5,r3]                          @@ load the upper record bits
    add     r3,#4                               @@ move to the next word
    str     r1,[r5,r3]                          @@ load the lower record bits

    add     r0,#PAGE_SIZE                       @@ move to the next page
    add     r3,#4                               @@ move to the next entry
    sub     r2,#1                               @@ one fewer mapping to do
    cmp     r2,#0                               @@ are there any left?
    bhi     .sysLoop                            @@ loop if we have more to do


@@
@@ -- now we can get into the process of mapping the kernel
@@    Again, this can be statically calculated:
@@    level 1 index: 0x80800000 >> 30 or 0x02 (so use the table in r8)
@@    level 2 index: 0x80800000 >> 21 & 0x1ff or index 0x004 or offset 0x0020 (so we want to upd addr [r8,#0x0020])
@@    level 3 index: 0x80800000 >> 12 & 0x1ff or index 0x000 or oddset 0x0000 (so we want to update addr [r5,#0x000])
@@    ---------------------------------------------------------------------------------------------------------------
    bl      MakePageTable                       @@ get a new frame
    mov     r5,r0                               @@ save this location in to r5 for use below
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA
    orr     r0,r1                               @@ this is a table record
    str     r0,[r8,#0x20]                       @@ store this table entry

    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r8,#0x24]


@@ -- we need a starting point
    ldr     r0,=txtPhys                         @@ get the kernel text physical address starting point
    ldr     r0,[r0]
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_EXEC
    orr     r0,r1                               @@ convert that the the low 32-bits

    mov     r1,#ARMV7_PAGE_UPPER_ATTRS_EXEC     @@ get the upper 32 bits

@@ -- and we need a number of frames
    ldr     r2,=txtSize                         @@ get the size of the kernel text
    ldr     r2,[r2]
    lsr     r2,#12                              @@ number of pages to write into the table

    mov     r3,#0x000                           @@ this is the offset we are loading


@@ -- now perform the mappings
.txtLoop:
    str     r0,[r5,r3]                          @@ load the upper record bits
    add     r3,#4                               @@ move to the next word
    str     r1,[r5,r3]                          @@ load the lower record bits

    add     r0,#PAGE_SIZE                       @@ move to the next page
    add     r3,#4                               @@ move to the next entry
    sub     r2,#1                               @@ one fewer mapping to do
    cmp     r2,#0                               @@ are there any left?
    bhi     .txtLoop                            @@ loop if we have more to do


@@
@@ -- now we can get into the process of mapping the kernel data
@@    Again, this can be statically calculated:
@@    level 1 index: 0x81000000 >> 30 or 0x02 (so use the table in r8)
@@    level 2 index: 0x81000000 >> 21 & 0x1ff or index 0x008 or offset 0x0040 (so we want to upd addr [r8,#0x0040])
@@    level 3 index: 0x81000000 >> 12 & 0x1ff or index 0x000 or oddset 0x0000 (so we want to update addr [r5,#0x000])
@@    ---------------------------------------------------------------------------------------------------------------
    bl      MakePageTable                       @@ get a new frame
    mov     r5,r0                               @@ save this location in to r5 for use below
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA
    orr     r0,r1                               @@ this is a table record
    mov     r4,#0x0040                          @@ ...  offset too big
    str     r0,[r8,r4]                          @@ store this table entry

    add     r4,#4
    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r8,r4]


@@ -- we need a starting point
    ldr     r0,=dataPhys                        @@ get the kernel data physical address starting point
    ldr     r0,[r0]
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA
    orr     r0,r1                               @@ convert that the the low 32-bits

    ldr     r1,=ARMV7_PAGE_UPPER_ATTRS_DATA     @@ get the upper 32 bits

@@ -- and we need a number of frames
    ldr     r2,=dataSize                        @@ get the size of the kernel data
    ldr     r2,[r2]
    lsr     r2,#12                              @@ number of pages to write into the table

    mov     r3,#0x000                           @@ this is the offset we are loading


@@ -- now perform the mappings
.dataLoop:
    str     r0,[r5,r3]                          @@ load the upper record bits
    add     r3,#4                               @@ move to the next word
    str     r1,[r5,r3]                          @@ load the lower record bits

    add     r0,#PAGE_SIZE                       @@ move to the next page
    add     r3,#4                               @@ move to the next entry
    sub     r2,#1                               @@ one fewer mapping to do
    cmp     r2,#0                               @@ are there any left?
    bhi     .dataLoop                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  @@ loop if we have more to do


@@
@@ -- map the stack:
@@    level 1 index: 0xff800000 >> 30 or 0x03 (so use the table in r9)
@@    level 2 index: 0xff800000 >> 21 & 0x1ff or index 0x1fc or offset 0xfe0 (so update address [r9,#fe0])
@@    level 3 index: 0xff800000 >> 12 & 0x1ff or index 0x00 (so update address [r5,#0])
@@    ----------------------------------------------------------------------------------------------------
    bl      MakePageTable                       @@ get a new frame
    mov     r5,r0                               @@ save this location in to r5 for use below
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA
    orr     r0,r1                               @@ this is a table record
    str     r0,[r9,#0xfe0]                      @@ store this table entry

    ldr     r0,=ARMV7_PAGE_UPPER_ATTRS_DATA
    str     r0,[r9,#0xfe4]


@@ -- we need a starting point
    ldr     r0,=ldrStackFrame                   @@ get the kernel data physical address starting point
    ldr     r0,[r0]
    mov     r1,#ARMV7_PAGE_LOWER_ATTRS_DATA
    orr     r0,r1                               @@ convert that the the low 32-bits

    ldr     r1,=ARMV7_PAGE_UPPER_ATTRS_DATA     @@ get the upper 32 bits

@@ -- Perform the mapping
    str     r0,[r5]                             @@ load the upper record bits
    str     r1,[r5,#4]                          @@ load the lower record bits


@@
@@ -- The following appear to need to happen in a proper order
@@    --------------------------------------------------------


@@ -- before we enable paging, we need to decorate the MAIR registers.
    mov     r2,#(ARMV7_MAIR0_VAL & 0xffff)
    movt    r2,#(ARMV7_MAIR0_VAL >> 16)
    mcr     p15,0,r2,c10,c2,0

    mov     r2,#(ARMV7_MAIR1_VAL & 0xffff)
    movt    r2,#(ARMV7_MAIR1_VAL >> 16)
    mcr     p15,0,r2,c10,c2,1
    dsb
    isb

@@ -- set the TTBR0/1 registers
    ldr     r0,=mmuLvl1Table
    ldr     r0,[r0]
    mov     r1,#0

@@ -- set the paging tables
    mcrr    p15,0,r0,r1,c2                      @@ write the ttl1 table to the TTBR0 register
    mcrr    p15,1,r0,r1,c2                      @@ write the ttl1 table to the TTBR1 register; will use later
    dsb
    isb

@@ -- Updage the TTBCR
    mov     r1,#0x3501                          @@ the low bits for the TTBCR
    movt    r1,#0xb500                          @@ set the long descriptor format
    mcr     p15,0,r1,c2,c0,2                    @@ write these to the TTBCR
    dsb
    isb

@@ -- set the DACR register
    mov     r1,#0xffffffff                      @@ All domains can manage all things by default
    mcr     p15,0,r1,c3,c0,0                    @@ write these to the domain access register
    dsb
    isb

@@ -- Print some debugging information
    push    {r0}

    mov     r0,#0xf000
    movt    r0,#0xffff
    bl      OutputAddrTables

    mov     r0,#0xe000
    movt    r0,#0xffff
    bl      OutputAddrTables

    mov     r0,#0xf000
    movt    r0,#0x7fff
    bl      OutputAddrTables

    mov     r0,#0xe000
    movt    r0,#0x7fff
    bl      OutputAddrTables

    mov     r0,#0x1000
    movt    r0,#0xff40
    bl      OutputAddrTables

    ldr     r0,=.paging
    bl      OutputAddrTables

    mov     r0,#0x80000000
    bl      OutputAddrTables

    mov     r0,#0
    movt    r0,#0x8040
    bl      OutputAddrTables

    mov     r0,#0
    movt    r0,#0x8080
    bl      OutputAddrTables

    mov     r0,#0x1000
    movt    r0,#0x8080
    bl      OutputAddrTables

    mov     r0,#0x2000
    movt    r0,#0x8080
    bl      OutputAddrTables

    mov     r0,#0x3000
    movt    r0,#0x8080
    bl      OutputAddrTables

    mov     r0,#0
    movt    r0,#0x8100
    bl      OutputAddrTables

    mov     r0,#0
    movt    r0,#0xff80
    bl      OutputAddrTables

    mov     r0,#0x0000
    movt    r0,#0xffc0
    bl      OutputAddrTables

    ldr     r0,=pagingEnable
    bl      OutputString
    bl      OutputNewline

    pop     {r0}



@@ -- now we enable paging
    mrc     p15,0,r1,c1,c0,0                    @@ This gets the cp15 register 1 and puts it in r0
    orr     r1,#1                               @@ set bit 0
    mcr     p15,0,r1,c1,c0,0                    @@ Put the cp15 register 1 back, with the MMU enabled
    dsb
    isb

@@ -- fix the stack
    ldr     sp,=STACK_LOCATION

.paging:


pg:
.if ENABLE_BRANCH_PREDICTOR
    mcr     p15,0,r0,c7,c5,6                    @@ invalidate the branch predictor (required maintenance when enabled)
.endif

.if ENABLE_CACHE
    mcr     p15,0,r0,c7,c1,0                    @@ invalidate all instruction caches (required maintenance when enabled)
.endif

@@ -- finally jump to the loader initialization function
    b       LoaderMain                          @@ straight jump -- not a call


@@
@@ -- Make a paging table -- clear out to be sure it's blank
@@    ------------------------------------------------------
MakePageTable:
    push    {r1-r9,lr}
    bl      NextEarlyFrame                      @@ get a frame

    mov     r4,r0                               @@ this is the address to start clearing
    add     r9,r4,#PAGE_SIZE                    @@ get the end of the block

    mov     r5,#0
    mov     r6,#0
    mov     r7,#0
    mov     r8,#0


.ptloop:
    stmia   r4!,{r5-r8}                         @@ clear the memory and increment r4

    cmp     r4,r9                               @@ are we done?
    blo     .ptloop                             @@ loop if not

    pop     {r1-r9,pc}


@@
@@ -- this function will simple get the next frame and  increment the value
@@    Clobbers: r0 (return value), r1, r2
@@    --------------------------------------------------------------------
NextEarlyFrame:                                 @@ also called from C
    ldr     r1,=earlyFrame                      @@ get the address of the frame
    ldr     r2,[r1]                             @@ get the next frame to allocate
    mov     r0,r2                               @@ this will be our return value
    add     r2,#1                               @@ increment the value
    str     r2,[r1]                             @@ save the new next frame
    lsl     r0,#12                              @@ turn this into an address
    mov     pc,lr                               @@ and return


@@
@@ -- jump to the kernel, resetting the stack in the process
@@    ------------------------------------------------------
JumpKernel:
    mov     sp,r1                               @@ set the stack
    mov     pc,r0                               @@ very simply set the new program counter; no fuss


@@
@@ -- we need a small stack for out first calls to get a stack
@@    --------------------------------------------------------
    .section    .data.entry
    .align      4
stack:
    .long       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    .long       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    .long       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
stack_top:





@@
@@ == Debugging code
@@    ==============


    .section    .mboot,"ax"



@@
@@ -- Output a character
@@    ------------------
OutputChar:
    push    {r0,r1,lr}
    and     r0,#0xff

OutputChar.loop:
    mov     r1,#0x5054                          @@ get the status register
    movt    r1,#0x3f21
    ldr     r1,[r1]                             @@ get the status
    and     r1,#(1<<5)                          @@ test the bit
    tst     r1,r1                               @@ is it 0?
    beq     OutputChar.loop

    mov     r1,#0x5040
    movt    r1,#0x3f21
    str     r0,[r1]

    pop     {r0,r1,pc}



@@
@@ -- Output a hex number
@@    -------------------
OutputHex:
    push    {r0-r4,lr}

    mov     r2,r0                               @@ save the value to print
    mov     r0,#'0'                             @@ output "0x"
    bl      OutputChar
    mov     r0,#'x'
    bl      OutputChar

    mov     r1,#28                              @@ set the number of bits to shift

OutputHex.loop:
    mov     r0,r2                               @@ get the nibble
    lsr     r0,r1
    and     r0,#0xf                             @@ and mask out the value

    ldr     r3,=hex                             @@ get the hex offset
    add     r3,r0
    mov     r4,#0
    ldrb    r4,[r3]
    mov     r0,r4

    bl      OutputChar

    cmp     r1,#0                               @@ was this the last nibble
    popeq   {r0-r4,pc}

    tst     r1,#0x0f                            @@ is this an even digit
    moveq   r0,#' '                             @@ output a space
    bleq    OutputChar

    sub     r1,#4

    b       OutputHex.loop


@@
@@ -- Output a 64-bit hex number (r1 is high bits; r0 is low bits)
@@    ------------------------------------------------------------
OutputHex64:
    push    {r0-r9,lr}

    mov     r2,r1                               @@ r2 now holds the high bits
    mov     r1,r0                               @@ r1 now holds the low bits

    @@ -- output preamble.
    mov     r0,#'0'
    bl      OutputChar

    mov     r0,#'x'
    bl      OutputChar

    mov     r3,#28                              @@ number of bits to shift

OH64.loop1:
    mov     r0,r2                               @@ get the nibble
    lsr     r0,r3
    and     r0,#0xf                             @@ and mask out the value

    ldr     r4,=hex                             @@ get the hex offset
    add     r4,r0
    mov     r5,#0                               @@ zero out the reg that will hold the hex digit
    ldrb    r5,[r4]                             @@ get the hex digit

    mov     r0,r5                               @@ set the digit to print
    bl      OutputChar

    tst     r3,#0x0f                            @@ is this an even digit
    moveq   r0,#' '                             @@ output a space
    bleq    OutputChar

    cmp     r3,#0                               @@ was this the last nibble
    beq     OH64.next                           @@ go to the next byte

    sub     r3,#4
    b       OH64.loop1

OH64.next:
    mov     r2,r1                               @@ r2 now holds the low bits

    mov     r3,#28                              @@ number of bits to shift

OH64.loop2:
    mov     r0,r2                               @@ get the nibble
    lsr     r0,r3
    and     r0,#0xf                             @@ and mask out the value

    ldr     r4,=hex                             @@ get the hex offset
    add     r4,r0
    mov     r5,#0                               @@ zero out the reg that will hold the hex digit
    ldrb    r5,[r4]                             @@ get the hex digit

    mov     r0,r5                               @@ set the digit to print
    bl      OutputChar

    cmp     r3,#0                               @@ was this the last nibble
    popeq   {r0-r9,pc}                          @@ all done

    tst     r3,#0x0f                            @@ is this an even digit
    moveq   r0,#' '                             @@ output a space
    bleq    OutputChar

    sub     r3,#4
    b       OH64.loop2


@@
@@ -- Output a string
@@    ---------------
OutputString:
    push    {r0,r1,lr}
    mov     r1,r0                               @@ save the string

OutputString.loop:
    mov     r0,#0
    ldrb    r0,[r1]                             @@ get the byte to output
    add     r1,#1
    cmp     r0,#0
    popeq   {r0,r1,pc}                          @@ if NULL char we can exit

    bl      OutputChar                          @@ output the char
    b       OutputString.loop                   @@ loop to the end


@@
@@ -- Output a number of spaces
@@    -------------------------
OutputSpaces:
    push    {r1,lr}
    mov     r1,r0

.loopSpaces:
    mov     r0,#' '
    bl      OutputChar
    subs    r1,#1
    bne     .loopSpaces

    pop     {r1,pc}

@@
@@ -- Output New line
@@    ---------------
OutputNewline:
    push    {r0,lr}

    mov     r0,#13
    bl      OutputChar

    mov     r0,#10
    bl      OutputChar

    pop     {r0,pc}



@@
@@ -- Dump the MMU Tables for an address
@@    ----------------------------------
OutputAddrTables:
    push    {r1-r11,lr}
    mov     r11,r0                  @@ save the address to print...

    @@ -- output the top line
    bl      OutputNewline
    bl      OutputNewline
    ldr     r0,=line1
    bl      OutputString
    mov     r0,r11
    bl      OutputHex
    bl      OutputNewline
    bl      OutputNewline

    @@ -- output line1a
    ldr     r0,=line1a
    bl      OutputString
    mrrc    p15,0,r0,r1,c2                      @@ write the ttl1 table to the TTBR0 register
    bl      OutputHex64
    mov     r0,#' '
    bl      OutputNewline

    @@ -- output line1aa
    ldr     r0,=line1aa
    bl      OutputString
    mrrc    p15,1,r0,r1,c2                      @@ write the ttl1 table to the TTBR0 register
    bl      OutputHex64
    bl      OutputNewline

    @@ -- output line1b
    ldr     r0,=line1b
    bl      OutputString
    mrc     p15,0,r0,c2,c0,2                    @@ write the ttl1 table to the TTBR0 register
    bl      OutputHex
    bl      OutputNewline
    bl      OutputNewline


    @@ -- output line #2
    ldr     r0,=line2
    bl      OutputString
    bl      OutputNewline

    @@ -- output line #3
    ldr     r0,=line3
    bl      OutputString
    bl      OutputNewline

    @@ -- output line #4 or level 1 entry
    ldr     r0,=line4a
    bl      OutputString
    ldr     r0,=mmuLvl1Table
    ldr     r0,[r0]
    mov     r10,r0
    bl      OutputHex
    mov     r0,#4
    bl      OutputSpaces
    mov     r0,r11,lsr #30      @@ get the index
    mov     r9,r0,lsl #3        @@ and convert that to an address
    bl      OutputHex
    mov     r0,#3
    bl      OutputSpaces
    add     r0,r10,r9           @@ the address of the index
    mov     r10,r0
    bl      OutputHex
    mov     r0,#4
    bl      OutputSpaces
    ldr     r0,[r10]            @@ get the low 32 bits
    ldr     r8,[r10,#4]         @@ get the high 32 bits
    mov     r9,#0xfff
    and     r9,r0
    bic     r0,r9
    mov     r10,r0
    bl      OutputHex
    mov     r0,#4
    bl      OutputSpaces
    mov     r1,r8
    mov     r0,r9
    bl      OutputHex64
    bl      OutputNewline


    @@ -- output line #5 or level 2 entry
    ldr     r0,=line5a
    bl      OutputString
    mov     r0,r10
    bl      OutputHex
    mov     r0,#4
    bl      OutputSpaces
    mov     r0,r11,lsr #21      @@ get the index
    mov     r1,#0x1ff
    and     r0,r1
    mov     r9,r0,lsl #3        @@ and convert that to an address
    bl      OutputHex
    mov     r0,#3
    bl      OutputSpaces
    add     r0,r10,r9           @@ the address of the index
    mov     r10,r0
    bl      OutputHex
    mov     r0,#4
    bl      OutputSpaces
    ldr     r0,[r10]            @@ get the low 32 bits
    ldr     r8,[r10,#4]         @@ get the high 32 bits
    mov     r9,#0xfff
    and     r9,r0
    bic     r0,r9
    mov     r10,r0
    bl      OutputHex
    mov     r0,#4
    bl      OutputSpaces
    mov     r1,r8
    mov     r0,r9
    bl      OutputHex64
    bl      OutputNewline

    @@ -- output line #6 or level 3 entry
    ldr     r0,=line6a
    bl      OutputString
    mov     r0,r10
    bl      OutputHex
    mov     r0,#4
    bl      OutputSpaces
    mov     r0,r11,lsr #12      @@ get the index
    mov     r1,#0x1ff
    and     r0,r1
    mov     r9,r0,lsl #3        @@ and convert that to an address
    bl      OutputHex
    mov     r0,#3
    bl      OutputSpaces
    add     r0,r10,r9           @@ the address of the index
    mov     r10,r0
    bl      OutputHex
    mov     r0,#4
    bl      OutputSpaces
    ldr     r0,[r10]            @@ get the low 32 bits
    ldr     r8,[r10,#4]         @@ get the high 32 bits
    mov     r9,#0xfff
    and     r9,r0
    bic     r0,r9
    mov     r10,r0
    bl      OutputHex
    mov     r0,#4
    bl      OutputSpaces
    mov     r1,r8
    mov     r0,r9
    bl      OutputHex64
    bl      OutputNewline


    pop     {r1-r11,pc}




    .section    .data.entry
    .align      4
hex:
    .byte       '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'


    .align 4
line1:
    .asciz      "Pre-Paging MMU Tables Dump: Walking the page tables for address "


    .align 4
line1a:
    .asciz      ".. TTBR0 entry is "


    .align 4
line1aa:
    .asciz      ".. TTBR1 entry is "


    .align 4
line1b:
    .asciz      ".. TTBCR is "


    .align 4
line2:
    .asciz      "Level  Tabl-Addr      Index         Entry Addr     Next PAddr     Attr Bits"



    .align 4
line3:
    .asciz      "-----  -----------    -----------   -----------    -----------    ---------------------"



    .align 4
line4a:
    .asciz      "  1    "



    .align 4
line5a:
    .asciz      "  2    "



    .align 4
line6a:
    .asciz      "  3    "


    .align 4
pagingEnable:
    .asciz      "Enabling Paging..."
