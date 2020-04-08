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
@@ -- make sure we have a value for whether we include debug code
@@    -----------------------------------------------------------
.ifndef ENABLE_DEBUG_ENTRY
    .equ        ENABLE_DEBUG_ENTRY,0
.endif


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

@@
@@ -- The first thing we need it the TTB1 table -- which must be 16K aligned
@@    ----------------------------------------------------------------------

@@ -- first a little housekeeping -- we need to allocate the ttb1 table (see below)
    bl      MakePageTable                       @@ call 4 times to get 4 frames!  need 16K
    ldr     r1,=mmuLvl1Table                    @@ get the location we will share the ttb1 frame
    str     r0,[r1]                             @@ save that value
    bl      MakePageTable
    bl      MakePageTable
    bl      MakePageTable


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

    mcr     p15,0,r0,c12,c0,0                   @@ !!! physical addr (identity mapped); need to chg in kernel mmu


@@===================================================================================================================

    ldr     r0,=mmuLvl1Count                    @@ get our frame
    mov     r1,#4                               @@ calculate the size of the
    str     r1,[r0]                             @@ set the frame count


@@
@@ -- Now we want to set up paging.  The first order of business here is to map the TTL1 table for managing
@@    entries.  This requires a new page to be allocated and mapped into the TTL1 table itself.
@@
@@    +-------//-------++-------//-------++-------//-------++---------------------//---------------------------+
@@    |                ||                ||                ||                              |.|F|F|F|F|.|F|F|F|F|
@@    |   0xff404000   ||   0xff405000   ||   0xff406000   ||       0xff407000             |.|F|F|F|F|.|F|F|F|F|
@@    |                ||                ||                ||                              |.|4|5|6|7|.|C|D|E|F|
@@    +-------//-------++-------//-------++-------//-------++---------------------//---------------------------+
@@
@@    So, the first order of business here is to allocate 2 TTL2 tables and map them into `0xff4`-`0xff7` and into
@@    `0xffc`-`0xfff`.
@@
@@    r10 -- the address of the TTL1 table
@@    r11 -- the address of the TTL2 table for `0xffc` - `0xfff`
@@    r12 -- the adddres of the TTL2 table for `0xff4` - `0xff7`
@@
@@    ------------------------------------------------------------------------------------------------------------
@@
@@    First order of business: create a TTL2 table for the TTL1 table pages
@@    ---------------------------------------------------------------------
    bl      MakePageTable                       @@ get a new TTL2 table
    mov     r12,r0                              @@ save this in r12


@@
@@ -- insert this into the TTL1 table for 4 pages
@@    -------------------------------------------
    ldr     r1,=mmuLvl1Table                    @@ get the ttl1 table address
    ldr     r1,[r1]
    mov     r10,r1                              @@ save this in the proper register

    mov     r2,#0xff4                           @@ this is the index where we need to add the ttl2 table
    lsl     r2,#2                               @@ convert this into an offset
    add     r2,r1                               @@ r2 now contains the address of the TTL1 Entry

    mov     r9,#(ARMV7_MMU_TTL1_ENTRY)          @@ set the flags we will use to set the TTL1 entry
    orr     r9,r12                              @@ r9 now contains the value to place in the TTL1 entry

    mov     r8,#0                               @@ start a counter

.iter1:
    str     r9,[r2]                             @@ complete the mapping

    add     r9,#1024                            @@ next TTL2 Table
    add     r2,#4                               @@ next TTL1 Entry
    add     r8,#1                               @@ next iter

    cmp     r8,#4                               @@ are we done?
    blo     .iter1


@@
@@ -- Next we need to do the same thing for the TTL2 managment table space
@@    --------------------------------------------------------------------
    bl      MakePageTable                       @@ get a new TTL2 table
    mov     r11,r0                              @@ save this in r11


@@
@@ -- insert this into the TTL1 table for 4 pages
@@    -------------------------------------------
    mov     r1,r10                              @@ get the TTL1 table location

    mov     r2,#0xffc                           @@ this is the index where we need to add the ttl2 table
    lsl     r2,#2                               @@ convert this into an offset
    add     r2,r1                               @@ r2 now contains the address of the TTL1 Entry

    mov     r9,#(ARMV7_MMU_TTL1_ENTRY)          @@ set the flags we will use to set the TTL1 entry
    orr     r9,r11                              @@ r9 now contains the value to place in the TTL1 entry

    mov     r8,#0                               @@ start a counter

.iter2:
    str     r9,[r2]                             @@ complete the mapping

    add     r9,#1024                            @@ next TTL2 Table
    add     r2,#4                               @@ next TTL1 Entry
    add     r8,#1                               @@ next iter

    cmp     r8,#4                               @@ are we done?
    blo     .iter2


@@
@@ -- Next we go back and map the individual entries in the TTL2 table for the TTL1 table pages to their final
@@    location.
@@
@@    0xff400000:
@@    ff4___________________ ff5___ ff6___ ff7___
@@    +-------------------//-+--//--+--//--+--//--+      * Entry 04 will point to the frame for 0xff404000
@@    |-|-|-|-|0|0|0|0|.|    |      |      |      |      * Entry 05 will point to the frame for 0xff405000
@@    |-|-|-|-|4|5|6|7|.|    |      |      |      |      * Entry 06 will point to the frame for 0xff406000
@@    +-------------------//-+--//--+--//--+--//--+      * Entry 07 will point to the frame for 0xff407000
@@
@@    Now, we need to go back and update the TTL1 table entries to the TTL2 tables for the TTL1Entry management
@@    space.  This will involved updating the address in r10 to point to the correct addresses in r12
@@    ---------------------------------------------------------------------------------------------------------
    mov     r0,r12                              @@ r0 holds the base address for the ttl2 table
    mov     r1,r10                              @@ r1 holds the base address for the ttl1 table frames
    mov     r2,#4                               @@ r2 is the index of the ttl2 entry we need
    lsl     r2,#2                               @@ ... and make that into an offset

    add     r0,r2                               @@ r0 now holds the address of the entry to set
    mov     r8,#0                               @@ start a counter

    mov     r9,#(ARMV7_MMU_KRN_DATA&0xffff)     @@ set the flags we will use to set the TTL1 entry
    movt    r9,#(ARMV7_MMU_KRN_DATA>>16)        @@ ... part 2
    orr     r1,r9                               @@ r1 now holds the value to map

.l3:
    str     r1,[r0]                             @@ perform the mapping

    add     r8,#1                               @@ increment the counter
    add     r0,#4                               @@ next TTL2 entry
    add     r1,#4096                            @@ next TTL1 page

    cmp     r8,#4                               @@ are we done?
    blo     .l3                                 @@ loop if not


@@
@@ -- finally we need a TTL2 table for the address `0xff401000` for use in clearing frames.  We will add that
@@    though we do not need to actually map anything therein.  We do, however, need to map that in the management
@@    tables.
@@    -----------------------------------------------------------------------------------------------------------
    bl      MakePageTable                       @@ gp get a new page table; r0 holds this address
    mov     r7,r0                               @@ save that value for later
    mov     r2,#0xff0                           @@ r2 is the index of the ttl1 entry we need
    lsl     r2,#2                               @@ ... and make that into an offset

    add     r2,r10                              @@ r2 now holds the address of the entry to set
    mov     r8,#0                               @@ start a counter

    mov     r9,#ARMV7_MMU_TTL1_ENTRY            @@ set the flags we will use to set the TTL1 entry
    orr     r0,r9                               @@ r1 now holds the value to map

.l4:
    str     r0,[r2]                             @@ perform the mapping

    add     r8,#1                               @@ increment the counter
    add     r2,#4                               @@ next TTL1 entry
    add     r0,#1024                            @@ next TTL2 page

    cmp     r8,#4                               @@ are we done?
    blo     .l4                                 @@ loop if not


@@
@@ -- At this point we can use a full-featured function to complete the mappings.
@@    ... and we have a lot of them to do.
@@
@@                             3322222222221111111111
@@                             10987654321098765432109876543210
@@                             --------------------------------
@@    This value needs to be 0b00000000000000010001110000001110 for regular normal memory
@@    This value needs to be 0b00000000000000010000110000000110 for devices (MMIO memory)
@@                             +----------+|||||+-+++|+--+|||++
@@                                   |     ||||| | | N  | ||| |
@@              The 1MB frame addr --+     ||||| | | o  | ||| +-------- Section/Supersection entry
@@              Non Secure ----------------+|||| | | t  | |||
@@              Indicates super section ----+||| | |    | |||
@@              Not Global ------------------+|| | | U  | ||+---------- B (set to 1)
@@              Sharable ---------------------+| | | s  | |+----------- C (set to 1)
@@              Access Permissions no wrt [2] -+ | | e  | +------------ Execute Never
@@              Memory Region -------------------+ | d  |
@@              Access Permissions any priv [1:0] -+    +--------------- Domain (use 0 for now)
@@
@@    Identity map the mboot section at 1MB
@@
@@    Measureable results:
@@    * 0x1000000 maps to TTL2 table (expect 0x1008000)
@@    * 0x1000004 maps to TTL2 table 0x1008400
@@    * 0x1000008 maps to TTL2 table 0x1008800
@@    * 0x100000c maps to TTL2 table 0x1008c00
@@    * 0x1008400 maps to frame 0x100000
@@    * 0x1008404 maps to frame 0x101000
@@    ---------------------------------------------------------------------------------------------------------
    ldr     r0,=mbPhys                          @@ get the mboot physical address
    ldr     r0,[r0]

    ldr     r3,=mbSize                          @@ get the size of the mboot
    ldr     r3,[r3]
    lsr     r3,#12                              @@ number of pages to write

    mov     r2,#(ARMV7_MMU_KRN_ANY&0xffff)      @@ load the flags
    movt    r2,#(ARMV7_MMU_KRN_ANY>>16)         @@ load the flags

    mov     r8,#0                               @@ start the counter

@@ -- perform the mapping
.loop2:
    mov     r1,r0                               @@ identity map
    bl      MapPageFull                         @@ map the page
.if ENABLE_DEBUG_ENTRY
    bl      DumpMmuTables
.endif
    add     r8,#1
    add     r0,#4096

    cmp     r8,r3                               @@ are we done
    blo     .loop2

@@
@@ -- now map the loader code/data section (0x80000000)
@@    -------------------------------------------------
    ldr     r0,=ldrVirt                         @@ get the virtual address of the loader
    ldr     r0,[r0]

    ldr     r1,=ldrPhys                         @@ get the physical address of the loader
    ldr     r1,[r1]

    ldr     r3,=ldrSize                         @@ get the size of the loader
    ldr     r3,[r3]
    lsr     r3,#12

    mov     r2,#(ARMV7_MMU_KRN_DATA&0xffff)     @@ load the flags
    movt    r2,#(ARMV7_MMU_KRN_DATA>>16)        @@ load the flags

    mov     r8,#0                               @@ start a counter

.loop3:
    bl      MapPageFull                         @@ map the page
    add     r8,#1
    add     r0,#4096
    add     r1,#4096

    cmp     r8,r3                               @@ are we done?
    blo     .loop3


@@
@@ -- now map the pergatory code/data section (0x80400000)
@@    ----------------------------------------------------
    ldr     r0,=sysVirt                         @@ get the virtual address of the syscall section
    ldr     r0,[r0]

    ldr     r1,=sysPhys                         @@ get the physical address of the syscall section
    ldr     r1,[r1]

    ldr     r3,=sysSize                         @@ get the size of the syscall section
    ldr     r3,[r3]
    lsr     r3,#12

    mov     r2,#(ARMV7_MMU_KRN_ANY&0xffff)      @@ load the flags
    movt    r2,#(ARMV7_MMU_KRN_ANY>>16)         @@ load the flags

    mov     r8,#0                               @@ start a counter

.loop4:
    bl      MapPageFull                         @@ map the page
    add     r8,#1
    add     r0,#4096
    add     r1,#4096

    cmp     r8,r3                               @@ are we done?
    blo     .loop4


@@
@@ -- now map the kernel code section (0x80800000)
@@    --------------------------------------------
    ldr     r0,=txtVirt                         @@ get the virtual address of the kernel code
    ldr     r0,[r0]

    ldr     r1,=txtPhys                         @@ get the physical address of the kernel code
    ldr     r1,[r1]

    ldr     r3,=txtSize                         @@ get the size of the kernel code
    ldr     r3,[r3]
    lsr     r3,#12

    mov     r2,#(ARMV7_MMU_KRN_CODE&0xffff)     @@ load the flags
    movt    r2,#(ARMV7_MMU_KRN_CODE>>16)        @@ load the flags

    mov     r8,#0                               @@ start a counter

.loop5:
    bl      MapPageFull                         @@ map the page
    add     r8,#1
    add     r0,#4096
    add     r1,#4096

    cmp     r8,r3                               @@ are we done?
    blo     .loop5


@@
@@ -- now map the kernel data section (0x81000000)
@@    --------------------------------------------
    ldr     r0,=dataVirt                        @@ get the virtual address of the kernel data
    ldr     r0,[r0]

    ldr     r1,=dataPhys                        @@ get the physical address of the kernel data
    ldr     r1,[r1]

    ldr     r3,=dataSize                        @@ get the size of the kernel data
    ldr     r3,[r3]
    lsr     r3,#12

    mov     r2,#(ARMV7_MMU_KRN_DATA&0xffff)     @@ load the flags
    movt    r2,#(ARMV7_MMU_KRN_DATA>>16)        @@ load the flags

    mov     r8,#0                               @@ start a counter

.loop6:
    bl      MapPageFull                         @@ map the page
    add     r8,#1
    add     r0,#4096
    add     r1,#4096

    cmp     r8,r3                               @@ are we done?
    blo     .loop6


@@
@@ -- map the stack
@@    -------------
    mov     r0,#(STACK_BASE&0xffff)             @@ stack location
    movt    r0,#(STACK_BASE>>16)                @@ stack location
    mov     r7,r0                               @@ save the stack, we wll replace it in a bit
    add     r7,#STACK_SIZE                      @@ and calculate the proper top of stack
    mov     r1,sp                               @@ current stack
    mov     r9,#0xf000
    movt    r9,#0xffff
    and     r1,r9                               @@ mask out the the base physical address
    mov     r2,#(ARMV7_MMU_KRN_DATA&0xffff)
    movt    r2,#(ARMV7_MMU_KRN_DATA>>16)
    bl      MapPageFull


@@
@@ -- finally map the Exception Vector Table
@@    --------------------------------------
    mov     r0,#(EXCEPT_VECTOR_TABLE&0xffff)
    movt    r0,#(EXCEPT_VECTOR_TABLE>>16)

    ldr     r1,=intTableAddr
    ldr     r1,[r1]
    mov     r0,r1                               @@ this needs to be fixed later (identity mapped; see MmuInit.c)

    mov     r2,#(ARMV7_MMU_KRN_DATA&0xffff)
    movt    r2,#(ARMV7_MMU_KRN_DATA>>16)

    bl      MapPageFull


@@
@@ -- The last thing to do here is to loop through the TTL1 table for TTL1 Entries that are mapped (every 4th
@@    index where i % 4 == 0) and map that frame into the TTL2 management table.
@@    -------------------------------------------------------------------------------------------------------
    mov     r0,r10                              @@ get the address of the TTL2 table
    mov     r1,#0                               @@ this is the address for which we are checking
    mov     r8,#0                               @@ start a counter

.iter4:
    ldr     r2,[r0]                             @@ get the TTL1 Entry
    and     r3,r2,#3                            @@ get the fault bits
    cmp     r3,#0                               @@ is it unmapped?
    beq     .next                               @@ if so, we skip the mapping

    lsr     r4,r1,#22                           @@ move the index into the TTL2 table
    lsl     r4,#2                               @@ convert that to an offset
    add     r4,r11                              @@ get the address of the TTL2 Entry

    mov     r9,#0xfff                           @@ the mask to be inverted (and not)
    bic     r3,r2,r9                            @@ mask out the frame address
    mov     r9,#(ARMV7_MMU_KRN_DATA&0xffff)     @@ set the flags we will use to set the TTL1 entry
    movt    r9,#(ARMV7_MMU_KRN_DATA>>16)        @@ ... part 2
    orr     r3,r9                               @@ r3 now holds the value to map
    str     r3,[r4]                             @@ complete the mapping

.if ENABLE_DEBUG_ENTRY
    push    {r0}
    mov     r0,r8
    bl      OutputHex           @@ iteration count
    mov     r0,#':'
    bl      OutputChar
    mov     r0,#' '
    bl      OutputChar
    pop     {r0}
    push    {r0}
    bl      OutputHex           @@ TTL1 Entry address
    mov     r0,#' '
    bl      OutputChar
    mov     r0,r1
    bl      OutputHex           @@ Virtual address
    mov     r0,#' '
    bl      OutputChar
    mov     r0,r2
    bl      OutputHex           @@ TTL1 entry value
    mov     r0,#' '
    bl      OutputChar
    mov     r0,r4
    bl      OutputHex           @@ TTL2 entry address
    mov     r0,#' '
    bl      OutputChar
    mov     r0,r3
    bl      OutputHex           @@ TTL2 entry value
    bl      OutputNewline
    pop     {r0}
.endif

.next:
    add     r0,#16                              @@ next TTL1 entry (skip 3 entries)
    add     r8,#1                               @@ next iteration
    mov     r9,#0x0000                          @@ the iteration value
    movt    r9,#0x0040                          @@ the iteration value upper bits
    add     r1,r9                               @@ the next TTL1 base address

    cmp     r8,#1024                            @@ are we done yet?
    blo     .iter4


@@ -- now we enable caches
    ldr     r0,=mmuLvl1Table
    ldr     r0,[r0]

    mcr     p15,0,r0,c2,c0,0                    @@ write the ttl1 table to the TTBR0 register
    mcr     p15,0,r0,c2,c0,1                    @@ write the ttl1 table to the TTBR1 register; will use later

    mov     r1,#0                               @@ The number of bits to use to determine which table; short format
    mcr     p15,0,r1,c2,c0,2                    @@ write these to the TTBCR0

    mov     r1,#0xffffffff                      @@ All domains can manage all things by default
    mcr     p15,0,r1,c3,c0,0                    @@ write these to the domain access register

.if ENABLE_DEBUG_ENTRY
    ldr     r0,=pg
    bl      DumpMmuTables

    mov     r0,#0x03f8
    movt    r0,#0xffc0
    bl      DumpMmuTables
.endif


@@ -- now we enable paging
    mrc     p15,0,r1,c1,c0,0                    @@ This gets the cp15 register 1 and puts it in r0
    orr     r1,#1                               @@ set bit 0
    mcr     p15,0,r1,c1,c0,0                    @@ Put the cp15 register 1 back, with the MMU enabled

pg:
.if ENABLE_BRANCH_PREDICTOR
    mcr     p15,0,r0,c7,c5,6                    @@ invalidate the branch predictor (required maintenance when enabled)
.endif

.if ENABLE_CACHE
    mcr     p15,0,r0,c7,c1,0                    @@ invalidate all instruction caches (required maintenance when enabled)
.endif

@@ -- finally jump to the loader initialization function
    mov     sp,r7                               @@ replace the stack
    b       LoaderMain                          @@ straight jump -- not a call


@@
@@ -- Make a paging table -- clear out to be sure it's blank
@@    ------------------------------------------------------
MakePageTable:
    push    {r1-r9,lr}
    bl      NextEarlyFrame                      @@ get a frame

    mov     r4,r0                               @@ this is the address to start clearing
    add     r9,r4,#4096                         @@ get the end of the block

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
@@ -- This function will make a new TTL2 table for the address given in r0.
@@    This function operates only on the TTL1 table physical address.  It is assumed
@@    that the TTL2 table does not exist and if it does will be overwritten.
@@    The management tables are not maintained by this function.
@@    ------------------------------------------------------------------------------
NewTTL2Table:
    push    {r1-r9,lr}                          @@ save a whole bunch of registers

    mov     r5,r0                               @@ our address to map
    bl      KrnTtl1Entry4                       @@ and get the address of the "mod 4" entry in r5
    bl      MakePageTable                       @@ r0 will hold the physical address of the new table

.if ENABLE_DEBUG_ENTRY
    push    {r0}
    ldr     r0,=mmu6
    bl      OutputString
    mov     r0,r5
    bl      OutputHex
    bl      OutputNewline
    pop     {r0}
.endif

@@ -- finally, we need the value to load into the TTL1 table entry
    mov     r9,#ARMV7_MMU_TTL1_ENTRY
    orr     r4,r0,r9                            @@ r4 holds the value value to load

    mov     r8,#0                               @@ this is the number of entries we loaded

@@ -- we can now load the entries
.ttl2loop:
    str     r4,[r5]

    add     r4,#0x400                           @@ move to the next TTL2 table of 4
    add     r5,#4                               @@ move to the next TTL1 entry

    add     r8,#1                               @@ one more done
    cmp     r8,#4                               @@ are we done?
    blo     .ttl2loop                           @@ loop

@@ -- all done; exit
    pop     {r1-r9,pc}                          @@ clean up our mess


@@
@@ -- This function will complete a page mapping in the physical table address.  It operates on a
@@    4K aligned 4 X 1K group of TTL tables in physcial memory (not in virtual management space).
@@    r0 -- the address of the TTL2 table in physcal memory
@@    r1 -- the address to map (will be converted into an index to the TTL2 table)
@@    r2 -- the address of the frame to complete the mapping OR'd with the flags (the value to store in the table)
@@    r0 to r9 are all preserved.
@@    ------------------------------------------------------------------------------------------------------------
MapPage:
    push    {r0-r2,r7,r9,lr}

    mov     r7,r1                               @@ set the register for the call
    bl      KrnTtl2EntryOffset                  @@ go get the offset into the r0 table

    add     r0,r7                               @@ get the proper physicasl address
    str     r2,[r0]                             @@ complete the mapping

    pop     {r0-r2,r7,r9,pc}

@@
@@ -- This fucntion will perform a full mappng -- nap the page and if neessary make a new TTL2 table
@@    with the necessary mgmt entry to go along with it.  This funtion is only available once the management
@@    tables have been set up.  This function will read those tables to determine how to complete the
@@    mappings.  This differs from the OS functions in that this reads physical addresses whereas the OS
@@    will use the manaement tables to perfoem the same function.
@@    r0 -- the address to map -- this does not need to be a "clean" address
@@    r1 -- the frame to which to map the page (will also be cleaned up)
@@    r2 -- the flags that will be used to decorate the page in the tables
@@    returns a cleaned up virtual address
@@    -------------------------------------------------------------------------------------------------------
MapPageFull:
    push    {r1-r9,lr}

    mov     r9,#0xf000                          @@ establish the cleanup mask
    movt    r9,#0xffff                          @@ establish the cleanup mask
    and     r1,r9                               @@ clean frame
    and     r0,r9                               @@ clean page
    mov     r9,#0xfff
    and     r2,r9                               @@ clean bits
    orr     r1,r2                               @@ r1 holds the complete ttl2 entry
    push    {r0}                                @@ save the return address


@@
@@ -- now, r0 has the address to map; r1 has the frame with its bits; r2-r9 are scratch regs
@@    the first order of business is to determine if we need a new TTL2 table
@@    ---------------------------------------------------------------------------------------
    mov     r5,r0
    bl      KrnTtl1Entry4                       @@ get the address of the TTL1 entry (r5)

.if ENABLE_DEBUG_ENTRY
    push    {r0}
    mov     r0,r5
    bl      OutputHex
    bl      OutputNewline
    pop     {r0}
.endif

    ldr     r3,[r5]                             @@ r3 has the ttl1 entry
    mov     r9,r3                               @@ we need a copy
    and     r9,#0x03                            @@ get the fault bits
    cmp     r9,#(ARMV7_MMU_TTL1_ENTRY)          @@ is there a ttl2 table

    beq     .haveTtl2                           @@ if we have one, no need to make a new one

@@ -- we need to make a new ttl2 table
    push    {r0}                                @@ save our work
    bl      NewTTL2Table                        @@ go get a new table
    mov     r3,r0                               @@ put that entry in the right register
    mov     r4,r0                               @@ we need to save te physical address as well
    pop     {r0}                                @@ restore the saved work

@@ -- map the ttl2 management entry
    push    {r0-r2}                             @@ save our work again
    mov     r1,r3                               @@ get the address to map
    mov     r2,r4                               @@ get the physical frame
    bl      MapTtl2Mgmt                         @@ take care of manaement mapping
    pop     {r0-r2}                             @@ restore our work

@@ -- we now have a ttl2 address; r3 has the address of the TTL2 table
.haveTtl2:
.if ENABLE_DEBUG_ENTRY
    push    {r0}
    ldr     r0,=mmu5
    bl      OutputString
    bl      OutputNewline
    pop     {r0}
.endif

    mov     r9,#0xf000                          @@ establish the cleanup mask
    movt    r9,#0xffff                          @@ establish the cleanup mask
    and     r4,r3,r9                            @@ r4 now holds the physical ttl2 frame address

@@ -- now the ttl2 entry address
    mov     r7,r0                               @@ the address to map
    bl      KrnTtl2EntryOffset                  @@ get the offset into the physical table

    add     r5,r7,r4                            @@ r5 now holds the address of the ttl2 entry

@@ -- complete the mapping
    str     r1,[r5]                             @@ after all that, this is it

.if ENABLE_DEBUG_ENTRY
    push    {r0}
    mov     r0,r5
    bl      OutputHex
    bl      OutputNewline
    pop     {r0}
.endif

    pop     {r0}                                @@ restore the return address
    pop     {r1-r9,pc}




@@
@@ -- Map a TTL2 frame in Management space.  This function will map a 4K page into the
@@    proper location in the TTL2 table management space (i.e. from 0xffc00000 on).  For this,
@@    we need the top 12 bits of the address, which will put us to an index.   That will then
@@    need to be converted to an offset.
@@    r1 -- an address anywhere in the TTL2 table (will be cleaned up)
@@    r2 -- the physical address of the TTL2 table as a frame of the page (bits will be added)
@@    returns nothing of value
@@    -----------------------------------------------------------------------------------------
MapTtl2Mgmt:
    push    {r2,r6,r9,lr}

    mov     r6,r1                               @@ get the address to map
    bl      KrnTtl2Mgmt                         @@ r6 will not contain the address

    mov     r9,#(ARMV7_MMU_KRN_DATA&0xffff)     @@ get the flags we want to set
    movt    r9,#(ARMV7_MMU_KRN_DATA>>16)
    orr     r2,r9                               @@ r2 holds the value value to load with the extra bits

    str     r2,[r6]                             @@ complete the mapping

    pop     {r2,r6,r9,pc}


@@
@@ -- This function will calculate the Kernel TTL1 Entry address for a given address in r5.
@@    The proper TTL1 Entry address will be returned in r5.  No other registers will be
@@    changed.
@@    -------------------------------------------------------------------------------------
KrnTtl1Entry:
    push    {r0, lr}

    lsr     r5,#20                              @@ get the index of the entry
    lsl     r5,#2                               @@ convert that to an offset

    ldr     r0,=mmuLvl1Table                    @@ load the base address of the table
    ldr     r0,[r0]

    add     r5,r0                               @@ now, calculate the address

    pop     {r0, pc}


@@
@@ -- This function will calculate the Kernel TTL1 Entry address for the first of a block
@@    of 4 TTL2 tables.
@@    -----------------------------------------------------------------------------------
KrnTtl1Entry4:
    push    {r9,lr}

    bl      KrnTtl1Entry
    mov     r9,#0xfff0
    movt    r9,#0xffff
    and     r5,r9

    pop     {r9,pc}


@@
@@ -- This function will calculate the Management address for a TTL2 table for an address
@@    provided in r6.  The address will be returned in r6.  All other registers are
@@    unchanged.
@@    -----------------------------------------------------------------------------------
KrnTtl2Mgmt:
    push    {r0, lr}

    lsr     r6,#22                              @@ get the index of the entry
    lsl     r6,#2                               @@ convert that to an offset

    ldr     r0,=ttl2Mgmt                        @@ load the base address of the table
    ldr     r0,[r0]

    add     r6,r0                               @@ now, calculate the address

    pop     {r0, pc}


@@
@@ -- This function will calculate the offset into a physical frame for a TTL2 entry.
@@    The address to calculate is passed in r7 and the offset is returned in the same
@@    register.  No other registers are changed.  Unlike the 3 above functions, this
@@    merely returns an address offset, not the final address itself.
@@    -------------------------------------------------------------------------------
KrnTtl2EntryOffset:
    push    {r0,lr}

    lsr     r7,#12                              @@ get the index
    mov     r0,#0x3ff                           @@ mask out the top bits
    and     r7,r0
    lsl     r7,#2                               @@ make that an offset


    pop     {r0,pc}                             @@ return



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



.if ENABLE_DEBUG_ENTRY
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

    sub     r1,#4
    b       OutputHex.loop


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
@@ -- Dump the page tables for an address
@@    -----------------------------------
DumpMmuTables:
    push    {r0-r10,lr}

    mov     r10,r0                              @@ save off the address we are working with


@@ -- Output the header
    ldr     r0,=mmu1
    bl      OutputString
    mov     r0,r10
    bl      OutputHex
    bl      OutputNewline

    ldr     r0,=mmu2
    bl      OutputString
    bl      OutputNewline


@@ -- Now, get the TTL1 entry for the address
    ldr     r0,=mmu3
    bl      OutputString

    mov     r0,r10,lsr #20
    lsl     r0,#2                               @@ this is an offset

    ldr     r1,=mmuLvl1Table
    ldr     r1,[r1]

    add     r0,r1                               @@ this is the address of the TTL1 entry
    mov     r2,r0

    bl      OutputHex
    mov     r0,#' '
    bl      OutputChar

    ldr     r0,[r2]                             @@ ..  and now we have the entry
    mov     r9,r0                               @@ save this value
    bl      OutputHex
    bl      OutputNewline

    and     r0,#3                               @@ get the fault bits
    cmp     r0,#0                               @@ and if they are 0, we can exit
    popeq   {r0-r10,pc}

@@ -- Now, get the TTL2 entry for the address
    mov     r7,#0x3ff
    bic     r8,r9,r7                            @@ mask off the table address

    mov     r0,r10,lsr #12                      @@ adjust for the index
    and     r0,#0xff                            @@ get the index
    lsl     r0,#2                               @@ and convert that to an address
    add     r8,r0                               @@ r8 now holds the TTL2 entry address

    ldr     r0,=mmu4
    bl      OutputString


    mov     r0,r8                               @@ get the address of the entry
    bl      OutputHex
    mov     r0,#' '
    bl      OutputChar

    ldr     r0,[r8]                             @@ ..  and now we have the entry
    bl      OutputHex
    bl      OutputNewline


    pop     {r0-r10,pc}
.endif

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


.if ENABLE_DEBUG_ENTRY
    .align      4
hex:
    .byte       '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'

    .align      4
mmu1:
    .asciz      "Dumping MMU tables for address "

    .align      4
mmu2:
    .asciz      "-----------------------------------------"

    .align      4
mmu3:
    .asciz      "TTL1 entry at address "

    .align      4
mmu4:
    .asciz      "TTL2 entry at address "

    .align      4
mmu5:
    .asciz      "Mapping a new TTL2 table.  "

    .align      4
mmu6:
    .asciz      "New TTL2 table at entry "

    .align      4
mmu7:
    .asciz      "Jumping to Loader\r\n"

.endif

