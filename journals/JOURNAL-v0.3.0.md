# The Century OS Development Journal - v0.3.0

## Version 0.3.0

This version will specifically separate the architecture and platform abstractions and reconcile the code better that it is organized.  There will be no new functionality added with this version, with two exception: 1) I will properly separate the kernel and user paging structures for the rpi2b architecture, and 2) I will eliminate the loader in favor of putting the initialization in the kernel where it can be reclaimed later.

At the same time, I may have an opportunity to clean up the virtual address space and if it is clean to do so, I will take that on.

The goals for this version are tracked here: http://eryjus.ddns.net:3000/versions/7

---

### 2019-Feb-07

Today I start reorganizing the source tree and taking better care of the arch and platform abstractions.  At the same time, I will be eliminating the loader and performing all the initialization in the kernel, organized in a way that I can better reclaim the space.

I want to talk about this second goal for a bit.  I specifically stated this as a goal in the beginning.  But I ran into trouble:

> However, I need to revisit the goal of having all the one-time initialization completed in the loader.  I'm nearly positive at this point that I am not going to be able to meet this goal.  For example, I will need to have kernel exception handlers that for things like divide by 0 that will be responsible for shutting down the offending process.  I am not able to initialize that in the loader (at least not without some really fancy data structure in the kernel that I find and read for addresses -- which might be a possibility at a later time) without having a symbol table to read and I am not really interested in forcing debugging information to be available in the kernel.  While it is a great goal, I think it should be set aside for a moment and revisited once I have a better working system (read: working system).

... and later:

> These are implemented in the `loader` and cannot be moved to the kernel.  They will have to be re-implemented.  However, there is an additional concern: When I re-implement these functions, I will need to have a working PMM again.  So, the solution is going to be to allocate more space in the pmm binary and to purpose build the MMU functions I need to get the PMM running.  All of the one-time initialization was part of what I wanted to avoid in the kernel by moving to the loader, but I do not believe that is going to every come to fruition at this point -- not with my little hobby kernel.

These both indicate that my goal was unrealistic -- at least physical separation.  However, if I take a step back I might be able to accommodate this goal by placing all the initialization code in its own segment and then reclaim that when the initialization is complete.  This way, I can share code properly (which is the underlying problem from both quotes above).

This, then, brings me to where to start....  The best place is going to be to develop out arch and platform and then redo the loader (the other way around has me making significant changes in this crappy code base).  At the same time, if I chose to defer the loader changes until v0.3.1, I am not committed to anything yet.

Tomorrow, I think I will take a hard look at how I am going to reorganize the source tree.  One of the things I have working for me is the concept of one function per source file....  It should make this process relatively easy to get through.  Ha!!

---

### 2019-Feb-08

I was thinking today about the organization of the source tree.  I am not sure, but I may end up combining the PMM and the loader and the kernel all into the same executable and then manage the access through the paging tables.  They key to this plan in my opinion would be to set up lots of sections and then abandon them when I am done (reclaiming the pages and frames when I do).  But, the thing that I am not certain over at this point is how to organize the modules I would create.  For example, I would like to end up with a GUI, but I would be happy porting `bash` as a first step.  Even before that, I will need a file system driver which will likely be an external module loaded by grub for me.  The point is where do I fit these into the source tree?

Taking some inspiration from [lk](https://github.com/littlekernel/lk), there is a folder for each of the following:
* kernel
* apps (or what I call modules)
* lib (well, lots of things here)
* tools (which I assume are related to building the OS)
* and the all-important arch and platform folders

`lk` also creates a `build` folder to build a project.  It is a fully integrated project where one build command yields everything for a particular target (arch + platform).

Well, for me, I will start with the kernel, arch, and platform folders.  These will be organizes in the following manner:

```
century-os
+-- arch
|   +-- arm
|   |   +-- armv7
|   |   +-- armv8
|   +-- x86
|       +-- i686
|       +-- x86_64
+-- kernel
|   +-- inc
|   +-- src
+-- modules
|   +-- module1
+-- platform
|   +-- bcm2835
|   +-- bcm2836
|   +-- pc-isa
|   +-- pc-pci
+-- targets
    +-- rpi2b
    +-- i686-isa
```

Well, I think anyway.  Here are some thoughts:
* I will keep common code in `arch/<arch>` while keeping things that are specific to the specific bitness of a cpu in the respective folders (a perfect example of this would be the MMU tables which are quite different between 32-bit and 64-bit systems).
* I will still keep `sysroot`, `iso` and the like -- just not included in the changes for this version.
* `bin` will be eliminated eventually (but I need it for now to keep things compiling properly).
* `module`s may end up with their own `platform`s folders as needed (likely omitting the `platform` parent folder); I cannot yet see a module needing its own `arch` folders.

Another thought here is which to work on first.  I was thinking last night that I should probably work on the arm stuff before the x86 stuff.  The thinking here is that arm is so foreign to me that I can keep the x86 requirements in the back of my mind while I am actively working on arm and still end up in far less of a mess when I change architectures to catch up than I would if I worked in the opposite order.

So, now, in the kernel folder, do I want to keep the component folders I have today?  I think I do -- even through it will create a bit more complicated build system and a slightly deeper organization structure.  Because I have a single file per function (generally), this will keep where to look a little easier find.

---

At this point I have been able to move the heap over.  This was rather trivial since it uses absolutely no architecture-specific or platform-specific code.  At the same time, I was able to identify 2 things that need to be cleaned up a bit, both of which are logged in Redmine.  These will be taken care of later, once I have completed `kernel.elf` for both targets compiling.

---

I was also able to bring in ipc and frame-buffer.  This then is the end of what works without needing platform- or architecture-specific code.

From here I am going to copy in `kprintf.cc` and `kInit.cc` (which will force all the remaining `*.h` files to be copied over).  And I am going to create the build rules to link the `kernel.elf` file which will drive all the missing functions.

---

### 2019-Feb-09

Well, I was able to get `kernel.elf` to compile today.  I was not required to move every source file from the old folder (several low-level functions are not used in the kernel).  I was also able to identify several things that need to be cleaned up once I get the i686 arch and pc-isa platform copied over as well -- each of these are in Redmine now.  Here is the living document for the [v0.3.0 roadmap](http://eryjus.ddns.net:3000/versions/7).  A quick test and this kernel still operates as it did.

So, I now have a decision to make: Do I move over the i686 stuff next or start to resolve the issues raised in Redmine?  After reviewing the goals and current Redmine issues, I think it best to sort out the i686 code before I start breaking the execution where I might have days of debugging to take on.

So, that leaves me with an easy decision to build out the same x86 stuff.

---

OK, so I have the kernel code reorganized -- think broad brush strokes.  I did find several things I want to clean up and I created several Redmines to go with them.  I think the next step here though is going to be to make a copy of the existing kernel module stuff and remove it from this project.  I will need to be able to make sure I can test properly when I do this, but it will be the only way I can make sure the kernel works properly at the moment.

I will do this and then test both architectures.

Everything still looks clean, so I am going to commit these changes.

---

Good news!!  I have decided to make my Redmine work visible read-only.  Well, some of it.  It is available at http://eryjus.ddns.net:3000.  If you are reading this, feel free to have a look (if you are not there already).

So, now what?  Well, I'm torn....  I really want to start combining the loader, kernel, and pmm into a single binary to reduce initialization.  That's the exciting work for me.  However, there are a number of clean-up tasks that need to be taken care of as well.  I hate to go there, but I think I really need to take those on first.  I just *hate* the bitch work.

So here are the Redmines I will work on this afternoon:
* #385 -- FRAME_BUFFER_ADDRESS and FRAME_BUFFER_VADDR are both defined
* #374 -- Separate the cpu-specific code from the platform or chip set-specific code (which I completed earlier today)
* #394 -- rpi2b supervisor call does nothing at the moment
* #396 -- x86 has a GetCBAR() function, which should not be needed if the platforms are properly separated
* #399 -- x86 arch is using 586 code -- is this correct?

---

OK, I think I want to turn my attention to the loader and how to eliminate it.  The key here is going to be to move the loader code into the kernel but without making a mess of the kernel.  To be successful here, I will need to get paging enabled quickly.  For x86, this will mean getting a 4MB page up and running quickly for initialization.  I will also need a GDT and an IDT ready to go.

For arm, this will be 4 X 1MB sections (see ARM ARM B3.5) to map the same 4MB of memory.  I will also need an interrupt Vector Table set (VBAR).

These large format pages will allow me to perform the early initialization quickly -- spanning all the code and data for setup.

I am going to start with those early steps now.

---

### 2019-Feb-10

I started setting up some `#define`s to help with location management of the loader.  Eventually, I will duplicate this with the pmm as well. I should be able to direct the loader functions into the `.ldrtext` and `.ldrdata` sections with ease.  The `.ldrbss` should also be able to be used, but *this data will not be zeroed out*, meaning I will have to initialize every element myself.

Side note -- while researching how string constants are going to be declared, I found the following article, which I found interesting: https://eklitzke.org/declaring-c-string-constants-the-right-way.  In short, I will need to declare a string constant as:

```C
    char str[] _ldrdata = "This is a string\n";
    kprintf(str);
```

This will force the string into the loader-specific data section.  It will also have the added benefit of generating faster code, since only the data is stored, not the data with an extra pointer to the data.

---

I have the arm code for the entry point written (I am not able to test it yet).  The entry takes care of the following tasks:
1. if the CPU is in hyp mode, thunk it down to svc mode
1. stall all other CPUs than cpu0
1. clear the kernel bss (not the loader or the pmm -- they're on their own)
1. allocate a ttl1 (initialized later)
1. allocate an interrupt vector table and initialize it -- set the VBAR
1. initialize the ttl1 table for 1MB pages, set the TTBR0 and enable paging

Note that at this point, no MMIO addresses have been mapped.  So serial output is not available.

I'm debating whether to map the MMIO addresses.  The problem is that the linker will put the addresses in the of the kernel code in higher locations (so, `kprintf()` would have an address somewhere about `0x80000000`), which cannot be called from the kernel.  It's entirely possible I can produce a suite of function labels that are calculated at runtime....  Something along the lines of the following:

```C
typedef int (*kprintf_t)(const char *fmt, ...);
kprintf_t lprintf _ldrdata = (kprintf_t)((archsize_t)kprintf - 0x80000000 + 0x100000);

// ...

lprintf("Test\n");
```

---

OK, I want the MMIO addresses.  I am going to identity map those.  This would put the map at `ttl1[0x3f0]` or offset `0xfc0`.  In reality I read the CBAR and built this mapping dynamically.

At the same time it has dawned on me that I have some architecture-specific terminology for architecture-agnostic structures.  I really need to fix this, so I will create a `loadervars.cc` file with these values.

---

Now, `SerialInit()` is referenced from both the kernel and from the loader.  `SerialInit()` calls `BusyWait()`, which I will not have any control over.  So, I need to pull that out that call to `BusyWait()` and build an alternative wait -- it only needs to wait for 150 cpu cycles.  It allowed me to remove `BusyWait()` from the new kernel.

---

Here is where I am at as I near the end of my day:
* I am getting a simple lock-up when trying to boot on real hardware.  Likely an exception of some sort where the handlers are not in the right address space.
* I've been very careful to not call anything that is out of the loader section (except the exception handlers).
* I have inserted a `b` to bypass all the initialization and jump right to the `LoaderMain()` function, but that has had no effect on the situation.
* I have added `SerialInit()` and `SerialPutChar()` into the loader code properly, and that has not helped the situation.
* I have debugged the creation of the `rpi2b.img` file so that it does not use privileged commands.  This was just a means to an end -- and a win for my build system.
* I have set up to debug in qemu, but `rpi-boot` is parsing the Section Headers (and trying to load the sections into virtual memory that does not exist), rather than using the program header.  I have opened an [issue with jncronin](https://github.com/jncronin/rpi-boot/issues/24), but I may have to sort this out on my own.

With all this, I am currently debugging in the dark.

Oh, damn!!  I keep forgetting..  I cannot run this on qemu because the mini-UART is not emulated -- I will never get any output anyway!

---

### 2019-Feb-11

One of my last thoughts at the end of the day yesterday was whether I inadvertently changes compile options when I moved things around.

---

After some tests and debugging, I believe the problem might be one of 2 things.  Either:
1. I am outputting code for something different than I am running on
1. There is something wrong with my `SerialPutChar()` code, even though it was copied

Really at this point not much else makes any sense to me.

So, on `pi-bootloader` the machine code that loads the stack (the first thing in my code at the moment) is:

```
    8038:       e3a0d902        mov     sp, #32768      ; 0x8000
```

And the code that loads the stack in my new all-on-one binary is:

```
  104924:       e3a0d902        mov     sp, #32768      ; 0x8000
```

The locations are different, but the bitcode is the same.

As for `SerialPutChar()`, the `pi-bootloader` code is:

```C
void SerialPutChar(char c)
{
    if (c == '\n') SerialPutChar('\r');
    while ((GET32(AUX_MU_LSR_REG) & (1<<5)) == 0) { }
    PUT32(AUX_MU_IO_REG, c);
}
```

... and the code in my new all-in-one is:

```C
void _ldrtext SerialPutChar(char byte)
{
    if (byte == '\n') SerialPutChar('\r');
    while ((MmioRead(AUX_MU_LSR_REG) & (1<<5)) == 0) { }
    MmioWrite(AUX_MU_IO_REG, byte);
}
```

Effectively, this is the same code.  `GET32` and `PUT32` are implemented as `#define`s while the MMIO functions are inline.

The resulting code is:

```
    8180:       e350000a        cmp     r0, #10
    8184:       e92d4010        push    {r4, lr}
    8188:       e1a04000        mov     r4, r0
    818c:       0a000006        beq     81ac <SerialPutChar+0x2c>
    8190:       e3a02a05        mov     r2, #20480      ; 0x5000
    8194:       e3432f21        movt    r2, #16161      ; 0x3f21
    8198:       e5923054        ldr     r3, [r2, #84]   ; 0x54
    819c:       e3130020        tst     r3, #32
    81a0:       0afffffc        beq     8198 <SerialPutChar+0x18>
    81a4:       e5824040        str     r4, [r2, #64]   ; 0x40
    81a8:       e8bd8010        pop     {r4, pc}
    81ac:       e3a0000d        mov     r0, #13
    81b0:       ebfffff2        bl      8180 <SerialPutChar>
    81b4:       eafffff5        b       8190 <SerialPutChar+0x10>
```

and in the all-in-one:

```
  104538:       e350000a        cmp     r0, #10
  10453c:       e92d4010        push    {r4, lr}
  104540:       e1a04000        mov     r4, r0
  104544:       0a000006        beq     104564 <_Z13SerialPutCharc+0x2c>
  104548:       e3a02a05        mov     r2, #20480      ; 0x5000
  10454c:       e34f2221        movt    r2, #61985      ; 0xf221
  104550:       e5923054        ldr     r3, [r2, #84]   ; 0x54
  104554:       e3130020        tst     r3, #32
  104558:       0afffffc        beq     104550 <_Z13SerialPutCharc+0x18>
  10455c:       e5824040        str     r4, [r2, #64]   ; 0x40
  104560:       e8bd8010        pop     {r4, pc}
  104564:       e3a0000d        mov     r0, #13
  104568:       ebfffff2        bl      104538 <_Z13SerialPutCharc>
  10456c:       eafffff5        b       104548 <_Z13SerialPutCharc+0x10>
```

While I honestly thought I would have to justify some slight variations the code is exceptionally close.  The only variation is in the location of the MMIO register.  In the all-in-one, the register is at 0xf221500 (which is wrong!!).  Now, why is that??

Well, I had it defined in `kernel/inc/hw.h`.  Commenting it out give me the results I am looking for.  Now to undo all my debugging code!

Well, this of course still has problems.  However, I now know that the kernel will load.  I know I have some simple debugging to do, which should be as simple as putting in some jumps at various places to make sure the code will load.  I'm betting it related to the MMU.

Enough for the night.

---

### 2019-Feb-12

As the first test, I disabled the write to the register to enable the mmu:

```
@    mcr     p15,0,r0,c1,c0,0            @@ Put the cp15 register 1 back, with the MMU enabled
```

The code works with the above line disabled.  So, I have malformed paging tables.  Looks like I will need to do some reading to get this working properly....

---

As an interesting test, I enabled the MMU and then disabled it again.  I was able to get my output.  This means a couple of things:
1. the MMU is working as the next instructions executed
1. the real problem appears to be with the MMIO output and may be in the way I am mapping this page (TEX, B, and C fields)

Well, maybe I'm trying to be too cute with the CBAR and should just map it manually.  Still a dead end.

So, now I need to disable the paging code and get into `LoaderMain()` so I can dump some data from the paging table.  WAIT!!!  Before I do that, am I trashing the register that hold the proper value for the TTBR0?  No, I wasn't, but I did not have the domains set properly which I corrected to no effect.

---

I finally worked through my location issues (with strings -- what a damned mess!) and was able to produce some output from `LoaderMain()`.  I had to put my strings in the assembly source for now and link them in from there.

Anyway, here is my initial output:

```
Booting...
In the loader
The ttb1 is located at: 0x00c50879
```

Now this address can't be right.  I am expecting `0x003fc000`.  I have to be clobbering this register somehow.  And after fixing the clobber (which would have no effect on the problem), I am still getting something odd:

```
Booting...
In the loader
The ttb1 is located at: 0x00100448
```

This is the address of the variable that holds the address of the table.  Not the table itself.  Fixing that, I new realize that I am using an address for a frame number with this output:

```
Booting...
In the loader
The ttb1 is located at: 0xffffc000
```

After finally getting the correct address, I thought I would give it a try.  It did not work.  So, now I need to output more debugging info.  Say the contents of entry 0.

---

OK, I was able to run this through qemu and capture an execution trace.  The problem is that I have not mapped enough of the MMIO address space.  I need 16 pages mapped.

And that worked.

I was able to get my loader to call a function in local memory, but I had to change this line:

```C
#define PHYS_OF(f)  ((archsize_t)(f) - kern_loc + phys_loc + 0x1000)
```

In particular, I had to add `+ 0x1000` and I am not totally sure I understand why.  Ahhhh..  It is in the size of the loader section.  Currently it is 1 ELF page.  As that grows, I will have a problem having to change this constant value.

---

### 2019-Feb-13

I have been able to sort out all my paging and physical address issues.  They were numerous.  And, unfortunately, I did not keep track of them all since I was bouncing around so much.

I have switched gears to create an architecture-specific `EarlyInit()` function.  The objective of this function is to eliminate the need to worry about memory mapping, which means to complete the MMU setup for the kernel at minimum.

---

Well, I'm fighting with my nemesis the MMU again.  Currently, I am trying to make sure I have a good ttl2 address, which is my current problem.

Well, I worked out the issues with the mapping, but I am still not able to use `kprintf()` yet.  That will be my task tomorrow.

---

### 2019-Feb-14

OK, it looks like I still have a mapping concern.  And it turns out that I was not mapping the addresses to the proper frames -- bad calculations!

---

I have gotten down to the `PmmInit()` function.  This one is going to be a near complete rewrite.  This is because so much of the initialization has been completed already and I will have access to the results once initialization is complete from the pmm task.

---

Holy crap!!  I had not idea it was so late!  I'm off to bed.  But I managed to get the `PmmInit()` function updated, but it does not make it to the end.

---

### 2019-Feb-15

Well, I think I am using some addresses of addresses in place of just addresses.  Trying to prove that and figure out how to get my head around this.  I used to be better at this crap....  Too much work these days.  Yup!

```
Marking the first 1MB and kernel binary frames as used,up to 0x8007f
```

Nope!  It was a virtual address, not an address of an address.

So, now the problem is when to initialize the FrameBuffer.  I wanted to do that early to be able to greet the user, but that was when the MMU was being prepared for the kernel and already had things mapped for the frame buffer.  So, I have pulled the `FrameBufferClear()` function call from the `FrameBufferInit()` and will have to do that after I get the pages mapped.

Interestingly enough, the next up is the `MmuInit()` function.  Let talk about what is left to be accomplished for the complete MMU initialization (ignoring the loader cleanup for the moment):

* The management mappings need to be completed to the proper locations (both the ttl1 and ttl2 tables)
* The frame buffer needs to get mapped
* The MMIO addresses need to be mapped to the kernel locations
* The initial Heap locations need to be identified and mapped
* Map the exception vector table (VBAR)
* Stack (and we need to relocate that!)

So, let's start with simple and important: the TTL1 table address should be mapped to `0xff404000`.

---

Well I have a good part of `MmuInit()` written.  I am now in a position where I need to still allocate frames for the PMM but I do not have my formal PMM task up and running yet.  On one hand, I can continue to allocate from `NextEarlyFrame()` but collision is risky with the modules I will load.  On the other hand, I can write an intermediate frame allocator which would also live in the `__ldrtext` section and that would be duplicated code.  The final option is to map the pmm section into the address space and call it like I have been bouncing between the loader and kernel.  This last option would guarantee code reuse -- something I was not concerned about earlier with `MmuInit()`.

In this case, I would love to be able to reuse the code somehow.  This differs from the other case where it was all 100% loader and initialization -- stuff that will be executed once.  In this case the code will become a permanent fixture of the kernel and a running process.  I think I have my answer.

---

### 2019-Feb-16

OK, so I am going to fold the PMM allocator into the kernel binary, but keep it in its own section.  This is because the pmm will end up being its own process.  But the challenge is where to put it in virtual space (and as a result where to put it in physical memory).

No matter what I do, I will not be able to get the pmm to align to the user memory map (which I want to start at `0x100000`) since the loader is already there.  I could make a dedicated virtual memory map for the pmm, starting at `0x40000000` for example.  This will make the PMM a special case.

Or, I can keep using the `NextEarlyFrame()` for now.  I think this is going to be the best answer -- at least for now.  However, I should be able to reset the allocation location so that I am not encroaching on the kernel binary itself.

---

OK, after rewriting the `MmuMapToFrame()` and `MmuMakeTtl2Table()` functions, I am now getting a Data Exception when I try to update the TTL1 table through its management address.  This should not happen!

```
MMU: Mapping the frame buffer at 0x0f95f000 for 0x9d bytes
Mapping address 0xfb000000 to frame 0x0000f95f
Checking if the TTL1 entry has a TTL2 table at address 0xff407ec0
Data Exception:
At address: 0x00007f68
 R0: 0x00000042   R1: 0x800061cc   R2: 0x80005a5a
 R3: 0x00000020   R4: 0xff407ec0   R5: 0x0000f95f
 R6: 0xfb000000   R7: 0x00000001   R8: 0x00000000
 R9: 0x00000001  R10: 0x00000001  R11: 0x00000003
R12: 0xfffff000   SP: 0x80001dbc   LR_ret: 0x80001dbc
SPSR_ret: 0x600001d3     type: 0x17
```

This was supposed to have been taken care of in `MmuInit()` earlier in the function.

---

Once again, I am having a hell of a time getting my head around this..  The problem is the amount of recursive lookups that are needed to make this all work.  I gotta draw this out!!

---

I think I finally got this figured out.  I am going to duplicate the paging tables documentation I have added to `MmuInit.c`:

```C
//  So, I also want to make sure I have the structure documented here.  I am going to do my best to draw with ASCII
//  art.
//
//  The TTL1 table is located at over 4 pages from 0xff404000 to 0xff407000 inclusive.  There are 2 blocks of
//  TTL1 entries we will be concerned with here: 0xff4-0xff7 and 0xffc-0xfff.  The first group is needed to map
//  the TTL1 table for management -- keep in mind here we will only map 4 pages.  The second group is needed to
//  map the TTL2 table for management.  This will be 4MB of mappings and will be an entire frame of TTL2 tables
//  (which is really 4 tables by the way).
//
//  +-------//-------++-------//-------++-------//-------++---------------------//---------------------------+
//  |                ||                ||                ||                              |.|F|F|F|F|.|F|F|F|F|
//  |   0xff404000   ||   0xff405000   ||   0xff406000   ||       0xff407000             |.|F|F|F|F|.|F|F|F|F|
//  |                ||                ||                ||                              |.|4|5|6|7|.|C|D|E|F|
//  +-------//-------++-------//-------++-------//-------++---------------------//---------------------------+
//
//  So, the TTL1 management table will look like this:
//
//  0xff400000:
//   ff4___________________ ff5___ ff6___ ff7___
//  +-------------------//-+--//--+--//--+--//--+      * Entry 04 will point to the frame for 0xff404000
//  |-|-|-|-|0|0|0|0|.|    |      |      |      |      * Entry 05 will point to the frame for 0xff405000
//  |-|-|-|-|4|5|6|7|.|    |      |      |      |      * Entry 06 will point to the frame for 0xff406000
//  +-------------------//-+--//--+--//--+--//--+      * Entry 07 will point to the frame for 0xff407000
//
//  This then leaves the TTL2 management addresses.  This is a 4MB block that needs to be managed.  This area can
//  be managed with a single frame or 4 TTL2 tables inserted into the TTL1 table at indices 0xffc, ffd, ffe, fff.
//  So, this is the last group above.  This will look like the following:
//
//  0xffc00000:
//   ffc___ ffd___ ffe___ fff
//  +--//--+--//--+--//--+--//------------------------+      * Entry fc will not point to anything on init
//  |      |      |      |                    |F|F|F|F|      * Entry fd will not point to anything on init
//  |      |      |      |                    |C|D|E|F|      * Entry fe will not point to anything on init
//  +--//--+--//--+--//--+--//------------------------+      * Entry ff will be recursively pointed to this frame
//
//  Now, this is not to say that not other entries will be initialized.  Quite the contrary.  I am just saying that
//  the other entries are not needed for managing the paging tables.
```

I think I have figured out what my problems are.  That final recursive mapping in TTL2 is what I have been missing -- and what has been scrambling my mind.  I kept feeling like I was going to be recursively mapping frames forever, which once I reached that top frame I had not yet had that mapped and things would degrade from there.

This also tells me what I need to allocate to get the MMU tables initialized.  To manage the MMU tables, I only need to allocate 2 additional frames for TTL2 tables and make sure the final entry in the TTL2 table is recursively mapped.

At least I finally have a picture and a target.  I will work on implementing it tomorrow.

---

### 2019-Feb-17

I started today by cleaning up my `MmuInit()` function.  It certainly had some problems.  Now, since I have finally figured out that the TTL2 tables for managing the ttl2 tables needs a recursive mapping, I think it best to consider the types of structure entries I need to be able to manage to map a frame into virtual address space.  So, for any given address `addr`, I need to be able to find:
1. The TTL1 Entry for that address: `TTL1_KRN_VADDR[addr >> 20]`.  This is used when we need to add a TTL2 table.  This space is already mapped and ready to be used.
1. The TTL2 Management Table TTL2 Entry for that address: `0xfffff000[addr >> 22]` (taking into account the "`0x3ff`" comments from `MmuInit()`).  This address is used to map the new TTL2 table into the management space.  This table is already mapped and ready to be used.
1. The TTL2 Entry for that address: `TTL2_KRN_VADDR[addr >> 12]`.  This is used to actually perform the mapping for the MMU.  Once the above steps are complete (meaning the entries are checked/mapped), every thing is in place to perform this function.

With these 3 bits of information, I should be able to create a proper function to manage this information.  The key missing piece here is the "TTL2 Management Table TTL2 Entry" which I have been missing all this time.  And no recursion!

Now, I do need to name that constant and create some macros to help with this mess.

---

I am debugging the new code I have written.  I think it is right but I am still getting Data Faults.  I added debugging code and everything is set up the way I expect it to be:

```
Pre-clear checks to see what is wrong:
.. The TTL1 Entry is at address 0xff407fd0
.. The entry does have a ttl2 table at frame 0x00000fc4
.. The Management TTL2 entry is at 0xfffffff4
.. The entry does have a page table at frame 0x00003f10
.. The TTL2 entry itself is at 0xffffd000
.. The entry does have a page table at frame 0x00000400
Data Exception:
```

After reading some more about what is available to me with a Data Fault, I added some code to pull several bits of information, which now gives me the following:

```
Pre-clear checks to see what is wrong:
.. The TTL1 Entry is at address 0xff407fd0
.. The entry does have a ttl2 table at frame 0x00003f10
.. The Management TTL2 entry is at 0xfffffff4
.. The entry does have a page table at frame 0x00003f10
.. The TTL2 entry itself is at 0xffffd000
.. The entry does have a page table at frame 0x00000400
Data Exception:
.. Data Fault Address: 0xff400000
.. Data Fault Status Register: 0x00000807
.. Fault status 0x7: Translation fault (Second level)
.. Fault occurred because of a write
At address: 0x00007f50
 R0: 0xff400000   R1: 0x00000000   R2: 0x00001000
 R3: 0x00000020   R4: 0xfb000000   R5: 0x00000000
 R6: 0x80063400   R7: 0x00000fb0   R8: 0x0000f95f
 R9: 0x800063d0  R10: 0x80005ee0  R11: 0xffffd0ff
R12: 0x003f2000   SP: 0x80001ed4   LR_ret: 0x80004b3c
SPSR_ret: 0x200001d3     type: 0x17
```

This should not happen as I was supposed to have mapped that address right before I attempted the write.  Everything I look at tells me that the frame is mapped into virtual address space.

OK, where have I gone wrong???

```
Performing a table walk for address 0xff400000
.. TTL1 table is at 0x003fc000
.. The index for this table is 0xff4
.. The TTL1 Entry is at address 0x003fffd0
.. The TTL2 table is located at phys address 0x003f1000
.. The TTl2 index is 0x0
.. The TTL2 Entry is located at physical address 0x003f1000
.. The actual frame for this address is 0x00000000
```

The page is not mapped to frame `0x400` like I would expect.

OK, this line has the shit going the wrong direction in my debugging code:

```
.. The entry does have a ttl2 table at frame 0x00003f10
```

However, the line that prints:

```
The entry does have a page table at frame 0x00003f10
```

... is not being manipulated by the debugging code at all.  This means I have a shifting problem when that entry is being created.

Having sorted that, I now have this working properly.  I was so close, but was confused between ttl2 and frames and the shifting requirements between them.

---

The final thing was going to be to pre-allocate space for the Heap.  Since `MmuMapToFrame()` is working properly now, there is no need to do this.  I believe `MmuInit()` is complete.

So, with that, I have been able to get well into the `kInit()` function before it crashes.  I think the next thing to do is to catch up the x86 kernel while the loader is still fresh in my mind.

I am going to start with the loader script, which does not even close to match the rpi2b loader script in particular with the addressing.  I know when I make this change, things will break.

Once that is complete, I need to get into the `entry.s` entry point.  Things I need to accomplish in this file before handing off control to `LoaderMain()` are:
* Save off the Multiboot structure info
* Establish a stack
* Clear the BSS
* Allocate room for the Page Directory from the early pmm
* Get an interrupt table/GDT table from the early pmm -- set them both up!
* Execute both `lgdt` and `lidt` and perform the proper jump
* Map a 4MB section of memory in the Page Directory
* Enable paging
* Finally, jump to `LoaderMain()`

These function should put the CPU into its native state before handing control to the Loader to complete the discovery and initialization.

---

### 2019-Feb-18

I spent a good part of the evening writing the `entry.s` file for x86.  The last thing is to clean up the `grub.cnf` file so that this kernel boots.  I had actually managed to shock myself for a moment because it appeared to have executed for quite a while before triple faulting -- then I realized what I did wrong.

Now, having run this in Bochs, I get the exact results I expected: triple fault really early:

```
00159146725e[CPU0  ] check_cs(0x0008): conforming code seg descriptor dpl > cpl, dpl=3, cpl=0
00159146725d[CPU0  ] exception(0x0d): error_code=0008
00159146725d[CPU0  ] interrupt(): vector = 0d, TYPE = 3, EXT = 1
00159146725e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x0d)
00159146725d[CPU0  ] exception(0x0d): error_code=006b
00159146725d[CPU0  ] exception(0x08): error_code=0000
00159146725d[CPU0  ] interrupt(): vector = 08, TYPE = 3, EXT = 1
00159146725e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x08)
00159146725d[CPU0  ] exception(0x0d): error_code=0043
00159146725i[CPU0  ] CPU is in protected mode (active)
00159146725i[CPU0  ] CS.mode = 32 bit
00159146725i[CPU0  ] SS.mode = 32 bit
00159146725i[CPU0  ] EFER   = 0x00000000
00159146725i[CPU0  ] | EAX=001010bc  EBX=00ee0000  ECX=00000000  EDX=000003fd
00159146725i[CPU0  ] | ESP=00001000  EBP=00000000  ESI=001010b4  EDI=00000bfd
00159146725i[CPU0  ] | IOPL=0 ID vip vif ac vm RF nt of df if tf sf zf af pf cf
00159146725i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00159146725i[CPU0  ] |  CS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00159146725i[CPU0  ] |  DS:0018( 0003| 0|  0) 00000000 ffffffff 1 1
00159146725i[CPU0  ] |  SS:0018( 0003| 0|  0) 00000000 ffffffff 1 1
00159146725i[CPU0  ] |  ES:0018( 0003| 0|  0) 00000000 ffffffff 1 1
00159146725i[CPU0  ] |  FS:0018( 0003| 0|  0) 00000000 ffffffff 1 1
00159146725i[CPU0  ] |  GS:0018( 0003| 0|  0) 00000000 ffffffff 1 1
00159146725i[CPU0  ] | EIP=00100805 (00100805)
00159146725i[CPU0  ] | CR0=0x60000011 CR2=0x00000000
00159146725i[CPU0  ] | CR3=0x003fe000 CR4=0x00000010
(0).[159146725] [0x000000100805] 0010:0000000000100805 (unk. ctxt): jmpf 0x0008:0010080c      ; ea0c0810000800
00159146725p[CPU0  ] >>PANIC<< exception(): 3rd (13) exception with no resolution
```

The segments are still from grub.  However, it looks like I am jumping to some user section.  This means I have my GDT written poorly and I need to clean that up.

So, from the old `loader.elf`, I found the following:

```
 11e020 00000000 00000000 ffff0000 009acf00  ................
 11e030 ffff0000 0092cf00 ffff0000 00facf00  ................
 11e040 ffff0000 00f2cf00 00000000 00000000  ................
 11e050 00000000 00000000 ffff0000 009acf00  ................
 11e060 ffff0000 0092cf00 ffff0012 40e90fff  ............@...
 11e070 00000000 00000000 00000000 00000000  ................
 11e080 00000000 00000000 00000000 00000000  ................
 11e090 00000000 00000000 00000000 00000000  ................
```

Comparing entry 1 (not null) from above (`ffff0000 009acf00`) to the same entry from `entry.s` (`0x0000ffff,0x00a9f300`), I got this all messed up!

---

### 2019-Feb-19

Interestingly enough, in x86 I am getting a call to an upper memory location before I am ready for it:

```
0100100 <LoaderMain>:
  100100:       56                      push   %esi
  100101:       53                      push   %ebx
  100102:       e8 8a ff ef 7f          call   80000091 <__x86.get_pc_thunk.bx>
  100107:       81 c3 05 96 f5 7f       add    $0x7ff59605,%ebx
  10010d:       50                      push   %eax
```

This `__x86.get_pc_thunk.bx` function is causing me issues because it is in upper memory which is not mapped yet.  I have no clue how to deal with this...  Yet!

Adding the option `-fno-pic` to the compiler and to the linker options worked.  I no longer have the thunker in the function calls.

---

I was able to (for ARM) able to update the Serial code to use the correct addresses.  Now I need to do the same for x86.  In order to do this, I am going to have to suck up some inline assembly and turn the `outb` and `inb` calls into macros.

Now, I have gotten to the part where I need to develop the `MmuEarlyInit()` function.  This function (like the ARM counterpart) will need to complete the initialization of the MMU so that we can access the kernel code properly.  The goals for this function are:
* Clear out the rest of the Page Directory
* Recursively map the Page Directory
* Properly map the kernel to upper memory (creating additional Page Tables as needed) -- ARM was a 2-pass at this; not sure how to handle this here yet

Again the overall objective is to ensure that we can run kernel code and access kernel data cleanly.

---

### 2019-Feb-20

I am still having trouble with crashes and triple faults.  I think the thing I am worried about the most at the moment is that I am not getting any serial output.  I really need that for debugging purposes.  So, getting that output will be my focus until I have that working.

---

OK, I have the most basic serial output working -- I moved the `Serial*()` functions into the `__ldrtext` section as needed.  However, I do have some output to help me along:

```C
    SerialPutHex((uint32_t)SerialPutS); SerialPutChar('\n');
    SerialPutHex(kern_loc); SerialPutChar('\n');
    SerialPutHex(phys_loc); SerialPutChar('\n');
    SerialPutHex((uint32_t)&_loaderEnd); SerialPutChar('\n');
    SerialPutHex((uint32_t)&_loaderStart); SerialPutChar('\n');

    SerialPutHex((uint32_t)&_loaderEnd - (uint32_t)&_loaderStart); SerialPutChar('\n');

    SerialPutHex((uint32_t)PHYS_OF(SerialPutS)); SerialPutChar('\n');
    SerialPutHex(*(uint32_t *)PHYS_OF(SerialPutS)); SerialPutChar('\n');
```

Which gives the results:

```
0x800050b0
0x80000000
0x00100000
0x00103000
0x00100000
0x00003000
0x001080b0
0xfffffec4
```

I have looked through all the different addresses and this value (`0xfffffec4`) is not located at any "`0b0`" address.  This means that I am likely overwriting this value at some point.

So, even with paging not enabled, I am still getting the same results.  I am not getting anything showing up when I watch that location.

---

OK, I finally found my code.  It is loaded at `0x80000000`.  I think I need to update my linker script.  The following confirms it:

```
Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x00100000 0x00100000 0x02001 0x02001 RWE 0x1000
  LOAD           0x004000 0x80000000 0x80000000 0x079ec 0x079ec R E 0x1000
  LOAD           0x00c000 0x80008000 0x80008000 0x5aa64 0x91c88 RW  0x1000
```

The PhysAddr is not correct.  And having corrected the linker script, I am now getting a decent result:

```
!
0x800050b0
0x80000000
0x00100000
0x00103000
0x00100000
0x00003000
0x001080b0
0x08ec8353
Clearing the remainder of the Page Directory
Recursively mapping the Page Directory
Preparing to create page tables as needed for the kernel
Mapping the actual pages for the kernel
```

So, the next step is to undo all the changes I made (one at a time of course) to put things back to what they are supposed to be.

This will be tomorrow.

---

### 2019-Feb-21

Today I am going to start out by backing out all the debugging changes I made yesterday...  Pedantically.

---

With that done, I can now take on debugging `MmuEarlyInit()`.

---

This turned out to be that I was taking an address and treating it like a frame; I had to shift the address to the right by 12 bits.

So, now it looks like I am not properly clearing the `.bss` section:

```
MMap Entry count is: 0x7461646f
```

---

### 2019-Feb-22

So far is a short time I have gotten all of the x86 code to the point where I need to complete the MMU initialization.  This function should be quite easy to complete since the `MmuEarlyInit()` function was able to set up the recursive mapping.  Therefore, there is nothing more to do; we should be able to map and unmap with ease.

OK, the `LoaderInit()` function is completing.  But...  nothing is being written to the frame buffer.  Therefore, I get no output to the screen.  Nothing crashes...  just no output.

Actually, it was working right, the emulator was resetting too quickly.  I am now finally getting a GPF in the kernels code.  I want to clean up a few things and then commit this code.

---

OK, so what is next?  Well, it's really too early to take on the PMM yet.  I need to get the kernel executing properly.  I should not need the PMM to complete the initialization (which I previously needed for the heap initialization).  Once again, I am going back the rpi2b target to start my work.

I am now at a point where I need to be able to use the `arch-hw.h` file between both address spaces.  This means I will need to address the hardware base location dynamically.  I am not exactly sure how I am going to handle this, but because I cannot use a variable to store this data because of the mapping location.  I need to be able figure out an architecture agnostic way to handle this at compile time.

This change alone is breaking the majority of this code for rpi2b.  I need to think on this tonight....

---

### 2019-Feb-23

I think the best answer if going to be to completely change the `hw-arch.h` file to only have relative offsets from some defined base.  No matter what I do, I am going to need to take care of that.  This will eliminate the `HW_BASE` macro and everything I have done so far (and in the future) will need to know the base address it is working with.

That still leaves me with the last statement I made last night -- this is going to break everything and I will need to work through everything one at a time.  Now, in theory, everything that touches one of these addresses should be in the platform section already.  Mailboxes were not, so I moved them....

The best approach here I think is going to be to change `MmioRead()` and `MmioWrite()` to have an additional parameter for the base hardware address.  This will break the compile for everything that is relevant and force changes and thinking for everything (i.e.: nothing will be missed).

---

God, what a damned mess...  I cannot just remove `HW_BASE` because that breaks the hardware abstraction.  I think I am going to need to actually set up each device as an offset from it individual base address.  Since it is the most critical for debugging, let's start with the serial port.  Since we have to initialize the serial port with `MmioWrite()`, this needs to be addressed ahead of the serial port.

---

So, I decided to remove `hw.h` and `hw-arch.h` from the project (saved a copy of course).  I really need to be able to get a little more granular with this management to maintain hardware abstraction.  I'm going to have to replace it with something, but I need a different name (`hardware.h`) to make sure I don't miss anything.

So, as a result this will drive a number of changes with both architectures to be able to get the loader even to work up to calling the kernel code in higher memory again.

---

All-in-all, I have a good part of the serial functions implemented.  These mostly look like a device structure with operations -- object-oriented without being C++ classes.  The only thing missing at this point for rpi2b to be complete (for the serial device) is to figure out what I am going to do with the GPIO -- abstract it as a device or incorporate it as part of each device that needs to reference it.  I believe I will probably abstract it -- but then how to incorporate it...??

As I call it a night, I am nowhere close to an object that can compile at all.

---

### 2019-Feb-24

OK, this GPIO thing....   I think the bet thing to do on this is going to be to add in a platform-specific attribute into the structure where it may be NULL.  I would prefer to use some sort of inheritance for better/stronger typing, but I am not getting into the C++ realm.

---

OK, I think I have all the platform-specific device info set up and ready to be used.  At this point, the next steps here is to correct every place I have used the serial port to debug stuff.  The challenge here is going to be all the compile-time debugging `#if DEBUG_xxx == 1` statements which need to also be corrected.

Since there are so many changes, I will keep track of several of the more interesting ones here:
* `SerialPutS.cc` and `SerialPutHex.cc` can be eliminated
* as a result, `kernel/src/serial` can be removed
* `kprintf()` can only be called once everything is set properly; can assume kernelSerial

... and I have gotten to the point where I need to abstract a timer device as well now.  Ahhh... but should I also abstract the PIC?  This is bundled into the Timer Initialization at the moment....

---

While I am on the topic of the PIC, I asked about the definition of mask and unmask on `freenode#osdev` and the following was the related conversation:

```
[12:47] <eryjus> common terminology question -- when an irq is "masked", what is that intended to mean?  enabled or disabled?  I think I have been backwards all my life.
[12:47] <geist> disabled
[12:47] <geist> masked off, as in, covered
[12:47] <geist> like you put tape over the irq hole
[12:48] <eryjus> hmmm..  then i have been right..  thank you
[12:48] <geist> yah then you unmask it and it's available again
[12:49] <geist> i think i have heard from time to time someone trying to use it i the other way
[12:49] <eryjus> me too..
[12:49] <geist> mostly in that sometimes you get irq enable/disable bits that have the 'other' polarity
[12:50] <geist> and then folks will call the whole operation of having a register to enable/disable the irqs 'masking'
[12:50] <geist> which is a way to read it, but it doesn't imply polarity in that case
[12:50] <geist> but when polarity is desired, then mask == disable, unmask == enable
[12:50] <geist> independent of what the bits may or may not be
[12:51] <jmp9> mask = disable, unmask = enable
... snip ...
[13:01] <doug16k> yeah about that mask thing. I have an irq_set_mask function that takes a parameter named "unmask" because I wanted true to mean allow the interrupt. thinking along those lines has probably led to a bit of confusion
[13:02] <doug16k> should it take false to "un" mask it? maybe. that's the problem with the term
[13:02] <doug16k> eryjus, ^
[13:04] <eryjus> doug16k -- i've had to read that about 3 times to see if I can absorb it...  very confusing.
[13:04] * eryjus decides to use "enable" and "disable" in his own code]
```

---

It finally compiled!!!

---

OK, I have some problems with addressing.  A few of these I was not expecting.  For example:

```C
//
// -- This is the device description that is used to output data to the serial port during loader initialization
//    ----------------------------------------------------------------------------------------------------------
__ldrdata SerialDevice_t loaderSerial = {
    .base = COM1,
    .SerialOpen = _SerialOpen,                   // -- already in the __ldrtext section
    .SerialHasRoom = (bool (*)(SerialDevice_t *))PHYS_OF(_SerialHasRoom),    // -- in the kernel address space
    .SerialPutChar = (void (*)(SerialDevice_t *, uint8_t))PHYS_OF(_SerialPutChar), // -- in the kernel address space
};
```

The `PHYS_OF` macro looks like this:

```C
//
// -- This macro is intended to generic enough to convert a virtual address to a physical one.  However,
//    keep in mind that this works on one address only.  Therefore if a function calls another function,
//    this macro will fix the first one, but not the deeper call.
//    --------------------------------------------------------------------------------------------------
#define PHYS_OF(f)  ((archsize_t)(f) - kern_loc + phys_loc + ((archsize_t)&_loaderEnd - (archsize_t)&_loaderStart))
```

I know that this calculation works at runtime.  However, at compile time, I think the result is 0 (or I am discarding some section that would perform this initialization -- or maybe not calling the function to do this).

---

OK, I have x86 working.  Rpi2b is working as well except the output is getting stomped on.  There is a problem with waiting for the buffer to accept data or the baud rate is wrong.  Something did not carry over properly.

---

The pi is not outputting serial any more.  I thought it was about not being able to determine if there was room for data, but none of my changes are working.  I need to call it a night and come back tomorrow.

---

### 2019-Feb-25

I have gone back to basics today -- halting the loader right out of the chute and outputting some debugging info.  The serial port output routines are working properly.  At least early.  I am even able to get through the `SerialOpen()` function, which is here I assumed I had a problem.

It looks like somewhere in `MmuEarlyInit()` something is not right (or I am unable to use `kprintf()` right after anymore).  And that is the actual problem: I am trying to use `kprintf()` before I have the kernel mmio address space mapped.

So, I have 3 ways I can handle this:
1. Map the kernel mmio address space in `MmuEarlyInit()`
1. Change all my code to continue to use the `LoaderSerial*()` functions
1. Change `kprintf()` to be flexible in the serial device it uses

Each is a valid option.  It is also import to note that x86-pc does not have this problem since the I/O port remains the same for both the kernel and the loader code -- the only thing that changes is the location of the function calls which will work.

---

I went with the last option.  I am now through the loader and ready to jump into upper memory kernel code.  I will have a new wave of bugs and crashes to address at this point.

---

Both architectures have issues with IRQ interrupts from the timer.  For the x86, I appear to be having an issue with EOI; for rpi2b I am having a problem determining the interrupt IRQ number.  I will start with the rpi2b first.

---

OK, I was able to get the rpi firing the timer IRQ again.  Now, on to x86.

---

I got that working and I committed the code.  The next step is to get the heap working again.  This is where I left off before, so this is all new territory from here.

---

### 2019-Feb-26

Well, it looks like I am getting some serious interrupt flooding on rpi2b.  The problem I think I am having is that I am not resetting the interrupt flag and as soon as I re-enable interrupts I am getting creamed.

---

I am all but completely convinced that my implementation of the BCM2836 timer is totally fubar.  To research this, I have printed sections B8 and D5 of the ARM ARM and I need to read through these notes to figure out which way is up.  I think the short story here is I got lucky and did not really get the timer programmed properly.

One of the things I want to take a look at is the CNTFRQ register.  This register will hold the timer frequency -- which means I can get the timer programmed to provide an interrupt at the same frequency as I do with x86 and make the architectures operate much closer to each other.  Or perhaps I can set it.

I think I am going to need some test code to figure out how to handle this (I did this before, and I now do it again).

---

### 2019-Feb-27

I did some reading and I'm pretty sure I have this whole timer thing over-complicated.  There really should be only 3 special registers to update to get this working.  Let's see how this goes...

OK, I am not able to update the clock frequency, so I need to control this by the reload value I place in the timer.  This will be something to add to the device structure.

---

After some additional testing and a few changes, I do have the timer working in a more simplistic manner and probably far more robustly.  Since I am in a plane, I am going to commit this code but will not be able to push it yet.

---

Now on to the Heap for real.

The heap is not located where it needs to be given the new virtual memory layout.  I do not recall where it was intended to go and I do not have access to Redmine at the moment to look it up.  However, I should be able to make some minor changes to get this working again:
* Remove the early heap allocation from the linker script; this whole unmap and remap business goes away.
* Change the code that maps the heap to pull memory from the pmm since it is available now from the early kernel.
* Increase the size of the initial heap, but don't go too crazy...  It still pulls from physical memory to have those pages mapped.

That was a relatively clean change.  Now I just need to change the location.  Now, if this is really working like I want, I really can't get any farther without getting the pmm up and running.  I have 2 ways I can go with this:
1. I can fold the pmm into the kernel binary and map the bitmap pages in `PmmStart()` to a memory address in the pmm process.
1. I can keep the pmm as its own process and map the bitmap pages in `PmmStart()` to a memory address in the pmm process.

I think I want to do the latter.

---

Going back to the heap address, I have updated the wiki to use addresses starting at `0x90000000` for the heap.

---

With this change, I was able to commit the code.

Now, I will need to start thinking about the memory map I will use for the pmm.  Overall, I want it to be very similar to the loader, but scaled back.  All address space must below `0x80000000` for the process (all user processes), and I want the virtual starting address to be `0x100000` -- though I'm not sure why.

Now, I also need to consider how I am going to fold this module into the rest of the code.  I have decided to keep it a separate executable.  But, do I move it out to the same level as `kernel` is now?  Or let it remain in `modules`?  And, at the same time, I will need to take care of `libc`.

Ultimately, I think leaving them in `modules` is the right thing to do.  I also ultimately want to move `kernel` back into `modules`.

---

### 2019-Feb-28

So I am debating over how much of the `libc` code to implement myself.  I can write my own C library or I can port a `libc` to Century-OS.  According to [this site](https://stackoverflow.com/questions/34433976/is-there-a-difference-between-libc-newlib-and-uclibc-interface), `newlib` is only ISO C compliant.  This means that any messaging that I want to include is going to be an add-on feature to this under newlib.

Now, if I want to make the system Posix compliant (which is a bitch to be sure), GNU `libc` has all the support built into that.  However, the kernel will need to be Posix compliant to start with and I am not there at all.

My goal here is to get the Physical Memory Manager process operational.  For the moment, this will end up being a custom solution (which I will have to later retrofit to any other library I end up going with).  So, the end result is that I need to stick with what I have at the moment to keep things moving forward and port a library when I am ready -- not now as it will be a distraction.

Focus back on the pmm to get it building again.

---

### 2019-Mar-01

This evening I am working on updating the build to get the `pmm.elf` to build in the new targets.

---

I am struggling with the header files.  In my current situation, I need several types from the kernel, but I really am not interested in including all the kernel header folders.  This will mean some duplication...  and I am debating how to go about this....

---

### 2019-Mar-02

So, I really need to get into this, starting with `types.h`.  I do not believe that I will need anything that is arch-specific for the PMM.  So, I should be able to create a symbolic link and update the `types.h` include with `__has_include()` checks.

---

I was finally able to get the PMM to compile.  At the same time I think there are some improvements that can be made to the build system.  I did add [Redmine #404](http://eryjus.ddns.net:3000/issues/404) for the build system.  There is a lot of duplication to resolve.

---

### 2019-Mar-03

Working on the build system today....

---

### 2019-Mar-04

Still working on the build system today....



