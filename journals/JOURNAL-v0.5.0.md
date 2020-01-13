# The Century OS Development Journal - v0.5.0

This version begins a cleanup and alignment effort for the 2 arch's.  There are several things that need to be done to make this code more efficient from start to finish.  There are several things that are no longer needed (since they were to get the kernel loaded and that is rather solid at this point).  There is also quite a bit of debug code that needs to be replaced with `assert()` calls and then prepared to be removed as needed for a release candidate (I know, I am nowhere near that at the moment).

I am going to start using [this wiki page](http://eryjus.ddns.net:3000/projects/century-os/wiki/Low_Memory_Usage_Map) to help me document the Low Memory Usage Map (this is most relevant to x86-pc, but will also cover rpi2b when appropriate).

I am also going to use [this wiki page](http://eryjus.ddns.net:3000/projects/century-os/wiki/GDT) to document the GST setup.  This in particular is specific to x86 as ARM has no concept of segmentation.

## Version 0.5.0a

### 2019-Dec-08

Starting with the `loader` portion of the kernel, I am going to start the cleanup process.  At the same time, I am going to develop out [this wiki page](http://eryjus.ddns.net:3000/projects/century-os/wiki/Loader_Call_Stack) for the execution stack of the loader.

---

OK, so I am really debating about how to move forward from here.  I really thing I want to take the time to write a few routines to help me get the MMU set up properly only and then transfer control into the upper address space for the kernel.  This would mean completely relocating the sections.  And this would once again break everything.

The one thing I am certain about is that I really need to move toward getting into higher memory *much* faster than I am today.  In order to get there, I need to completely map the kernel in `entry.s`.

---

### 2019-Dec-09

The more I think about this the more I like the idea that I should be able to move all code into kernel memory space and only have the `entry.s` file sitting in lower 1MB memory space.  I'm pretty sure the key to success here is going to be ordering the kernel memory properly, in particular so that the kernel space is early on in the memory map.

Another thought here is that I probably want to separate out the kernel entry (`syscall` targets) from the other kernel space, so that my user space has just the syscall targets and kernel entry code while the actual kernel space is loaded up once we have properly changed stacks and upgraded to kernel address space.  I'm not exactly certain how I want to do this yet.

OK, so to think this through.  The top 2GB of virtual memory space is reserved for kernel address space.  Generally, these are broken into 4MB blocks, so I have 512 blocks to play with.  I have plenty of space here.

I think I want to do something like this:
* `0x80000000` (4MB): Loader and 1-time initialization code (frames will be reclaimed)
* `0x80400000` (4MB): Kernel syscall target code (pergatory)
* `0x80800000` (8MB): Kernel code
* `0x81000000` (16MB): Kernel data and bss
* `0x82000000` (992MB): Kernel heap

This, then, means that I really only need to map the memory at `0x00100000` and then at `0x80000000` to get running from `entry.s`.  Moreover, the memory at `0x00100000` should be able to be limited to a few pages which can also be quickly unmapped.

Additionally, I should be able to pick an arbitrary frame from which to start allocating physical memory for pages.  This could easily be set at 8MB and then increased statically as the kernel and its loaded modules grow.  A runtime check (for now) will be needed to verify this it is a good size.

---

I went through Redmine and I have cleaned up the issues.  Though I will add things to the Roadmap, here is what is targeted for [v0.5.0](http://eryjus.ddns.net:3000/versions/17).

The first changes to the `x86-pc.ld` script create a triple fault:

```
    _mbStart = .;
    .entry : {
        *(.mboot)                       /* lead with the multiboot header */
        *(.text.entry)                  /* followed by the code to get the mmu initialized */
    }
    _mbEnd = .;
```

Since I moved all the entry code out of the first page, it does make sense.

---

Question for `freenode#osedv`:

```
[17:04:44] <eryjus> hmmm...  it is better to have 1 GDT across all CPUs with 1 TSS for each CPU?  or multiple GDTs with 1 TSS per CPU?
[17:05:02] <Mutabah> I prefer the former
[17:05:42] <eryjus> ok, what make you prefer that solution?
[17:05:48] <Mutabah> but the latter is more scalable (no need to handle running out of GDT slots)
[17:05:50] <Mutabah> Simpler
[17:06:06] <Mutabah> And uses less memory
[17:06:33] <eryjus> fair enough
...
[17:23:54] <geist> yah i agree with mutabah
[17:24:09] <geist> generally speaking you will probably compile with some sort of MAX_CPUS #define that's reasonable anyway
[17:24:14] <geist> so you can just pre-reserve that many slots
```

---

### 2019-Dec-10

Still triple faulting.  I had to remove the `ltr` to load the TSS since the TSS descriptor no longer resides in the GDT.  This will have to be loaded later.  [This Redmine](http://eryjus.ddns.net:3000/issues/433) was created to track this work.

So, I am triple faulting right after enabling paging.  I expect this -- that tables are not yet complete.

---

OK, I have the `*.ld` files reorganized to the target locations.  I did manage to consolidate several sections making the kernel a little but smaller.  However, I do not have the MMU mapped yet and I need to get on that.

---

For x86-pc I am getting all the way through the entry code.  When I get to the loader, I do not have a stack mapped and that is throwing a `#PF`.  No IDT properly set to handle that, so I get a `#DF` followed by a Triple Fault for the same reasons.

The good news at this point is that I am finally getting to the loader code for x86-pc.

Now, I need to do the same for rpi2b.  But here's the deal with that: I am about to go on vacation.  I have an rpi2b I can take with me, but no ability to debug it.  Then again, I have no ability to debug it here, either -- just a serial connection.  So, maybe I can take the real hardware with me and give it a try.

At the same time, I am not going to have full access to Redmine, so I will have to make some notes on things to add to that system when I have access again.  In the meantime, I just need to "copy" the work I have done into the rpi2b target, however trickier since there is no cute little recursive mapping trick I can do for the management tables.

Before that, though, I will need to get a proper stack for x96-pc.  This should be based at `0xff800000`, and I will need to build a table for that one as well.

---

This, then, gives me a stack to work with.  I should be able to work with that from here.  One more commit and then I'm on to the rpi2b.

---

### 2019-Dec-11

OK, the problem I am going to have with the rpi2b entry point is that there is not cute little recursive mapping trick and I need to be able to maintain all these management tables myself.

---

I think I have all this code done.  I need to figure out how to test it.  I know it will not run on qemu based on the load address.  I beleive I will need to create a special rpi2b-qemu target to get this to work.

---

I was able to find and compile `rpi-boot`.  This allowed me to be able to run my code in qemu (no need for the rpi2b-qemu target).  With that, I have been able to confirm that my code is getting control.  However, I am gettnig a prefetch abort, meaning I have a problem with the paging tables.  Shocker.

---

### 2019-Dec-12

OK, since I have not been using QEMU to debug anything so far, I am going to haev to start learning how to do this.  I need to be able to inspect the contents of the paging tables as I go.

OK, `mmuLvl1Table` and `mmuLvl1Count` both currently reside in high memory.  They are going to have to move.

---

That done, I am finally able to investigate the memory.  And it appears I have an alignment problem with the tables:

```
        ...
 1000004:       01008001        tsteq   r0, r1
 1000008:       01008191                        ; <UNDEFINED> instruction: 0x01008191
 100000c:       01008321        tsteq   r0, r1, lsr #6
 1000010:       010084b1                        ; <UNDEFINED> instruction: 0x010084b1
```

Memory location `0x1000000` holds `0x00000000`, and it should hold the 1K table (which I assume is at `0x01008000`).  However, that table appears to be at the next memory location.

Well, it appears I have 2 problems here:
1.  The alignmment problem.
2.  A problem incrementing the top 22 bits properly (least significant 10 bits remain 0 for the flags).

It looks like I was able to get most of that cleaned up.  But I still have lingering issues:

```
        ...
 1000004:       01008001
 1000008:       01008191
 100000c:       01008321
 1000010:       010084b1
        ...
 1003fd0:       01006001
 1003fd4:       01006191
 1003fd8:       01006321
 1003fdc:       010064b1
        ...
 1003ff0:       01007001
 1003ff4:       01007191
 1003ff8:       01007321
 1003ffc:       010074b1
```

Of the 3 blocks above, the top one is wrong while the bottom 2 are correct.

I also appear to be missing the kernel space mappings.

---

OK, they are not correct.

Picking apart the bottom 12 bits of the address `0x1006xxx` block...
| bits | use | index 1 | index 2 | index 3 | index 4 |
|:----:|:----|:-------:|:-------:|:-------:|:-------:|
| 11:10 | AP[1:0] | 0 | 0 | 0 | 1 |
| 9 | Not used | 0 | 0 | 1 | 0 |
| 8:5 | Domain | 0 | 0xc | 9 | 5 |
| 4 | Execute Never | 0 | 1 | 0 | 1 |
| 3 | Cashable | 0 | 0 | 0 | 0 |
| 2 | Bufferesd | 0 | 0 | 0 | 0 |
| 1 | Set to 1 | 0 | 0 | 0 | 0 |
| 0 | Page Execute Never | 1 | 1 | 1 | 1 |

So, this is a mess!

---

### 2019-Dec-16

Over the last few days or so, I have been able to spend a few minutes here and here sorting out the issues the rpi2b entry code.  There have been several issues and have not properly documented them all.

I do, however, believe I have this code ready to commit.  For both targets, I have been able to get the `entry.s` file to boot all the way through and get paging enabled.  I have a proper stack defined for `LoaderMain()`.

Now, to be fair, I have not accomplished my goals.  The CPU is not is its complete native state.  In particular, interrupts cannot be handled properly on either arch and the GDT is not mapped in kernel space for the x86 arch.  However, what I am able to do is call functions in kernel address space.

Execution is still broken (it will not load/execute the test code), but entry.s is cleaned up.  This is ready for a commit.

---

## Version 0.5.0b

So, the loader is going to be the next thing to address.  This version is concerned with getting that code sorted out.  To do that, I need to articualte the new goals of the loader.  These are:
1. Perform any config that was not able to be done with the `entry.s` file (handle interrupts, proper GDT)
2. Collect any information about hardware (including multiboot structures) and place those in reasonable locations.

This portion is about hardware, not OS structures.

Since the loader is running in kernel-space memory, I should be able to elminate all special loader tricks I was using.  `loader.h` goes away.  Also, the loader variables go away.

This, now, gets me to the point where I can address the serial port for outputting debugging information.  The purpose of the `loaderSerial` variable was the address at which the device was placed.  Essentially, they are the same except for fucntion addresses.

---

I was able to get a successful compile.  I'm certain it will not run so I am not even going to test it.  But I am going to commit this code loally.

---

OK the first order of business is going to be to get the output working again in the loader.  This will be a major part of the rework.

That done, this then brings me to `MmuEarlyInit()`...  This function is going to be very architecture dependent.  So, starting with rpi2b, I need to map the MMIO memory space to the virtual memory location.

---

### 2019-Dec-17

Today I am working on debugging the rpi2b target to get to the point where I can output text for debugging.

---

I was finally able to get the additional issues with `entry.s` addressed and I am now booting into the loader.  With that, I am finding that I am nto able to get past `EntryInit()` due to a problem with `MmuEarlyInit()` due to a problem with `MmuMapToFrame()` due to a problem with the macro `TTL1_ENTRY()` due to the fact that the TTL1 table is not properly mapped to the management space.

In addition, I had to remove all the debugging calls to `kprintf()` from several functions.  I will need to come up with something to replace it.

---

### 2019-Dec-18

OK, let's think this through a little bit (I have been doing some debugging of the paging tables for the rpi2b target):

* The TTL1 table is 16K (4 pages) at phyiscal address `0x1000000`.
* These 4 pages need to be mapped to address `0xff404000`, `0xff405000`, `0xff406000`, and `0xff407000`.
* This means that TTL1 Entry `0xff4` (`0xff404000 >> 20`) needs to have a table assigned to it.
* TTL1 Entry `0xff4` equates to offset `0x3fd0` (`0xff4 << 2`), or at address `0x1003fd0`.
* This also happens to fall on a `4 X 1K` boundary, so no additional adjustment is necessary (`0xff4 & 0xffc` is `0xff4`).
* The TTL2 table for this is allocated at `0x1006000`.
* Given the above information, the following 4 tables will be put in place:
    * Address `0x1003fd0` will receive the table `0x1006000` for TTL1 index `0xff4`
    * Address `0x1003fd4` will receive the table `0x1006400` for TTL1 index `0xff5`
    * Address `0x1003fd8` will receive the table `0x1006800` for TTL1 index `0xff6`
    * Address `0x1003fdc` will receive the table `0x1006c00` for TTL1 index `0xff7`
* With that, the TTL2 table base physical address will be `0x1006000` for index `0xff4`.
* The virtual addresses for the TTL2 tables begin at `0xffc00000`.
* The index from the base of the TTL2 table is `0xff404` (`0xff404000 >> 12`).
* The offset from the base address of the TTL2 table is `0x3fd010` (`0xff404 << 2`).
* The TTL2 entry I need to map will be at address `0xffffd010`.
* The additional TTL2 entries to map will be:
    * Address `0xffffd014` for index `0xff405`.
    * Address `0xffffd018` for index `0xff406`.
    * Address `0xffffd01c` for index `0xff407`.
* The page `0xffffd000` is mapped to frame `0x1007000`
* The index into this particular frame is `0x04`.
* The offset into this particular frame is `0x10` (`0x04 << 2`).
* The table at physical address `0x1007010` is `0`, meaning that this is not mapped.
* The other tables at physical addresses `0x1007014`, `0x1007018`, and `0x100701c` are also `0`.

Now, the macro `TTL1_ENTRY()` is trying to read physical memory starting at address `0xff404000` while the MMU is enabled.  This is not working because of the failure at physical address `0x1007010`.

I think I have defined my first problem to fix.  But not tonight.

---

### 2019-Dec-19

For the start of the first mappings, I have have the following comment block:

```
@@
@@ -- Now we want to set up paging.  The first order of business here is to map the TTL1 table for managing
@@    entries.  This requires a new page to be allocated and mapped into the TTL1 table itself.
@@
@@    The measureable results of this section are expected to be:
@@    * 0x1000000 (for 4 pages) contains the TTL1 table
@@    * create a new TTL2 table block (for managing 4 frames) exp: 0x1006000
@@    * associate a TTL1 entries:
@@        * index 0xff4 (offset 0x3fd0) to address 0x1006000
@@        * index 0xff5 (offset 0x3fd4) to address 0x1006400
@@        * index 0xff6 (offset 0x3fd8) to address 0x1006800
@@        * index 0xff7 (offset 0x3fdc) to address 0x1006c00
@@    * map pages into TTL2 table at 0x1006000:
@@        * index 0x04 (offset 0x10) to address 0x1000000
@@        * index 0x05 (offset 0x14) to address 0x1001000
@@        * index 0x06 (offset 0x18) to address 0x1002000
@@        * index 0x07 (offset 0x1c) to address 0x1003000
```

I should be able to properly measure these results and make any adjustments necessary.

And I need to consider some adjustments:

```
(gdb) p /x *0x1003fd0
$1 = 0x1006001
(gdb) p /x *0x1003fd4
$2 = 0x1006401
(gdb) p /x *0x1003fd8
$3 = 0x1006801
(gdb) p /x *0x1003fdc
$4 = 0x1006c01
(gdb) p /x *0x1006010
$5 = 0x1000003
(gdb) p /x *0x1006014
$6 = 0x0
(gdb) p /x *0x1006018
$7 = 0x0
(gdb) p /x *0x100601c
$8 = 0x0
```

The last 4 entries are `0`.  So, either my test criteria are wrong or the test results are wrong....  And I was incrementing the wrong register, so my code was wrong.

Moving on to the next section....  Which I cleaned up and tested successfully.

---

### 2019-Dec-20

I need to get my head around this.  I know the following logic works:

```C++
//
// -- This is the location of the TTL1/TTL2 Tables in kernel space
//    ------------------------------------------------------------
#define TTL1_KRN_VADDR      0xff404000
#define MGMT_KRN_TTL2       0xfffff000
#define TTL2_KRN_VADDR      0xffc00000


//
// -- These macros assist with the management of the MMU mappings -- picking the address apart into indexes
//    into the various tables
//    -----------------------------------------------------------------------------------------------------
#define KRN_TTL1_ENTRY(a)       (&((Ttl1_t *)TTL1_KRN_VADDR)[(a) >> 20])
#define KRN_TTL1_ENTRY4(a)      (&((Ttl1_t *)TTL1_KRN_VADDR)[((a) >> 20) & 0xffc])
#define KRN_TTL2_MGMT(a)        (&((Ttl2_t *)MGMT_KRN_TTL2)[(a) >> 22])
#define KRN_TTL2_ENTRY(a)       (&((Ttl2_t *)TTL2_KRN_VADDR)[(a) >> 12])
```

So, the question is: how to change this so that it works with my `entry.s` module before I have all these address up and running?

First of all, this is an array in C++ and I need to change this to be addresses of 32-bit words.  So, every one of these will be shifted-left by 2 to multiply by 4.

I think the first order os business here is going to be to code these all out in assembly.  I am using `r9` as a scratch register in `entry.s` and `r8` as a counter.  So each of these 3 key macros (not `KRN_TTL1_ENTRY4`) will use registers `r5`, `r6`, or `r7`.  `KRN_TTL1_ENTRY4` will reuse the same register as `KRN_TTL1_ENTRY` and will be a prerequisite call.  Each should only need to operate on its own register.

---

OK, I was finally able to get into the loader space.  Plus, I was able to get past the management table issues..., well the first of them anyway.

I want to commit this code while I have it.

---

OK, now I can complete the rest of the MMU setup that is needed from the upper memory space.

---

### 2019-Dec-21

Today, my primary goal is to get serial output happening again on the rpi2b.  I have moved the hardware MMIO address space and I have not updated the new location in the `#define`s.  However, I am not yet getting to that place in code.  So, I need to make sure I am getting out of `MmuEarlyInit()`.

I am faulting trying to move the MMIO addresses.  My suspicion is that I have a problem with creating a new TTL2 table and getting that set up properly.

I was able to get a log of this fault sequence and I am getting the following:

```
IN:
0x808018e8:  e1a00006  mov      r0, r6
0x808018ec:  e8bd41f0  pop      {r4, r5, r6, r7, r8, lr}
0x808018f0:  ea00065f  b        #0x80803274

R00=81004a00 R01=00000000 R02=ffffd000 R03=0000d000
R04=ff400000 R05=81004a00 R06=000000c0 R07=0100d000
R08=00000001 R09=0100d000 R10=000287a0 R11=000254cc
R12=05000000 R13=ff400fa8 R14=808018e8 R15=808018e8
PSR=200001d3 --C- A NS svc32
Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x9600003f
...with DFSR 0x7 DFAR 0xff400fa8
Taking exception 3 [Prefetch Abort]
...from EL1 to EL1
...with ESR 0x21/0x8600003f
...with IFSR 0x5 IFAR 0x1005010
Taking exception 3 [Prefetch Abort]
...from EL1 to EL1
...with ESR 0x21/0x8600003f
...with IFSR 0x5 IFAR 0x100500c
```

Now, the `DFSR` register tells me that this is a Second Level translation fault (as in the TTL2 entry could not be found) and the `DFAR` register tells me that this is on address `0xff400fa8`.  What bothers me is the address.  That address is supposed to be used for clearing a frame before placing it into the tables in its final location.  But if there was a problem with that address, I would expect it to fail much closer to `0xff400000`, not near the end of the page.

Now, the value in `r15` confuses me as well.  This address is at the end of the `MmuClearFrame()` function, but there is no memory reference near that instruction (other than stack references).  For a data abort, the problem happened 2 instructions before the value in `pc`, so that instruction would be:

```
808018e0:       e1a00005        mov     r0, r5
```

That makes no sense.  If I take the instruction itself, it was:

```
808018e8:       e1a00006        mov     r0, r6
```

That makes no sense either!

I could consider this a cache consistency problem, but qemu does not emulate cache that I know of.

Ahhh...  but if you look at the value in `r13` (`sp`), there is the offending value!  So, it appears that my stack and the frame to clear are getting mixed up somehow.

I am loading this value from `entry.s`.  I definitely am stomping on myself here.

---

I have that fixed and I have the rpi2b target running to the point just before it opens the serial port for debugging output.

The x86-pc target still triple faults.  And I need to make sure it is getting at least that far.  And it is.

So, I am now able to make the `EarlyInit()` fuctions match between the targets, so I can simplify to 1 version of this function.  It is now called `LoaderEarlyInit()`.

I have some additional cleanup to do for all the work I had done to get to this point.

One of the goals at this point is to figure out a way to have a common location for defining constants but output the proper header file for any given architecture, compiler, or assembler.  I believe I might be able to use `sed` for this...  maybe `awk`.  Or, all `awk`.

Anyway, I now have a `constants` file I can maintain centrally and output the different constants for each target as appropriate.  Later, I will need to be able to selectively output constants depending on the target I am using.  But that will indeed be later.

I also believe I have properly cleaned up the `entry.s` file for both archs.  This moves me on to the `LoaderMain()` function.

`LoaderMain()` is rather trivial, but I cannot execute all the way through this function yet.  The first call is `LoaderFunctionInit()`.

`LoaderFunctionInit()` has some linker-defined variables that I also want to centralize.  I believe I can place those in `loader.h` and have them be in the right spot.  They are part of the loader sections anyway and will go away at a leter time.  If I need to capture them, it is appropriate to move them into kernel data space.

Next in `LoaderMain()` is the call to `LoaderEarlyInit()`.  This then calls `MmuEarlyInit()`.

Now, `MmuEarlyInit()` really needs to be renamed to `MmuInit()`, but I have one of those already.  `MmuInit()` should go away since the MMU really needs to be completely initialized after this call to `MmuEarlyInit()` -- there should be nothing more than routine maintenance to do after `MmuEarlyInit()` is done.  Several things could not be completed at the `entry.s` level since we did not yet have everything ready to go.

So, the first thing to do now it safely remove `MmuInit()`.  This will be done by temporarily renaming the file.

I have also realized that I do not yet have the framebuffer mapped on the rpi2b target.  This is also throwing a fault.

Let me see if I can identify the fault....

I never would have figure on this:

```
IN:
0x80000228:  e30002f0  movw     r0, #0x2f0
0x8000022c:  e3480100  movt     r0, #0x8100
0x80000230:  e5903008  ldr      r3, [r0, #8]
0x80000234:  e12fff33  blx      r3

R00=40040000 R01=fb000000 R02=80003000 R03=00000001
R04=00112008 R05=00000000 R06=00000000 R07=00000000
R08=ff802000 R09=fffff000 R10=000287a0 R11=000254cc
R12=40040000 R13=ff801ff0 R14=fffff1c8 R15=80000228
PSR=400001d3 -Z-- A NS svc32
Taking exception 3 [Prefetch Abort]
...from EL1 to EL1
...with ESR 0x21/0x8600003f
...with IFSR 0x7 IFAR 0x0
Taking exception 3 [Prefetch Abort]
...from EL1 to EL1
...with ESR 0x21/0x8600003f
...with IFSR 0x5 IFAR 0x100500c
```

In this case, I am trying to jump to a `0x00000000` address.

Hmmm, could it be that I am skipping the framebuffer load altogether?  Possible I guess.  But I am returning back to `LoaderEarlyInit()`.

So, thinking about this, what I think to be the case is that I am trying to map the framebuffer in the MMU before I have been able to determine where that it -- from the hardware scan in `PlatformInit()`.  This will have to move.

I am also realizing that the x86-pc `MmuEarlyInit()` function does nothing and it also needs to be properly addressed.  Most of that goes away for real, but I need to finish up with mapping the IVT into kernel space.  Also, with a properly placed infinite loop, I am actually still getting a triple fault, so I need to track that down.  It looks like that final bit of code is not needed, so it was also removed.  What is left to do is set up to map the IVT properly.

---

### 2019-Dec-22

I was able to get this all sorted out this morning.  I found that I was trying to map an address that did not have a Page Table and had assumed it was there.  This was not a problem for rpi2b since I had the TTL1 management space mapped into this same TTL1 entry.  For x86-pc, this was not the case and I had to artifically create this Page Table for this address space.  That identified and fixed, the kernel now boots to the `SerialOpen()` call.

I can continue on with the cleanup.

Next, I want to consider if I can eliminate `LoaderEarlyInit()`.  I think I should because it's only purpose is to call `MmuInit()` and `PlatformInit()`.  It does a couple of other it also does a couple of other things I will be moving to `PlatformInit()`.  So, in short I have a function whose sole purpose is to call 2 functions.  Sounds like unnecessary overhead to me.

---

I committed my changes so far and pushed those to GitHub.

So, now on to the serial port.  I am going to start with x86, since that really should work out of the box.  It does not, so there is some silly problem, I am sure.  In the meantime, it's a triple fault.

That's right, the code is jumping to `0x00000000`.

---

I have several issues sorted at this point, but the rpi2b is not outputting data to the serial port.  There are no faults (that I know of), just no serial output.

Now, I recall there were issues with that using qemu (only 1 serial port was emulated -- whatever one I am not using...), so I cannot take that at face value.

I also think I remember having similar problems earlier on and it ended up being mappings in the MMU.  Hmmmm....

---

OK, the qemu emulator will not emulate the mini-UART.  So, I cannot use qemu as too much of a test-bed.  I also remember having this problem earlier.

Now, I am not certain where I am getting to in real hardware.

So, the reality is I am going to have to figure out how to write to the serial port before I get to the kernel -- at least for now.

---

### 2019-Dec-23

I am wondering if I have a problem with the loader....  I doubt it, but I need to test it to be sure.

Actually, I am struggling to prove that the kernel loader (my code) actually gets control.

---

### 2019-Dec-24

The more I think about this, the more I am drawn to the thought that I really messed up the linker script.

So, from v0.4.6, this is the results from `readelf`:

```
ELF Header:
  Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           ARM
  Version:                           0x1
  Entry point address:               0x1025f4
  Start of program headers:          52 (bytes into file)
  Start of section headers:          575880 (bytes into file)
  Flags:                             0x5000400, Version5 EABI, hard-float ABI
  Size of this header:               52 (bytes)
  Size of program headers:           32 (bytes)
  Number of program headers:         4
  Size of section headers:           40 (bytes)
  Number of section headers:         11
  Section header string table index: 10

Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .loader           PROGBITS        00100000 001000 006001 00  AX  0   0 4096
  [ 2] .text             PROGBITS        80000000 008000 004060 00  AX  0   0  8
  [ 3] .rodata           PROGBITS        80005000 00d000 0028ac 00   A  0   0  4
  [ 4] .stab             PROGBITS        80008000 010000 07376d 00   A  0   0  4
  [ 5] .data             PROGBITS        8007c000 084000 0003dc 00  WA  0   0  8
  [ 6] .bss              NOBITS          8007d000 0843dc 004644 00  WA  0   0  4
  [ 7] .ARM.attributes   ARM_ATTRIBUTES  00000000 0843dc 000039 00      0   0  1
  [ 8] .symtab           SYMTAB          00000000 084418 006600 10      9 1418  4
  [ 9] .strtab           STRTAB          00000000 08aa18 001f1d 00      0   0  1
  [10] .shstrtab         STRTAB          00000000 08c935 000052 00      0   0  1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  y (purecode), p (processor specific)

There are no section groups in this file.

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x00100000 0x00100000 0x06001 0x06001 R E 0x1000
  LOAD           0x008000 0x80000000 0x00107000 0x7b76d 0x7b76d R E 0x1000
  LOAD           0x084000 0x8007c000 0x00183000 0x003dc 0x05644 RW  0x1000
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x10

 Section to Segment mapping:
  Segment Sections...
   00     .loader
   01     .text .rodata .stab
   02     .data .bss
   03
```

The big change here was to combine several sections.  The current sections are:

```
ELF Header:
  Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           ARM
  Version:                           0x1
  Entry point address:               0x100090
  Start of program headers:          52 (bytes into file)
  Start of section headers:          539476 (bytes into file)
  Flags:                             0x5000400, Version5 EABI, hard-float ABI
  Size of this header:               52 (bytes)
  Size of program headers:           32 (bytes)
  Number of program headers:         6
  Size of section headers:           40 (bytes)
  Number of section headers:         11
  Section header string table index: 10

Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .entry            PROGBITS        00100000 001000 001000 00  AX  0   0 16
  [ 2] .loader           PROGBITS        80000000 002000 004000 00  AX  0   0 4096
  [ 3] .syscall          PROGBITS        80400000 006000 001000 00  WA  0   0  1
  [ 4] .text             PROGBITS        80800000 007000 007000 00  AX  0   0  8
  [ 5] .data             PROGBITS        81000000 00e000 005000 00  WA  0   0  8
  [ 6] .stab             PROGBITS        81005000 013000 068000 00   A  0   0  4
  [ 7] .ARM.attributes   ARM_ATTRIBUTES  00000000 07b000 000039 00      0   0  1
  [ 8] .symtab           SYMTAB          00000000 07b03c 006900 10      9 1433  4
  [ 9] .strtab           STRTAB          00000000 08193c 0021c0 00      0   0  1
  [10] .shstrtab         STRTAB          00000000 083afc 000055 00      0   0  1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  y (purecode), p (processor specific)

There are no section groups in this file.

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x00100000 0x00100000 0x01000 0x01000 R E 0x1000
  LOAD           0x002000 0x80000000 0x00101000 0x04000 0x04000 R E 0x1000
  LOAD           0x006000 0x80400000 0x00105000 0x01000 0x01000 RW  0x1000
  LOAD           0x007000 0x80800000 0x00106000 0x07000 0x07000 R E 0x1000
  LOAD           0x00e000 0x81000000 0x0010d000 0x6d000 0x6d000 RW  0x1000
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x10

 Section to Segment mapping:
  Segment Sections...
   00     .entry
   01     .loader
   02     .syscall
   03     .text
   04     .data .stab
   05
```

It's not that the resulting elf is malformed, but that the `pi-bootloader` may not be able to handle it.

I think.

---

Turns out that I am no longer able to think in decimal...  I was trying to print the number `'1'`, but I was printing decimal `31` rather than hexadecimal `0x31`.  When I cleaned that up, it worked.

So, now to figure out how far I get.

I was able to narrow down the problem to the first call to `MapPageFull` in `entry.s`.  I finally have something to work on here.

OK, to save my debugging code:

```
    mov     r0,#0x5040
    movt    r0,#0x3f21
    mov     r1,#'#'
s:  str     r1,[r0]
    b       s
```

This outputs a stream of `'#'` characters.

My problem is this block of code:

```
    mov     r9,#(ARMV7_MMU_TTL2)                @@ get the bits to set
    orr     r3,r9                               @@ make the proper ttl1 entry
    str     r3,[r2]                             @@ and put it in place
```

The `str` opcode does not have a valid `r2` value.  I need to clean that up.

OK, so the paging tables are still messed up, even after fixing up the above issue.

So, I am going to have to invest is some time into some additional debugging code.

---

### 2019-Dec-26

So, I am unable to get the code to boot again.  I am not sure what the heck is going on, but it feels very fragile to me!  Even my output at the start of the entry point no longer works.  CRAP!

Hmmm... OK...  When I write a loop of characters, I get the results I am looking for.  A single character does not work.

So, I need to make sure I can output a single character.

Ok, seriously, all I really changed was to clean up some alignments and now it runs.

---

### 2019-Dec-27

More stepping through the bits I need to understand to get the rpi booting again....

---

### 2019-Dec-29

I have finally been able to get the loader executing.  However, I am still not able to get the serial port to output anything.  So, since the MMIO memory is being identity mapped and mapped to the proper virtual memory locations.

---

I was finally able to finally get the rpi2b serial port to output characters properly.  I cleaned up my debugging mess and now I am getting problems with the hardware sections of both archs.  With that, I should commit my code.  I will also push it since this is a significant milestone.  I was supposed to get farther than this with v0.5.0b, but I think I will also branch this and continue on a new branch since so much work went into this.

---

## Version 0.5.0c

I will continue on this branch with the work to get all the hardware reporting and initializing properly.

It appears that my fault handlers are not working right yet.  This should be the next task.

The following code is in `LoaderMain()`:

```C++
void LoaderMain(archsize_t arg0, archsize_t arg1, archsize_t arg2)
{
    LoaderFunctionInit();               // go and initialize all the function locations
    MmuInit();                          // Complete the MMU initialization for the loader
    PlatformEarlyInit();
    kprintf("Welcome\n");
```

This code does nothing to relocate the GDT or to set up the IDT in its final location for any form in either arch.  I could add this to any of the functions, but I would prefer to have that completed sooner than later.  In reality, this really should have been done in `entry.s` but was not convenient.  Therefore, I believe a function ahead of `LoaderFunctionInt()` is in order to get these final CPU structures initialized.

Now, here is the problem: to complete the GDT setup, I am going to need a TSS per CPU.  To determine the number of CPUs, I am going to need to complete `PlatformEarlyInit()` (or at least `HardwareDiscovery()` therein).  And `PlatformEarlyInit()` needs to come after `MmuInit()` to get all the mappings settled.  Finally, `LoaderFunctionInit()` really does need to come first to makes sure that the structures are properly initialized before we get into anything that might use that (like a serial port) -- stuff that the C++ runtime expects to have been called.

So, this is a bit of a chicken-and-egg problem.

I really need to leave the setup as it is and get this all settled down properly knowing I do not have a properly configured GDT or IDT for anything yet -- until I get past `PlatformEarlyInit()`.  So, I need to make sure that the GDT and IDT structures are identity mapped for now.

Having done that for both archs, I am able to get the proper exception for rpi2b (x86-pc has not yet been set up, so I cannot yet expect the same behavior).

Now, with that said, I should be able to get the IDT up and running and handle exceptions.  This may yet be able to be handled in `entry.s`.

I have an IDT set up now, but I am getting some odd exceptions (`#UD` in particular).  I will need to run logs on this.

---

### 2019-Dec-30

So, logs....  which turned out to be a quick and easy cleanup.

After that, I discovered a problem with mapping a page from the kernel for rpi2b.  The management space is not mapped properly.

I realize now that I have no function to dump the mmu tables for rpi2b.  I need to get that function implemented.

---

### 2019-Dec-31

Well,  managed to get my arm `entry.s` too far out of whack and checked out the one from the last commit....  and now I am redoing some debugging.   rpi2b exceptions are not working again.

---

### 2020-Jan-01

Well, Happy New Year!!  I started by ticking all the copyright (c) dates.  And broke a few things in the process (I really need to learn `git` better).  I will have to sort them all out.

OK, so x86-pc works fine.

rpi2b is getting this data abort:

```
IN:
0x808034cc:  e5941000  ldr      r1, [r4]
0x808034d0:  e2812054  add      r2, r1, #0x54
0x808034d4:  e5923000  ldr      r3, [r2]
0x808034d8:  e3130020  tst      r3, #0x20
0x808034dc:  0afffffc  beq      #0x808034d4

R00=810002f4 R01=00000001 R02=00000000 R03=80803494
R04=810002f0 R05=00000054 R06=810002f4 R07=000000c0
R08=ff800fbc R09=3fd01000 R10=000287a0 R11=808063f4
R12=05000040 R13=ff800f40 R14=808034cc R15=808034cc
PSR=600001d3 -ZC- A NS svc32
Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x9600003f
...with DFSR 0x5 DFAR 0xf8215054
```

The DFAR is the mini-UART and is the mapped address.  So I need to figure out the DFSR.  And I see it is a First Level translation fault -- I do not have a TTL1 Entry for this address.  But how?

OK, turns out I added some debugging code into `MmuMapToFrame()` and that is being executed too early.  And removing it fixed the problem.
So, I need to figure out how to "turn on" debugging output.                                                           z

---

I am still faulting trying to map pages on rpi2b.  I still am missing somehow some epiphany I had at some point in an earlier version.  There is some page that is not mapped to be able to map a page.  I think it has something to do with recursively mapping a TTL2 table.  So, let's think this through:

The formula for calculating the TTL1 Entry address is `(0xff404000 + ((<addr> >> 20) * 4))`.
The formula for calculating the TTL2 Entry address is `(0xffc00000 + ((<addr> >> 12) * 4))`.

| Page Base Address | TTL1 Entry Addr | TTL2 Entry Addr |
|:------------------|:----------------|:----------------|
| `0xff401000`      | `0xff407fd0`    | `0xffffd004`    |
| `0xff407000`      | `0xff407fd0`    | `0xffffd01c`    |
| `0xffffd000`      | `0xff407ffc`    | `0xfffffff4`    |
| `0xfffff000`      | `0xff407ffc`    | `0xfffffffc`    |

So:
* `0xff401000` has a TTL1 Table and Entry, no TTL2 Table; check `0xffffd000`
* `0xffffd000` has a TTL1 Table and Entry, a TTL2 Table, but no TTL2 Entry
* `0xfffff000` has a TTL1 Table and Entry, a TTL2 Table, but no TTL2 Entry

OK, so with that, I know that from `0xffc00000` to `0xffffffff` is 4MB.  This fits in 4 TTL1 entries, or in 4 TTL2 tables or 1 frame.  So, this means that TTL1 entries `0xffc` to `0xfff` should all be mapped (and I think they are).  These each will point to a TTL2 table of 256 entries, or 1 frame in total.  Therefore, address `0xfffffffc` should point to this frame.

---

So, I find the following comments from the v0.3.0 journal from 2019-Feb-17:

> ### 2019-Feb-17
>
> I started today by cleaning up my `MmuInit()` function.  It certainly had some problems.  Now, since I have finally figured out that the TTL2 tables for managing the ttl2 tables needs a recursive mapping, I think it best to consider the types of structure entries I need to be able to manage to map a frame into virtual address space.  So, for any given address `addr`, I need to be able to find:
> 1. The TTL1 Entry for that address: `TTL1_KRN_VADDR[addr >> 20]`.  This is used when we need to add a TTL2 table.  This space is already mapped and ready to be used.
> 1. The TTL2 Management Table TTL2 Entry for that address: `0xfffff000[addr >> 22]` (taking into account the "`0x3ff`" comments from `MmuInit()`).  This address is used to map the new TTL2 table into the management space.  This table is already mapped and ready to be used.
> 1. The TTL2 Entry for that address: `TTL2_KRN_VADDR[addr >> 12]`.  This is used to actually perform the mapping for the MMU.  Once the above steps are complete (meaning the entries are checked/mapped), every thing is in place to perform this function.
>
> With these 3 bits of information, I should be able to create a proper function to manage this information.  The key missing piece here is the "TTL2 Management Table TTL2 Entry" which I have been missing all this time.  And no recursion!

And I am still struggling with that "TTL2 Management Table TTL2 Entry" concept.  Damn!!

---

### 2020-Jan-02

I am spending most of my day reading.  Most of what I need to duplicate is in a previouis version of [`MmuInit()`](https://github.com/eryjus/century-os/blob/v0.3.0/arch/arm/MmuInit.cc).  This will need to be taken care of in `entry.s`, and once complete `MmuMapToFrame()` should work.

The order of things that were done in `MmuInit()`:
1. Map the TTL1 table into its management address
1. Create and map the TTL2 table for the TTL2 management addresses (0xffc - 0xfff)
1. Recursively map the last TTL2 entry to itself
1. Map all existing TTL2 tables into the TTL2 management space

From there, `MmuMapToFrame()` works.  At least in that version.

---

### 2020-Jan-03

Finally!!  The key was to add a block of code at the end to loop through the `TTL1 % 4 == 0` TTL1 Entries and mapping them into the TTL2 management table.  This was the part I was missing.

Now, the rpi2b boots to the Welcome message.  The x86pc page faults before then.  So, now to clean that up.

OK, that problem exists in parsing the ACPI tables.  These need to be mapped and unmapped as I consume them.  And I have that worked out.

---

I have been able to get to the kernel.  In both archs, I am able to see the greeting "Welcome to CenturyOS -- a hobby operating system".  And then in both cases, I get a page fault.  So, I think it is time to commit this version.

---

## Version 0.5.0d

OK, so for this version, I will work on completing the cleanup of the loader code.  This will include cleaning up the debug code.  I will also review my Redmine list for anything that can be cleaned up in the loader.  At the same time will be a review of all the Platform code -- cleaning up any issues found therein.

---

### 2020-Jan-04

I am working on platform cleanup....

---

### 2020-Jan-05

I have been making a number of changes trying to get the code cleaned up.  I have quite a bit of it done, but not all.

However, I think I need a commit at this piont.  Nearly every source file has been changed and the risks are getting higher by the minute.

---

## Version 0.5.0e

In this version I continue the cleanup.  There is still a lot to settle down.  However, I want to specifically focus on getting the tests to run again.

---

The first thing I need to do for x86-pc is to get the GDT and IDT properly prepared.  This needs to be done after we discover the number of CPUs so that we can also determine the number of TSS entries that are needed.

I started by performing some calculations on the number of CPUs I can support in a single frame of GDT entries.  This came to 167 CPUs:

> The number of GDT entries we need is easily calculated: 9 + (CPU_count * 3).  Each GDT Entry is 8 bytes long.  Therefore, the number of CPUs we can support in a single frame is: `floor(((4096 / 8) - 9) / 3)` or `167`.  167 CPUs is quite simply a ridiculous number of CPUs at this juncture.

So, considering it a bit, I think I only need to be able to support 16 at this time (167 is really out of the question and 128 is just as ridiculous).  So, in short, this calculation and decision making result in requiring only 1 frame for the GDT.  This means, then, that the other frames will be freed up later when the butler cleans up.

Now, there will be a difference between the Physical GDT address and the Virtual GDT address.  The virtual GDT address will end up in high memory (>0x80000000).

---

### 2020-Jan-07

I am still working on cleanup.  However, I have been able to confirm I am getting to the proper new GDT.

---

### 2020-Jan-08

Today, I am going to work on making certain the x86-pc has a proper IDT in the proper location.

---

I have my stack mapped to the Page Directory.  CRAP!!!

---

### 2020-Jan-10

Ok, so somewhere between the 2 `BOCHS_BREAK` lines I am overwriting my paing table:

```C++
    BOCHS_BREAK;    // stack mapping is still good here: 0x1001000

    kprintf("Initializing the IDT properly\n");

    BOCHS_BREAK;    // still mapped, but 0x1001000
```

Not exactly sure where that would be; `kprintf()` should be unintrusive.

However, on the first break, I have the following mapping:

```
CR3=0x000001001000
    PCD=page-level cache disable=0
    PWT=page-level write-through=0
...
<bochs:5> page 0xff8000000
 PDE: 0x0000000000000015    ps         a PCD pwt U R P
 PTE: 0x00000000f000ff53       G pat D a PCD pwt S W P
```

whereas for the second breal, I have:

```
CR3=0x000001001000
    PCD=page-level cache disable=0
    PWT=page-level write-through=0
...
<bochs:7> page 0xff8000000
 PDE: 0x0000000000200002    ps         a pcd pwt S W p
physical address not available for linear 0x0000000ff8000000
```

So, since the PDE is different, this means that the entry at offset `0xff8` is being overwritten.  That would be at physical address `0x01001ff8`.

---

### 2020-Jan-11

This morning (well even yesterday while I could think), I started looking at the Bochs instrumentation.  Where this comes in is that I think I can use that to track to actual execution instruction by instruction, placing the relevant values into the output.  This, then, will become a full execution log but will slow bochs tremendously!!!.  To overcome this, I will need to also create some way to toggle the instrumentation (turn on/off).  A `nop` is technically `0x90` or `xchg ax,ax`.  The bochs magic breakpiont is `xchg bx,bx`.  I can use `xchg dx,dx` to toggle the debugging output.

Well, point of correction here...  `xchg ax,r16` is the `0x9x` family of instructions.  That will not work for a simple compare here.  However, the `xchg edx,edx`/`xchg dx,dx` (depending on the native CPU mode) will still work.  This should result in a byte sequence of `0x87 0xd2` which should be able to be easily trapped.

So the first thing to do here is to build Bochs with instrumentation enabled.  To do that, I am going to copy the instrumenatation `example1` to a new folder and create a symlink under the Bochs instrument folder to my project.  This will allow me to make sure I have this working right before I get into my own version.

---

I was not able to keep the symlink for the folder.  I had to move the physical files into pleace.  But, now I get beautiful debugging output like this:

```
----------------------------------------------------------
CPU 0: mov eax, dword ptr ss:[ebp-112]
LEN 3   BYTES: 8b4590
MEM ACCESS[0]: 0x000000000007fadc (linear) 0x00000007fadc (physical) RD SIZE: 4

----------------------------------------------------------
CPU 0: test eax, eax
LEN 2   BYTES: 85c0

----------------------------------------------------------
CPU 0: jnz .+26 (0x0000001c)
LEN 2   BYTES: 751a     BRANCH TARGET 00000000bffea65f (TAKEN)

----------------------------------------------------------
CPU 0: mov eax, dword ptr ss:[ebp-112]
LEN 3   BYTES: 8b4590
MEM ACCESS[0]: 0x000000000007fadc (linear) 0x00000007fadc (physical) RD SIZE: 4

----------------------------------------------------------
CPU 0: test eax, eax
LEN 2   BYTES: 85c0

----------------------------------------------------------
CPU 0: jnz .+26 (0x0000001c)
LEN 2   BYTES: 751a     BRANCH TARGET 00000000bffea680 (TAKEN)

----------------------------------------------------------
CPU 0: mov eax, dword ptr ss:[ebp-380]
LEN 6   BYTES: 8b8584feffff
MEM ACCESS[0]: 0x000000000007f9d0 (linear) 0x00000007f9d0 (physical) RD SIZE: 4

----------------------------------------------------------
CPU 0: mov eax, dword ptr ds:[eax]
LEN 2   BYTES: 8b00
MEM ACCESS[0]: 0x00000000bffe6cb0 (linear) 0x0000bffe6cb0 (physical) RD SIZE: 4

----------------------------------------------------------
CPU 0: mov eax, dword ptr ds:[eax+860]
LEN 6   BYTES: 8b805c030000
MEM ACCESS[0]: 0x00000000bffe757c (linear) 0x0000bffe757c (physical) RD SIZE: 4

----------------------------------------------------------
CPU 0: test eax, eax
LEN 2   BYTES: 85c0

----------------------------------------------------------
```

And I get lots of it.  Lots and lots.

This is a good starting place, but I have got to be able to disable this or I am never going to get anything done with bochs.  So, I am going to start a new git and a new project.  The first order of business is to get the minimal stuff in place and then to write the repo.  But then I need to add code to toggle enable/disable of the code so I can be far more surgical with where I get the data.  So, I set this aside for a while....

---

OK, I have something!!

Bochs debugger reports in particular for `cr3`:

```
<bochs:2> creg
CR0=0xe0000011: PG CD NW ac wp ne ET ts em mp PE
CR2=page fault laddr=0x00000000ff800f28
CR3=0x000001001000
    PCD=page-level cache disable=0
    PWT=page-level write-through=0
CR4=0x00000000: cet pke smap smep osxsave pcid fsgsbase smx vmx osxmmexcpt umip osfxsr pce pge mce pae pse de tsd pvi vme
CR8: 0x0
EFER=0x00000000: ffxsr nxe lma lme sce
```

and the instrumentation log reports:

```
00164631098i[      ] CPU 0: mov esp, 0xff801000
00164631098i[      ] LEN 5	BYTES: bc001080ff
```

Now, later down the execution (less than a dozen instructions), we have:

```
00164631108i[      ] CPU 0: call .-88 (0xffffffad)
00164631108i[      ] LEN 5	BYTES: e8a8ffffff
00164631108i[      ] 	BRANCH TARGET 00000000800025a0 (TAKEN)
00164631108i[      ] MEM ACCESS[0]: 0x00000000ff800ff0 (linear) 0x000001001ff0 (physical) WR SIZE: 4
```

This means that the stack (at `0xff800ff0` which points to physical address `0x1001ff0` is occupying the same frame as `cr3`.

The stack virtual address is correct.  However the frame is wrong.

So, this code is used to create a Page Table and add it to the Page Directory.  It then loads the base address of the stack into the page.

```S
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
```

In particular, I am interested in how `stackBase` is populated.

```S
;;
;; -- The next order is business is to get a stack set up.  This will be done through `NextEarlyFrame()`.
;;    ---------------------------------------------------------------------------------------------------
initialize:
    mov         esp,stack_top                   ;; This will set up an early stack so we can call a function
    call        NextEarlyFrame                  ;; get a frame back in eax
    add         eax,STACK_SIZE                  ;; go to the end of the stack
    mov         esp,eax                         ;; set the stack
    mov         [stackBase],eax                 ;; save that for later
```

`eax` is returned from `NextEarlyFrame` with the frame that will be mapped.  We use that to create a proper-depth stack, but we also adjust `eax` before we store that value into `stackBase`.  This is the problem -- the `mov [stackBase],eax` is out of sequence.  Also, I checked and this is the first call to `NextEarlyFrame` so I would expect to get `0x1000000` back from that call, not `0x1001000`.

Interestingly enough, this was amazingly easy to track down with the instrumentation package!!!

---

### 2020-Jan-12

Let's see here.  I need to figure out where I am faulting at this point.  I forgot where I was in the cleanup.  For x86-pc, it am faulting in `kMemSetW()`, which is only used as I recall in clearing the frame buffer.  For rpi2b, it is also faulting in `kMemSetW()`.  So, it is clear at this point I need to go back and make sure that the framebuffer is mapped in virtual memory.

So, I might have to go back to instrumenation here....

```C++
void LoaderMain(archsize_t arg0, archsize_t arg1, archsize_t arg2)
{
    LoaderFunctionInit();               // go and initialize all the function locations
    MmuInit();                          // Complete the MMU initialization for the loader
    kPrintfEnabled = true;
```

This means I cannot get any output from `MmuInit()` because output is not enabled until after `MmuInit()` is complete.  Or..., maybe I can move that because it is dependent on the mapping of the MMIO address space for rpi2b.  Let do that!

OK, realization here... I am working in the wrong space.  That is because the function `FrameBufferInit()` is collecting the data I need to map the framebuffer and that has not been called yet.  I really need to move my work onto that function.  Now, I also have a problem with rpi2b where it is being mapped twice -- and obviously neither are working properly so I have to work to do there as well.  I also have several bits of hard-coded information there that I need to get cleaned up as well.

I have the rpi trying to start processes and scheduling.  It is not working 100% but that is a good place to get the pc to get caught up to.  The pc is to the point of trying to start the other cores by copying the trampoline code.

However, neither arch is outputting to the monitor even though they should be at this point.  I need to get that debugged first.

This also turned out to be an easy sequencing problem.  Well for x86-pc.

For rpi2b, the screen is not updating and I am getting an interrupt vector 0x44:

```
Attempting the clear the monitor screen at address 0xfb000000
.. Done!
Request to map vector 0x41 to function at 0x80803668
IsrHandler registered
Enabling IRQ bit 0x0 at address 0xf800b218
Done
Timer Initialized
Enabling interrupts now
PANIC: Unhandled interrupt: 0x44
```

What the heck is *that*??  This is IRQ 68, which is not on the BCM2835 IRQ list.  So, it must be from the GPU....

OK, this makes more sense:

```C++
//
// -- BCM2835 defines IRQs 0-63, plus a handfull of additional IRQs.  These additional ones are being placed
//    starting at IRQ64 and up.
//    ------------------------------------------------------------------------------------------------------
#define IRQ_ARM_TIMER       64
#define IRQ_ARM_MAILBOX     65
#define IRQ_ARM_DOORBELL0   66
#define IRQ_ARM_DOORBALL1   67
#define IRQ_GPU0_HALTED     68
#define IRQ_GPU1_HALTED     69
#define IRQ_ILLEGAL_ACCESS1 70
#define IRQ_ILLEGAL_ACCESS0 71
```

---

OK, so the GPU is halting.  But why???   Well, hang on....  I think I need the physical address of the structure I am communicating with the GPU with..  but I am in virtual memory.  This means I need to either identity map the structure OR I need to get a frame specifically for this so I know what the hardware address is.  I could also write an MMU function to get the frame for a page so I can calculate the hardware address.  Several options....

So, let's consider the options:
1. **Create an identity mapping for this structure.**  This will mean that to do this, I will need to find the physical address in runtime.  And then create a mapping for this address and hope that is will not conflict with something that is already mapped.  This is really not a good option.
1. **Create a space in memory where this is a dedicated structure.**  I may end up doing that later for the video driver, but not likely.  The problem here is that this removes a complete frame for 64 * 4 or 256 bytes for something that has other options.  Not smart.
1. **Write a general purpose MMU function to determine the physical address of a virtual one.**  This has a lot more promise in my opinion.  In reality, this will have a lot more use than any of the other solutions and it is not a uni-tasker (to steal a term from AB..)!  I think I will need something like this for DMA later -- I think (but mixing ARM and x86 technologies).

Anyway, the MMU function is the way I am going to take this.

---

The first attempt at this did not work.  Simple cleanup though.

So now the next step is to fix up the trampoline code.

OK, I have both archs working without starting the extra APs.  I am going to commit this now and create a Redmine to get the APs working again.

---






