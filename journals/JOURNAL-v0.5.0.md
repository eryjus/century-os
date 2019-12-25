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





