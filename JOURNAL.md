# The Century OS Development Journal

## Prologue

I found some old code that I want to reuse.  The most successful iteration I have had was the second 32-bit version.  I thought I had lost that code (see below).  So I am going to try to resurrect it with this iteration.  Plus, I will clean up after myself a bit.

My goals from the previous iteration are all still valid, I am just hoping to move this along a little faster.  I plan on combining code from several sources here.


---

## Prologue from Century

So as I am sitting down today to start laying out how my virtual memory manager will be used, I really should be documenting my progress and thoughts if this is really going to be something others can learn from.  The README.md file is not really suited for the type of "conversation" I want to have here.  One of the goals of this project is, after all, to have a result that others can learn from -- even if it is from my mistakes.

So, what has happened to this point that I really should take a moment to catch up on before I get into the rest of the decision making.  Well....

I have decided on the initial architectures I would like to support:  i686, x86_64, and Raspberry Pi 2b (or 'rpi2b' as you will come to recognize throughout).  I also have an rpi3b sitting on my desk, but the ARM architecture if foreign enough at this point I want to make sure I can get through that architecture before I take on a new one.

I have been able to set up an emulator for each of the architectures and I am able to use make to invoke each architecture to test.  My memory is not as sharp as it used to be and so I cannot always recall the exact command I want to type for each emulator -- there are slight differences.

I have built up a massive (and admittedly difficult to maintain) make system, and have torn the core of that make system down in favor of tup.  I did talk about it a bit on osdev.org, here: http://forum.osdev.org/viewtopic.php?f=13&p=275461.

I also have taken on the rpi-boot second stage to boot the rpi2b architecture (compare that to GRUB for x86-family).  I have a loader (which has much my focus to-date), and a kernel (which is loaded my the second stage but never gets control, yet).

And I have a bitmap physical memory allocator (at least initialization) started.  Everything boots and the kernel module is loading.

I did start to parse the ACPI tables, and then decided to rip that code back out.  This deserves a bit of conversation.  I have conflicting goals and I had to figure out how to reconcile them.  The conflicts were: 1) all 1-time initialization was going to be performed in the loader; and 2) the CPU was going to be placed into its native mode when jumping from the loader to the kernel.  Well, the issue was that the ACPI tables were not all guaranteed to be locatable by 32-bit code.  This means the for 64-bit systems, I would need to either read the table in the kernel module, or would need to get into the native CPU mode prior to the jump to the kernel code in order to complete the initialization.  I chose to delay this decision by postponing the integration of the ACPI tables.

So, my priority is to get into the native CPU mode and then transfer control to the kernel module.  This change of course makes my Virtual Memory Manager (VMM) the next on the list.

I wanted to incorporate all the thinking I had done with Century64 (the assembly-based 64-bit version), but I just realized that the documentation was lost in the "Big SAN Crash of 2015!"


---

### 2018-May-23

The first orders of business are the basic structure and the build system.  First, I would like to structure to be relatively flat, but broken up into modules.  The recovered code from Century32 is a monolithic kernel, so everything was in the main directory.  On the other hand, several things are going to be hardware-specific and need to be placed into their own folders.  I may not be able to keep the goal to be flat as cleanly as I would prefer.  Back to that in a bit...

As for the build system, I prefer `tup`.  Is is very fast and have never let me down and forcing me to do a total rebuild.  However, there are just some things that `tup` requires that I do not like as well, such as forcing build definition files into the target folders.  Well, we can use `make` to manage `tup`, and that is the approach I will take.  For more information, check out the [readme](./README.md) file.

So, back to the directory structure...  This might sound a little odd, but I think the starting point should be the `Makefile` from century.  That `Makefile` is optimized for `tup` already and already takes care of some of the management I was referring to above.  It will be an easy clean-up from that start, so I will go and copy it.


---

### 2018-May-24

Today I am still working on getting the structure aligned.  I am continuing to follow the structure laid out in `Makefile`.

I also have another thought I want to work through...  NASM vs. GAS.  One is a dedicated assembler (NASM) while the other is part of the GNU compiler suite and therefore more integrated.  The problem with GAS is that it uses AT&T syntax -- which I believe leads to bugs.  I personally prefer the NASM syntax.  However, if I am recalling properly there is something I have to do when I link...  Stay tuned on that one, but I will use `nasm`.

At this point, I am starting to write `lists.cc`.  I wanted to take a moment to talk about the high-level language I am using.  I am writing my code largely in C.  However, I am going to make use of several features of the C++ compiler (inline functions as an example).  My call-out here is that you should expect to be reading Procedure Oriented code.  This does, however, set up a situation of name mangling with assembly.  To overcome this, any asm function will also be declared as `extern "C"`.

Another thought about organization is the number of functions to put into a single file.  Compiles run much faster with multiple functions in a single file (reducing the number of files to compile).  However, it is easier to maintain and read/understand the entire system with a single or few functions in each file.  Also, I believe it will aid portability.  So I am going to change back to a single function in each file.


---

### 2018-May-25

Today I will continue laying out the basic of the kernel support functions.  In this case, lists.

I have decided I will not be using `doxygen` for the kernel (at least not at this point).  Therefore, I am pulling out all the tags from the migrated code.  In the meantime, I am also working on thinking through the layout of the source and build system.  As an example, do I really need to compile asm from an architecture-independent folder and C from an architecture-specific folder?  Well, I have several things to work out as I lay in this first bit of code.

So I have finished the lists, changing the entire interface into inline functions.  At this point, I am going to perform a local interim commit to hold the state.

---

I have also been able to get the kernel to boot and immediately lock up (which is the intent -- no triple fault).  This is a rather big milestone, so I will locally commit again.

---

So, now after the commit, what is next?  Well I need to get data to the screen.  Let's face it: any sort of debugging is going to be impossible without that ability.  Century32 has a number of kScreen functions that outputs to the screen.  However, the rpi2b does not have a text mode.  For the moment, I am going to ignore that fact and start in text mode.  I will revisit this decision when I start the rpi2b version.

Also, another note is that I am going to start putting my TODO list into my own internal Redmine bug tracker.  However, one feature of century32 that I want to leverage into century-os is the status bar on the bottom of the screen.  This was a good tool to determine what was going on for the session -- a feature I lifted from the AS/400 green screens.

As I get into the early screen functions, I am struck by the amount of code duplication I had to deal with in century32.  I recall that this bothered me then as well and I never really addressed it.  The concern is that I had `kprintf` and a `ttyprintf` functions and the majority of the body of both functions were similar.  I was able to leverage some of the `kprintf` code, but that was not good enough.  So, I would rather write a `kputs` function that writes early messages to the screen and keep the bulk of the `kprintf` stuff really optimized for the tty functions.  I will be giving up the ability to print numbers and other things using `%` tags, but I think that is OK for the moment.  Just like `puts`, `kputs` will append a newline after writing the string.


---

### 2018-May-26

Today I will start to bring over early console functionality.  This will also drag over several supporting functions and I will make the effort to bring over all the similar functions at the same time.  I will start with a `ConsolePutChar` function and then build from there.


---

### 2018-May-27

Today I have finished up copying in several functions and getting my `ConsolePutS()` implementation working.  I now have a greeting on the screen.  This is a milestone so I will commit these changes locally.

---

So, since I am working on the screen, I think the next thing to do is to write the status bar.  This way the screen is actually 'complete'.  The status bar will have to be blank for now since I don't have too much to actually paint into the status bar, but I will start laying the foundation.  What I am thinking about is the Redmine (my internal bug tracker, sorry for the loss of visibility on that) I have put in for the `ConsolePutChar()` to be more architecture independent.  I will think on this a bit....

So, I did go ahead and make the `ConsolePaintChar()` function, as I had identified in Redmine #347.

The 2 key functions calls that I have ahead of what has been implemented so far is the function to set the cursor type and the other to clear the screen.  I think those are the next 2 I will be implementing.  This way, I will be able to look forward rather than backward.


---

### 2018-May-28

Today I started to wrap up the rest of the early console functions.  The last thing is the cursor geometry function -- and I do not see the value in it at the moment.  I may pull it in later.  So, I will commit locally at this point and move on to CPU initialization.

Actually, I pushed the commits to GitHub since I figured there was enough working to make complete an interim push....

---

So, the next major step to work on is CPU initialization.  I will be supporting several architectures here and I need to be able to determine several things during the initial startup.  One critical task is the figure out the system capabilities.  There are going to have to be several minimal capabilities that I require in order to boot and I need to make sure that they are available before I really get into the core of the OS.  An example is paging.  Another is the CPU instruction set.  After I determine all that, I need to put the CPU into a consistent state across all architectures.

This is a huge task that I will not complete all at once at this point.  However, I need to lay in the foundation for all this work and I will add to it as I add features.  The x86 systems require a GDT and IDT to start up, but are quite different in implementation.  ARM also will require some interrupt initialization as well and I want a consistent foundation for these.

So, the first order of business is the CPU capabilities collection.

Some thoughts on my research for CPUID.  This opcode is not available on all x86 CPUs and therefore may cause problems.  I want to be able to catch as many problem as I can and report them clearly.  However, there are some limitations and thoughts:
* I will be using a multiboot boot loader to load the OS, which requires at least a 286 to get into protected mode -- and PAE may also be required.  GRUB should be able to identify the CPU and handle any errors before getting to my kernel.  So, that responsibility is not mine to handle.  As a matter of fact, I am going to assume that I am running on at least a 386.

So, now that leaves only the CPUID instruction.  If the instruction is not available, the CPU will triple fault and reboot since I am not yet ready for interrupts.  This is not what I intend.  Therefore I need to detect whether the CPUID opcode is supported.

Now, what to do if it is not?  Well, my basic assumption will be that if CPUID is not supported, I will not have much of a chance to determine if the rest of what I need is supported.  Therefore, I will panic the kernel if it is not.  I may need to revisit this later with I get into ACPI parsing.  Maybe...

Ahhh...  my bad!!  The CPUID opcode is not available until Pentium.  Therefore, I will need to work out capabilities in other ways.

So, that now leaves the question: what kinds of capabilities do I think I might need to detect.  There are certainly differences between the 386 and 686 and x86_64 CPU capabilities; even more between the x86 family and ARM.  For now, I will assume only the 386 CPU capabilities as the baseline and determine how to handle this later.  For 32-bit I am leaving an empty function to fill in.

So, on to the base initialization for the GDT and IDT...  These need to happen for the x86 CPU family, but 32- and 64-bits are different.  Well, no they are not.  Here is a snip of the JOURNAL.md from *century*:

> ***2017-05-28***
>
> ...
>
> Both the 32-bit and 64-bit modes use 64-bits to describe the GDT entries.  Therefore they can coexist.  However, the order is quite important if we are to use SYSENTER and SYSEXIT for system calls.  I have not made any decision on this either way, but I also want to make sure that any decision I make in this next step will not limit my choices to a single option either.  Therefore, wherever these selectors are, they must appear in this order:
> 1.  Kernel Code
> 2.  Kernel Stack (this may be kernel data as well)
> 3.  User Code
> 4.  User Stack (and this may be data as well)

So, that helps to clear that up.  I also had an order defined:

> ***2017-05-29***
>
> ...
>
> So, that leaves the GDT layout.  Here is the target layout:
> * 0x00<<3: NULL descriptor
> * 0x01<<3: Kernel Code
> * 0x02<<3: Kernel Data (includes stacks)
> * 0x03<<3: User Code
> * 0x04<<3: User Data (includes stacks)
> * 0x05<<3: Unused (left open for user data if I want it later)
> * 0x06<<3: Unused (left open for kernel data if I want it later)
> * 0x07<<3: Loader Code
> * 0x08<<3: Loader Data
> * 0x09<<3: TSS
> * 0x0a<<3: TSS Second half (for 64-bit; NULL for 32-bit)
> * 0x0b<<3: Future use call gate? (need to research the rpi2b capabilities)
> * 0x0c<<3: Future use call gate?
> * 0x0d<<3: Future use call gate?
> * 0x0e<<3: Future use call gate?
> * 0x0f<<3: Future use call gate?
>
> For now, I will use only 16 GDT entries.  This will be frame aligned and have some room to grow.  The call gates may be able to be aligned with the SVC instruction for rpi2b.

With this outstanding research, I see no need to change my thinking.  I will leverage the GDT from the century code base.

---

### 2018-May-29

So century put the GDT at address 0x00000000.  It was also set prior to leaving the assembly language loader.  I want to duplicate that here, but I am not convinced that the address is really the best thing to use due to null pointer assignments.  Now, on the other hand, since I will be using paging, I can keep that page unmapped and be relatively safe.  So I think I will keep that setup.

The other thing to make sure I bring up is the BIOS Data Area (BDA) and the Multiboot info.  These really should work in conjunction with the `cpuid` instruction.  At some point, I will need to implement those into the CpuGetCapabilities function.

So I am not starting to build out the Interrupt Descriptor Table (IDT).  In reading, not every interrupt will clear the interrupt flag.  However, if the interrupt is an exception, the CPU (i686) will clear the IF.  Therefore, I will `cli` myself just to be sure.  Now, since they are all interrupts, the `iret` opcode will also restore the previous flags value.

The problem is where to put the IDT.  For 256 interrupts, the table will be 256*8 bytes long, or 2K, or 1/2 of a page frame.  The GDT is 128 bytes long in its current configuration.  I think it might make sense to put them into the same 4K frame, with the IDT being in the last 2K if this frame.  That would be at address 0x00000800 and a limit of 0x000007ff.  Like the GDT, I am going to load this in the in the loader before call `kInit()`.

The final thing to do tonight as I wrap up for the evening is to build the actual IDT able in place (at address 0x800) as part of the initialization.  That will be a task for tomorrow.

---

### 2018-May-30

So, I need to be able to construct an IDT so that I can handle interrupts.  That IDT needs to land at linear address 0x00000800 and be 256 gates long.  It will have to be constructed in-place.

I have this working now and just need to document my changes.  Also, with this new function, I have completed phase I initialization and I am able to enable interrupts.  I am able to type on the keyboard and the emulator does not lock up.

So for the record, what are the phases of initialization?
1. Anything required by the processor to set up initial state
1. Any OS internal structure initialization
1. Hardware Discovery (preliminary interrupts)
1. User Space ready
1. Become Butler process

With this, we are at a substantial point and I will commit these changes and push to GitHib.

---

So, the next thing to work on is Phase 2 (OS internal structures).  One of the first things to initialize is the Kernel Heap.  I am actually pretty happy with my heap implementation from century32.  It has been ported from one instance to another and this is no exception.  I will work on pulling that in from century32 to century-os.  This is the next task.

---

### 2018-May-31

I am working today on bringing over the heap functions.

---

###  2018-Jun-01

OK, I have been able to bring over all the heap functions.  Having done that, I think I may be running into a problem where the source folder is going to get quite full of files.  One one hand, I am not too concerned about it since the file is named the same as the function and all names start with the module (except maybe CPU).  On the other hand, it would be much easier for anyone else to find what they are looking for if all the heap functions and files were in a heap folder.  For now, I will leave it as it is but I will likely revisit this as I grow the kernel.

In the meantime, I commit and push since this is a rather significant milestone.

---

Now that the commit os done, I really need to be able to identify how much heap I really can have.  For the moment I have hard-coded the upper limit.  I was thinking I should get the upper memory limit from MBI, but that is not really a good solution since I will have paging enabled and will have a virtual limit.  I need to think this through but for the moment, I will keep it hard-coded.

The next thing to work through is paging.  However, I also need to set up the kernel into upper memory, which was not done in Century32.  It was in Century64 (assembly) and Century (multi-architecture).  This will be the hardest part of the port so far.  Tomorrow....

---

### 2018-Jun-02

I am looking at the best method to get paging turned on and have the kernel in upper memory.  I will need to set up a temporary page table to get into upper memory during early initialization.  This temporary table will likely need to be 1MB pages until I can construct a new table.  I'm not totally certain on this and have concerns with the frame size.

One of the things I will need to get settled is a memory map for all virtual space.  However, the thing that needs to be considered is the physical memory manager.  How can I allocate a physical memory frame to use in the paging tables if the PMM has not yet been initialized.  Once initialized, how to I make it continue to work?

So, I think I am going to change gears to work on the Physical Memory Manager.  I like to use a Bitmap to implements the PMM, especially since very little will be identity mapped.

A single page (4K) can manage 4096 * 8 * 4096 = 128M physical memory locations.  That will certainly handle the initial startup until I can officially put the Virtual Memory Manager (VMM) in charge.

One thing that I want to call out here is that a loader would be responsible for much of this work and would transfer control to the kernel with all the initialization completed.  All of these memory management structures could be built independently and control could be passed to the kernel in native mode.  This may be desirable since there is no way to load a 64-bit kernel with GRUB without using UEFI.  Since I am not, I will need a loader anyway when I get to x86_64 support.  Above that, I also will need to handle something for the rpi2b architecture and that has yet to be determined.  The rpi-boot project still has promise.

So I think the best thing to do is to change gears and work on the loader prior to the kernel.  This loader will be responsible for CPU initialization, getting the CPU ready for its native mode.  Native mode will be triggered with the jump to the kernel.  Getting native mode ready will also require some OS structure early initialization.  However, once the loader completes its work, the frames can be reclaimed.  It will also be much easier to get into a higher-half kernel.

So, the first order of business is to create a loader elf file that will be able to be readable as a GRUB and GRUB2 signature.  In order to do this, we will print a welcome message on the screen, taking over this responsibility from the kernel.

OK, so do I really need to know and understand *both* the Multiboot1 and Multiboot2 entry and resulting information structures?  I think it is best for the results I am looking for, but maybe parsing each of the respective data structures should be a loadable module.  Something to thing about.  I will leave that alone for the moment....

I've been a bit all over the place today.  However, I am happy with the results so far.  I think the loader is the right thing to do since I will have to normalize the architectures to a common starting point.  It also allows me to abandon the loader (which are also launched at different addresses on different systems and will be different sizes) once I have all the setup -- which might be quite a bit of code.  I recognize that I need to better document this starting point for entering the kernel, and that will come soon.  At the moment I can boot to both mb1 and mb2 and see the screen change to graphics (1024 X 768 X 16).

---

### 2018-Jun-03

So, today I will work on getting paging set up.  I want to get interrupts set up, but I cannot put any code in place that cannot be abandoned.  However, on the other hand, I will need to be able to receive page faults in order to debug the loader with paging on.  WAIT!!!  No I won't.  Here's why:

I will be setting up the structures and putting them into CR3.  The very last thing I will do is enable paging by setting the flag in CR0 and then jumping to the kernel proper.  With that, there is no need to handle interrupts until the kernel gets control.  The key to success here is going to be to set up a communication area where the loader can drop all the relevant data for the kernel to read and to set up some specific locations for things like the PMM and Paging tables that are configured by standards.

So, I think I will start with configuring the physical memory manager.  I think I will need to pick a single frame and use it for the first 128MB and then hand the responsibility for the remaining initialization over to the kernel -- or initialize the whole thing dynamically.  I think the best thing is going to be to to setup the first 128MB into a single frame so that we can have a fixed initialization step.

To do this, I will need to establish the functions to read the memory map from from the boot loader and load them into a frame for the kernel as well.

So a couple of things I need to make available:
* The hardware communication data area (assume at 0x200000 or 2MB), and will take 1 frame
* The PMM bitmap for 128MB (assume at 0x200100 or 2MB + 4KB), and will take 1 frame
* The Paging tables starting at 0x200200 (or 2MB + 8KB), and will take many frames depending on architecture

All of the above are subject to change.

However, since there are components (the hardware communication data area in particular) that will be used by both the loader and the kernel, it makes sense to start a common library that will be used by both.  This will be `libk`.

---

### 2018-Jun-04

Today I start with organizing the include files a little better.  I have 2 types that I need to maintain: 1) the internal kernel ones and 2) the ones required by the standalone OS programs.  The former should never make it to the output image and are expected to only be used for the loader, libk, and kernel modules.  The latter will need to be written to the /include folder on the resulting boot image.  Note that the loader, libk, and kernel will pull from both locations.

---

### 2018-Jun-05

I am working on getting the memory map pulled out so I can map out the physical memory.  More work on this today.

---

### 2018-Jun-06

I have the Multiboot 1 Information structure detailed out.  Now, I need to figure out how to get output and debugging information.  I have 2 choices at this point: 1) output to the screen, and 2) output to the serial port.  On one hand, I may want to output to the screen before the screen is ready for output and on the other hand the rpi serial port is not perfect with the cable I have been using.  A third potential option would be to output the log to a fixed point in memory and use a debugger to read it.  I think the best bet is going to be the serial port since I will be able to use that consistently with the emulators.  This will be only for the loader as the kernel will be able to write logs to files for debugging.

I will work on a UART driver for the loader.

---

### 2018-Jun-07

OK, I have the most basic serial output enabled for the loader, which will allow for the most basic debugging.  I am only able to output strings (no numbers, pointers, or hex), but I will add that as I need them.

I do need to go back and revisit the structure (which has not been checked in) and I have several files I need to properly comment.

With that complete, I will commit these changes at this point publicly as there are so many changes in structure and code, but not functionality.

---

Now, with that done, I can continue to parse the multiboot information structures.

---

### 2018-Jun-08

I am pretty happy with where I have gotten with the MB1 parsing.  It is not complete but I have enough to begin the physical memory manager initialization.

Next is to implement the same things with the MB2 Information structure.

And now that this is complete, it is time to set up the first 128MB of the PMM.  For this, I will need a bitmap and a location to put the data.

Actually, the PMM initialization I have written for century will initialize the entire physical memory space into the bitmap, and I will copy that to century-os.  That is important to note is that the PMM bitmap is placed just prior to either the EBDA or the 640K low memory cap, whichever is lower.  I have not yet found the EBDA, so I need to go after that first.

---

### 2018-Jun-09

OK, now to pull data from the BDA, which will include identifying the EBDA location.

---

### 2018-Jun-10

This morning I have been able to complete the code that reads the BDA and stores its contents in the internal hardware communication area.  Included in this is the location of the EBDA, which if it exists will serve as the upper boundary for the PMM bitmap in physical memory (alternatively, it will be the traditional 640K low memory limit).

So, returning to the top-down-ish approach for the loader, I can resume development on the PMM initialization.

---

### 2018-Jun-11

Today I am working on completing the PMM initialization.

---

### 2018-Jun-12

I have completed the fundamental PMM initialization given all the information I have mined so far.  I still have the video buffer and the modules to map, but I am not yet prepared for this.  I am going to commit these changes to the public repo and figure out what to work on next.

---

I think the next order of business will be to print out the greeting.  Since I am already in graphics mode from the boot loader, I will need to "paint" the characters on the screen.  I have already included the system font in the loader binary, so I just need to know the frame buffer address.  Thankfully, I have the frame buffer data in the multiboot data.  So, I need to extract this data from the loader, and allocate the frames in the PMM as part of the PMM initialization.  Finally, I will be able to paint a greeting on the screen (one pixel at a time).

---

### 2018-Jun-13

Today I was able to get the frame buffer to clear, setting the background to blue.  The next things to collect are the functions that will write the greeting on the screen and output that greeting.

The greeting is completed, so I will commit publicly again.

---

### 2018-Jun-17

So, the last 2 major things to do to get the minimal loader operational it to A) Enable paging, and B) parse the ELF file of the kernel proper (and other modules) and move them to the final location.

So, I think the best thing to start with is getting the ELF modules copied into the proper locations in memory (I need to map that memory for the PMM anyway).  So, this is not something that I have every fully implemented before, so it might take some time to get it right.  I cannot just copy the code from another OS attempt into this one.  This means that I might just have to implement some additional functions to output hex values and the like -- I'm likely going to need them for debugging.  I will build them as I go, but I do expect that this will be rather quickly once I get something that will execute.

The other things is where to write the debugging output.  The best place at this point I think will be the serial port as that is maintained on the console with a decent scroll-back buffer.  Not to mention I can easily redirect the output to a file if I need to.

Now, there are 2 standards I can look at implementing..  a 32-bit and a 64-bit ELF standard.  I am not yet familiar with them enough to discern the difference.  So, reading it is...

It looks like if I am able to write the ELF data types properly, I should be able to merge the 2 specifications together cleanly.  The 64-bit spec is supposed to be a minimal extension of the 32-bit spec.  So, I start with merging the data types and the program header structure.

So, I managed to get the ELF File header structure written and now need to identify the modules that were loaded by multiboot and figure out what I am going to do about them.  At the moment, I am parsing the multiboot information prior to initializing the PMM.  This means I will need to capture the module information as part of the multiboot parsing, then allocate the memory that is already in use by these modules in the PMM, and then finally parse each module's ELF information to extract the program and data and prepare each module for execution.

So, I go and get the module information from both MB1 and MB2....

---

### 2018-Jun-23

Yeah, I have had to walk away for a couple of days.  Ok, nearly a week.  But this is also one of the reasons for this journal; I can review my thinking and get back into the code much faster.

A quick review shows me that I am collecting the module information from Multiboot but I have not yet updated the PMM to have that space allocated.  So, I will go and clean that portion up so that the PMM is accurate again.  I have also been working on the ELF structures, trying to get them combined in a manner that will work for me.  But I missed the PMM from before so I need to get that addressed.

So, now as I'm trying to start the parsing process for the modules and elf information, I am realizing that I am going to need to copy the module named into a holding area in the hw-disc structure.  Currently, I am limited to 4096 bytes and I check for this at compile time, throwing an error if it's too long.  However, I think I'm going to need this to be longer.  At the moment, it appears that I am stepping on the original MB1 module name (likely with the PMM memory map, but that's a guess at the moment).  I know that when I add the module to the structure, the name is available and when I am trying to initialize the module the name is garbage.  The things that take place between are the PMM initialization and the frame buffer initialization.  The frame buffer should not be updating anything, so I am convinced it is the PMM structures.

The multiboot specification says that these structures can be anywhere in the first 1MB memory and to collect all data needed before overwriting the structures.  My own hardware discovery structure I am building is above that mark and I will be copying it back into low memory once I have everything I need initialized.  So, long story short, I will need to make a copy of the strings in higher memory.  But the best way to do this....?

So, I have added a short character array into the module structure.  I am not checking for overruns so there is a risk here and I will likely have to come back around to this at a later time.  However, this will allow me to move forward.  I will leave an open Redmine for this to address.

Now as I start to look into where to put the kernel in physical memory, it dawns on me that I probably need a physical memory map.  This will be very different than the virtual memory map that I will use once I get paging started.  I do not need this memory map for all of the memory, but probably only up to 2MB.  The reason is that there will be nothing else I will want to look at beyond 2MB other than the modules.  It is the lower memory that I will be most interested in.  Therefore, I will only map 32 bits of memory space, but the same will hold true for 64-bit systems.

| From Address (inclusive) | To Address (exclusive) | Size | Use |
|:------------------------:|:----------------------:|:----:|:----|
| 0000 0000                | 0000 1000              |  4K  | GDT and IDT tables |
| 0000 1000                | 0001 F000              | 120K | Free Space |
| 0001 F000                | 0009 F000              | 512K | PMM Bitmap for 16GB |
| 0009 F000                | 000A 0000              |  4K  | EBDA |
| 000A 0000                |                        |      | 640K boundary |

This map brings 2 things into very sharp focus:
1. 120K of free space is not enough to put a kernel, so it will have to be relocated to above the last loaded module.
1. 16GB of PMM bitmap will take most of the rest of the available low memory (which I will need to keep some for DMA accesses I understand), so I will need to limit the amount of PMM Bitmap I put into low memory.  This one will be put into a Redmine so that I can address it at a later time and not forget about it.

---

### 2018-Jun-24

OK, so today I will need to identify the last frame that the modules occupy and then begin the process of copying the kernel module into its new location.  All of this is to get to the point where I can launch the kernel (and issue a second greeting) in upper memory and then reclaim the loader space.

Wait a minute!!!  Do I actually need to copy anything??  I mean seriously with the kernel?  If I have been able to architect the format of the kernel.elf file properly, I should be able to just create the VMM mappings to the kernel in memory wherever it landed and viola!

For this to be a viable option, I will need to make sure that the segments are 4K (or page) aligned.  After that, I should only need to read the program header sections to determine the virtual memory location for the page mappings.  I will also need to be able to determine if the kernel is page aligned in the disk file.  However, this should all be doable.  And this will save some work and quite a few CPU cycles.  It's also far more elegant than a brute force copy.

With that said, a quick test illustrates that the current kernel meets these requirements.  Therefore, I just need to move on to the MMU (Memory Management Unit), or the paging setup and start mapping the pages to frames.

The MMU is highly architecture dependent.  So, as usual, I will focus on the i686 architecture first.  We are able to cover 3.5GB of memory with a 32-bit processor.  This is a complicated concept for many, so I will try to lay things out a little bit here.

In the 32-bit architecture, the tables are 3 levels deep.  There is the `CR3` register which holds the physical address (or frame) of the Page Directory (PD) table.  A Page Directory Entry (PDE) will hold the physical address (or frame) of the Page Table (PT).  And a Page Table Entry (PTE), holds the physical address (or frame) of the Page.  A page will map 1:1 with a frame.  Many PTEs can refer to a single frame.

One of the keys for managing these tables is how to manage the Tables themselves.  We accomplish this by doing what is referred to as a recursive mapping.  This takes the last Page in the entire table structure and points it to the same physical address (frame) of the PD table, or the value in `CR3`.  In this way, we can use Virtual Memory Addresses to maintain the table easily.  More on this as we get into it.

For my loader, I am dealing with Physical or Linear memory, not virtual memory.  There may end up being some wasted code here.  But the first thing I need to do is to establish the location of the PD table in physical memory.  Looking back on the physical memory map from yesterday, this will be at address 0x0000_1000.  It takes up one frame only and should be initialized to 0.

After that, I just need routines to walk the VMM Tables and create new ones as needed with the help of the PMM to offer us frames.  Since we are dealing with physicals addresses at this point, the translation is trivial and there is no need to use the recursive mapping -- but I need to set it up for sure!

So, there are a few functions I will need to create in order to get this all working.  First of all, I will not be releasing or un-allocating any pages, so whatever I write I only need to be concerned with allocation.  I will need to be able to build new tables dynamically from the PMM and initialize them.  I will also need to be able to walk the tables to determine what is missing and what to build.  At the core of all this functionality, is a `MmuMapToFrame()` function.

So, now that I am starting to set up the MMU, I am also going to need a Virtual Memory Map.  Each process is going to have its own paging tables, so each process can have its own virtual memory address space which can overlap others.  I will have to work on that tomorrow, but I have several iterations I can draw from.

---

### 2018-Jun-25

OK, so maybe there isn't a version of the VMM layout I can pull from...  I cannot seem to find it anyway.  But I'm going to keep looking a while longer.

Well, it looks like I'm going to be making this up again....

First up is the recursive mapping for the paging tables.  The top Page Directory entry controls access to 4MB of pages.  This entry is remapped to recursively point back to the Page Directory.  Therefore, we will lose 4MB of data space, from ffc0 0000 to ffff ffff.

| Start Address | End Address | Size | Usage |
|:-------------:|:-----------:|:----:|:------|
| 0000 0000     | bfff ffff   |  3G  | User Space & More |
| c000 0000     | cfff ffff   | 256M | Kernel Code |
| d000 0000     | efff ffff   | 512M | Kernel Data & Heap|
| f000 0000     | ffbf ffff   | 252M | Kernel Stacks |
| ffc0 0000     | ffff ffff   |  4M  | Recursive Mapping |

* I want to note that I implemented recursive mapping incorrectly and will need to be corrected.  It has not yet been committed so you will not see the mistake before I correct it, but it does happen and is important to be aware of the complexity of this concept.  I have a Redmine for this and I want to continue with my thinking.

So, it's also important to make a formal note of how the paging tables are found.  The Page Directory Table is located at 0xffff f000.  Then, Page Tables [0] to [1022] ([1023] does not exist!) are located at `0xffc0 0000 + (idx * 0x1000)` -- More on idx [1023]: note that 0xffc0 0000 + (1023 * 0x1000) == 0xffff f000, which is explicitly the address of the Page Directory Table.

Take some time to get your head around this concept.  It can be difficult to fully grasp and even if you think you have it, you may still stumble.  For reference, see my own mistake above.  For more information, see: https://wiki.osdev.org/Page_Tables.

---

### 2018-Jun-26

Today I realize that I am going to need to debug the paging table maintenance.  In particular, I need to write a hex number to the serial port.  I will start working on that feature today.

I managed to get a bunch of debugging done today.  I will need to comment out all the changes I have made, but I am not sure I have all that in me this evening.  I will start tonight and probably finish this work tomorrow.

---

### 2018-Jun-27

OK, I have managed to get everything documented the way I want (I think anyway).  I now need to initialize all the rest of the kernel mappings, including the loader which will have to be de-allocated by the kernel.  There are also a couple of additional thing I need to map into a final location as well, so the virtual memory map from 25-Jun will change.

---

### 2018-Jun-28

So, I start today looking at the virtual memory map and what I have left out.  I'm going to start a list:
1. GDT/IDT
1. A Page location for clearing new frames that have been allocated
1. PMM Bitmap
1. Mapped Frame Buffer
1. Memory Mapped I/O (driven by hardware -- so I need to research this a bit)

The place in memory I am looking at putting most of this stuff is to co-mingle it with the kernel stacks.  All except for the Frame Buffer.  That I may carve out 1GB right below the kernel.

So, video buffer sizes...  The site https://www.digitalcitizen.life/what-screen-resolution-or-aspect-ratio-what-do-720p-1080i-1080p-mean (admittedly a two-year-old web site) reports the largest resolution size 3840×2160 as the largest resolution I might need to support.  Now, it has been 2 years and so the largest resolution is certainly bigger that this already.  However, with that said, looking at 3840×2160×4 (bytes per pixel) results in just under 32MB of memory needed to represent the frame buffer.  If I want double buffering, I will need 64MB (which leaves a little room I can use to get cute with in the future if I want).  Of course, by reducing the number of bits per pixel I can still support somewhat higher resolutions.  However, I think the point of all this is that for the 32-bit kernel, this size is plenty and sized reasonably well for the memory limits (2% of all supported physical memory in non-PAE 32-bit memory).

Now, for the PMM...  The same type of calculations need to apply here.

---

### 2018-Jun-30

Well, I was not able to complete my analysis and so I need to pick that up here today.  The thinking I departed with was that on a 32-bit OS, only 4GB of memory is addressable.  4GB = 4096MB and since I can keep track of 128MB of PMM frames in a single 4096 byte (32768 bit) frame, I only need 32 frames of memory to control all the possible address space.  For 32-bit; 64-bit will be different.

But I was researching why many 32-bit operating systems will only report 3.5GB memory.  In particular, Windows is like this.  I believe it is due to the shared video frame buffer taking up 512MB of memory and I am Googling for confirmation of this. This web page from Microsoft (https://support.microsoft.com/en-us/help/978610/the-usable-memory-may-be-less-than-the-installed-memory-on-windows-7-b) indicates that the total usable memory is the total physically installed minus any "hardware reserved" memory.  The video frame buffer qualifies as hardware reserved memory in the example provided on that web site.  So, that answers the question about 3.5GB vs. 4GB.  Again this is all non-PAE.

All of this research leads me to the conclusion that I will only ever need a maximum of 32 frames (or 128KB) of memory, which is what I need to find room for in the virtual memory map.

Now, going back to the frame buffer for a moment, I want to call out that I am not going to support multiple monitors at the moment.  All of the calculations above assume a single monitor.

Where to put all this memory?  Well, I need to take a look at the stacks I will use as well.  Do I really need 252MB worth of stacks for the kernel?  Part of that answer will be determined by the syscall method I chose to implement.  I am going to assume a simple interrupt-based method at this point.  There are a couple of reasons for this:
1. The method is simple
1. The method relies on the interrupt table for stack management
1. I should be able to implement a simple jump table to determine what gets control, which will be relatively fast (versus a long line of compares)

So, for the 32-bit intel architecture, there is a limit to the number of stacks I can provide to the interrupt handlers.  With that said, 252MB of stacks does not seem reasonable (16128 individual 16KB stacks...).  So, what does?

The Intel 64-bit architecture has the ability to specify 1 of 7 stacks as part of an interrupt call.  Plus it can use the calling user-space stack.  That is not a hard limit as the stacks can also be managed once the interrupt gets control if need be.  In the Intel 32-bit architecture, this is all dedicated to the user space stack and an interrupt-specific stack is not set by the CPU.  However, that does not prevent us from setting up our own dedicated stack in software.

We have 256 possible interrupts.  If we wanted to have each with its own stack that would be 256 stacks.  16KB for a stack seems reasonable, especially for 32-bit CPUs and might even be over-kill.  Since the kernel is not "running" but rather "called" using a system call (syscall), there is no reason to keep a dedicated stack for the kernel.  However, there will be several functions (drivers?) that will require their own stack space.  Each of those will have the stack available from the process itself and not from this pool.  So, I am down to 256 stacks as my upper limit.

256×16KB = 4096KB = 4MB

Now, that is an interesting result of a calculation.  4MB is the exact amount of memory that is controlled by a Page Directory Entry.  Just like the Recursive Table mapping, it would be really nice to be able to keep the stacks in a single PDE.  Not sure why...  Maybe it's my own personal OCD speaking to me.  But, I will update the table and add this info:

| PDE Entries | Start Address | End Address | Size | Usage |
|:-----------:|:-------------:|:-----------:|:----:|:------|
|    0-511    | 0000 0000     | 7fff ffff   |  2G  | User Address Space & More |
|   512-767   | 8000 0000     | bfff ffff   |  1G  | Device Driver Address Space (Code, Data, Stack, etc.) |
|  768-1003   | c000 0000     | faff ffff   | 944M | Kernel Code, Data, and Heap |
| 1004-1019   | fb00 0000     | feff ffff   |  64M | Video Frame Buffer |
|    1020     | ff00 0000     | ff3f ffff   |  4M  | Unused at this time - forces page faults |
|    1021     | ff40 0000     | ff7f ffff   |  4M  | Miscellaneous Small Elements (GDT, Temp Pages, PMM Bitmap, etc.) |
|             |      1000     |             |      | GDT/IDT |
|             |      2000     |             |      | Temp Page for clearing frames by the PMM |
|             |      3000     |             |      | PMM Bitmap Page  1 of 32 |
|             |      4000     |             |      | PMM Bitmap Page  2 of 32 |
|             |      5000     |             |      | PMM Bitmap Page  3 of 32 |
|             |      6000     |             |      | PMM Bitmap Page  4 of 32 |
|             |      7000     |             |      | PMM Bitmap Page  5 of 32 |
|             |      8000     |             |      | PMM Bitmap Page  6 of 32 |
|             |      9000     |             |      | PMM Bitmap Page  7 of 32 |
|             |      a000     |             |      | PMM Bitmap Page  8 of 32 |
|             |      b000     |             |      | PMM Bitmap Page  9 of 32 |
|             |      c000     |             |      | PMM Bitmap Page 10 of 32 |
|             |      d000     |             |      | PMM Bitmap Page 11 of 32 |
|             |      e000     |             |      | PMM Bitmap Page 12 of 32 |
|             |      f000     |             |      | PMM Bitmap Page 13 of 32 |
|             |    1 0000     |             |      | PMM Bitmap Page 14 of 32 |
|             |    1 1000     |             |      | PMM Bitmap Page 15 of 32 |
|             |    1 2000     |             |      | PMM Bitmap Page 16 of 32 |
|             |    1 3000     |             |      | PMM Bitmap Page 17 of 32 |
|             |    1 4000     |             |      | PMM Bitmap Page 18 of 32 |
|             |    1 5000     |             |      | PMM Bitmap Page 19 of 32 |
|             |    1 6000     |             |      | PMM Bitmap Page 20 of 32 |
|             |    1 7000     |             |      | PMM Bitmap Page 21 of 32 |
|             |    1 8000     |             |      | PMM Bitmap Page 22 of 32 |
|             |    1 9000     |             |      | PMM Bitmap Page 23 of 32 |
|             |    1 a000     |             |      | PMM Bitmap Page 24 of 32 |
|             |    1 b000     |             |      | PMM Bitmap Page 25 of 32 |
|             |    1 c000     |             |      | PMM Bitmap Page 26 of 32 |
|             |    1 d000     |             |      | PMM Bitmap Page 27 of 32 |
|             |    1 e000     |             |      | PMM Bitmap Page 28 of 32 |
|             |    1 f000     |             |      | PMM Bitmap Page 29 of 32 |
|             |    2 0000     |             |      | PMM Bitmap Page 30 of 32 |
|             |    2 1000     |             |      | PMM Bitmap Page 31 of 32 |
|             |    2 2000     |             |      | PMM Bitmap Page 32 of 32 |
|    1022     | ff80 0000     | ffbf ffff   |  4M  | Kernel Stacks -- 256×16KB |
|    1023     | ffc0 0000     | ffff ffff   |  4M  | Recursive Mapping |

One of the things I think I like about this layout is the gap up unused address space between the Video Frame Buffer mapped location and the other small elements.  In the event I have some sort of video buffer overrun, I will have a place in memory that will force a page fault that I can trap.  Now that not to say that I cannot achieve this withing PDE 1004-1019, but I really like the extra buffer space for safety with larger calculations.

Now, a note for those following along at home...  This Virtual Memory Map is where things will be located in address space only.  Not every page will be mapped to physical memory.  Therefore, not every address space will be used.  This map is used (and in particular the memory above 0xc000_0000) is used to build out the Page Directory only.  In the event a page is not mapped to a physical frame, a page fault occurs.  At some point, I will start swapping pages from memory to disk and retrieve them on demand, which will mean I will need these page faults as a feature and not just a problem.  That's a bit down the road.

So, I have spent several days thinking and documenting my thoughts.  I have been able to leverage a large amount of code so far to get to this point, but now I am starting to write much more from scratch.  This thinking is critical.  I have said before and I will say again:

> Writing a hobby Operating System is more of a research project than a coding project.

But I can finally write some code again and will go about mapping all these components I have.

It turns out that one of the things I have discovered was that the EBDA is either taking more than 1 frame or is not butted right up against the 640K boundary.  I need to figure out which.  For now, I have made the EBDA 1 frame in length but I may be truncating lots of information from the EBDA I will want to collect.  For this one, I am adding it to Redmine to make sure I do not lose track of this need.

So, as I end the day today, I have everything except the video frame buffer mapped into the paging tables.  This does include the kernel mapped to the proper location as identified in the ELF file and how the executable was linked.

I now have all the MMU mappings completed for in preparation for the kernel.  With this, I wll check my code for comments and will commit it publicly tomorrow.  I am not yet enabling paging or jumping to the kernel, so there is still a lot of debugging yet to do on all this work I have done.  That will be a major task on its own.

---

### 2018-Jul-01

So, the first order of business today is to get this code committed.  I need to work through all the files I have modified and make sure the comments are proper and up-to-date.

---

With that commit pushed to github, on to the next item on the list.  That is going to be to get the kernel to launch and to print a second greeting on the console.  This will require that paging is enabled which is going to require a lot of debugging.  The problem is going to be that I do not have a Page Fault handler that I have written or installed, so any issues at the moment are going to triple fault the PC and cause it to reboot, with no indication of where the problem is.  For now, I will start with the `MmuDumpTables()` function to check on things, but I have a feeling I will need a rudimentary Page Fault handler to dump data to the serial port.  I will write it when I just cannot tolerate not having it anymore, which will be far later than I really should write it.

But before I get into that, I am a formatting change I want to make to this journal file.  I am going to go back an put horizontal lines between the different days to aid in reading.  I am not going to make any changes in the content (I'm not even going to correct any typos).

And,...  as I predicted,... I have a triple fault as soon as I enable paging.  So I have some debugging to do.

OK, so the stack was not mapped into the paging tables.  It needed to be frame-allocated and then identity-mapped.  Once I got that completed, I was able to get paging enabled and not have a triple fault.  The key here is that the stack is also hard-coded at the frame right below 2MB and that will eventually get stepped on by the loaded modules.  I have entered a Redmine for that to fix and will likely be built into the loaded executable (loader.elf) at some point.  For now, I can get away with it.

So, the final thing is going to be to jump into the actual kernel code and acknowledge the arrival somehow.

Just to make a note of where I am at this point, I am jumping to a kernel location (which I think is a good location) but the hardware structure is not available to the kernel.  As a result, I am not able to confirm it with what happens on the screen.  I mistakenly moved `hw-disc.cc` to libk but it really needs to be copied to a good location in low memory by the loader and then pointed to in the kernel.

OK, I was not able to get the frame buffer to work, but I was able to product serial output that demonstrates that I am executing the kernel.  I do have some things to work out, but the video driver will probably move the frame buffer if possible.  So, I am going to call this objective complete (the kernel is being executed) and commit this the code at this point.  I am also going to zip up an .iso image that can be run from a command line and drop that on the github project as well.

---

Well, I am not able to upload the file, but I have it stored on my own hard disk.  I will live with that.

---

### 2018-Jul-02

Today I start thinking about what I really want in my kernel proper.  I know I want to write a microkernel.  But, a microkernel has limited functionality.  So, what do I want to include in my kernel.  Hmmm....  lots of thinking.

---

### 2018-Jul-03

So, I think the first thing I have settled on for inclusion in the kernel is the management of the Interrupt Table.  The reason for this is that the device drivers are going to have to register an interrupt for the top half of a device driver.  As an extension of that function, the prologue and epilogue to the interrupt will also be handled by the kernel (this will save the context on entry and restore the context on exit).  However, I believe that will be the extent of the functionality required of the kernel for interrupts.

The next thing that needs to be included in the kernel is messaging, or Inter-Process Communication (IPC).  We will need the ability to send and receive a message that is handled by the kernel.  This also means that there will be a heap to store the message while it is en-route.  I will need to set timeouts for messages and be able to respond to those as well.  This means that a basic timer management will need to be included at the kernel level -- at least to manage timeouts.

I want to note that process synchronization will be tightly coupled with the scheduler.  I hope to be able to implement that in user space.

One final thought will be a kernel debugger.  This debugger will need to have visibility to all the structures for the kernel and possibly the device drivers.  As such, it might need to be a kernel function rather than a user-space function.

Beyond that, I am not sure what more is really needed.  I think everything else can be moved to ring 3....  Time will tell.  In the meantime, I will start with the interrupts and IPC capabilities.

---

### 2018-Jul-04

The first thing to do today is to strip the kernel of everything that I no longer need -- things that were moved from the kernel to the loader.  This will be significant and will result in most of the kernel source going away.  This is going to be scary, but at least everything I am about to delete is still available on github.

I really did not delete as much as I thought I would....  I guess that's a good thing.  I still have all the heap code I moved over near the beginning and I have the start of an Interrupt Table manager built in.

However, I need to revisit the goal of having all the one-time initialization completed in the loader.  I'm nearly positive at this point that I am not going to be able to meet this goal.  For example, I will need to have kernel exception handlers that for things like divide by 0 that will be responsible for shutting down the offending process.  I am not able to initialize that in the loader (at least not without some really fancy data structure in the kernel that I find and read for addresses -- which might be a possibility at a later time) without having a symbol table to read and I am not really interested in forcing debugging information to be available in the kernel.  While it is a great goal, I think it should be set aside for a moment and revisited once I have a better working system (read: working system).

So, this will meant that I will need to perform some initialization for the IDT in the kernel.  It will involve setting up some basic exception handlers and registering them to the IDT.  I also want to have a version of `kprintf()` that writes to the serial port rather than the console.  `kprintf()` is a simple port from a previous version and I believe I have several of these exception handlers that I can copy as well.

---

### 2018-Jul-05

I was able to finish up the ISR initialization routines.  I have no ISRs yet, certainly nothing for the errors.  These are all written in century32, so I will be able to copy them into century-os and fix up the comments.

As I start copying the Interrupt Handler for INT0...

---

### 2018-Jul-06

Well I never finished my thought yesterday.  I got distracted....  But it's all good.

So, what I was saying about the INT0 interrupt handler...  I was doing some research on the best way to handle a divide by 0.  Many applications development languages can capture that error and handle it within that application, but if it ever makes it to the OS, we really need to kill the process.  Now, the question come up about what to do when the exception occurs in the kernel itself, or from a driver -- how should that be handled?

Well the questions do need to be considered, but I have nowhere near the base code written to address the issue.  I will come back to that, but it is certainly food for thought.

So, for the moment, I will just dump the contents of the registers to the serial port and report the exception.  At the moment, the only 2 choices I have are to `Halt()`, or continue on.  I think I will `Halt()` at this point since I am still deep into development and need to solve these problems as they come up.  Once I have user-space functionality I will reconsider.

So, I am going to need a basic `IsrDumpState()` function to report the contents of the registers.

---

Now, with a basic ISR0 handler I am able to perform some basic testing.  A few things I noticed right away:
* I forgot to load the IDT in the loader.  I make that happen in the kernel entry and cleared the location as well
* I needed to identity map the IDT to get that to work.  This is risky and I will need to clean that up at some point, since it leaves the GDT and IDT exposed to NULL pointer dereferences.
* The `IsrDumpState()` function is not writing everything to the serial port.  Not sure why.  I need to figure that out and clean it up, but there are some issues for sure.  Whatever is going on, it appears consistent on the surface.
* I cannot be sure that the register values are aligned.  I need to dig a bit more.

---

### 2018-Jul-07

This morning I will start looking at what is going on with `kprintf()` -- I need to make sure I can get accurate debugging information to solve other problems.

After some testing, it looks like my `kprintf()` implementation is not perfect.  This is something I have used in several other versions and I am not sure why it is breaking now.  It is taken from http://www.jbox.dk/sanos/source/lib/vsprintf.c.html.

I'm going to have to set up some debugging to get to the bottom of the issues.

---

### 2018-Jul-08

So..., I am left wondering if I should work on my own `kprintf()` implementation or try to fix this one.  My bet is that the problem is not in `kprintf()` but in the ISR handler or stub.  However, `kprintf()` should not stop printing if there is something wrong.  What is interesting is that if I split the prints into separate lines I have the same problem as if I have 1 line of code for each full line of output.

I decided to make it my own and abandon the one from the internet.

---

I was able to get a basic version started.  It is not as fully functional as the normal `printf()` function.  However, for my needs at the moment it is a good solution.

---

### 2018-Oct-09

Well, It's been a while.  Let me start by saying how much I appreciate the journal I am keeping!

A quick test tells me that the last concern appears to have been addressed.  I am able to test a divide by 0 and get debugging information to the serial port.

So, this leaves me looking at the Redmine issues that are documented to see what is next.  I see that the kernel is not able to write to the frame buffer once we get to the kernel.  This is Redmine #361 (http://eryjus.ddns.net:3000/issues/361).  I will take a look at this next..., well after dinner.

---

So, there is a line in the kernel that should greet the user: `kprintf("Hello from the kernel!!\n");`  This text is not appearing on the screen.  So, this is the bug.

However, I am not certain if anything is being written to the screen once paging has been enabled.  Additionally, I want to make sure that the frame buffer is being updated properly (or in totality) once paging has been enabled. -- Actually on review, the new `kprintf()` function only writes to the Serial Port.  So I need to add the a call to `void FrameBufferPutS()`.  Adding this into the kernel causes a triple fault.

I believe it is a page fault that needs to be investigated....  To that end, I will build out a Page Fault handler which in `IsrInt0e()`.

---

Well, the function pulled together quickly, but yielded no information.  My problem is not a Page Fault.  So, the next 2 faults to look at are a GPF (13 or 0x0d) and Double Fault (8 or 0x08).

And again nothing....  So, there are a couple of things that might be in play here:
1. The IDT is not working properly even though IsrInt00 is working properly.
1. There is a failure ahead of the one I am trying to debug.
1. There is some other thing happening (interrupts enabled?) that is causing the problem.

Well, it looks like the problem was really that of sequencing.  I was looking for a page fault before the `IdtBuild()` function was called.  I set the `IdtBuild()` function to be called ahead of the greeting.  This revealed a Page Fault.  Also, CR2 holds the value `0xfd010000`.  So, I will need to review the Page Table construction.

---

### 2018-Oct-10

Some debugging code reveals that I am trying to write to the frame buffer at address `0xfd010000`.  However, the frame buffer is only mapped through `0xfb17ffff`.  So, there is something going wrong with the (x,y) coordinates for where I am trying to write in the kernel.  I might even have a problem with the starting location for the frame buffer.  But I am on the right path here.

It turns out that I am using `GetFrameBufferAddr()`, which looks at the fbAddr in the hardware discovery structure.  I am populating this with the physical address of the frame buffer, not the virtual address.  Since this function is used across both the loader and kernel, the fix for this will be delicate.  The way I chose to fix this is to reset the frame buffer address to the virtual address once paging has been enabled.

I believe it's time to commit my changes.

---

### 2018-Oct-11

Today I will take care of the remainder of the interrupt handlers, making sure that all the exceptions are caught and outputs the system state properly.  This ended up being quick work.  Copy and paste works.

I reviewed the Redmine issues and several appear to have been resolved.  I am cleaning these up and getting things ready for the next major coding element.  Not everything is critical for this moment.

So, I am going back through my old `kInit()` function.  I am about done with Phase I initialization.  All I need to do is reset the greeting and remove the commented lines from the other phases that are now complete with this phase.

After completing the Phase I initialization, I committed my changes.

---

I still have concerns about the directory structure.  I commented on this back on 2018-Jun-01.  I am still not too sure about what I am going to do, but I think I will need to sort that out before I bring over any more files.  On one hand, I want it to be easy to read and maintain.  On the other hand, the `Tupfile`s will need to be maintained with each new folder.  Since I already have the heap functions imported into the kernel, I will kick this down the road until I get through the heap initialization debugging.

Initializing the kernel heap begins Phase II initialization.  Which of course immediately ends in a page fault.

---

I was able to determine that the initial heap is not mapped in the paging tables.  This should be a relatively quick fix.  I hope.  I will take that on tomorrow.

Well, looking a little further into it, I do not have a Physical Memory Manager (PMM) implemented for the kernel.  I will need to take this on ahead of mapping the heap memory.

---

### 2018-Oct-12

Starting out today, I am struck by the need to rebuild (well, the proper term is 'build' since I don't really have one) my PMM.  What I have done so far is just get the basics taken care of for the loader.  Now I will need to copy the results into the area controlled by the kernel and put the kernel in charge.  But with that, I now have another chicken-and-egg problem.  I would like my PMM to work as a user-space process.  But I also need to get it set up and running prior to having legitimate user space.  This is also going to mean that I need to have some code running as a service that takes care of the allocation/de-allocation and when I write that a read duplicated code.

The question I have at the moment is how the 'microkernel' is defined.  I can easily see the kernel only being that part that is running at privilege but the source containing the other components that run in user space -- all of which is compiled into the `kernel.elf` binary.  On the other hand, I can also see the measure of the kernel being the kernel.elf binary and if more things than the code that runs at privilege then the kernel is more hybrid or monolithic.  I'm spending some quality time with Google on this question.

One of the things that strikes me with this additional PMM initialization is that anything I allocate in the kernel initialization is not going to be de-allocated in this kernel initialization.  I'm not sure if this will help me at this point or not.

After some research, I notice the following:
* There is a common measurement of the number of lines of code for a kernel where a microkernel is somewhere in the 10K lines order of magnitude.  Compare that to the monolithic kernel which is in the 1M lines or more order of magnitude.
* All the formal definitions I can read talk about the privilege level any given process or function runs at.
* Ultimately, the design of my own OS is up to me, but I want to develop a microkernel.

So, I think I am going to build my OS to include the PMM compiled into the kernel binary but will execute the PMM as a user-space process.  This may make initialization a little more difficult, but I am not writing code for the sake of easy initialization.

Now, with that said, I need to allocate space for the heap.  I need to consider the initial heap size and keep it relatively small.  I should be able to expand the heap as the need arises.  This means I should be able to allocate some heap frames in the loader and then map before handing control over to the kernel.

I have changed the starting heap size to 64K from 1M.  This means I only need to find 16 frames to cover the starting heap size.

---

After making an effort to pull this together, I end up with a page fault with the PMM.  This means I will have some work to do to get to the heap initialized (big heavy sigh...).  This is not what I wanted to have to to at this point.

So, what does this mean?  It means I need to get a PMM implementation built into the kernel.  Now, with the discussions above, I want this to be able to be run from user space.  So I will need to build an interface to "thunk" the CPU into the user space and to execute the PMM code when called directly and then "thunk" the CPU back into kernel mode.

This change is going to be pervasive and will disrupt nearly everything.  One of the things I am considering is whether I don't want to not initialize the PMM in the loader at all but keep track of what I can hand out and then walk the resulting Paging Tables and allocate the frames that have been mapped.  If I do this, what would I need to walk?  Well, the butler process running from the kernel and the PMM for two.  From there, I am not certain I would need to deal with much else before I was able to put the PMM in charge as a process servicing requests when I got to Phase IV (User Processes).

Well, do I need to wait that long?  If I delay setting up the PMM until **after** all the processes are established and then walk the resulting Paging Tables, I might actually be able to set up the process tables and IPC ahead of the PMM initialization.  With this approach, I know that the Butler and PMM process structures can be statically allocated at compile time.  This looks like it might come together.

I will think on this tonight....

---

### 2018-Oct-14

Thinking about setting up processes ahead of the PMM initialization, I think this is going to be the way to go.  In fact, I should be able to run processes and enable process swapping ahead of this as well.  The only challenge will be the process stacks for the Butler and PMM processes -- both will have to be carefully allocated.  Century32 has a process.h implementation that I will pull into this kernel, but it will require some modification for the new kernel.

As I am pulling this structure over, I am left wondering if I will have enough virtual memory space for all the pointers based on the current memory definitions.  I might have to go back and revisit this.

I have created the `ProcessNewPID()` function to allocate a new PID (not the process structure) for a new process.  This will typically only be used from CreateProcess function.

After creating this function, I realize that I need to implement some form of locking for when the system is running fully.  In a multi-process environment, there is a risk where the same PID might be handed out to 2 separate processes.  For this, I will need to implement a locking structure, which is typically a Spinlock.  For the a Spinlock to operate properly, I need some atomic operations.

In a single threaded environment such as while initialization is not yet complete, this will be trivial to get the lock.  So, there are no challenges to add the locks at the point.

A Spinlock is a specialized implementation of an Atomic Integer, and more specifically an Atomic Boolean.  The key here is that the value needs to be read and updated at the same time so that the current value can be compared to the expected value of the new assignment.

With this, I am going to take a top-down approach and start with the Spinlock.

But, why a Spinlock and not a Mutex?  well, in short, I am going to need to simple quick locks to handle the low level structure updates.  These simple locks will also be necessary to handle the larger structures such as Mutexes.

---

I was able to implement a spinlock structure and the related functions.  This was relatively simple to do as most of the work is done in inline functions.

So, I have functions to `ProcessNewPID()` and `ProcessFreePID()`.  I also need processes to do the following:
1. `ProcessCreate()` -- Create a new `Process_t` structure and allocate a new PID for it.  Decorate the stack for processing.
1. `ProcessReady()` -- Ready a process and put it at the end of the respective priority queue.
1. `ProcessHold()` -- Hold a process and put it on the held list.
1. `ProcessRelease()` -- If a process is held (only), release it and put it back in the correct queue.
1. `ProcessWait()` -- Place a process on the waiting queue.
1. `ProcessTerminate()` -- Start to force kill a process which is not the same as one terminating normally.  Put the process on the reaper queue.
1. `ProcessEnd()` -- End a process normally.  This will typically be done by the process itself.  Put the process on the reaper queue.
1. `ProcessReschedule()` -- If the running process just got blocked for some reason, reschedule to a new process.
1. `ProcessSwitch()` -- Switch to a new process, already determined.

As I start to write `ProcessHold()`, it dawns on me that I probably should start creating error codes I can use in reporting if nothing else, but also to include as return values for functions.  If I am comprehensive enough in this, I should be able to pinpoint errors quickly.

I did manage to get the `ProcessHold()` function written today, with all the supporting errors.  I also documented the errors in the `kernel/errors/process` folder.  It occurred to me as I wrote this that there is more error checking that I have started to take on in this iteration of the OS than I have taken on in previous versions.

---

### 2018-Oct-22

So, I had some power problems with the winds we had last week.  Living in Southern California, we get Santa Ana winds.  They were particularly bad last week which managed to disconnect the service line at the pole.   Well, we only had power to half the house and with those issues I did not want to power up my virtual machines or SAN.  Anyway, I am picking back up where I had left up a week ago and it's good I keep this journal.

I started today be reformatting how the error messages are written.  I moved these into macros in `errors.h`.  I reserve the right to revisit that decision.

Based on the time have left for the day and the size of each other function to write, I am going to take on `ProcessRelease()` as the next function to write.

---

### 2018-Oct-24

I have several functions left to work on for my process scheduler.  I will start to fill in the ones I need to get the kernel to compile first.

I have most of the Process functions worked out at this point.  I do still have a few extra processes to write.

---

### 2018-Oct-26

Today, I finished up all the work on processes.  This version still page faults -- but I committed the code anyway.

---

Now, I will start on the IPC communication -- specifically passing messages.  Do I need to support Mutexes and Semaphores in the kernel?  I do not think so at this point.  I think messaging will be enough and with the Spinlocks, I think I have enough locking mechanisms built in.

So, with that said, I only think I need to support `MessageSend()` and `MessageReceive()` functions.  Do I need a timeout?  No, not at the kernel level.

---

### 2018-Oct-27

Century-32 had a messaging implementation.  It's a rather simple design and I already have the fields necessary in the `Process_t` structure.  However, what bothers me is that the message is copied into a temporary structure and then copied back to a target structure.  It's a lot of copying.  I might not be able to get around that, but I want to try to think that through today.

https://wiki.osdev.org/IPC_Data_Copying_methods contains some algorithms for messaging.  It has valuable information about what I am considering today.  It occurs to me that I am able to get around it but I really have no interest in blocking a sending process.  On top of that, I will plan that most of my kernel messages will not have extended data elements to allocate additional memory for.  I should be able to allocate a slab of memory during initialization and then allocate message structures to be used by processes.  I run the risk of running out of messages (at which point I can allocate from heap if I want to go that far), but I think I can ignore that risk for now.  One thing I want to be able to do is allocate and pass a message from a lower-half interrupt to a higher-half driver process (where generally small messages work well) quickly and only when an extended block is needed allocate it on top of the message.

I also think I will need a function to check if there is a message waiting.  This function would be `MessageWaiting()`.  This function will be particularly useful in the Butler process.

I am going to allocate from the Heap internally for now.  This may be something I work on changing in the future.

The messages have been written.  This was relatively simple to implement.  I also reorganized the Heap* functions into their own folder to match the other files.

So, what is left before I can get the PMM written?  Well, I still need a `ProcessCreate()` function to add the PMM driver into the mix and then a customer method to create the Butler process (which is the kernel initialization at this point in time).  I also will need some form of `ProcessInit()` to get the process structures set up properly.

I'm feeling a bit undecided on where to focus my attention next.  When I get to this point, I find it helpful to list out all my current wants.  So, here goes:
* I need to complete the Heap initialization
* To do this, I need to clean up the Page Fault during preparing the initial 'hole'
* To do this, I need to have the PMM up and running in the kernel
* I want the PMM to be running as a separate process in user mode
* To get the PMM up and running as a separate process, I need `ProcessCreate()` written
* To get any additional processes running, I need to complete the preemptive scheduler
* To get the scheduler completed, I need to initialize the PIC/PIT or APIC based on what is available
* To initialize the PIC or APIC, I will need to determine which is on the system in hardware discovery (I will need to support both in the kernel code, or swap the driver the supported timer later in kernel initialization
* Finally, I will need to get the linker to reformat the resulting executable into separate segments for the kernel and PMM (I think) for security reasons

So, with this list, I think I will take on the PIC/PIT and get it configured and initialized.  If I am able to determine an APIC is present, I will swap it out before initialization is complete.  This will allow me to ensure beyond a shadow of a doubt that the PIC/PIT code is working even if all my own systems have an APIC.

For this to work properly, I want to be able to define a structure of functions that can be called to perform operations.  I know I am using a C++ compiler and I know that this can happen easily enough in the C++ language.  I may come back at a later time and clean this up, but for now I am going to stick with the C-compatible constructs.

So the PIC initialization was simple to copy from Century32.  I placed that in the `IdtBuild.cc` file.

---

### 2018-Oct-28

Century32 had a status bar that was at the bottom of the screen.  It helped with things like which TTY device you were logged in to; it showed the heart-beat of the kernel, and would occasionally show how much quantum was remaining on your slice.  All-in-all, it was pretty cool.  But there was also a lot of time that was caught up in writing this data, so I had to break that down into updating less frequently.  With the graphical screen I am using now, this will now take far longer to process.  But eventually, I think I want to have this status bar available to me since it provides a place I can put information about what is happening internally.

I bring this up now, because I am reviewing the code from Century32 for IRQ0 -- the timer interrupt.  There was more code in that handler to manage the status bar than there was for managing processes.

Having completed all the PIT timer functions, I now have a working interrupt timer that will trigger IRQ0.  This version will produce a dot to the debugging serial port each time the timer fires.  While I have the controls in place for a process switch, this is still disabled because I have not yet completed all of the process initialization.

So, from my list above, where does this code stand?
* Heap init -- not done
* Page Fault during Heap init -- not done
* PMM process -- not done
* PMM user mode -- not done
* Write `ProcessCreate()` -- not done
* Preemptive scheduler -- written not tested
* PIC/PIT init -- **done**
* APIC determination -- not going to do yet
* PMM in own segments -- not considered yet

However, with that said, this is a huge milestone for many hobby OS developers -- setting up the timer and getting interrupts to fire.  I will commit this version of the code.

---

I managed to get `CreateProcess()` written, but I realize I am unable to use that for the PMM process.  The reason is that it is totally dependent on `HeapAlloc()`, which itself is dependent on the PMM -- a circular dependency.  To get around this, I will have to ensure that the PMM has its own `Process_t` structure in the kernel code and be able to allocate its own stack without using `HeapAlloc()`.

So, at this point, I will have shift in focus to start developing out the PMM in the kernel binary.  To reiterate, I want the PMM to have the following characteristics:
1. It will be part of the kernel binary
1. It will run as its own process
1. It will run in user mode
1. It will have separate segments than the kernel (by definition)

As a side note, I am going to redo the formatting of this JOURNAL.md file.  I will not be changing the content.

---

I was able to get the `kernel.ld` set and ready to put the PMM into its own sections.  So, now I need to start coding the actual PMM.  But not tonight.

---

### 2018-Oct-30

The key challenge is going to be communicating the existing PMM setup to the new PMM manager.  They are on separate segments and separate VM spaces.  This will be the biggest issue related to completing the initialization.  However, it is not good practice to design a system for easy initialization (which is why I typically will write the initialization functions last).  This is no different.

So, as I recall, the initial PMM configuration is only 1 page (1 frame) worth of bitmap (from 3-Jun above):

> So, I think I will start with configuring the physical memory manager.  I think I will need to pick a single frame and use it for the first 128MB and then hand the responsibility for the remaining initialization over to the kernel -- or initialize the whole thing dynamically.  I think the best thing is going to be to to setup the first 128MB into a single frame so that we can have a fixed initialization step.
>
> To do this, I will need to establish the functions to read the memory map from from the boot loader and load them into a frame for the kernel as well.
>
> So a couple of things I need to make available:
> * The hardware communication data area (assume at 0x200000 or 2MB), and will take 1 frame
> * The PMM bitmap for 128MB (assume at 0x200100 or 2MB + 4KB), and will take 1 frame
> * The Paging tables starting at 0x200200 (or 2MB + 8KB), and will take many frames depending on architecture

So, I reviewed what I had written in the Century32 code.  What a mess!!  I clearly did not know what I was doing with that as I had all the terminology wrong at the very least.  I certainly had a half-hearted PMM in that version.  So, this will be a ground-up rewrite in this iteration.  I will start with an undefined location for the PMM bitmap since I believe that will have to move once the PMM is fully in charge.

That should be enough to get me started....

It occurs to me that a mere 128K of memory will hold all the memory I can possible have installed on a 32-bit system.  I can statically allocate that....

---

As I am coding the PMM, I realize I will likely need a sender pid in the `Message_t` structure, so that the receiver knows to which process to reply when needed.  I could use one of the `Message_t.parm`s for this but why take do that for something that will probably be ubiquitous.  I have this TODO captured in http://eryjus.ddns.net:3000/issues/369.

---

So, as I am writing `PmmMain()`, I realize that I am using kernel functions like they are part of the user process.  I really need to think this through...  I am likely going to have to implement the system calls (or SYSCALL) functionality to get this to work.  In particular, `MessageReceive()` and `MessageSend()` will be only available as a kernel function.  And, if this is the case, then I probably could simply separate the PMM into its own module.

So, at this point, I call it a day.

---

### 2018-Oct-31

While I have a little time, I figured I would discuss what my concerns are (at this point) with including the PMM in the kernel binary (well, among other things).  Here are several:
1. The compiler is going to allow things that it should not.
1. I am able to statically allocate the 128K for managing 4GB of memory in a separate module -- in particular I can direct its placement in asm.
1. I should be able to significantly reduce the amount of code in libk.
1. I will need to set up for system calls anyway, so why not handle that at this point?
1. I will eventually need to start a newlib port for a "C" compiler, which I expect to drive the elimination of libk.

For the moment, I can duplicate code in the PMM when needed.  I do not mind maintaining multiple versions of the same function when the situation warrants.  I can also handle this through includes to the original code if needed.  This is particularly easy because of the single-function nature of my code.

This will also handle the situation where the build was getting really messy with all the required includes.  I should be able to significantly simplify this as well.

The challenge will be that this will take some work to accomplish.

WOW!!!  On a whim I tried `man msgctl`.  I was shocked to see all the documentation related to the system call.  NICE!!!  For more than 2 decades playing with Linux and Unix proper, I never knew the system call documentation was available.

Now, I do want a relatively easy port of newlib when I get to that point.  I only want to have to provide my own system call library.  That said, I have been avoiding building my own version of `errno.h` -- and certainly having that match Linux.  However, I think it's time to give that up.  I am going to eliminate `errors.h` and replace it with `errno.h` build using Linux as a template.  This will be my first task and will probably end up in a commit.

---

### 2018-Nov-01

Today I started by creating `errno.h` from lifting the error codes from GCC.  I will model my own errors from these codes to make the kernel compatible and porting other applications easier.  It has always been my plan to start by porting certain certain utilities to the OS rather the rewrite them all myself.  On the other hand, I did plan on writing my own GUI.

I am now working on a system call implementation.  This will be my first one, so I have some reading to do.  I opted to hold off on eliminating `errors.h` until I can get the system calls working.  I need a functioning kernel to be able to test compiles while I do this.

I found the following site: https://www.freebsd.org/doc/en_US.ISO8859-1/books/developers-handbook/x86-system-calls.html.  This site shows that it is possible by pushing the parameters on the stack and then issuing an interrupt to perform a system call.  I think I can leverage this with my system calls.

So, what I am thinking (which is not vetted) is something like this:
* Push the parameters for the system call on the stack.
* Perform the system call (INT 0x80 or maybe 0x64 for CenturyOS).
* From the kernel not changing the stack, create a pointer to the list of parameters and push that on the stack.
* Then call the proper function to service the System Call, passing the pointer to the parameters as the only required parameter.

I think there are going to be problems with this, but I should be able to use this as a starting point.

Scratch that....  the approach I need to take is to handle the system call from the kernel only before I start considering the user applications.  This will un-complicate this quickly.

Since x86-32bit does not replace the stack on an interrupt, I want to pass the parameters in the general purpose registers.  I have 7 to play with: `eax` - `edx`, `esi`, `edi`, `ebp`.  One will need to hold the function to call (`eax`), so there will be up to 6 parameters.  There may be a need to hold out another register for additional manipulation.  I will assume that to be `ebp`.

---

### 2018-Nov-02

OK, I was able to get the framework of system calls established yesterday before I called it a day.  It is generally a duplication of the Interrupt Handler to create a jump table for the value in `eax` on entry.  Since the 32-bit architecture does not replace the stack for me, this will work rather cleanly.  64-bit might be a bit messier when I get there.

So, now there is this whole issue about POSIX, Linux, standards....  I have always had it in mind to port certain utilities to CenturyOS, write my own to replace certain things at a later time, and even write some things from the ground up.  I do not need my system to be perfectly aligned with the POSIX standard for example, but it would be nice to be able to take that as a starting point.  Now, on the other hand, there is just so much fussiness in that standard that I am not interested in that it might make sense to skip it altogether.

Why am I bringing this up now?  Well, I will need to implement a messaging system call in very short order.  The POSIX interface about messaging is obnoxious.  As I understand it, most systems will simply pass the parameters to the system with little to no translation.  So, a C function that is set to use a POSIX message structure *could* be translated into a `Message_t` structure by my run-time library before making the system call.  The trade-off there is that need to do quite a bit of porting to get newlib working for CenturyOS.

I think for now the decision needs to be that I will focus on my micro-kernel first and then when the time comes make upgrades to comply with the POSIX standard.

---

I think I am going to take a break from this and work on cleaning up my build system a bit.  I have lots of references from the current folder to walk back up the directory structure to find a file in another part of the tree.  I want this to be more absolute, and there is a `tuprules.tup` file I can take advantage of.  However, this will be unique to my installation and will need to be updated with other installations.  For this, I will use `make` to output the file.

---

Now, back to the system calls....

In figuring this out, I had to take care of `errors.h` at the same time.  This touched a crap-ton of files.  But I still execute at the same level as I did.  I have one more system call to implement.

---

I have all the binaries compiling again.  I am also able to run the kernel and get the timer to fire.  However, I am not yet creating a process for the PMM.  It is loaded as a module (not yet confirmed, but included it in the `grub.cfg` file.  I feel like I have made a hell of a mess making this all compile and work again.  Certainly, nothing is tested other than the ability to compile and run the elements that have all worked so far without the new stuff.  I have also been victim of a squirrel moment and need to get back on track.  I stated down this road on 2018-Oct-26 when I was not able to get the heap to initialize due to PMM issues and resulting Page Faults.

So, long story short, I need to start pulling this all back together.  I will start with a dump of the loaded modules so that I can confirm that the pmm.elf is loaded.   I did find that it is loaded:

```
Module information present
kernel
pmm
```

With this, I now need to be able to parse an ELF file properly to load it by the kernel.  This will require some work with the ELF specification.  I have some of this built into the loader, but will likely need to move that into a purpose-built library.  I will start by looking into what is required to read the ELF file.

Well, it turns out that I am trying to map my PMM using the same paging tables as the kernel.  This is not going to work.  I have 2 choices here:
1. I can not build the Paging Tables for any other module except for the kernel.  However, if I go down this path, I do not have a working PMM in the kernel yet to allocate a new `cr3`.
2. I can load build the new Paging Tables from the loader since I have a PMM-light implementation to work with.  However, in this case, I will need to add in the kernel pages when I get to the kernel before launching the process.

Option 2 seems the better solution to me so that I can avoid a chicken-and-egg problem.

It turned out to be a relatively simple change to make.

---

### 2018-Nov-03

This morning I realize that I am mapping all my VMM pages to be writable.  I caught this as I was looking for a place to indicate a common page for the kernel that will be mapped into the user processes.  This is obviously a security concern.  I have changed `MmuMapToFrame()` to accept a parameter to indicate writable pages and now I have some cleanup to fix every place this is called.

Now, I also need to be able to identify the kernel pages and map them into user processes as well.  This is going to be dynamic as the the kernel heap can expand.  What I don't want to have to do is walk every single process's paging tables every time I allocate a new frame for the kernel heap.  That would end up being a total mess.  I think I am going to ignore that particular issue (the heap and new frames) for now as it will be a while before that is a problem and focus on just marking the kernel pages so that I can identify what needs to be copied later.

I did cannibalize one of the available PageTableEntry bits to identify the kernel maps:

```C
typedef struct pageEntry_t {
    unsigned int p : 1;                 // Is the page present?
    unsigned int rw : 1;                // set to 1 to allow writes
    unsigned int us : 1;                // 0=Supervisor; 1=user
    unsigned int pwt : 1;               // Page Write Through
    unsigned int pcd : 1;               // Page-level cache disable
    unsigned int a : 1;                 // accessed
    unsigned int d : 1;                 // dirty (needs to be written for a swap)
    unsigned int pat : 1;               // set to 0 for tables, page Page Attribute Table (set to 0)
    unsigned int g : 1;                 // Global (set to 0)
    unsigned int k : 1;                 // Is this a kernel page?
    unsigned int avl : 2;               // Available for software use
    unsigned int frame : 20;            // This is the 4K aligned page frame address (or table address)
} __attribute__((packed)) pageEntry_t;
```

This new bit field is `pageEntry_t.k`.  A couple of key points with this decision:
1. When I get to swapping pages to disk, the kernel pages are not going to be eligible for swapping for all kinds of reasons -- but mostly because I do not want to risk swapping the page the is responsible for loading the pages from disk.  Undesirable results will occur....
1. These pages are effectively shared across multiple process and multiple Page Directories and if I swap one, I need to update all those -- oh, I don't even want to think about it.
1. Finally, when I do happen to swap a page to disk, I will be able to set the `pageEntry_t.p` bit to 0.  Nothing else on that structure will need to be maintained (`pageEntry_p.k` is guaranteed to be 0), so I will be able to use bits 1:31 to hold the location of that page in the swap file system.  Bit 0 must remain 0 in this case.

So, at this point, I need to switch back to the kernel and take care of the new `ProcessCreate()` requirements -- get the starting address from the module list and also properly augment the Paging structures.  But before I go there, I think I need ot revisit the VMM memory maps.  I last worked on this on 2018-Jun-30.

I am going to look at this specifically from the PMM process perspective, but without all the ad-nauseam detail.

| PDE Entries | Start Address | End Address | Size | Kernel | Mapped? | Usage |
|:-----------:|:-------------:|:-----------:|:----:|:------:|:-------:|:------|
|    0-511    | 0000 0000     | 7fff ffff   |  2G  |   N    |    N    | User Address Space & More |
|   512-767   | 8000 0000     | bfff ffff   |  1G  |   N    |    Y    | Device Driver Address Space (Code, Data, Stack, etc.) |
|  768-1003   | c000 0000     | faff ffff   | 944M |   Y    |    Y    | Kernel Code, Data, and Heap |
| 1004-1019   | fb00 0000     | feff ffff   |  64M |   Y    |    Y    | Video Frame Buffer |
|    1020     | ff00 0000     | ff3f ffff   |  4M  |   N    |    N    | Unused at this time - forces page faults |
|    1021     | ff40 0000     | ff7f ffff   |  4M  |   N    |    N    | Miscellaneous Small Elements |
|    1022     | ff80 0000     | ffbf ffff   |  4M  |   N    |    N    | Kernel Stacks -- 256×16KB |
|    1023     | ffc0 0000     | ffff ffff   |  4M  |   N    |    Y    | Recursive Mapping |

Well, what does this mean?  Well, it means that I did not really need to mark a page as a kernel page quite yet.  I only really need to look through the Page Directory in entries 768 to 1019 and copy these entries to the new Page Directory for a new process.  I do not even need to drive any deeper than the Page Directory, just copy the Page Directory Entries.

For this, I will need to map the new process cr3 to a temporary location and build it out, and then un-map it.

---

So, as I start writing the `PmmStart()` function to create the PMM process from the loaded module, I notice that I am getting some VMM errors from the `ModuleInit()` changes I have done in the loader.  I will have to debug those since the messages do not make sense to me:

```
      Attempting to map page 0xc0028000 to frame 0x00000131
      Attempting to map page 0xc0029000 to frame 0x00000132
      Attempting to map page 0xc002a000 to frame 0x00000133
MMU: Address 0xfffff000 is already mapped to frame 0x00000001
      Attempting to map page 0xc002b000 to frame 0x00000142
MMU: Address 0xfffff000 is already mapped to frame 0x00000001
```

In particular, the address `0xfffff000` is an odd reported error address since it is not the address I am requesting to be mapped.  As it turns out, I had the recursive mapping for the new process Page Directory in the wrong place.  Moving is solved my problems:

```
      Attempting to map page 0xc0028000 to frame 0x00000131
      Attempting to map page 0xc0029000 to frame 0x00000132
      Attempting to map page 0xc002a000 to frame 0x00000133
      Attempting to map page 0xc002b000 to frame 0x00000142
```

As a side note at this point, I know I have started adding a lot of context into the JOURNAL.md file.  This will increase the length of it very quickly, so I may have to split these and possibly move them into their own folder at some point.  However, I really like the resulting story it tells.  I will not go back to update the older entries but will continue to adopt this going forward.  Keep in mind I will not go back and update any code snips I put here if they change -- this is intended to tell a story.  Call it an early New Year's resolution.

---

I finally got to the point where I am able to compile all the modules and when `ld` when to link the binary, it was not able to find several functions.

```
/home/adam/workspace/century-os/obj/kernel/i686/MmuGetFrameForAddr.o: In function `MmuGetFrameForAddr(unsigned long, unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/mmu/MmuGetFrameForAddr.cc:32: undefined reference to `MmuMapToFrame(unsigned long, unsigned long, unsigned long, bool, bool)'
/home/adam/workspace/century-os/modules/kernel/src/mmu/MmuGetFrameForAddr.cc:42: undefined reference to `MmuMapToFrame(unsigned long, unsigned long, unsigned long, bool, bool)'
/home/adam/workspace/century-os/obj/kernel/i686/MmuUnmapPage.o: In function `MmuUnmapPage(unsigned long, unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/mmu/MmuUnmapPage.cc:14: undefined reference to `MmuGetTableEntry(pageEntry_t*, unsigned long, int, bool)'
/home/adam/workspace/century-os/modules/kernel/src/mmu/MmuUnmapPage.cc:19: undefined reference to `MmuGetTableEntry(pageEntry_t*, unsigned long, int, bool)'
/home/adam/workspace/century-os/obj/kernel/i686/PmmStart.o: In function `PmmStart(Module_t*)':
/home/adam/workspace/century-os/modules/kernel/src/pmm/PmmStart.cc:73: undefined reference to `MmuMapToFrame(unsigned long, unsigned long, unsigned long, bool, bool)'
/home/adam/workspace/century-os/modules/kernel/src/pmm/PmmStart.cc:85: undefined reference to `MmuMapToFrame(unsigned long, unsigned long, unsigned long, bool, bool)'
```

These are implemented in the `loader` and cannot be moved to the kernel.  They will have to be re-implemented.  However, there is an additional concern: When I re-implement these functions, I will need to have a working PMM again.  So, the solution is going to be to allocate more space in the pmm binary and to purpose build the MMU functions I need to get the PMM running.  All of the one-time initialization was part of what I wanted to avoid in the kernel by moving to the loader, but I do not believe that is going to every come to fruition at this point -- not with my little hobby kernel.

So, I will take on a couple of tasks at this point:
1. I will allocate another 8K page aligned memory for the PD and first PT for the PMM process.  This should cover it since everything is 4M aligned.
2. I will have to write some purpose-built functions specifically for setting up the MMU for the PMM, pointing directly at the 2 frames allocated by the binary.

Once again, I have a feeling that this is getting away from me and I will have to circle back around with a clean-up task once I get everything written and working.  It's time for another list of things to do:
* Allocate space for a Page Directory and Page Table in the PMM binary -- actually this only needs to be a Page Table since I already have a Page Directory allocated by the loader.
* I will need to map the Page Table to the proper location on the Page Directory.
* From there, I will need to implement the minimal `Mmu*()` functions needed to get the PMM implemented -- the key here is that the tables are specific to the PMM and not generalized.  This should help.  I hope.

See, a list helps.  I am feeling better about this already, even if I may not be able to get this all done today.  I will certainly have a lot of testing with such wholesale changes to the code.

---

I was able to get to the point where I can run a test and I got a page fault:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)

Page Fault
EAX: 0xffc010b4  EBX: 0xc002acc4  ECX: 0xc000492f
EDX: 0x000001cc  ESI: 0x00106321  EDI: 0x00000000
EBP: 0x001fff64  ESP: 0x001fff28  SS: 0x40
EIP: 0xc0002f5e  EFLAGS: 0x00200097
CS: 0x38  DS: 0x40  ES: 0x40  FS: 0x40  GS: 0x40
CR0: 0x80000011  CR2: 0xffc010b4  CR3: 0x00001000
Trap: 0xe  Error: 0x0
```

The thing that stood out to me more than anything else is that I am still using the loader segments.  I changed them to the kernel segments and the system triple faults.  So, I have a bit to work out there.

It turns out that I was referencing a segment that was defined as NULL.  Correcting that resolved my segment concern:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)

Page Fault
EAX: 0xffc010b4  EBX: 0xc002ad18  ECX: 0xc000492f
EDX: 0x000001cc  ESI: 0x00106321  EDI: 0x00000000
EBP: 0x001fff64  ESP: 0x001fff28  SS: 0x10
EIP: 0xc0002f5e  EFLAGS: 0x00200097
CS: 0x8  DS: 0x10  ES: 0x10  FS: 0x10  GS: 0x10
CR0: 0x80000011  CR2: 0xffc010b4  CR3: 0x00001000
Trap: 0xe  Error: 0x0
```

Now for the page fault....  which I was able to track down to the `KernelMap()` function.  I'm betting there something silly with the math.  Whatever it is, it will be the same problem with `KernelUnmap()` function as well.  Copy and paste has its disadvantages....

So, lets go through the calculations to be sure.
* The Page Directory is located at 0xfffff000.
* The Page Tables are located at 0xffc00000 through 0xffffd000.
* The table I am looking at is at index 1021 or 0x3fd.
* Multiply that by 0x1000 to adjust for the table size and I end up with 0x3fd000.
* Add 0xffc00000 and 0x003fd000 and the resulting address of the Page Table is 0xffffd000 (and this is where I went wrong!)
* From there the offset I am looking for is (addr >> 12) & 0x3ff.

With these changes I get past where I started and then triple fault.  Commenting out the `TimerInit()` call makes no difference, so the problem is still in `PmmStart()`.

Well, check that...

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
PmmStart(): Setting up the tables to be managed
PmmStart(): clearing the mmu tables
PmmStart(): Copying the kernel Page Tables
PmmStart(): Mapping the pmm itself
PmmStart(): Build the stack
PmmStart(): All done
```

I am able to get to the end of he `PmmStart()` function, so is this an interrupt problem?  Indeed, if I do not enable interrupts, it does not triple fault.   Now, if I do not call `PmmStart()`, it does not triple fault either.  So, what I think is happening is that I am clobbering the IDT somehow.  The IDT is located at virtual address 0xff401000.

---

### 2018-Nov-04

Well, I thought the IDT might be clobbered because I was using the same VM space.  This is not the problem; I put the IDT at location 0xff401000:

```
    // -- Map the GDT/IDT to the final location
    SerialPutS("\nMap GDT/IDT\n");
    MmuMapToFrame(cr3, 0xff401000, PmmLinearToFrame(0x00000000), true, false);
```

... and the other elements in a higher memory range:

```
//
// -- Some specific memory locations
//    ------------------------------
#define PROCESS_PAGE_DIR    0xff430000
#define PROCESS_PAGE_TABLE  0xff431000

// -- these are dedicated to the function `MmuGetFrameForAddr()`, but documented here.
#define MMU_FRAME_ADDR_PD   0xff432000
#define MMU_FRAME_ADDR_PT   0xff433000

#define PROCESS_STACK_BUILD 0xff441000
```

So, this means I am not clobbering it by reusing the virtual memory address.  Now, is it possible that the page is already mapped to frame 0 and I am not remapping it?  Yes.  Is it possible that I am actually performing a map to frame 0?  Highly unlikely but something to check.

Well, imagine that!!  I am mapping over frame 0!

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
PmmStart(): Setting up the tables to be managed
PmmStart(): Mapping addr 0xff430000 to frame 0x00000000
PmmStart(): Mapping addr 0xff431000 to frame 0x0000013a
PmmStart(): clearing the mmu tables
PmmStart(): Copying the kernel Page Tables
PmmStart(): Mapping the pmm itself
PmmStart(): Mapping addr 0xff441000 to frame 0x00139000
PmmStart(): Build the stack
PmmStart(): All done
```

And, based on the address I am mapping, this is the page table that is supposed to be in the binary itself.  Throwing in some more debugging output to see what I am getting for a starting address, and I get the following (and found a bug in `kprintf()` while I as at it):

```
Jumping to the kernel
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
PmmStart(): installing module for %s, located at 0x000031eb
PmmStart(): Setting up the tables to be managed
PmmStart(): Mapping addr 0xff430000 to frame 0x00000000
PmmStart(): Mapping addr 0xff431000 to frame 0x0000013a
PmmStart(): clearing the mmu tables
PmmStart(): Copying the kernel Page Tables
PmmStart(): Mapping the pmm itself
PmmStart(): Mapping addr 0xff441000 to frame 0x00139000
PmmStart(): Build the stack
PmmStart(): All done
```

I was expecting to see an address well above 1MB for the location, but what I have appears to be a frame number.  I know I am adjusting that as if it was an address.  What is also odd is that I am adding to that and should still have a frame number other than 0 by the time I get to mapping the address to the frame.  Tracing back to the loader and reading the GRUB information, this is a physical address.  I am not manipulating that at all.

As a side note, I am also seeing the stack frame in my debug logs.  This looks like a physical address and not a frame number (in fact, if it shift that by 12 [`<< 12`] to convert it from a frame to an address, it will overflow 32 bits.

I think I need to get `kprintf()` working properly so I can figure out what the hell went wrong with the module information.

OK, having cleaned up `kprintf()`, it was not outputting anything for a `%s` format specifier.  This means I was skipping the parameter and picking that up with the `%p` format specifier.  With that resolved, I am getting better output now:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
PmmStart(): installing module for pmm, located at 0x00138000
PmmStart(): Setting up the tables to be managed
PmmStart(): Mapping addr 0xff430000 to frame 0x00000000
PmmStart(): Mapping addr 0xff431000 to frame 0x0000013a
PmmStart(): clearing the mmu tables
PmmStart(): Copying the kernel Page Tables
PmmStart(): Mapping the pmm itself
PmmStart(): Mapping addr 0xff441000 to frame 0x00139000
PmmStart(): Build the stack
PmmStart(): All done
```

So, the module location looks good.  This is a good physical address.  The calculation for the Page Table is bad somehow.  And the frame for the stack is really a physical address from GRUB as well.  The offset between the start and the stack is `0x1000` which is good as well -- what I want it to be.

Here is the buggy code, and I see the problem with `tos` right away.
```
    frame_t pageDirFrame = pmmMod->cr3 >> 12;
    frame_t pageTblFrame = (pmmMod->modStart + 4096 + 4096) >> 12;  // This is the ELF header and stack
    ptrsize_t tos = (pmmMod->modStart + 4096);
```

Some cleanup and some more debugging code and my calculations are correct, but somehow the value is getting clobbered:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
PmmStart(): installing module for pmm, located at 0x00138000
PmmStart(): calculated pageTblFrame to be 0x0000013a; tos to be 0x00000139
PmmStart(): Setting up the tables to be managed
PmmStart(): Mapping addr 0xff430000 to frame 0x00000000
PmmStart(): Mapping addr 0xff431000 to frame 0x0000013a
PmmStart(): clearing the mmu tables
PmmStart(): Copying the kernel Page Tables
PmmStart(): Mapping the pmm itself
PmmStart(): Mapping addr 0xff441000 to frame 0x00000139
PmmStart(): Build the stack
PmmStart(): All done
```

Wait a minute!!!  My coffee just kicked in!  I'm looking at the wrong thing!  I'm not sure ***what*** I was thinking!

OK, this is the Page Directory I am looking at.  I need to go back to the loader to figure this out -- it is in the module initialization code.

Hmmm....  Did I forget to store the module `cr3` value?

```
        } else {
            modCr3 = PmmFrameToLinear(PmmNewFrame());

            // -- clear the Page Directory
            kMemSetB((void *)modCr3, 0, 4096);

            // -- create the recursive mapping
            MmuMapToFrame(modCr3, 0xfffff000, PmmLinearToFrame(modCr3), true, false);
        }
```

Also, since I am clearing this table in `PmmStart()`, there is no need to recursively map the Page Directory.  After correcting this issue, I am now able to get interrupts to fire again properly.  Now all I have left to do is ready the process and deal with all the crashes from that....

I'm completely shocked.  It didn't crash.  Either, I am not swapping tasks, or I wrote some really great task swapping code that had absolutely no bugs.  I'm betting on the former.  However, there are a crap-ton of changes and I want to commit this code.  The PMM initialization is not complete, but it is not crashing.

---

OK, with that out of the way, I need to make sure that the PMM is actually getting some CPU time.  I have a global flag about the scheduler being enabled.  This is not being set I'm quite sure at the moment.  So, the scheduler will never see the light of CPU.

```
    kprintf(".");
	if (!ProcessEnabled) {
        timerControl->eoi(0);
        return;
    }
```

So I need to set `ProcessEnabled`.  I theoretically have everything in place to do this.  And the rest of the day to debug.  And the result is a GPF:

```
PmmStart(): All done
.
General Protection Fault
EAX: 0x00000020  EBX: 0xc002ad4c  ECX: 0x00000020
EDX: 0xc002b069  ESI: 0xc002b0a0  EDI: 0xc002b020
EBP: 0x001ffed4  ESP: 0x001ffe78  SS: 0x10
EIP: 0xc0006dd4  EFLAGS: 0x00200046
CS: 0x8  DS: 0x10  ES: 0x10  FS: 0x10  GS: 0x10
CR0: 0x80000011  CR2: 0x00000000  CR3: 0x00001000
Trap: 0xd  Error: 0x20
```

I need to look up error code `0x20`, but this is certainly coming from the kernel.  Reading the Intel Manuals, I need to know the faulting instruction.  `eip` is `0xc0006dd4`.  So I look at the map for the binary.  The instruction is:

```
c0006dd4:	8e d0                	mov    %eax,%ss
```

So, since I am loading a segment register, the error code it the value segment descriptor that is invalid.  `0x20` is the user data (and the `ss` register).  Was this supposed to be `0x23`?  Like I said, this is the first time I have written anything to run in ring 3.

So, I know that the problem is occurring in `ProcessSwitch()` where I am loading `ss`:

```
;;
;; -- This completes saving the current process and below we start to restore the target process context
;;    --------------------------------------------------------------------------------------------------
		mov     eax,[edi+PROC_SS]           ;; get the ss register
		mov     ss,ax                       ;; and restore it
		mov     esp,[edi+PROC_ESP]          ;; restore the stack pointer
```

What is interesting is that I am loading the register with `0x23` and the value still comes up as `0x20`.  Perhaps that is the CPU making that change since it is a data segment.  But the problem is likely going to be in the GDT and in the way I have the segments configured.  One thing I might try as a test is to keep the pmm running using the kernel segment descriptors.  Let me see what I find in the GDT first.

OK, reading the Intel Manuals, I do not set the privilege level in the segments when I move them...  these are set by the CPU.  This is one problem I have to fix.  I know there are more.

OK, I did 2 tests with very different results.  In test 1, I changed the PMM to run in the kernel segments (`0x08` and `0x10`).  This resulted in a triple fault.  I then changed the GDT so that the user segments (`0x18` and `0x20`) are running in ring 0 and the kernel locked at the first interrupt (which might be OK -- I'm not quite sure where it locked and perhaps it's just that interrupts never got enabled again).

The following few sentences from the Intel Manuals have my attention while I am researching this:

> ... it performs a privilege check by comparing the privilege levels of the currently running program or task (the CPL), the RPL of the segment selector, and the DPL of the segment's segment descriptor.  The process loads the segment selector into the segment register if the DPL is numerically greater than or equal to both the CPL and the RPL.  Otherwise, a general-protection fault is generated and the segment register is not loaded.

So, let's see here.  I am trying to load the `ss` register with value `0x20` when it has `0x10` in it:
* The CPL is 0 (from `0x10`).
* The RPL is 0 (from `0x20`).
* The DPL is 3 (from the `0x20` descriptor).
* Is DPL > CPL? Yes.
* Is DPL > RPL? Yes.

Hmmm... maybe some other rule that is violated?

I do not have a TSS yet:

```
struct SegmentDescriptor gdt[16] = {
    SEG_NULL,                                   // 0x00<<3: NULL descriptor
    SEG32(GDT_CODE, 0, 0xffffffff, 0),          // 0x01<<3: Kernel Code Selector
    SEG32(GDT_DATA, 0, 0xffffffff, 0),          // 0x02<<3: Kernel Stack (Data) Selector
    SEG32(GDT_CODE, 0, 0xffffffff, 3),          // 0x03<<3: User Code Selector
    SEG32(GDT_DATA, 0, 0xffffffff, 3),          // 0x04<<3: User Stack (Data) Selector
    SEG_NULL,                                   // 0x05<<3: Reserved for user data if desired
    SEG_NULL,                                   // 0x06<<3: Reserved for kernel data if desired
    SEG32(GDT_CODE, 0, 0xffffffff, 0),          // 0x07<<3: Loader Code Selector
    SEG32(GDT_DATA, 0, 0xffffffff, 0),          // 0x08<<3: Loader Data & Stack Selector
    SEG_NULL,                                   // 0x09<<3: TSS Part 1
    SEG_NULL,                                   // 0x0a<<3: TSS Part 2
    SEG_NULL,                                   // 0x0b<<3: Future use call gate
    SEG_NULL,                                   // 0x0c<<3: Future use call gate
    SEG_NULL,                                   // 0x0d<<3: Future use call gate
    SEG_NULL,                                   // 0x0e<<3: Future use call gate
    SEG_NULL,                                   // 0x0f<<3: Future use call gate
};
```

Hmmm....  but there is a parenthetical statement in the section about Stack Switching that reads:

> (The TSS does not specify a stack for privilege level 3 because the processor does not allow a transfer of control from a procedure running at a CPL of 0, 1, or 2 to a procedure running at a CPL of 3, except on a return.)

Now, could it be that I need to set up for an `iret` instead of just moving the `ss` directly?  I took a look at how Klange does this with Tauros (https://github.com/klange/toaruos) and he is performing this with an `iret` instruction as well.  The `iret` instructions has the following line:

> If the return is to another privilege level, the IRET instruction also pops the stack pointer and the SS from the stack before resuming program execution.

This, then implies that I will need to determine if I am changing privilege levels in order to determine the proper method to replace the `cs` and `ss` segment selectors.

I went and looked at the `mov` instruction's operation section (since that is what is generating the `#GP`) in the Intel Manuals, and I have the reason for the `#GP`:

```
IF SS is loaded
    THEN
        IF segment selector is NULL
            THEN #GP(0); FI;
        IF segment selector index is outside descriptor table limits
        or segment selector's RPL <> CPL
        or segment is nto a writable data segment
        or DPL <> CPL
            THEN #GP(selector); FI;
        ...
```

RPL and CPL are the same, but `DPL <> CPL` which is the reason for the `#GP`.  So, I now have the concrete root cause.  On to determine a fix.

I modified the `ProcessSwitch()` function to perform an `iret` and on my first test I received another `#GP`:

```
PmmStart(): All done
.
General Protection Fault
EAX: 0xc0006df8  EBX: 0xc002ad4c  ECX: 0x00000020
EDX: 0xc002b069  ESI: 0xc002b0a0  EDI: 0xc002b020
EBP: 0x001ffed4  ESP: 0x001ffe64  SS: 0x10
EIP: 0xc0006df0  EFLAGS: 0x00200016
CS: 0x8  DS: 0x10  ES: 0x10  FS: 0x10  GS: 0x10
CR0: 0x80000011  CR2: 0x00000000  CR3: 0x00001000
Trap: 0xd  Error: 0x18
```

This one is faulting on the code segment `0x18` and it is happening on the `iret` instruction.  I then noticed that I needed to update my RPL for the `iret` instruction and corrected that in the `PmmStart()` function.  When I ran it again, `qemu` picked up the kernel crash rather than issuing an `int 10`.

```
PmmStart(): All done
.qemu: fatal: invalid tss type
EAX=c0006df8 EBX=c002ad4c ECX=00000020 EDX=c002b069
ESI=c002b0a0 EDI=c002b020 EBP=001ffed4 ESP=7ffff05c
EIP=c0006df8 EFL=00200046 [---Z-P-] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0000 00000000 00000000 00000000
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]
SS =0023 00000000 ffffffff 00cff200 DPL=3 DS   [-W-]
DS =0000 00000000 00000000 00000000
FS =0000 00000000 00000000 00000000
GS =0000 00000000 00000000 00000000
LDT=0000 00000000 0000ffff 00008200 DPL=0 LDT
TR =0000 00000000 0000ffff 00008b00 DPL=0 TSS32-busy
GDT=     00000000 0000007f
IDT=     00000800 000007ff
CR0=80000011 CR2=7ffff05c CR3=00001000 CR4=00000000
DR0=00000000 DR1=00000000 DR2=00000000 DR3=00000000
DR6=ffff0ff0 DR7=00000400
CCS=00000044 CCD=0000001b CCO=EFLAGS
EFER=0000000000000000
FCW=037f FSW=0000 [ST=0] FTW=00 MXCSR=00001f80
FPR0=0000000000000000 0000 FPR1=0000000000000000 0000
FPR2=0000000000000000 0000 FPR3=0000000000000000 0000
FPR4=0000000000000000 0000 FPR5=0000000000000000 0000
FPR6=0000000000000000 0000 FPR7=0000000000000000 0000
XMM00=00000000000000000000000000000000 XMM01=00000000000000000000000000000000
XMM02=00000000000000000000000000000000 XMM03=00000000000000000000000000000000
XMM04=00000000000000000000000000000000 XMM05=00000000000000000000000000000000
XMM06=00000000000000000000000000000000 XMM07=00000000000000000000000000000000
```

But, I know that I now need to manufacture a TSS and load that in the GDT.  But at the same time I notice that I have my `cs` and `ss` registers loaded with proper values.  Since I have not had to implement a TSS with a 32-bit kernel yet, I will have to do some more digging in the Intel Manuals....

... and a read of https://wiki.osdev.org/Getting_to_Ring_3.

Needless to say, this is going to be messy to get all the kinks worked out.  There will be a lot to do.  So, take them one at a time, Adam.  First: the TSS since I cannot even address anything else and properly test it until I have that working.

---

After a lot of fussy bit checking I finally have a TSS working from the loader.  The problem is that the actual TSS is located in the loader and not visible/available from the kernel.  I will need to fix that eventually.

Now, I am back to a triple fault.  I'm sure it's for a different reason than any before.  I am able to get all the way to a timer IRQ (I can see the dot in the serial output) and then there is an immediate triple fault from there.  I will have to track my way into the code to determine where.  Here is the output at the moment:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
PmmStart(): installing module for pmm, located at 0x00138000
PmmStart(): calculated pageTblFrame to be 0x0000013a; tos to be 0x00000139
PmmStart(): Setting up the tables to be managed
PmmStart(): Mapping addr 0xff430000 to frame 0x00000169
PmmStart(): Mapping addr 0xff431000 to frame 0x0000013a
PmmStart(): clearing the mmu tables
PmmStart(): Copying the kernel Page Tables
PmmStart(): Mapping the pmm itself
PmmStart(): Mapping addr 0xff441000 to frame 0x00000139
PmmStart(): Build the stack
PmmStart(): All done
.
```

With some additional temporary debugging code, I am able to determine that I am getting into `ProcessSwitch()`:

```
PmmStart(): Build the stack
PmmStart(): All done
.Looking for a new process
OS: executing process switch
```

Actually, I believe the triple fault to be related to the TSS and the ss0 and esp0 fields.  I really need to pull the TSS into the kernel and set it up from there.

So, I checked and frame 0 has room for the TSS.  So, I hard coded it to be there, and I will have direct access to it from the kernel page at 0xff401000.  It probably makes some sense to lay out what is really in that frame.

| Start | Length | End  | Data |
|:-----:|:------:|:----:|:-----|
|   0   |  128   |  127 | GDT |
| 128   |  384   |  511 | Unused (available for GDT expansion) |
| 512   |  512   | 1023 | TSS |
| 1024  |  1024  | 2047 | Unused (available for TSS expansion) |
| 2048  |  2018  | 4095 | IDT |

With this information I can build out a reasonable structure in the kernel for managing this memory, which will include the TSS.

---

So, I got this all coded and when I went to run again, I got another triple fault.  I added an `hlt` instruction right before `iret` and the system halted like I would expect:

```asy
		;; -- push the new eip register
		mov		eax,ChangeCPL				;; get the address of the return point
		push	eax							;; and push that on the stack
		hlt
		iret								;; effectively load the ss/esp and cs/eip registers
```

However, when I moved the same `hlt` to be after `ChangeCPL`, I got another triple fault:

```asy
ChangeCPL:
		hlt
		pop		eax					        ;; pop the gs register
		mov		gs,ax				        ;; and restore it
```

This tells me very clearly that I still have some sort of problem with the `iret` and what is required to make that work.  WAIT!!!  I never loaded the task register!

But with that, I am back to a #GP fault:

```
General Protection Fault
EAX: 0x0000004b  EBX: 0xc002b9b8  ECX: 0xc0004abf
EDX: 0xc002d200  ESI: 0x00106321  EDI: 0x00000000
EBP: 0x001fffdc  ESP: 0x001fff88  SS: 0x10
EIP: 0xc00029e4  EFLAGS: 0x00200016
CS: 0x8  DS: 0x10  ES: 0x10  FS: 0x10  GS: 0x10
CR0: 0x80000011  CR2: 0x00000000  CR3: 0x00001000
Trap: 0xd  Error: 0x48
```

My current map confirms that this is on the `ltr` instruction:

```
c00029e4:	0f 00 d8             	ltr    %ax
```

---

Reading the `ltr` instruction operation, there are 3 reasons for getting a `#GP` fault with a non-zero error code:
1. The offset is greater than the limit of the GDT
2. The type <> global (not really sure what this means)
2. The segment descriptor is not an available TSS

I think I better get my head around what a global type is....

Well, with some additional debugging code, I get the following:

```
Jumping to the kernel
The TSS contains: 0x02000000:0x0000eb00
```

I see 2 problems with this:
1. The limit is 0
1. The type is contains the busy (`B`) bit set

Since this is printed ahead of executing `ltr`, both can be the cause of the `#GP`.  But that will be a task for tomorrow.

---

### 2018-Nov-05

So, today I am going to start by setting the limit to 0xffffffff and see what the result provide.  This yielded no change in behavior, but then again did I really expect it to with 2 problems.

Let me go back to the above result and pull apart the bits:

```
0x0200: the base address or 512
0x0000: the segment limit or 0
then, there is the colon
0x00: high bast bits
0x0: G-0-0-Avl bits
0x0: limit high (still 0)
0xe: p-dpl-0 or present and dpl = 3
0xb: 1-0-B-1 or the busy bit is set (which is part of the problem I think)
0x00: base middle bits
```

OK, so the reason for the busy bit is because I am loading the Task Register in the loader and loading segment `0x4b`.  And then I am performing a second load from the kernel initialization.  Since it is already loaded, the busy flat is set.  I'm going to comment out the `ltr` instruction in the loader and check the results now.  This puts me back to a triple fault.

I feel like I'm caught in a loop.

Except when I put the `hlt` instruction back before the `iret` in `ProcessSwitch()` the system still triple faults:

```asy
		;; -- push the new eip register
		mov		eax,ChangeCPL				;; get the address of the return point
		push	eax							;; and push that on the stack
		hlt
		iret
```

So, my problem is not there.  Let's see....

```
PmmStart(): All done
.Looking for a new process
OS: executing process switch
```

The problem is somewhere between the last output and the `hlt`.  So, I'm going to try at the start of `ProcessSwitch()` to see what happens.

```asy
ProcessSwitch:
		hlt
		push	ebp					        ;; save the caller's frame pointer
		mov		ebp,esp				        ;; setup up a frame
        pushfd                              ;; save the current flags register
		cli
```

The system halted as I expected.  OK, I'm spinning my wheels needlessly.  Time for a different approach.  I am going to launch Bochs to see what the output is there.  I had to do some cleanup of the `.bochsrc` file and update the `Makefile` to reset the iso image to get Bochs to work properly.

This at least (and at last) gives me a root cause:

```
00235585314e[CPU0  ] interrupt(): SS.dpl != CS.dpl
00235585314e[CPU0  ] interrupt(): SS.dpl != CS.dpl
00235585314i[CPU0  ] CPU is in protected mode (active)
00235585314i[CPU0  ] CS.mode = 32 bit
00235585314i[CPU0  ] SS.mode = 32 bit
00235585314i[CPU0  ] EFER   = 0x00000000
00235585314i[CPU0  ] | EAX=c0006ed8  EBX=c002c228  ECX=00000020  EDX=000003f8
00235585314i[CPU0  ] | ESP=7ffff05c  EBP=001ffe40  ESI=c002d0a0  EDI=c002d020
00235585314i[CPU0  ] | IOPL=0 ID vip vif ac vm RF nt of df if tf sf ZF af PF cf
00235585314i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00235585314i[CPU0  ] |  CS:001b( 0003| 0|  3) 00000000 ffffffff 1 1
00235585314i[CPU0  ] |  DS:0000( 0002| 0|  0) 00000000 ffffffff 1 1
00235585314i[CPU0  ] |  SS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235585314i[CPU0  ] |  ES:0000( 0002| 0|  0) 00000000 ffffffff 1 1
00235585314i[CPU0  ] |  FS:0000( 0002| 0|  0) 00000000 ffffffff 1 1
00235585314i[CPU0  ] |  GS:0000( 0002| 0|  0) 00000000 ffffffff 1 1
00235585314i[CPU0  ] | EIP=c0006ed8 (c0006ed8)
00235585314i[CPU0  ] | CR0=0xe0000011 CR2=0x7ffff05c
00235585314i[CPU0  ] | CR3=0x00001000 CR4=0x00000000
00235585314i[CPU0  ] 0x00000000c0006ed8>> pop eax : 58
00235585314e[CPU0  ] exception(): 3rd (10) exception with no resolution, shutdown status is 00h, resetting
00235585314i[SYS   ] bx_pc_system_c::Reset(HARDWARE) called
00235585314i[CPU0  ] cpu hardware reset
```

So it looks like I am getting a `#GP` fault followed by a `#DF` followed by a system reset or triple boot.  Interrupts are disabled, so this cannot be any IRQ.  But, the privilege levels of the `ss` and `cs` are the same....  I think.  The indexes are certainly still right and the dpl is set properly for the indexes.

Thinking, I still have multiple issues here:
1. The interrupt is not getting proper control of the system.  In this case, I am having a problem with the TSS structure.
1. The `iret` instruction is causing a `#GP` fault.  In this case, need to review the Intel manuals some more.

So, for the interrupts, it looks like my problem might be here:

```C
	cpuStructs->tss.ss0 = 0x20;
	cpuStructs->tss.esp0 = ((ptrsize_t)tssStack + TSS_STACK_SIZE);
```

I need to change `0x20` to be `0x10` for the kernel data selector.

This execution led to a hung system:

```
00235906635i[CPU0  ] WARNING: HLT instruction with IF=0!
09762124007i[      ] cpu loop quit, shutting down simulator
09762124007i[CPU0  ] CPU is in protected mode (halted)
09762124007i[CPU0  ] CS.mode = 32 bit
09762124007i[CPU0  ] SS.mode = 32 bit
09762124007i[CPU0  ] EFER   = 0x00000000
09762124007i[CPU0  ] | EAX=00000017  EBX=c002c228  ECX=00000000  EDX=000003f8
09762124007i[CPU0  ] | ESP=c002e12c  EBP=c002e148  ESI=00000000  EDI=00000000
09762124007i[CPU0  ] | IOPL=0 ID vip vif ac vm rf nt of df if tf SF zf af PF cf
09762124007i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
09762124007i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
09762124007i[CPU0  ] |  DS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
09762124007i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
09762124007i[CPU0  ] |  ES:0010( 0002| 0|  0) 00000000 ffffffff 1 1
09762124007i[CPU0  ] |  FS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
09762124007i[CPU0  ] |  GS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
09762124007i[CPU0  ] | EIP=c00053a5 (c00053a5)
09762124007i[CPU0  ] | CR0=0xe0000011 CR2=0x7ffff05c
09762124007i[CPU0  ] | CR3=0x00001000 CR4=0x00000000
09762124007i[CPU0  ] 0x00000000c00053a5>> jmp .-4 (0xc00053a3) : EBFC
```

The map confirms that I am in the `Halt()` function:

```
c00053a3 <Halt>:
c00053a3:	fa                   	cli
c00053a4:	f4                   	hlt
c00053a5:	eb fc                	jmp    c00053a3 <Halt>
```

Back to `qemu` to see some output.  This output is:

```
Page Fault
EAX: 0xc0006ed8  EBX: 0xc002c228  ECX: 0x00000020
EDX: 0x000003f8  ESI: 0xc002d0a0  EDI: 0xc002d020
EBP: 0x001ffed4  ESP: 0xc002e1d0  SS: 0x10
EIP: 0xc0006ed8  EFLAGS: 0x00200046
CS: 0x1b  DS: 0x0  ES: 0x0  FS: 0x0  GS: 0x0
CR0: 0x80000011  CR2: 0x7ffff05c  CR3: 0x00001000
Trap: 0xe  Error: 0x4
```

`cr2` has an interesting address for the faulting address.  I'm not sure where that is coming from, if it is not from the stack I am trying to pop from.  I did get an error code, so I will need to read up on that.

Actually, I'm going to bet the problem is with this line:

```C
	pmmProcess.esp = (PROCESS_STACK_BUILD - (regval_t)msp) + 0x80000000;
```

I have that subtraction backwards.  Making that correction, I now am getting this `#PF`:

```
Page Fault
EAX: 0xc0006ed8  EBX: 0xc002c228  ECX: 0x00000020
EDX: 0x000003f8  ESI: 0xc002d0a0  EDI: 0xc002d020
EBP: 0x001ffed4  ESP: 0xc002e1d0  SS: 0x10
EIP: 0xc0006ed8  EFLAGS: 0x00200046
CS: 0x1b  DS: 0x0  ES: 0x0  FS: 0x0  GS: 0x0
CR0: 0x80000011  CR2: 0x80000fa4  CR3: 0x00001000
Trap: 0xe  Error: 0x4
```

This deserves some conversation since I am not quite sure how to handle this yet.  First some context:

```asy
;;
;; -- Check how we need to actually perform the task change
;;    -----------------------------------------------------
		cmp 	eax,[edi+PROC_ESP]			;; are we moving to the same stack selector?
		je		NewProcess					;; if the same we can do a simple load

		;; -- Push the new ss register
		xor		eax,eax						;; clear eax
		mov		ax,[edi+PROC_SS]			;; get the stack segment
		push	eax

		;; -- Push the new esp register
		mov		eax,[edi+PROC_ESP]			;; get the target esp
		push	eax							;; push it on the stack

		;; -- Push the current flags register
		pushfd								;; push the current flags (will be replaced later)

		;; -- Push the new cs register
		xor		eax,eax						;; clear eax
		mov		ax,[edi+PROC_SS]			;; get the stack segment
		sub		eax,8						;; calcualte the code segment -- this only works because of GDT layout
		push	eax							;; and push it on the stack

		;; -- push the new eip register
		mov		eax,ChangeCPL				;; get the address of the return point
		push	eax							;; and push that on the stack
		iret								;; effectively load the ss/esp and cs/eip registers
```

The problem here is that I am using the kernel `cr3` and these Page Tables are not mapped to the same space as the target ring-3 process.  However, I am pushing the stack pointer from the ring-3 process onto the stack.  The stacks are not in the same place in virtual address space.  When I get to the `iret` instruction, it replaces the stack to an address that is not mapped in the kernel `cr3` Page Tables.  Therefore, when I get to the next instruction (which is a `pop` by the way), I am generating a `#PF` because the stack is not mapped in the kernel Page Tables.

Actually the solution is probably pretty easy.  This function is part of the kernel.  As a result it will be available in every `cr3` Page Table structure.  So, the solution is relatively easy: move `cr3` before changing the stack.  This will mean that I need to add the `cr3` register into the `Process_t` structure and use the structure value rather than the stack value.

With these changes, I am now left with a `#GP` fault:

```
General Protection Fault
EAX: 0x0016c000  EBX: 0xc002c300  ECX: 0x00000020
EDX: 0x000003f8  ESI: 0xc002d0a0  EDI: 0xc002d020
EBP: 0x001ffed4  ESP: 0xc002e1d0  SS: 0x10
EIP: 0xc0006ee1  EFLAGS: 0x00200046
CS: 0x1b  DS: 0x0  ES: 0x0  FS: 0x0  GS: 0x0
CR0: 0x80000011  CR2: 0x00000000  CR3: 0x00001000
Trap: 0xd  Error: 0x0
```

The map yields:

```
c0006ee1:	0f 22 d8             	mov    %eax,%cr3
```

So, here I am in ring-3 trying to wite to a protected register.  This is a bit of a problem.  Since this function can be called from within a ring-3 process and may end up rescheduling to a ring-3 process, I need to come up with a way to get the kernel into ring-0 in order to replace the `cr3` value.  I can't imagine I can do a simple `iret` from ring-3 to ring-0 because that would break all the protections.  The only way I know how to get there is with an interrupt.

---

### 2018-Nov-06

I "snuck" away from work for a few minutes..  I want to move the `cr3` update to the portion of the code prior to the `iret` instruction, which means before the determination is made to change the privilege level.  This results in a triple fault.  `bochs` identifies the location at:

```
00235585504i[CPU0  ] 0x00000000c0006ec7>> push eax : 50
00235585504e[CPU0  ] exception(): 3rd (14) exception with no resolution, shutdown status is 00h, resetting
```

and the map indicates that this is `ProcessSwitch()`, which I expected, but the first stack operation after replacing `cr3`.  I suspect bad Paging Tables and a resulting `#PF`.

---

Thinking about this, I believe my kernel stack is not in the proper kernel space.  I am going to check this by forcing a `hlt` in `bochs` and getting a register dump.

```
00235585471i[CPU0  ] WARNING: HLT instruction with IF=0!
04033088001i[      ] cpu loop quit, shutting down simulator
04033088001i[CPU0  ] CPU is in protected mode (halted)
04033088001i[CPU0  ] CS.mode = 32 bit
04033088001i[CPU0  ] SS.mode = 32 bit
04033088001i[CPU0  ] EFER   = 0x00000000
04033088001i[CPU0  ] | EAX=00000010  EBX=c002c300  ECX=00000020  EDX=000003f8
04033088001i[CPU0  ] | ESP=001ffe0c  EBP=001ffe40  ESI=c002d0a0  EDI=c002d020
04033088001i[CPU0  ] | IOPL=0 ID vip vif ac vm rf nt of df if tf sf ZF af PF cf
04033088001i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
04033088001i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
04033088001i[CPU0  ] |  DS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
04033088001i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
04033088001i[CPU0  ] |  ES:0010( 0002| 0|  0) 00000000 ffffffff 1 1
04033088001i[CPU0  ] |  FS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
04033088001i[CPU0  ] |  GS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
04033088001i[CPU0  ] | EIP=c0006eb2 (c0006eb2)
04033088001i[CPU0  ] | CR0=0xe0000011 CR2=0x00000000
04033088001i[CPU0  ] | CR3=0x00001000 CR4=0x00000000
04033088001i[CPU0  ] 0x00000000c0006eb2>> mov eax, cr3 : 0F20D8
04033088001i[CMOS  ] Last time is 1541547362 (Tue Nov  6 15:36:02 2018)
04033088001i[XGUI  ] Exit
```

This confirms my thinking: the kernel stack is located outside the kernel memory address space.  This, I believe is set up in the loader at 0x200000.  I will need to track that down now and make sure it is in the kernel space properly.  For the moment, I am not encroaching on this, but it really needs to be cleaned up:

```
pmm
   Starting Address: 0x0013b000
   FileSize = 0x0000655e; MemSize = 0x00027000; FileOffset = 0x00001000
      Attempting to map page 0x80000000 to frame 0x0000013c
      [snip...]
      Attempting to map page 0x80026000 to frame 0x0000018d
```

And, I am correct, the loader is setting his up for now:

```
    mov         esp,0x200000                                ;; set up a stack at 2MB growing down
```

I really need a better place to put this, and I think it needs to be statically allocated in both the loader and in the kernel binaries.  Which brings me back to a triple fault.  Adding the `hlt` instruction back before `iret` again, gives me the following:

```
00235673846i[CPU0  ] WARNING: HLT instruction with IF=0!
01512640001i[      ] cpu loop quit, shutting down simulator
01512640001i[CPU0  ] CPU is in protected mode (halted)
01512640001i[CPU0  ] CS.mode = 32 bit
01512640001i[CPU0  ] SS.mode = 32 bit
01512640001i[CPU0  ] EFER   = 0x00000000
01512640001i[CPU0  ] | EAX=c0006ee5  EBX=c002c300  ECX=00000020  EDX=000003f8
01512640001i[CPU0  ] | ESP=c002de2c  EBP=c002de74  ESI=c002e0a0  EDI=c002e020
01512640001i[CPU0  ] | IOPL=0 ID vip vif ac vm rf nt of df if tf sf zf AF PF cf
01512640001i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
01512640001i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
01512640001i[CPU0  ] |  DS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
01512640001i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
01512640001i[CPU0  ] |  ES:0010( 0002| 0|  0) 00000000 ffffffff 1 1
01512640001i[CPU0  ] |  FS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
01512640001i[CPU0  ] |  GS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
01512640001i[CPU0  ] | EIP=c0006edd (c0006edd)
01512640001i[CPU0  ] | CR0=0xe0000011 CR2=0x00000000
01512640001i[CPU0  ] | CR3=0x0016e000 CR4=0x00000000
01512640001i[CPU0  ] 0x00000000c0006edd>> iret  : CF
01512640001i[CMOS  ] Last time is 1541548634 (Tue Nov  6 15:57:14 2018)
01512640001i[XGUI  ] Exit
```

So, where am I at now?  I have the new process-oriented `cr3` in place.  And from the code the way it stands now:

```asy
		mov		eax,cr3						;; get the cr3 value
		mov		[esi+PROC_CR3],eax			;; and store it in the process structure

;;
;; -- Check how we need to actually perform the task change
;;    -----------------------------------------------------
		mov		eax,[edi+PROC_CR3]			;; get the new cr3 value
		mov		cr3,eax						;; and set the new paging structure (flushes all TLB caches)

		cmp 	eax,[edi+PROC_ESP]			;; are we moving to the same stack selector?
		je		NewProcess					;; if the same we can do a simple load

		;; -- Push the new ss register
		xor		eax,eax						;; clear eax
		mov		ax,[edi+PROC_SS]			;; get the stack segment
		push	eax

		;; -- Push the new esp register
		mov		eax,[edi+PROC_ESP]			;; get the target esp
		push	eax							;; push it on the stack

		;; -- Push the current flags register
		pushfd								;; push the current flags (will be replaced later)

		;; -- Push the new cs register
		xor		eax,eax						;; clear eax
		mov		ax,[edi+PROC_SS]			;; get the stack segment
		sub		eax,8						;; calcualte the code segment -- this only works because of GDT layout
		push	eax							;; and push it on the stack

		;; -- push the new eip register
		mov		eax,ChangeCPL				;; get the address of the return point
		push	eax							;; and push that on the stack
hlt
		iret
```

... I am still pushing data on to the stack, so at least part of the kernel space is mapped properly.  Moving the `hlt` instruction to after the `iret` target confirms that the return is causing the triple fault.

---

Switching gears, I have tried to install the bochs debugger from the Fedora repo and it has problems.  I'm going to have to download the source and build it manually.

---

I got Bochs built and working.  I am working on debugging.  I need to focus first on getting the interrupts working again.  I need to be able to properly trap exceptions.

I read this post and I get the impression my problems are because the GDT and IDT are not mapped in the Paging Tables: https://forum.osdev.org/viewtopic.php?p=50927&sid=fbb01cb8e6a252d468c14cf6d59cc582#p50927.  If this is true, this I will need to move the GDT and IDT into the kernel memory.  It is mapped, I just need to reset them.

---

I did turn on reporting all the debugging messages and found what is going on:

```
00235680703d[CPU0  ] page walk for address 0x00000000c002de24
00235680713d[CPU0  ] page walk for address 0x0000000000000018
00235680713d[CPU0  ] PDE: entry not present
00235680713d[CPU0  ] page fault for address 0000000000000018 @ 00000000c0006edd
00235680713d[CPU0  ] exception(0x0e): error_code=0000
00235680713d[CPU0  ] interrupt(): vector = 0e, TYPE = 3, EXT = 1
```

This tells me that I have the stack built improperly for the `iret` instruction.  Certainly, address 0x18 is not mapped in the PMM `cr3`.

---

OK, I found one of my problems: the GDT needs to be mapped into kernel space so that the virtual address is always available.  But it still crashes:

```
<bochs:2> reg
CPU0:
rax: 00000000_00000000 rcx: 00000000_00000020
rdx: 00000000_000003f8 rbx: 00000000_c002c31c
rsp: 00000000_718c944a rbp: 00000000_c002de60
rsi: 00000000_c002e0a0 rdi: 00000000_c002e020
r8 : 00000000_00000000 r9 : 00000000_00000000
r10: 00000000_00000000 r11: 00000000_00000000
r12: 00000000_00000000 r13: 00000000_00000000
r14: 00000000_00000000 r15: 00000000_00000000
rip: 00000000_0000ffff
eflags 0x00200046: ID vip vif ac vm rf nt IOPL=0 of df if tf sf ZF af PF cf
<bochs:3> sreg
es:0x0000, dh=0x00001000, dl=0x00000000, valid=0
cs:0xffff, dh=0x00cf930f, dl=0xfff0ffff, valid=1
        Data segment, base=0x000ffff0, limit=0xffffffff, Read/Write, Accessed
ss:0x0010, dh=0x00cf9300, dl=0x0000ffff, valid=31
        Data segment, base=0x00000000, limit=0xffffffff, Read/Write, Accessed
ds:0x0000, dh=0x00001000, dl=0x00000000, valid=0
fs:0x0000, dh=0x00001000, dl=0x00000000, valid=0
gs:0x0000, dh=0x00001000, dl=0x00000000, valid=0
ldtr:0x0000, dh=0x00008200, dl=0x0000ffff, valid=1
tr:0x004b, dh=0x000feb00, dl=0x0200ffff, valid=1
gdtr:base=0x00000000ff401000, limit=0x7f
idtr:base=0x00000000ff401800, limit=0x7ff
```

The `cs` is very interesting.

---

I had several issues I had to resolve.  Most significantly wsa getting the stack aligned properly.  I was also trying to move to `cr0`, which is protected.  I also had a problem with getting the stack and user Page Directory calculations right.  At the moment, I an really in real mode and I am executing some code in the `pmm.elf` binary.  I know this by the value of eip:

```
00235680701i[CPU0  ] | EAX=00000000  EBX=00000000  ECX=00000000  EDX=00000000
00235680701i[CPU0  ] | ESP=80000ffc  EBP=00000000  ESI=00000000  EDI=00000000
00235680701i[CPU0  ] | IOPL=0 id vip vif ac vm RF nt of df if tf sf zf af pf cf
00235680701i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00235680701i[CPU0  ] |  CS:001b( 0003| 0|  3) 00000000 ffffffff 1 1
00235680701i[CPU0  ] |  DS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680701i[CPU0  ] |  SS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680701i[CPU0  ] |  ES:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680701i[CPU0  ] |  FS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680701i[CPU0  ] |  GS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680701i[CPU0  ] | EIP=80002115 (80002115)
00235680701i[CPU0  ] | CR0=0xe0000011 CR2=0x00000208
00235680701i[CPU0  ] | CR3=0x0016e000 CR4=0x00000000
(0).[235680701] [0x00000013f115] 001b:0000000080002115 (unk. ctxt): add byte ptr ds:[eax], al ; 0000
00235680701p[CPU0  ] >>PANIC<< exception(): 3rd (14) exception with no resolution
```

Now, with that said, it really is the first instruction that is creating the problem:

```
80002115 <PmmMain>:
80002115:	55                   	push   %ebp
```

BUT!!!  The map for the instruction does not match the disassembly for the same instruction.  This is being overwritten.  I also notice that interrupts are not enabled and really need to be at this point in execution.

---

### 2018-Nov-07

As I get into debugging today, I am left wondering if I have properly parsed the elf header or not.  I still appear to be overwriting the first code frame with 0's and I am not sure if this is an elf location problem, a misrepresentation of the module structure the loader passes to the kernel, or something else.  I can confirm the `eip` address is correct:

```
00235680702i[CPU0  ] | EAX=00000000  EBX=00000000  ECX=00000000  EDX=00000000
00235680702i[CPU0  ] | ESP=80001ffc  EBP=00000000  ESI=00000000  EDI=00000000
00235680702i[CPU0  ] | IOPL=0 id vip vif ac vm RF nt of df if tf sf zf af pf cf
00235680702i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00235680702i[CPU0  ] |  CS:001b( 0003| 0|  3) 00000000 ffffffff 1 1
00235680702i[CPU0  ] |  DS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680702i[CPU0  ] |  SS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680702i[CPU0  ] |  ES:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680702i[CPU0  ] |  FS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680702i[CPU0  ] |  GS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680702i[CPU0  ] | EIP=80002115 (80002115)
00235680702i[CPU0  ] | CR0=0xe0000011 CR2=0x00000208
00235680702i[CPU0  ] | CR3=0x0016e000 CR4=0x00000000
```

```
80002115 <PmmMain>:
80002115:	55                   	push   %ebp
```

However, the contents at that memory location are incorrect.

```
(0).[235680702] [0x00000013f115] 001b:0000000080002115 (unk. ctxt): add byte ptr ds:[eax], al ; 0000
```

Now, I am clearing these 2 blocks of memory, so what If I don't clear them?

```C
    // -- initialize the 2 tables to 0
    kprintf("PmmStart(): clearing the mmu tables\n");
    kMemSetB((void *)PROCESS_PAGE_DIR, 0, 4096);
    kMemSetB((void *)PROCESS_PAGE_TABLE, 0, 4096);
```

Well, there is no change in behavior.  But I forgot about this clear:

```C
    // -- clear the stack just for good measure
    kMemSetB((void *)PROCESS_STACK_BUILD, 0, 4096);
```

I will try to comment this out.  And again, no change in behavior.  Do I see a problem??

```
<bochs:3> page 0x80002115
 PDE: 0x000000000013f027    ps         A pcd pwt U W P
 PTE: 0x000000000013f027       g pat d A pcd pwt U W P
linear page 0x0000000080002000 maps to physical page 0x00000013f000
```

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
PmmStart(): installing module for pmm, located at 0x0013d000
PmmStart(): calculated pageTblFrame to be 0x0000013f; tos to be 0x0000013e
PmmStart(): Setting up the tables to be managed
PmmStart(): Mapping addr 0xff430000 to frame 0x0000016e
PmmStart(): Mapping addr 0xff431000 to frame 0x0000013f
PmmStart(): clearing the mmu tables
PmmStart(): Copying the kernel Page Tables
PmmStart(): Mapping the pmm itself
PmmStart(): Mapping addr 0xff441000 to frame 0x0000013e
```

I see that I am mapping the page table to be the same frame as the first frame of code.  OK, now I am on to something tangible.  But which is incorrect?  The first page of code, or the page table?

Some examinations:

* Frame/Page #1

```
<bochs:2> page 0x80000000
 PDE: 0x000000000013f027    ps         A pcd pwt U W P
 PTE: 0x000000000013d007       g pat d a pcd pwt U W P
linear page 0x0000000080000000 maps to physical page 0x00000013d000
...
<bochs:9> xp /100 0x13d000
[bochs]:
0x000000000013d000 <bogus+       0>:    0x464c457f      0x00010101      0x00000000      0x00000000
0x000000000013d010 <bogus+      16>:    0x00030002      0x00000001      0x80002115      0x00000034
0x000000000013d020 <bogus+      32>:    0x00007b48      0x00000000      0x00200034      0x00280001
```

Based on the signature, this is the elf header.

* Frame/Page #2

```
<bochs:3> page 0x80001000
 PDE: 0x000000000013f027    ps         A pcd pwt U W P
 PTE: 0x000000000013e027       g pat d A pcd pwt U W P
linear page 0x0000000080001000 maps to physical page 0x00000013e000
...
<bochs:8> xp /100 0x13e000
[bochs]:
0x000000000013e000 <bogus+       0>:    0x00000101      0x00000000      0x00000000      0x00000000
0x000000000013e010 <bogus+      16>:    0x00000000      0x00000000      0x00000000      0x00000000
0x000000000013e020 <bogus+      32>:    0x00000000      0x00000000      0x00000000      0x00000000
```

I am expecting that this is the stack.

* Frame/Page #3

```
<bochs:5> page 0x80002000
 PDE: 0x000000000013f027    ps         A pcd pwt U W P
 PTE: 0x000000000013f027       g pat d A pcd pwt U W P
linear page 0x0000000080002000 maps to physical page 0x00000013f000
...
<bochs:7> xp /100 0x13f000
[bochs]:
0x000000000013f000 <bogus+       0>:    0x0013d007      0x0013e027      0x0013f027      0x00140007
0x000000000013f010 <bogus+      16>:    0x00141007      0x00142007      0x00143007      0x00144007
0x000000000013f020 <bogus+      32>:    0x00000000      0x00000000      0x00000000      0x00000000
0x000000000013f030 <bogus+      48>:    0x00000000      0x00000000      0x00000000      0x00000000
0x000000000013f040 <bogus+      64>:    0x00000000      0x00000000      0x00000000      0x00000000
0x000000000013f050 <bogus+      80>:    0x00000000      0x00000000      0x00000000      0x00000000
```

This I expect to be the page table.

* Frame/Page #4 (for good measure)

```
<bochs:10> page 0x80003000
 PDE: 0x000000000013f027    ps         A pcd pwt U W P
 PTE: 0x0000000000140007       g pat d a pcd pwt U W P
linear page 0x0000000080003000 maps to physical page 0x000000140000
<bochs:11> xp /100 0x140000
[bochs]:
0x0000000000140000 <bogus+       0>:    0x56e58955      0x00ace853      0xc3810000      0x00004546
0x0000000000140010 <bogus+      16>:    0xff087d81      0x763fffff      0xfff4b807      0x53ebffff
0x0000000000140020 <bogus+      32>:    0xff0cec83      0x4fe80875      0x83000000      0xc08410c4
0x0000000000140030 <bogus+      48>:    0x01b80774      0xebffffff      0x08458b3a      0x8905e8c1
0x0000000000140040 <bogus+      64>:    0x08458bc2      0x8905e8c1      0x00c0c7c1      0x8b800070
0x0000000000140050 <bogus+      80>:    0x4d8b8804      0x1fe18308      0x000001be      0x89e6d300
0x0000000000140060 <bogus+      96>:    0x21d1f7f1      0x00c0c7c1      0x89800070      0x00b8900c
0x0000000000140070 <bogus+     112>:    0x8d000000      0x5e5bf865      0x8955c35d      0x2fe853e5
0x0000000000140080 <bogus+     128>:    0x05000000      0x000044cd      0xc108558b      0xc0c705ea
0x0000000000140090 <bogus+     144>:    0x80007000      0x8b90048b      0xe2830855      0x0001bb1f
0x00000000001400a0 <bogus+     160>:    0xd1890000      0xda89e3d3      0xc085d021      0x5bc0940f
0x00000000001400b0 <bogus+     176>:    0x048bc35d      0x1c8bc324      0x8955c324      0xe85356e5
0x00000000001400c0 <bogus+     192>:    0xffffffee      0x00448c05      0x087d8100      0x3fffffff
0x00000000001400d0 <bogus+     208>:    0xf4b80776      0xebffffff      0x08558b38      0xebc1d389
0x00000000001400e0 <bogus+     224>:    0x08558b05      0x8905eac1      0x00c2c7d1      0x8b800070
0x00000000001400f0 <bogus+     240>:    0x4d8b8a14      0x1fe18308      0x000001be      0x89e6d300
0x0000000000140100 <bogus+     256>:    0xc7ca09f1      0x007000c0      0x98148980      0x000000b8
0x0000000000140110 <bogus+     272>:    0x5d5e5b00      0xe58955c3      0x54ec8353      0xffff95e8
0x0000000000140120 <bogus+     288>:    0x2fc381ff      0xc7000044      0x0000f445      0x7d810000
0x0000000000140130 <bogus+     304>:    0x007ffff4      0xc7167700      0x007000c0      0xf4558b80
```

And this looks like code.  And the byte at offset `0x115` is `0x55`, which is correct (remember the dwords are in little Endian).  Going back to my mappings, I see the following:

```
Initializing Modules:
pmm
   Starting Address: 0x0013d000
   FileSize = 0x0000655e; MemSize = 0x00027000; FileOffset = 0x00001000
      Attempting to map page 0x80000000 to frame 0x0000013e
      Attempting to map page 0x80001000 to frame 0x0000013f
      Attempting to map page 0x80002000 to frame 0x00000140
      Attempting to map page 0x80003000 to frame 0x00000141
      Attempting to map page 0x80004000 to frame 0x00000142
```

So, `0x13d` is really the Elf Header, which matches reality.  I am building the Page Tables incorrectly and not offsetting this properly.

To solve this, I am pretty sure I need to capture an offset or header size in `ModuleInit()` as well.

Well, I was able to get farther this time.  I was able to actually get some processing done:

```
00235680980i[CPU0  ] | EAX=80007000  EBX=80006550  ECX=00000000  EDX=00000000
00235680980i[CPU0  ] | ESP=80000fa0  EBP=80000ff8  ESI=00000000  EDI=00000000
00235680980i[CPU0  ] | IOPL=0 id vip vif ac vm RF nt of df if tf SF zf AF pf CF
00235680980i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00235680980i[CPU0  ] |  CS:001b( 0003| 0|  3) 00000000 ffffffff 1 1
00235680980i[CPU0  ] |  DS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680980i[CPU0  ] |  SS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680980i[CPU0  ] |  ES:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680980i[CPU0  ] |  FS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680980i[CPU0  ] |  GS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00235680980i[CPU0  ] | EIP=80002140 (80002140)
00235680980i[CPU0  ] | CR0=0xe0000011 CR2=0x00000208
00235680980i[CPU0  ] | CR3=0x0016e000 CR4=0x00000000
```

I am at `eip` = `0x80002140`.

```
80002140:	c7 04 90 ff ff ff ff 	movl   $0xffffffff,(%eax,%edx,4)
```

This line happens to be where I am trying to initialize the PMM bitmap, which is located in the .bss section:

```
Disassembly of section .bss:

80007000 <pmmBitmap>:
	...
```

So, I have yet another problem with the loader and the module info passed to the kernel.  However, there are some concerns here....  The .bss section is being allocated in the loader, yes, but should it?  Here is the concern: the loader is allocating additional frames which are not contiguous with the loaded binary.  I have no good method of keeping track of these and passing them to the kernel.  The kernel is a special case since the loader is building the paging tables on the kernel's behalf.  Once the PMM is properly put in charge, any other module will be able to allocate additional frames from it, so the kernel will be able to handle this initialization.

The loader can build the Page Tables for each loaded module.  I am already storing the `cr3` location for each loaded module, but the `Mmu*()` functions are really built for the kernel.  They can, of course, be modified.

As an alternative, I can statically allocate the bitmap using assembly in the `.data` section.  I could even initialize is statically and forego the programatic initialization.

With this change, I got quite a bit farther.  I am still getting a triple fault.  The system is stil resetting.  But I am getting to a more significant location:

```
00237193846i[CPU0  ] | EAX=00000001  EBX=800060b4  ECX=00000000  EDX=00000020
00237193846i[CPU0  ] | ESP=80000f84  EBP=80000f88  ESI=00000000  EDI=80000fac
00237193846i[CPU0  ] | IOPL=0 id vip vif ac vm RF nt of df if tf SF zf AF pf cf
00237193846i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00237193846i[CPU0  ] |  CS:001b( 0003| 0|  3) 00000000 ffffffff 1 1
00237193846i[CPU0  ] |  DS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00237193846i[CPU0  ] |  SS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00237193846i[CPU0  ] |  ES:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00237193846i[CPU0  ] |  FS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00237193846i[CPU0  ] |  GS:0023( 0004| 0|  3) 00000000 ffffffff 1 1
00237193846i[CPU0  ] | EIP=800022ac (800022ac)
00237193846i[CPU0  ] | CR0=0xe0000011 CR2=0x00000208
00237193846i[CPU0  ] | CR3=0x0018f000 CR4=0x00000000
(0).[237193846] [0x0000001402ac] 001b:00000000800022ac (unk. ctxt): int 0x64                  ; cd64
```

This location happens to be where the SYSCALL `ReceiveMessage()` is called.  This in turn performs an `int 0x64` (`0x64` == `100` for century, get it).  This then generates a `#GP` fault, followed by a `#PF` followed by a `#DF` followed by a triple fault.  I am also getting an additional message which leads me to believe my IDT might be malformed for a ring-3 process:

```
00237193846d[CPU0  ] interrupt(): INTERRUPT TO INNER PRIVILEGE
```

So, the DPL on the IDT gate needs to be set to the highest (numerically) privilege level any given interrupt can be called from.  I had a bit of code commented out that added `| 0x60` to the flags to make sure that the interrupt could be called from ring-3.  I uncommented that code and I am noe getting farther yet again.  I am now getting a `#PF` and then a triple fault, so my interrupts are still not working right.  What I think is most fascinating is the address of the `#PF`: `0x208`.  This is an offset from the start of the physical memory and is associated with the GDT/IDT/TSS frame.  In fact, this is offset 520, and that is for the TSS.  So, the Task Register is full of an invalid address.  Are we back on the TSS again??

Getting the TSS relocated to the proper address in mapped memory, I now have a `#TS` exception (which I actually had to look up to figure out what it is!):

```
00236964580d[CPU0  ] exception(0x0a): error_code=0010
00236964580d[CPU0  ] interrupt(): vector = 0a, TYPE = 3, EXT = 1
00236964580d[CPU0  ] interrupt(): INTERRUPT TO INNER PRIVILEGE
00236964580e[CPU0  ] interrupt(): SS.rpl != CS.dpl
00236964580d[CPU0  ] exception(0x0a): error_code=0011
```

Actually, running that again, the cause is the following:

```
00236964580d[CPU0  ] interrupt(): vector = 64, TYPE = 4, EXT = 0
00236964580d[CPU0  ] interrupt(): INTERRUPT TO INNER PRIVILEGE
00236964580e[CPU0  ] interrupt(): SS.rpl != CS.dpl
```

And in particular the problem is `SS.rpl != CS.dpl`, which appears to be prevalent as well.

I changed the RPL values in the TSS itself and now I got a real life `#PF` that printed to the serial port.  This now means my interrupts are working again.  All I have to do now is work through the remaining bugs....

So, my faulting address is `0x00000000` again.  I'm pretty sure I have my short stack from the TSS.

```
Page Fault
EAX: 0xc002c33c  EBX: 0x00000000  ECX: 0x00000000
EDX: 0x00000000  ESI: 0x00000000  EDI: 0x80000fb0
EBP: 0xc002f0a8  ESP: 0xc002f060  SS: 0x10
EIP: 0xc0004b75  EFLAGS: 0x00000092
CS: 0x8  DS: 0x10  ES: 0x10  FS: 0x10  GS: 0x10
CR0: 0x80000011  CR2: 0x00000000  CR3: 0x0018f000
Trap: 0xe  Error: 0x2
```

The faulting instruction is:

```
c0004b75:	c7 03 00 00 00 00    	movl   $0x0,(%ebx)
```

`ebx` is `0x00000000`, so it cannot de-reference that pointer.

The procedure is `SpinlockClear()`:

```
c0004b70 <SpinlockClear>:
c0004b70:	53                   	push   %ebx
c0004b71:	8b 5c 24 0c          	mov    0xc(%esp),%ebx
c0004b75:	c7 03 00 00 00 00    	movl   $0x0,(%ebx)
```

But I have no call stack to work with.  Based on the value from `cr3`, this is from the PMM doing a SYSCALL (which I could easily deduce anyway).  Pretty much the only one it could be doing at the moment is `ReceiveMessage()`.

---

### 2018-Nov-08

Im going to start today by figuring out what is going wrong with the `null` pointer dereference.  So, I did find a logic problem with the held flag on the `Process_t` structure in `ProcessWiat()`.  I got that fixed and now it appears that I have a double-wait problem on the same SpinLock.  So, how about some debugging code.

---

OK, I have blocks that look like this:

```C
        if (!IsListEmpty(&procOsPtyList)) {
            Process_t *tgt = FIND_PARENT(procOsPtyList.next,Process_t,stsQueue);
            if (tgt == currentProc) return;
            ListMoveTail(&tgt->stsQueue, &procOsPtyList);
            SpinlockUnlock(&readyQueueLock);
            kprintf("OS: executing process switch\n");
            ProcessSwitch(currentProc, tgt);
        }
```

Where the lock can be obtained and when the next scheduled process it the current process, keep the lock by returning.  I got that figured out and now I am getting a triple fault where I am replacing the `cr3` with `0x00000000`.  From that, I need to add pid into the `Process_t` structure so that I can report against it.  I think I am going to need it for other reasons as well.

I believe it is my lists that need help.  I appear to be trying to pull from the idle process.

```
Lock at 0xc002f0e9 released
ProcessWait(): released process lock
Looking for a new process
Attempting lock by 0x0 at address 0xc0054668
Lock at 0xc0054668 released
IDLE: executing process switch: 0x0
```

I should not have anything on the idle queue at this point.  Unfortunately, most of this stuff is being executed for the very first time right now.  Reviewing the code, I'm having an "Oh Shit!" moment.  What have I done.  I need to get the compiler to work for me here.  The list entry and the list header are 2 different things but have the same type, which is creating me some trouble since I think I have them backwards in the actual function calls.  I need to leave this for tomorrow.

---

### 2018-Nov-09

OK, so after last night's revalation, I am going to take on rewriting my lists and the controlling structure.  Needless to say, this is going to break all kinds of stuff and will take a while to get a compiled executable again.  But, what was I have things coded now, I have everything all messed up and backwards in many cases.  I need to compiler to help me keep it all straight.  I thought I could be clever and lift most of this from the Linux implementation and it has not worked out for me.

So, I will start by defining a controlling list structure for the list head.  The list itself will continue to use the same structures.  Now, with that said, I will also be able to add the resulting list locks into the list head and reduce some complexity as well (I hope).

---

So, I got all that mess cleaned up.  I am back to getting `#PF`s again.  I did find that my current PID variable was not being updated in `ProcessReschedule()`, to I added it at the end:

```C
        kprintf("%s: Executing process switch: %x\n", q, tgt->pid);
        currentPID = tgt->pid;
        ProcessSwitch(currentProc, tgt);
```

---

At the moment, the system is locking up.  I am getting into a task check and then it appears to go to the end with no further interrupts.  I am not sure if I am issueing an EOI for each timer interrupt -- particularly if there is a check for a new task.  And, there it is:

```C
void TimerCallBack(UNUSED(isrRegs_t *reg))
{
    kprintf(".");
	if (!ProcessEnabled) {
        timerControl->eoi(0);
        return;
    }

	procs[currentPID]->totalQuantum ++;
	if (-- procs[currentPID]->quantumLeft <= 0) {
        timerControl->eoi(0);
        ProcessReschedule();
    }
}
```

Control can reach the end of this process without issuing an EOI.  An easy fix.

So, I am back to a `#PF`:

```
Timer EOI
.Timer EOI
.Timer EOI
.Timer EOI
.Timer EOI
.Timer EOI
.Timer EOI
Looking for a new process
Attempting lock by 0x0 at address 0xc0030960
Lock at 0xc0030960 released
OS: Executing process switch: 0x1
Attempting lock by 0x0 at address 0xc0030069

Page Fault
EAX: 0x00000000  EBX: 0xc002e3a4  ECX: 0x00000000
EDX: 0x00000001  ESI: 0x00000000  EDI: 0x80000fb0
EBP: 0xc00310d8  ESP: 0xc00310a8  SS: 0x10
EIP: 0xc0002dae  EFLAGS: 0x00000092
CS: 0x8  DS: 0x10  ES: 0x10  FS: 0x10  GS: 0x10
CR0: 0x80000011  CR2: 0x00000004  CR3: 0x00191000
Trap: 0xe  Error: 0x0
```

I am going to remove my "Timer EOI" debugging code to make thing a little cleaner.  The function throwing the error is `ListRemoveInit()`, which appears to be getting a `NULL` value.  A stack dump should help me figure out where it is coming from -- which I will snip and annotate with the functions on the call stack.

```
 | STACK 0xc00310c4 [0xc0002dae]    # ListRemoveInit()
 | STACK 0xc00310dc [0xc0002c83]    # MessageReveive()
 | STACK 0xc00310ec [0xc0002bfe]    # MessageReceive()
 | STACK 0xc003110c [0xc0005435]    # SyscallReceiveMessage
 ```

I am also able to confirm that this is coming from `currentPID == 1`.  Well, since I have never sent a message to this process, there is a definite problem with `MessageWaiting()`.  `MessageWaiting()` is rather trivial:

```C
inline bool MessageWaiting(void) { return !IsListEmpty(&procs[currentPID]->messages); }
```

So, this means to me that the `Process_t` structure is not being initialized properly in `PmmStart()`.  Sure enough:

```C
Process_t pmmProcess = {
    0,                      // esp
    0,                      // ss
    0,                      // cr3
    1,                      // pid
    0x80000000,             // stack location (fixed!)
    4096,                   // stack length
    "pmm",                  // process name
    0,                      // total quantum
    PROC_INIT,              // process status
    PTY_OS,                 // the priority
    0,                      // quantum left
    false,                  // not held
    {0},                    // the spinlock for this process
    {0},                    // the status queue
    {0},                    // the lock list
    {0},                    // pending messages
    NULL,                   // the previous payload
};
```

I am initializing the pending `messages` structure to 0 and never cleaning that up.  While I am at it, I will take care of `lockList` and save me some trouble later.

```
    ListInit(&pmmProcess.messages.list);
    ListInit(&pmmProcess.lockList.list);
```

I finally have a good result!!

```
PmmStart(): All done
.Looking for a new process
Attempting lock by 0x0 at address 0xc0030960
Lock at 0xc0030960 released
The current process is still the highest priority
Reached the end of initialization
..............................Looking for a new process
Attempting lock by 0x0 at address 0xc0030960
Lock at 0xc0030960 released
OS: Executing process switch: 0x1
ProcessWait(): Checking
Attempting lock by 0x0 at address 0xc0030069
ProcessWait(): Process lock obtained
Attempting lock by 0x0 at address 0xc0030960
Lock at 0xc0030960 released
ProcessWait(): removed from ready queue
Attempting lock by 0x0 at address 0xc00309b0
Lock at 0xc00309b0 released
ProcessWait(): added to wait list
Lock at 0xc0030069 released
ProcessWait(): released process lock
Looking for a new process
Attempting lock by 0x0 at address 0xc0030960
Lock at 0xc0030960 released
OS: Executing process switch: 0x0
.Looking for a new process
Attempting lock by 0x0 at address 0xc0030960
Lock at 0xc0030960 released
The current process is still the highest priority
..............................Looking for a new process
Attempting lock by 0x0 at address 0xc0030960
Lock at 0xc0030960 released
The current process is still the highest priority
..............................
```

So, this tells me that I am getting the pmm initialized and I am actually getting a switch to it.  The pmm process blocks (waiting for a message) and we switch back to the kernel initialization, where the scheduler stays since there is no other process ready to execute.

I want to commit this code.  A lot of debugging went into it and it is functioning.

---

OK, with that done, I realize now that my initialization function needs to turn into the Butler process, but I still need an Idle process.  I really want the Idle process to run as pid 0, and the butler to run at pid 1.  This means that both the Butler and the PMM need to move their positions in the `procs[]` table.

Of course this first test ends in a triple fault.  And Bochs has some interesting register values:

```
00236515315i[CPU0  ] EFER   = 0x00000000
00236515315i[CPU0  ] | EAX=c0055e54  EBX=c002e5c8  ECX=00000010  EDX=c0005a3f
00236515315i[CPU0  ] | ESP=c002ffc0  EBP=c002ffdc  ESI=00107349  EDI=00000000
00236515315i[CPU0  ] | IOPL=0 ID vip vif ac vm RF nt of df if tf SF zf af PF cf
00236515315i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00236515315i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
00236515315i[CPU0  ] |  DS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00236515315i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00236515315i[CPU0  ] |  ES:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00236515315i[CPU0  ] |  FS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00236515315i[CPU0  ] |  GS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00236515315i[CPU0  ] | EIP=c0000123 (c0000123)
00236515315i[CPU0  ] | CR0=0xe0000011 CR2=0xff401840
00236515315i[CPU0  ] | CR3=0xc0055e54 CR4=0x00000000
```

I had the operands backward!

```asy
GetCr3:
    mov     cr3,eax
    ret
```

Which correcting this problem resolved my triple fault.  Now to force the Idle process to get some CPU.  It appears to have worked.

---

So, the next step will be to pass off the PMM bitmap from my loader hardware discovery structure to the PMM as messages.  Since a message will allocate some space from the kernel heap, I need to get that working as well.  Some of this will have to be statically allcoated with the kernel itself -- or at least in the .bss section.  If I play my cards right, I should be able to remap this space in the Paging Tables to the proper location in `HeapInit()` and then let the loader be responsible for allocating several frames to get me started.  All I will need to do is keep track of the memory and size and then make sure I **NEVER** reference that memory.  Of course, this memory will need to be page-aligned.

So, I need to think through this line in the virtual memory map:

> | PDE Entries | Start Address | End Address | Size | Usage |
> |:-----------:|:-------------:|:-----------:|:----:|:------|
> |  768-1003   | c000 0000     | faff ffff   | 944M | Kernel Code, Data, and Heap |

I certainly have enough virtual memory for a rather large heap.  This is nice.  I think I will start the heap at `0xd0000000` and let it run to `0xfb000000`.  That is about 688M of possible kernel heap.  I hope that will cover it.  However, not all of it is going to be backed by physical frames whether it is to start or as time progresses.  So, lots of memory should be available for other processes.

Now, the other thing to call out here is that I am mapping the Page Tables from the kernel `PD[768]` to `PD[1021]`.  It is important to note that these Page Tables are included in this, so when a process needs to send a message (which allocates from the kernel Heap).  When I need to update a page in one of these tables, all processes will get this same update because I only map to the existing tables -- I am not copying them!

---

### 2018-Nov-10

This morning I got into resetting my heap initialization.  Since this was copied from an older version, I have some naming convention stuff to clean up.  This will force every file to be touched and updated just to get a good compile again.

At the same time, since I am mapping and unmapping virtual address pages, I will need to implement the MMU functions in the kernel (as opposed to the loader functions).  So, let's see what we need:

`MmuUnmapPage()` -- this function will unmap a page from memory given its virtual address (need not be page aligned).  It will return the frame number of the page that is being unmapped (since we might want to free it, or in the case of the heap remap that to a different page).
`MmuMapToFrame()` -- this function wlil map a frame to a page, creating the Page Table if needed.  It will assume that the frame number is a good one (will not check for allocation) and will not accept frame number 0 (this is where my GDT, IDT, and TSS are stored).

This should give me what I need to start coding again.  The problem I am also going to have is the shared `mmu.h` file.  I will need to relocate that to the loader if I can.

---

Without too much trouble, I was able to get my heap initialization to execute and I now have a working heap.  There is a little to clean up with this, so I am going to take a moment to review my code (I know I need to set a spinlock for the heap structures).  But in the meantime, the heap is created:

```
Heap Created
  Heap Start Location: 0xd0000000
  Current Heap Size..: 0x00010000
  Heap End Location..: 0xd0010000
```

I have not tested any real allocations with the heap yet, but since the system runs, I think I will commit this code.  The next step will involve completing the PMM initialization by sending initialization messages from `kInit()` to the PMM to fully enable the PMM.

---

OK, so now I need to get the kernel to initialize the PMM with the existing pmm bitmap information.  Since the kernel took posession of the CPU, no changes have been made.  Also, there is only a fraction of the total available memory that has been mapped into the bitmap....  Well, I assumed that, but it turns out that the entire (32-bit) memory has been mapped.  This, then, should be relatively simple to move to the PMM.

The first thing I will need to do is define a common set of messages for the PMM.  This set of messages really needs to be defined once and used for both the PMM and the kernel.  The untimate goal is to eliminate libk, so that locaiton will be out.  `kernel/inc` and `pmm/inc` are both out because they are too specific.  I really want to get away from directly including `inc`, but the files therein should be able to be copied into the resulting `bin` for any architecture.  That really is the most likely spot.

Now, when I do this, I will also want to make sure that `inc` is farther down the include path search order for all compiles.  That should be relatively easy to accomplish as well.  I will start with that.

---

Well that caused me to totally refactor my `Tupfile`s.  But they are cleaner and most of the settings were able to be pulled into the `Tuprules.tup` file.  The other thing that dawns on me that that I really need to start thinking about what I need to do to get to a final `v0.1.0` release -- things like what still needs to be documented?  What needs to be better organized?  Compile-time `#define`s for debugging code?  Eliminate `x86-common`?  Etc..

So, now back to the PMM initialization....   I made the assumption that the heap was not going to be set up when I was going to initialize the PMM.  However it is and I have enough room to pass the entire bitmap.  But the SYSCALL does not have that available.

But that will be tomorrow -- my wife wants to watch a movie.

---

### 2018-Nov-11

OK, first order of business today is to update the Send Message SYSCALL to take a data payload.  The kernel version should be good and already coded.  Well no it's not.  And I think I specifically opted not to code that.  Well, time to correct that....

I must just need more coffee -- it is coded.  Everything is built into the `Message_t` structure, so all I need to do is set the fields correctly.

And once I got that all worked out, I now get this `#PF`:

```
Page Fault
EAX: 0xcffffff8  EBX: 0xc0033860  ECX: 0xbab6badd
EDX: 0x0000fff8  ESI: 0x00117349  EDI: 0x00000000
EBP: 0xc0034f4c  ESP: 0xc0034f0c  SS: 0x10
EIP: 0xc00011da  EFLAGS: 0x00200092
CS: 0x8  DS: 0x10  ES: 0x10  FS: 0x10  GS: 0x10
CR0: 0x80000011  CR2: 0xcffffffc  CR3: 0x00001000
Trap: 0xe  Error: 0x0
```

So, this is for a faulting address `0xcffffffc` and in in the kernel code for `HeapMergeLeft()`.  Without looking at the code again yet, I have a feeling I am confusing the heap max limit address and the heap current end address.  Although...  the faulting address is actually 4 bytes before the start of the heap.

It actually ended up being a quick fix.  I was calculating a theoretical left footer posisiton and then dereferencing that to get to a theoretical left heder position before checking to make sure the footer was still in heap memory:

```C
	thisFtr = (KHeapFooter_t *)((char *)hdr + hdr->size - sizeof(KHeapFooter_t));
	leftFtr = (KHeapFooter_t *)((char *)hdr - sizeof(KHeapFooter_t));
	leftHdr = leftFtr->hdr;

  	if ((byte_t *)leftHdr < kHeap->strAddr) return 0;
```

The end result is just a reorganization of these statements:

```C
	thisFtr = (KHeapFooter_t *)((char *)hdr + hdr->size - sizeof(KHeapFooter_t));
	leftFtr = (KHeapFooter_t *)((char *)hdr - sizeof(KHeapFooter_t));

	// -- Check of this fits before dereferencing the pointer -- may end in `#PF` if first block
	if ((byte_t *)leftHdr < kHeap->strAddr) return 0;
	leftHdr = leftFtr->hdr;
```

For a microkernel, this should then complete the phase 2 initialization.  As a matter of fact, from this site (https://wiki.osdev.org/Microkernel), all I really need to be able to support is memory allocation, scheduling, and messaging.  I think I have at least at the base level all 3 of these working.  Do I really have a working (mostly complete) microkernel???

I started by going through Redmine and checking what was open.  Several issues were able to be closed because they were handled by debugging the process switching.  Several others I pulled out of this version roadmap (at least for the moment) since they brought no additional value to the `v0.1.0` code.

So, this means that what I started to consider last night very quickly came to reality and I need to consider what is left to call `v0.1.0` complete...:
1. I need to complete all the documentation headers for all files.
1. I need to make sure that all common files are not directly `#include`-able.  `pmm-msg.h` comes to mind inparticular.
1. I want to eliminate `x86-common`.  Anything there should be in `i686` at this point.

---

WOW!!  Going through all the source files to update the comments, I found that `PmmNewFrame()` was never implemented....

---

I think I touched every source file getting my comments cleaned up.  I'm a really crappy commenter when it comes to the source header.  On the other hand, I do rather well with commenting the actual code.

I think I am ready to commit this code as `v0.1.0`.

---

So, now with that commit, I have a decision to make: Do I develop wide first and add an architecture?  Or do I develop deep first and add functionality for more satisfaction?

I asked this question on the `#osdev` IRC channel and had this exchange:

```
[15:54] <eryjus> poll for those who have been there: I am happy with my v0.1 of my microkernel for now and ultimately my plan is to support other architectures -- would you recommend going wide and add an architecture first, or deep and build out other fucntionality first?
[15:54] <geist> probably adding at least one more is useful
[15:55] <geist> it tends to point out where your architecture abstractions are weak
[15:55] <eryjus> I'm certain there is a ton of opportunity there....
[16:01] <eryjus> Building deep on top of flaws like that does not seem like a good idea...
```

I think I am going to take that advice and work on the `rpi2b` architecture.  This is so significantly different than the x86 architecture that if there are any abstraction issues they will be evident quickly.  There will be lots of problems.

The first thing I need to do is to refresh my `rpi-boot` project to be able to boot an RPi2b emulator.  This was an easy recompile and copy the resulting `kernel-qemu.img` to `~/bin`.  To compile this, I also need to `export CROSS_COMPILE=arm-eabi-`.

I already have a cross-compiler for the rpi2b architecture, so all I really need to do is get into it.

---

I think it is going to be important to discuss the differences between the architectures as I work through the issues.  As a result, I will spend a little time discussing each file and the issues in encounter with each.  I expect to be bouncing around a bit as I work my way through the issues.  I am generally working on the last file that failed, so a lot is going to depend on what the last one is.

I started with creating the 3 `arch-*.h` files and moving what was appropriate to those files.  With that, I am working on the `LoaderMain.cc` file.  Here are the problems I am dealing with at the moment:
* The `loader` file `mmu.h` is poorly named.  It contains several architecture-dependent function prototypes.
* The x86 architecture uses register `cr3` to control the top of the paging tables.  However, the rpi2b architecture uses several CP15 registers to control the MMU.
* As a result of the issue above, the function `MmuSwitchPageDir()` is architecture dependent.
* The Framebuffer address is hard-coded and should be defined in an architecture-dependent header.
* The HW discovery structure location that is sent to the kernel is hard-coded and should be defined in an architecture-dependent header.

---

After the `LoaderMain.cc`, it took quite a while to get the `Tupfile`s worked out so that I could continue to try to get a full executable image ready for debugging.  So, with that said, `tup` is trying to link the kernel first.  The file `kinit.cc` appears to be the one reporting the errors last at this point.  But most of the errors are happening on `idt.h`, or at least the ones that are still left on the screen.  Certainly the Interrupt Descriptor Table (or IDT) is an intel-specific thing, so I will need to refactor that name and everything that touches it.  Once I rename this file to be `interrupt.h` all kinds of things break.

I now have the kernel's `MmuMapToFrame()` function that is coming up.  This is clearly going to be very architecture dependent.  But the questions now becomes how to organize it.  Do I create a folder with-in each sub-module for each architecture?  Or a sub-module within each architecture?  Tonight I am leaning toward the latter, but we will see how I feen about it tomorrow.

---

### 2018-Nov-12

Today I spent a bit of time looking at what would be the best way to handle the architecture question with `tup`.  There are several solutions but nothing that would make things super easy to handle.  So, I will create an architecture folder within each sub-module.  Starting with the MMU.

I am going to focus on getting the loader for rpi2b processed.  There are going to be enough issues with that to keep me busy for a while.

I was able to find my loader's `entry.s` from century and copied it in.  However, this is written in AT&T syntax while `nasm` is Intel syntax.  Moreover, `nasm` does not support anything other than x86, which really sucks.  So, it's back to GNU Asm....  However, there is a directive I can include in the source to switch to Intel syntax: `.intel_syntax noprefix`.  I will start by adding that to the rpi2b asm files and then retrofit that back into the i686 files once I have a working rpi2b architecture.  I'm not sure how much will break while I am transitioning this code.

---

Well, it looks like my cross-compiler was not built to be able to support intel syntax.  I will have to rebuild it at some point.  But for now, I am going to just suffer through the AT&T syntax -- after all it is not an Intel CPU I am assembling for!!   Duh!!!

Ok, `entry.s` is assembling.  But I have this error when trying to link that reads:

```
/home/adam/opt/cross/lib/gcc/arm-eabi/6.3.0/libgcc.a(pr-support.o): In function `_Unwind_GetTextRelBase':
/home/adam/devkit/build-gcc/arm-eabi/libgcc/../../../gcc-6.3.0/libgcc/config/arm/pr-support.c:384: undefined reference to `abort'
```

What bothers me is that I should not be including anything.  I am not sure where this is coming from and I need to try to dig that out.

Well, after a **lot** of digging and reading about issues on the inter-webs, I finally found the 2 options I needed: `-fno-unwind-tables -fno-excpetions`.  I have added that to CFLAGS and the loader compiles properly.  Well, as properly as it can while still missing functions that need to be ported.  My i686 loader still compiles properly as well.

---

### 2018-Nov-13

Today I am going to tackle the serial port for debugging.  There is a key difference between the x86 and ARM architectures where the x86 uses IO ports to read and write data for a device and to control the devices whereas the ARM uses Memory Mapped IO (MMIO) to do the same thing.  So, with MMIO, configuring and writing to the serial port is as easy as writing to memory locations.  I believe that later versions of x86-family have increasing MMIO abilities but leave the IO ports in place as well for backward compaitibility.

So, my current error list when trying to compile my `loader.elf` binary are:

```
[adam@os-dev century-os]$ tup bin/rpi2b/boot/loader.elf
 [ETA~=<1s Remaining=0 ] 100%
* 1) bin/rpi2b/boot: arm-eabi-gcc -T /home/adam/workspace/century-os/modules/loader/src/rpi2b/loader.ld -g -ffreestanding -O2 -nostdlib -L /home/adam/workspace/century-os/lib/rpi2b -z max-page-size=0x1000 -o loader.elf /home/adam/workspace/century-os/obj/loader/rpi2b/FrameBufferInit.o /home/adam/workspace/century-os/obj/loader/rpi2b/LoaderMain.o /home/adam/workspace/century-os/obj/loader/rpi2b/ModuleInit.o /home/adam/workspace/century-os/obj/loader/rpi2b/PmmInit.o /home/adam/workspace/century-os/obj/loader/rpi2b/PmmNewFrame.o /home/adam/workspace/century-os/obj/loader/rpi2b/entry.o /home/adam/workspace/century-os/obj/loader/rpi2b/hw-disc.o /home/adam/workspace/century-os/obj/loader/rpi2b/mb1.o /home/adam/workspace/century-os/obj/loader/rpi2b/mb2.o /home/adam/workspace/century-os/lib/rpi2b/libk.a /home/adam/workspace/century-os/modules/loader/src/rpi2b/loader.ld -lgcc -lk;
/home/adam/workspace/century-os/obj/loader/rpi2b/LoaderMain.o: In function `LoaderMain':
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:39: undefined reference to `SerialInit()'
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:40: undefined reference to `HwDiscovery()'
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:46: undefined reference to `MmuInit()'
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:51: undefined reference to `SetMmuTopAddr()'
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:56: undefined reference to `kMemMove'
/home/adam/workspace/century-os/obj/loader/rpi2b/ModuleInit.o: In function `ModuleInit()':
/home/adam/workspace/century-os/modules/loader/src/modules/ModuleInit.cc:73: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/modules/loader/src/modules/ModuleInit.cc:125: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/modules/loader/src/modules/ModuleInit.cc:137: undefined reference to `MmuMapToFrame(unsigned long, unsigned long, unsigned long, bool, bool)'
/home/adam/workspace/century-os/modules/loader/src/modules/ModuleInit.cc:158: undefined reference to `cr3'
/home/adam/workspace/century-os/obj/loader/rpi2b/PmmInit.o: In function `PmmInit()':
/home/adam/workspace/century-os/modules/loader/src/pmm/PmmInit.cc:66: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/obj/loader/rpi2b/PmmNewFrame.o: In function `PmmNewFrame()':
/home/adam/workspace/century-os/modules/loader/src/pmm/PmmNewFrame.cc:35: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/obj/loader/rpi2b/mb1.o: In function `AddModule(unsigned long long, unsigned long long, char*)':
/home/adam/workspace/century-os/inc/hw-disc.h:244: undefined reference to `kStrCpy'
/home/adam/workspace/century-os/lib/rpi2b/libk.a(FrameBufferClear.o): In function `FrameBufferClear()':
/home/adam/workspace/century-os/modules/libk/src/frame-buffer/FrameBufferClear.cc:36: undefined reference to `kMemSetW'
/home/adam/workspace/century-os/lib/rpi2b/libk.a(SerialPutChar.o): In function `SerialPutChar(char)':
/home/adam/workspace/century-os/modules/libk/src/SerialPutChar.cc:27: undefined reference to `inb'
/home/adam/workspace/century-os/modules/libk/src/SerialPutChar.cc:29: undefined reference to `outb'
/home/adam/workspace/century-os/modules/libk/src/SerialPutChar.cc:30: undefined reference to `serialPort'
collect2: error: ld returned 1 exit status
 *** tup errors ***
 *** Command ID=3975 failed with return value 1
tup error: Expected to write to file 'loader.elf' from cmd 3975 but didn't
 [ ] 100%
 *** tup: 1 job failed.
[adam@os-dev century-os]$ tup bin/i686/boot/loader.elf
 [  ETA~=<1s Remaining=0    ] 100%
 skipped 138 commands.
 ```

So, looking at this list, all of the Serial port implementations will need to be pulled into an architecture folder.  But the prototypes will all need to be consistent.  This is a good small subsystem to take on for a first exercise.

First, the `serial.h` file in in `libk`.  Since my goal is to eliminate the `libk` module, I will move that into the loader includes and copy it into the kernel includes at the same time -- yes, 2 copies of the same file.  I will have to take on some de-dup effort.

In the meantime, I have created a `serial` submodule with architecture folders therein.  I have moved the `Serial*()` functions from `libk` to the loader.  And I have started looking at the MMIO inlines that will be used to interact with the uart on rpi2b (Do I really need to type the architecture every time now? -- I guess I better get used to it...).  I started to put the MMIO functions into the `arch-cpu.h` file, but realized that with the APIC I will want these for the x86 family as well.  So, into `cpu.h` they go.  For now.

I was able to extract all the relevant function lines from `uart-dev.c` from century to match the configuration for i686.  The serial port is set for 38400 baud, 8-N-1 for both architectures now.  rpi2b `SerialInit()` function requires a `BusyWait()` function, which in turn required a read of a low level system timer.  I had to bring those all over as well.

I want to note that the low level system timer is not emulated properly in the `qemu` version I am running, so I had to put in a patch to work around it.  I believe that we should be able to pull that patch out at some point.  But, today is not the day.

When I try to link, I have a bunch of references to `SerialPutS()` now.  I will take that on next, along with `SerialPutChar()` since there is a strong dependency there.

Actually, looking at it, `SerialPutS()` is hardware independent!  I get a freebee!  Actually, so is `SerialPutHex()`.  So, that really only leaves `SerialPutChar()`.

That was not too bad....

I did get this message on linking:

```
/home/adam/workspace/century-os/modules/loader/src/rpi2b/SysTimerCount.s:28: undefined reference to `ST_CLO'
```

This is directly related to this function:

```asy
SysTimerCount:
	ldr		r0,=ST_CLO                  @@ load the base address of the system timer
	ldrd	r0,r1,[r0]                  @@ Get the 64-bit timer "count" into r1:r0
	mov		pc,lr						@@ return
```

The problem is that the assembler is not throwing an error on undefined references.  I will work on sorting that out next.  Well, it looks like `as` assumes that all unknowns are defined in another file and there is no way to change that behavior.  At least that's I found.

---

I think I will take on `kMemSetB()` next.  It will give me a reason to refresh my ARM assembler...  It dawns on me that I have not really actually done much ARM assembly coding.  I have written a few fuctions that get and return a value, but nothing that actually takes a parameter and does something with it.

---

I managed to get both `kMemSetB()` and `kMemSetW()` written.  I have no clue how successful I was in getting the memory block right...   I guess I will have to debug some when I can get it to compile.

I am down to a few functions left.  `kStrCpy()` is the next one I will tackle.

Ok, I have all the lower level memory functions done.  I have only a few issues left to tackle:

```
/home/adam/workspace/century-os/obj/loader/rpi2b/LoaderMain.o: In function `LoaderMain':
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:40: undefined reference to `HwDiscovery()'
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:46: undefined reference to `MmuInit()'
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:51: undefined reference to `SetMmuTopAddr()'
/home/adam/workspace/century-os/obj/loader/rpi2b/ModuleInit.o: In function `ModuleInit()':
/home/adam/workspace/century-os/modules/loader/src/modules/ModuleInit.cc:137: undefined reference to `MmuMapToFrame(unsigned long, unsigned long, unsigned long, bool, bool)'
/home/adam/workspace/century-os/modules/loader/src/modules/ModuleInit.cc:158: undefined reference to `cr3'
```

These should all be C functions and some will be quite complicated (I'm dreading `MmuInit()`!).

---

### 2018-Nov-14

I started the day by taking care of everything but the MMU for the rpi2b architecture.  Before I go on, I want to pull everything out of the rpi2b build except for the loader.  This way I can actually test the loader sooner.  It may not do much now, but I have a lot of reading to do before I can even attempt to set up the rpi2b MMU.

Here are some notes as I read through the documentation:
* The TTL1 must be 16K aligned and is 16K in length
* There are 4096 TTL1 entries, each is 4 bytes long
* Each TTL1 Entry can look at 1MB of physical memory
* A word to ARM is 4 bytes long (a half-word is 2 bytes)
* A TTL2 table is 1K in length, which makes a mess of the 4K frame unless we put multiple in a single frame
* There are 256 TTL2 entries in a single 1K table
* There are 1K X 4096 or 4MB of TTL2 Tables that would need to be mapped
* If I was to map the tables, I would need TTL1 entries 4095, 4094, 4093, and 4092 to accomplish the TTL2 table mappings
* This would also leave 4 additional TTL1 mappings I would also need to build out to access the TTL1 table
* TTL2 tables would be from addresses `0xffc00000` to `0xffffffff`; TTL1 tables would be from `0xffbfc000` to `0xffbfffff`

---

After checking on #osdev, I learned that there is a TTLB0 and TTLB1 register pair.  TTLB0 is intended to be used for user processes and TTLB1 is for system processes.  I can set the number of bits to distinguish between the 2 tables to be 1 bit.  This means that addresses from `0x00000000` to `0x7fffffff` will refer to TTLB0 and addresses from `0x80000000` to `0xffffffff` will refer to TTLB1.  The key here is that when I perform a task switch, I only have to be concerned with updating the TTLB0 for the new target process.  The TTLB1 is static.

Now, the drawback is that the address space for my processes just got reduced (or at least if I plan to have them remain the same between x86 and rpi2b).

If I am reading the documentation correctly, the TTLB1 address must be 16K in length and 16K aligned (but 1 for the entire OS) while the TTLB0 address would only need to be 8K aligned and 8K in length (1 per non-kernel process).

So, how then to manage the mappings to the table structures.  I think the original thoughts still apply: the top 2M + 16K (only half the addresses are relevant, so only half the TTL2 tables are needed) are used to map the kernel.  And then for processes, use the the top 2M + 8K for the map, up to address `0x7fffffff`.

I think I have a memory layout and a design to start with.  The good news is I only have to start with the kernel mapping in the loader since all I need is kernel mappings for the loader.

When I come back, I think it's time to put some code down to start the implementation.

---

Ok, so, I need to map this out...  starting with the kernel:
* `kTTL1` will be located at `0xffbfc000`.  The address of that table will be static.
* The index `n1` into `kTTL1` will be calculated as `vAddr >> 20` (0 to 4095)
* `kTTL2` tables will begin at `0xffc00000`.  This starting address will be static.
* The `kTTL2` table that backs `kTTL1[n1]` will be located at `kTTL2 + (1024 * n1)`.
* The index `n2` into `kTTL2` will be calculated as `(vAddr >> 12) & 0xff`

---

Well, crap!!!  I just lost an uncommitted copy of `Tuprules.tup`!!!  I'm depressed.

---

### 2018-Nov-15

OK, I need to start building out the `Tuprules.tup` file again.  What is really bothering me is that I really have no clue what happened and how to prevent it again.  I know I can stage the changes in `git` more often and I should have done that 5 minutes before it happened.  But I didn't and here I am.  Just in case, I am going to start with a reboot today.

---

Well, I was able to get everything to compile again.  I was able to stage the files and when I tried to make this command automatically overwrite the target image filesystem, it resets Tuprules.tup!

```
	sudo mkfs.ext2 /dev/mapper/loop0p1
```

I was able to recover my file from `git`.  But I will have to deal with the following prompt for a while:

```
20+0 records in
20+0 records out
20971520 bytes (21 MB, 20 MiB) copied, 0.0155924 s, 1.3 GB/s
mke2fs 1.44.2 (14-May-2018)
/dev/mapper/loop0p1 contains a ext2 file system
        last mounted on /home/adam/workspace/century-os/p1 on Thu Nov 15 15:18:08 2018
Proceed anyway? (y,N) y
Discarding device blocks: done
Creating filesystem with 18432 1k blocks and 4608 inodes
Filesystem UUID: 78feccc7-54a2-4b7c-9057-4d0e1ddc2d3e
Superblock backups stored on blocks:
        8193

Allocating group tables: done
Writing inode tables: done
Writing superblocks and filesystem accounting information: done
```

Actually I figured it out!!  I forgot I had added the following to the `Makefile`:

```
Tuprules.tup: Makefile
	echo "WS = `pwd`" > $@
	echo "II686 = \$$(WS)/bin/i686/usr/include/*" >> $@
```

I removed that part of the code.

So, running the rpi2b loader, and I am now getting these errors and is now the subject of my debugging efforts.

```
Welcome to Rpi bootloader
Compiled on Nov 11 2018 at 16:12:41
ARM system type is c43
EMMC: bcm_2708_power_off(): property mailbox did not return a valid response.
EMMC: BCM2708 controller did not power cycle successfully
EMMC: vendor 24, sdversion 1, slot_status 0
EMMC: WARNING: old SDHCI version detected
SD: found a valid version 1.0 and 1.01 SD card
MBR: found valid MBR on device emmc0
EXT2: not a valid ext2 filesystem on emmc0_0
MBR: found total of 1 partition(s)
MAIN: device list:
VFS: unable to determine device name when parsing /boot/rpi_boot.cfg
VFS: unable to determine device name when parsing /boot/rpi-boot.cfg
VFS: unable to determine device name when parsing /boot/grub/grub.cfg
MAIN: No bootloader configuration file found
```

I have mounted the image and I am able to confirm that `/boot/grub/grub.cfg` exists.

So, I need to back up a bit.  The EXT2 error message is certainly a problem.  So, is the Welcome message coming from `rpi-boot`?  It is; I found the following in the main.cc file:

```C
    printf("Welcome to Rpi bootloader\n");
	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("ARM system type is %x\n", arm_m_type);
```

So, let's go see what the problem is with EXT2.  This is the check that is being made:

```C
	// Confirm its ext2
	if(*(uint16_t *)&sb[56] != 0xef53)
	{
		printf("EXT2: not a valid ext2 filesystem on %s\n", parent->device_name);
		return -1;
	}
```

So, this is something I can check in the file system.  Let's see how to get this done...

```
[adam@os-dev century-os]$ sudo kpartx -as iso/rpi2b.img
[adam@os-dev century-os]$ sudo debugfs /dev/mapper/loop0p1
debugfs:  stats
Filesystem volume name:   <none>
Last mounted on:          /home/adam/workspace/century-os/p1
Filesystem UUID:          1a506bae-56cf-4450-9589-3d60b6691d00
Filesystem magic number:  0xEF53
...
```

So, there is a discrepancy between what the superblock is reporting and what `rpi-boot` is reading.  This all looks to be correct.  However, I am left wondering if the disk image that `rpi-boot` is expecting is un-partitioned.

---

So, I want to document what I have found so far.  I have been digging through the `rpi-boot` code and I belive the problem is the formation of my disk.  `rpi-boot` assumes that the EXT2 partition actually starts in sector 2 (0-based) while I am building my partition from 1MB.  Rather than take on fixing the `rpi-boot` code (which I think I may do at some point), I will see if I can get the `parted` script to build the disk right.

For example, when I create the disk image using this command, I get:
```
parted --script iso/rpi2b.img mklabel msdos mkpart p ext2 4s 100% set 1 boot on

MBR: partition number 0 (emmc0_0) of type 83, start sector 4, sector count 40956, p_offset 1be
EXT2: looking for a filesytem on emmc0_0
block_read: performing multi block read (2 blocks) from block 2 on emmc0_0
```

```
parted --script iso/rpi2b.img mklabel msdos mkpart p ext2 3s 100% set 1 boot on

MBR: partition number 0 (emmc0_0) of type 83, start sector 3, sector count 40957, p_offset 1be
EXT2: looking for a filesytem on emmc0_0
block_read: performing multi block read (2 blocks) from block 2 on emmc0_0
```

```
parted --script iso/rpi2b.img mklabel msdos mkpart p ext2 1 100% set 1 boot on

MBR: partition number 0 (emmc0_0) of type 83, start sector 2048, sector count 38912, p_offset 1be
EXT2: looking for a filesytem on emmc0_0
block_read: performing multi block read (2 blocks) from block 2 on emmc0_0
```

The short story here is that no matter how the disk is partitioned, `rpi-boot` is reading from block #2 every time.  It is not taking the partition table and its start offset into account.  This is the offending line:

```C
int r = block_read(parent, sb, 1024, 1024 / parent->block_size);
```

The `block_read` prototype looks like this:

```C
size_t block_read(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t starting_block);
```

The line that prints the debug code is:

```C
printf("block_read: performing multi block read (%i blocks) from "
			"block %i on %s\n", buf_size / dev->block_size, starting_block,
			dev->device_name);
```

This is telling.  It explains that I ahve a specific request to read from block 2 (passed in).  The buffer size / block size is reporting back as 2 blocks (or really 2 sectors) -- since this read is specifically looking at the superblock.  I have a feeling that the concern is complicated by the use of the term block when is should be sector.

So, I have tested through starting sector 8 and nothing will boot.

---

Well, it looks like I'm going to have to debug the `rpi-boot` solution.  It is not loading the correct sectory for me.  I am struggling because this is not a task I wanted to have to take on.  I have one more thing to try -- which is loading my loader directly.  And that did not work easily either.

On the other hand...

```
adam@os-dev century-os]$ sudo kpartx -uv iso/rpi2b.img
add map loop61p1 (253:60): 0 40958 linear 7:61 2
[adam@os-dev century-os]$ sudo kpartx -uv iso/rpi2b.img
add map loop62p1 (253:61): 0 40958 linear 7:62 2
[adam@os-dev century-os]$ sudo kpartx -uv iso/rpi2b.img
add map loop63p1 (253:62): 0 40958 linear 7:63 2
[adam@os-dev century-os]$ sudo kpartx -anv iso/rpi2b.img
add map loop64p1 (253:63): 0 40958 linear 7:64 2
[adam@os-dev century-os]$ sudo kpartx -anv iso/rpi2b.img
add map loop65p1 (253:64): 0 40958 linear 7:65 2
```

Each execution of kpartx is creating a new device map.  I'm too tired for this!

---

### 2018-Nov-16

It's amazing how a little sleep can help.  I recalled that I had the build working at one point -- the one I am currently having trouble with.  I was able to dig out the `Makefile` from an earlier version of the code: https://github.com/eryjus/century-old/blob/master/Makefile#L117.

The difference with the github version is that I have several statements in a couple of parenthetical statements -- executed as a group.  It is this grouping that I think I am missing now.  I'm going to set it up and then reboot to clean my system and then give it a try.  Or a couple of tries.

---

Ok, after the reboot, my `/dev/mapper` folder has the following:

```
[adam@os-dev ~]$ cd /dev/mapper
[adam@os-dev mapper]$ ls -al
total 0
drwxr-xr-x.  2 root root     120 Nov 16 14:37 .
drwxr-xr-x. 20 root root    3900 Nov 16 14:38 ..
crw-------.  1 root root 10, 236 Nov 16 14:37 control
lrwxrwxrwx.  1 root root       7 Nov 16 14:37 fedora-home -> ../dm-2
lrwxrwxrwx.  1 root root       7 Nov 16 14:37 fedora-root -> ../dm-0
lrwxrwxrwx.  1 root root       7 Nov 16 14:37 fedora-swap -> ../dm-1
```

So, everthing is cleaned up -- I am no longer at loop70p1....  Now, for a test compile.  With 2 test compiles, I end up with this:

```
[adam@os-dev mapper]$ ls -al
total 0
drwxr-xr-x.  2 root root     160 Nov 16 14:50 .
drwxr-xr-x. 20 root root    3980 Nov 16 14:50 ..
crw-------.  1 root root 10, 236 Nov 16 14:37 control
lrwxrwxrwx.  1 root root       7 Nov 16 14:37 fedora-home -> ../dm-2
lrwxrwxrwx.  1 root root       7 Nov 16 14:37 fedora-root -> ../dm-0
lrwxrwxrwx.  1 root root       7 Nov 16 14:37 fedora-swap -> ../dm-1
lrwxrwxrwx.  1 root root       7 Nov 16 14:50 loop0p1 -> ../dm-3
lrwxrwxrwx.  1 root root       7 Nov 16 14:50 loop1p1 -> ../dm-4
```

So, I still have the same problem.  This has got to be the first thing I tackle.

---

After a few reboots and some line-by-line testing, I have finally settled on the following:

```
#
# -- This rule and the following recipe is used to build a disk image that can be booted:
#    * create a disk image, size = 20MB
#    * make the partition table, partition it, and set it to bootable
#    * map the partitions from the image file
#    * write an ext2 file system to the first partition
#    * create a temporary mount point
#    * Mount the filesystem via loopback
#    * copy the files to the disk
#    * unmount the device
#    * unmap the image
#
#    In the event of an error along the way, the image is umounted and the partitions unmapped.
#    Finally, if the error cleanup is completely suffessful, then false is called to fail the
#    recipe.
#    ------------------------------------------------------------------------------------------------
rpi2b-iso: all
	rm -fR iso/rpi2b.img
	cp -fR bin/rpi2b/* sysroot/rpi2b/
	find sysroot/rpi2b -type f -name Tupfile -delete
	mkdir -p ./p1
	(																						\
		dd if=/dev/zero of=iso/rpi2b.img count=20 bs=1048576;								\
		parted --script iso/rpi2b.img mklabel msdos mkpart p ext2 1 20 set 1 boot on; 		\
		sudo losetup -v -L -P /dev/loop0 iso/rpi2b.img;										\
		sudo mkfs.ext2 /dev/loop0p1;														\
		sudo mount /dev/loop0p1 ./p1;														\
		sudo cp -R sysroot/rpi2b/* p1/;														\
		sudo umount ./p1;																	\
		sudo losetup -v -d /dev/loop0;														\
	) || (																					\
		sudo umount ./p1;																	\
		sudo losetup -v -d /dev/loop0;														\
	) || false
```

This appears to be working.

Now I need to back out all the changes to `rpi-boot` and start testing all over again.  I cannot assume that anything I was seeing before was not related to the mal-formed disk image I kept reading.  I get all kinds of new and different stuff!

```
SD: read() card ready, reading from block 2048
SD: issuing command CMD18
SD: multi block transfer, awaiting block 0 ready
SD: block 0 transfer complete
SD: multi block transfer, awaiting block 1 ready
SD: block 1 transfer complete
SD: command completed successfully
SD: data read successful
MULTIBOOT: no valid multiboot header found in /boot/loader.elf
cfg_parse: multiboot failed with -1
```

I certainly need to back out all the debugging `#define`s so that I can get a cleaner boot and try to get to the root of the problem now.  And I get cleaner messages:

```
Welcome to Rpi bootloader
Compiled on Nov 16 2018 at 15:43:38
ARM system type is c43
EMMC: bcm_2708_power_off(): property mailbox did not return a valid response.
EMMC: BCM2708 controller did not power cycle successfully
EMMC: vendor 24, sdversion 1, slot_status 0
EMMC: WARNING: old SDHCI version detected
SD: found a valid version 1.0 and 1.01 SD card
MBR: found valid MBR on device emmc0
EXT2: found an ext2 filesystem on emmc0_0
MBR: found total of 1 partition(s)
MAIN: device list: emmc0_0(ext2)
MAIN: Found bootloader configuration: /boot/rpi_boot.cfg
MULTIBOOT: no valid multiboot header found in /boot/loader.elf
cfg_parse: multiboot failed with -1
```

This is so much better!!

So, this is relatively easy to track down.  `readelf -a bin/boot/loader.elf` yields the following:

```
Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x010000 0x00100000 0x00100000 0x12004 0x13334 RWE 0x10000
```

THe offset must be in the first 4K for the Multiboot specification.  I am at 256K.  A simple flag addition solved this problem.

```
LDFLAGS += -z max-page-size=0x1000
```

So, now I am booting and getting nothing done!  I cannot say for sure just how far I have gotten.  But it is huge progress over last night.

```
Welcome to Rpi bootloader
Compiled on Nov 16 2018 at 15:43:38
ARM system type is c43
EMMC: bcm_2708_power_off(): property mailbox did not return a valid response.
EMMC: BCM2708 controller did not power cycle successfully
EMMC: vendor 24, sdversion 1, slot_status 0
EMMC: WARNING: old SDHCI version detected
SD: found a valid version 1.0 and 1.01 SD card
MBR: found valid MBR on device emmc0
EXT2: found an ext2 filesystem on emmc0_0
MBR: found total of 1 partition(s)
MAIN: device list: emmc0_0(ext2)
MAIN: Found bootloader configuration: /boot/rpi_boot.cfg
MULTIBOOT: loaded kernel /boot/loader.elf
BOOT: multiboot load
```

But at this point, I need to commit the code.  I am able to boot properly into the loader but it locks up.

---

Let's get into determining where the problem is...  I know I am passing all the edits to get to the loader.  Let's start with addresses.  I want to make sure that the address the loader is being loaded at is a good address.  To do this, I can compare to century where I was able to boot with `qemu` at one point.

The addresses look good.  So, I'm going to take a step back....  Thinking this through, the `rpi-boot` code is able to write to the serial port and it shows up on the command line as output.  This is what I want to do with my code.  `rpi-boot` works; mine does not.  One thing I am wondering is if `rpi-boot` uses the mini-UART or the PL011 UART.  I need to compare the 2 solutions to see what the differences are.

`rpi-boot` uses the following base UART address:

```C
#define UART0_BASE			0x20201000
```

I use:

```C
#define HW_BASE             (0x3f000000)
#define UART_BASE           (HW_BASE+0x201000)          // The UART base register
```

So, I am using the incorrect address.  However, changing the base hardware address did nothing.  I changed it back.

A line-by-line comparison shows that I had an inequality comparison as an equality check, fixed here:

```C
void SerialPutChar(char byte)
{
    while ((MmioRead(UART_BASE + UART_FR) & UARTFR_TXFF) != 0) { }

    MmioWrite(UART_BASE + UART_DR, byte);
}
```

Changing this check results in lots of additional debugging info:

```
BOOT: multiboot load
Serial port initialized!
Setting basic memory information
/boot/loader.elf
Module information present
Setting memory map data

Frame Buffer is at: 0x0c100000; The pitch is: 0x00000500; The height is: 0x000001e0
```

However, I am not getting a lot of information from `rpi-boot` about the system.  The good thing is that every rpi2b has basically the same config and this can be hard coded.  For the record, it looks like I am getting the following information from `rpi-boot1`:
* Basic Memory info
* Command line
* Module Info
* Memory Map
* Boot loader name (most likely, but blank)
* Frame buffer info

This covers the critical information for rpi2b (particularly the middle 2 points for century).  The next thing to look at with loader is the temporary PMM initialization.  The next step is to add some debugging statements to figure out where the problem is.

This check came up empty:

```C
    // -- pages now holds the bitmap aligned to 4K right up to the EBDA or 640K boundary; set to no available memory
    SerialPutS("Getting ready to init the bitmap\n");
    kMemSetB((void *)start, 0, PmmFrameToLinear(pages));
    SerialPutS("Done clearing the bitmap\n");
```

I got both lines.  However, I never get here:

```C
    // -- Allocate the loaded modules
    SerialPutS("About to allocate the module locations\n");
    if (HaveModData()) {
```

I do get past this point:

```C
       PmmFreeFrameRange(frame, length);
    }
    SerialPutS("Past the Memory Map\n");
```

So, in short, my problem is somewhere in these lines:

```C
    SerialPutS("Past the Memory Map\n");

    // -- The GDT is at linear address 0 and make it unavailable
    PmmAllocFrame(0);

    // -- Page Directory is not available
    PmmAllocFrame(1);

    // -- The area between the EBDA and 1MB is allocated
    PmmAllocFrameRange(PmmLinearToFrame(GetEbda()), 0x100 - PmmLinearToFrame(GetEbda()));

    // -- now that all our memory is available, set the loader space to be not available; _loader* already aligned
    frame_t ls = PmmLinearToFrame((ptrsize_t)_loaderStart);
    frame_t le = PmmLinearToFrame((ptrsize_t)_loaderEnd);
    PmmAllocFrameRange(ls, le - ls);        // _loaderEnd is already page aligned, so no need to add 1 page.

    // -- Allocate the Frame Buffer
    PmmAllocFrameRange(PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr()), 1024 * 768 * 2);

    // -- Allocate the loaded modules
    SerialPutS("About to allocate the module locations\n");
```

Which makes me wonder if the EBDA is a problem, since the rpi2b doesn't have one.  Well, a quick test begs to differ...

```C
    SerialPutS("Allocating EBDA\n");
    PmmAllocFrameRange(PmmLinearToFrame(GetEbda()), 0x100 - PmmLinearToFrame(GetEbda()));
    SerialPutS("Allocated EBDA\n");
```

... where I get both lines out output.  It may not be right, but they both execute.  Finally the following debug line executes, so the problem must be in the FrameBuffer allocation since the module locations debug locations do not print:

```C
    SerialPutS("Allocated EBDA\n");

    // -- now that all our memory is available, set the loader space to be not available; _loader* already aligned
    frame_t ls = PmmLinearToFrame((ptrsize_t)_loaderStart);
    frame_t le = PmmLinearToFrame((ptrsize_t)_loaderEnd);
    PmmAllocFrameRange(ls, le - ls);        // _loaderEnd is already page aligned, so no need to add 1 page.
    SerialPutS("Loader allocated\n");

    // -- Allocate the Frame Buffer
    PmmAllocFrameRange(PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr()), 1024 * 768 * 2);

```

I was finally able to whittle this down the the following frame buffer operation where it was allocating a huge number of frames:

```C
    SerialPutS("Frame Buffer Address: "); SerialPutHex((ptrsize_t)GetFrameBufferAddr()); SerialPutChar('\n');
    SerialPutS("Number of frames to allocate: "); SerialPutHex(1024 * 768 * 2); SerialPutChar('\n');
    PmmAllocFrameRange(PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr()), (1024 * 768 * 2) >> 12);
```

The bug was that the number of frames was not adjusted -- it was just a byte count.  With this change my boot sequence looks like this:

```
BOOT: multiboot load
Serial port initialized!
Setting basic memory information
/boot/loader.elf
Module information present
Setting memory map data

Frame Buffer is at: 0x0c100000; The pitch is: 0x00000500; The height is: 0x000001e0
Getting ready to init the bitmap
Done clearing the bitmap
Past the Memory Map
Loader allocated
Frame Buffer Address: 0x0c100000
Number of frames to allocate: 0x00180000
About to allocate the module locations
Phyiscal Memory Manager Initialized
Unable to locate Kernel...  Halting!
```

This is good news -- everything is executing and it is looking for the kernel which is not present.  I cleaned up the debugging code.  However, with that said, the screen is not clearing and I'm certain I did not complete the MMU initialization.  I am getting a message on the screen, but the screen is not clearing first.

I will look into the FrameBuffer tomorrow as it is the next thing in order in `LoaderInit()`.

---

### 2018-Nov-17

So, I was thinking last night...  Of the tracable portions that happen in the `LoaderInit()` function, the FrameBuffer is giving me trouble.  But, it's not the frame buffer.  I am getting the greeting message properly shown.  The problem is that the screen is not clearing properly.  This clear screen function is written in assembly and is using the registers from the ABI...  both of which are likely wrong.  I coudld re-implement these functions in C and bypass both problems, but I really need to make sure I have this dialed in.

Well, the ABI understanding was correct.  This is a good thing.  However, the assembly instruction was not correct.  I was trying to auto-increment an address:

```
    strh    r1,[r0]!                                @@ store the value in r1 to the mem at addr r0 and update r0
```

For whatever reason, this was creating a problem, and likely an alignment data abort that my software did not pick up.  I changed the code to this:

```
    strh    r1,[r0]                                 @@ store the value in r1 to the mem at addr r0
    add     r0,#2                                   @@ move to the next address
```

... and this worked.

This now brings me to `MmuInit()` -- the next big thing.  But first a commit.  Oh, did I mention I finally got all of libk eliminated?

---

### 2018-Nov-18

This morning I am going to use what time I have to put some details into the MMU initialization.  Right now, the function is merely a stub so that I could get the loader to compile and I could try to run it to make sure I had all my basics right.  Good thing I did since I had a couple days of some pretty big issues to work through for the rpi2b build system.

I will start by allocating 16K for the kernel TTL1 table.  Recall from **2018-Nov-14** that I will be splitting the address space in half for the user (where bit 31 == 0) and kernel (where bit 31 == 1).

So, I will allocate this table and set it up.

As I write the first lines of code, I realize that I really have no clue what the rpi2b memory map looks like.  Since my PMM allocates by specific frame or from the last frame we know of, this makes it difficult to allocate memory without knowing where things are.  So I will print the memory map to the serial port.  I get only 1 block of physical memory:

```
Upper memory limit: 0x0c000000
Grub Reports available memory at 0x00000000 for 0x0000c000 frames
```

Now, where to put the TTL1 table.  I think the best thing to do is to put the table up high, well out fo the way.  Let me start allocating at frame number 0xb000 or more to the point: `GetUpperMemLimit() - 0x1000000`.

Ok, now I have a design decision to make.  The TTL2 table is 1K, and I can fit 4 of them into a single frame.  The question is this: do I map the 4 TTL2 tables to be consecutive such that a request to set up a TTL2 table results in `% 4 == 0`, `% 4 == 1`, `% 4 == 2`, and `% 4 == 3` all being mapped?  Or do I just map the singular TTL2 table and leave the remaining 3 frames available for additional TTL2 tables to be mapped at a later time?  Here's the concern: I am planning on putting the TTL2 tables in order at specific memory addresses so that I can mimic the recursive mappings from x86.  Since 4 of them fit into a frame, this requires me to have them in order.  I would rather save on memory, but I think the management method I have chosen will trump that desire.

So, what to I need to implement?  Let's start with an `MmuMapToFrame()` function.  Taking a top-down approach, this will drive the additional functions I will need to implement.  I will follow the same parameter convention I have with x86.

---

Looking at `MmuMakeTtl2Table()`, this is quite a bit more work without the native "recurisve mapping" trick.  I have to handle the tables to access the tables separately and explicitly.  It's a bit more work but is very doable.

So, I have the function written.  It's recursive and I have one of those feelings I have written a function that will generate a stack overflow.  Testing will tell, but note that I have my doubts already.

I got the loader to compiler and I am now going to set up to complete the mapping for the TTL1 table into the management area.  This will be at address 0xffbfc000 for 4 pages.

Well, my first execution was anti-climactic:

```
Creating a new TTL2 table for address 0xffbfc000
Creating a new TTL2 table for address 0xffbfd000
Creating a new TTL2 table for address 0xffbfe000
Creating a new TTL2 table for address 0xffbff000
Unable to locate Kernel...  Halting!
```

So I commented out this line:

```
    if (addr >= 0xffbfc000 && addr < 0xffc00000) return;
```

... and I now have these results:

```
Creating a new TTL2 table for address 0xffbfc000
  The new frame is 0x0000b004
Creating a new TTL2 table for address 0xffc00000
  The new frame is 0x0000b005
Creating a new TTL2 table for address 0x3fc00000
  The new frame is 0x0000b006
Attempting do map already mapped address 0xffc00000
Creating a new TTL2 table for address 0x7fc00000
  The new frame is 0x0000b007
Attempting do map already mapped address 0xffc00000
Attempting do map already mapped address 0x3fc00000
Creating a new TTL2 table for address 0xbfc00000
  The new frame is 0x0000b008
Attempting do map already mapped address 0xffc00000
Attempting do map already mapped address 0x3fc00000
Attempting do map already mapped address 0x7fc00000
< Completed the table creation for 0xbfc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0x7fc00000
Attempting do map already mapped address 0x7fc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0x3fc00000
Attempting do map already mapped address 0x3fc00000
Attempting do map already mapped address 0x7fc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0xffc00000
Attempting do map already mapped address 0xffc00000
Attempting do map already mapped address 0x3fc00000
Attempting do map already mapped address 0x7fc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0xffbfc000
Creating a new TTL2 table for address 0xffbfd000
  The new frame is 0x0000b009
Attempting do map already mapped address 0xffc00000
Attempting do map already mapped address 0x3fc00000
Attempting do map already mapped address 0x7fc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0xffbfd000
Creating a new TTL2 table for address 0xffbfe000
  The new frame is 0x0000b00a
Attempting do map already mapped address 0xffc00000
Attempting do map already mapped address 0x3fc00000
Attempting do map already mapped address 0x7fc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0xffbfe000
Creating a new TTL2 table for address 0xffbff000
  The new frame is 0x0000b00b
Attempting do map already mapped address 0xffc00000
Attempting do map already mapped address 0x3fc00000
Attempting do map already mapped address 0x7fc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0xffbff000
Attempting do map already mapped address 0xffbff000
Unable to locate Kernel...  Halting!
```

This is neither the stack overflow I expected nor the result I really want.

So, let's pick apart the first bits of this:
1. A request to map address 0xffbfc000 -- which is the first page for the TTL1 table.
1. A new frame is allocated: 0xb004.
1. A level 2 request to map address 0xffc00000 -- which is the first location for virtual addresses 0x00000000 to 0x00400000.  *This is incorrect.*
1. A new frame is allocated: 0xb005.
1. A level 3 request to map address 0x3fc00000.  *No way this is right.*
1. There is not point in continuing the analysis.

I found a shifting problem with my calculations.

```
MmuMapToFrame(ttl1, 0xffc00000 + ((i << 20) * 1024), ttl2Frame, true, true);
```

The `<< 20` portion was removed and I am now getting the recursion problem I was expecting:

```
Mapping address 0xffbfc000 to frame 0x0000b000
Creating a new TTL2 table for address 0xffbfc000
  The new frame is 0x0000b004
  The base ttl2 index is 0x00000ff8
Mapping address 0xffffe000 to frame 0x0000b004
Creating a new TTL2 table for address 0xffffe000
  The new frame is 0x0000b005
  The base ttl2 index is 0x00000ffc
Mapping address 0xfffff000 to frame 0x0000b005
Creating a new TTL2 table for address 0xfffff000
  The new frame is 0x0000b006
  The base ttl2 index is 0x00000ffc
Mapping address 0xfffff000 to frame 0x0000b006
Creating a new TTL2 table for address 0xfffff000
```

---

### 2018-Nov-19

I need to start today by finishing up my thoughts from yesterday.  I am mapping the first address to the first frame of the TTL1 for management.  This is address 0xffbfc000.  This address will not yet have an associated TTL2 table.  I will get this table and map it into the TTL1 entry for address 0xffbfc000.  It will also need to be mapped into the address for the TTL2 management address.  The index into the TTL1 table is 0xffb, and the address for the TTL2 table for managing this frame is (0xffc00000 + (0xffb * 1024)), or 0xffffec00.  So, something is not right -- either my calculations or the code.

It looks like I am confusing the mapping for the management portion of the tables.  I need to back that recursive call back out and rethink that.

The TTL1 table has 4096 * 4-byte entries (16K).  Each entry therefore controls access to 1M of memory.

There are 4096 TTL2 tables, each 1K in length.  Each table contains 256 entries.  All of these tables will take up 1024 frames, or 4MB, and will have a total of 1048576 entries.  These tables will start at 0xffc00000 and will be contiguous.

Each entry in the TTL2 table could be indexed by the address from 0x00000 to 0xfffff, or 20 bits or vAddr >> 12.  And then to get the address of the TTL2 entry for a TTL2 table in the management addresses, it would be (((vAddr >> 12) * 4) + 0xffc00000).

I now see that I am working with frames and need to be working with 1K physical addresses, so I added a shift here and renamed the variable accordingly:

```C
    frame_t ttl2PAddr = allocFrom << 2;         // Adjust to 1K accuracy
```

Taking this step by step in the code, I finally think I have something that works.  It certainly does what I expect.

```C
    // Here we need to get the TTL1 entry for the management address.
    ptrsize_t mgmtTtl2Addr = 0xffc00000 + ((addr >> 12) * 4);
    int mgmtTtl1Index = mgmtTtl2Addr >> 20;
    Ttl1_t *mgmtTtl1Entry = &ttl1Table[mgmtTtl1Index];

    SerialPutS("  The TTL1 management index for this address is "); SerialPutHex(mgmtTtl1Index); SerialPutChar('\n');

    // If the TTL1 Entry for the management address is faulted; create a new TTL2 table
    if (mgmtTtl1Entry->fault == 0b00) {
        MmuMakeTtl2Table(ttl1, mgmtTtl2Addr);
    }
```

The resuls are:

```
Mapping address 0xffbfc000 to frame 0x0000b000
Creating a new TTL2 table for address 0xffbfc000
  The new frame is 0x0000b004
  The base ttl2 base addr is 0x0002c010
  The base ttl2 index is 0x00000ff8
  The TTL1 management index for this address is 0x00000fff
Creating a new TTL2 table for address 0xffffeff0
  The new frame is 0x0000b005
  The base ttl2 base addr is 0x0002c014
  The base ttl2 index is 0x00000ffc
  The TTL1 management index for this address is 0x00000fff
< Completed the table creation for 0xffffeff0
< Completed the table creation for 0xffbfc000
Mapping address 0xffbfd000 to frame 0x0000b400
Creating a new TTL2 table for address 0xffbfd000
  The new frame is 0x0000b006
  The base ttl2 base addr is 0x0002c018
  The base ttl2 index is 0x00000ff8
  The TTL1 management index for this address is 0x00000fff
< Completed the table creation for 0xffbfd000
Mapping address 0xffbfe000 to frame 0x0000b800
Creating a new TTL2 table for address 0xffbfe000
  The new frame is 0x0000b007
  The base ttl2 base addr is 0x0002c01c
  The base ttl2 index is 0x00000ff8
  The TTL1 management index for this address is 0x00000fff
< Completed the table creation for 0xffbfe000
Mapping address 0xffbff000 to frame 0x0000bc00
Creating a new TTL2 table for address 0xffbff000
  The new frame is 0x0000b008
  The base ttl2 base addr is 0x0002c020
  The base ttl2 index is 0x00000ff8
  The TTL1 management index for this address is 0x00000fff
< Completed the table creation for 0xffbff000
```

This should allow me to initialize the MMU from `MmuInit()`.

---

I am misusing the word 'frame' in all my rpi2b architecture... because it was grandfathered in from the x86 code in the form `MmuMapToFrame()`.  I need to rename this function to be architecture agnostic.  Later.

---

So, here's the problem I am having: I am just not getting my head around the fact that the second level table is only 1K in size.  In my head, I am not keeping track of where I am looking for an ordinal frame number and a 1K TTL2 table.  for example, the following block of code is horribly wrong:

```C
    frame_t ttl1 = allocFrom;

    // ---- snip ----

    // -- Map the TTL1 table to location 0xffbfc000
    MmuMapToFrame(ttl1, 0xffbfc000, ttl1, true, true);
    MmuMapToFrame(ttl1, 0xffbfd000, ttl1 + 1024, true, true);
    MmuMapToFrame(ttl1, 0xffbfe000, ttl1 + 2048, true, true);
    MmuMapToFrame(ttl1, 0xffbff000, ttl1 + 3072, true, true);
```

The reason is that the variable `ttl1` is a `frame_t` type and the very next frame should be the next one in the TTL1 table.

---

### 2018-Nov-20

Let me see if I can keep this straight today as I'm working through all this code....  I have some hope, but not high hopes.

---

OK, I whittled down the address I was mapping to just one and focused in on getting all my calculations consistent with the requirements.  I finally have a good page mapped (or at least consistently wrong if it is wrong).  The output with all my debugging code is:

```
Set up the TTL1 management table
Mapping address 0xffbfc000 to frame 0x0b000000
  Ttl1 index is: 0x0b000000[0x00000ffb]
Creating a new TTL2 table for address 0xffbfc000
  The new frame is 0x0000b004
  The base ttl2 1K location is 0x0002c010
  The ttl1 index is 0x00000ff8
  Set the TTL1 table index 0x00000ff8 to 1K location 0x0002c010
  Set the TTL1 table index 0x00000ff9 to 1K location 0x0002c011
  Set the TTL1 table index 0x00000ffa to 1K location 0x0002c012
  Set the TTL1 table index 0x00000ffb to 1K location 0x0002c013
  The management address for this Ttl2 table is 0xffffeff0
    The base location is 0xffc00000
    The table offset is  0x003fec00
    The entry offset is  0x000003f0
  The TTL1 management index for this address is 0x00000fff
Creating a new TTL2 table for address 0xffffeff0
  The new frame is 0x0000b005
  The base ttl2 1K location is 0x0002c014
  The ttl1 index is 0x00000ffc
  Set the TTL1 table index 0x00000ffc to 1K location 0x0002c014
  Set the TTL1 table index 0x00000ffd to 1K location 0x0002c015
  Set the TTL1 table index 0x00000ffe to 1K location 0x0002c016
  Set the TTL1 table index 0x00000fff to 1K location 0x0002c017
  The management address for this Ttl2 table is 0xfffffff8
    The base location is 0xffc00000
    The table offset is  0x003ffc00
    The entry offset is  0x000003f8
  The TTL1 management index for this address is 0x00000fff
< Completed the table creation for 0xffffeff0
< Completed the table creation for 0xffbfc000
  Ttl2 location is: 0x0b004c00[0x000000fc]
Checking our work

MmuDumpTables: Walking the page tables for address 0xffbfc000
Level  Tabl-Addr     Index        Entry Addr    Next PAddr    fault
-----  ----------    ----------   ----------    ----------    -----
TTL1   0x0b000000    0x00000ffb   0x0b003fec    0x0b004c00     01
TTL2   0x0b004c00    0x000000fc   0x0b004ff0    0x00000000     10
```

I have checked all the math and it all looks good for now.  No stack overflow.  Now I will start to put in the additional addresses I need to map.

I have removed a number of debugging statements using `#if 0` preprocessor directives, so you as can see what the heck I had to go through to get this code debugged.

With that, I have all the major components of the loader done except the module initialization.  For that to work, I need a kernel to compile.  So, I'm back to architecture abstraction.

---

For `interrupt.h`, nearly everything in this file is specific to i686, so I have moved the contents to `arch-interrupt.h`.  Done.

Now, for compiling kInit.cc, there is a structure `isrRegs_t` which is not defined on the rpi2b architecture.  I will need to get that defined.  Which means I need to figure out what the architecture really does.  Additionally, I am getting a message: `Error: selected processor does not support 'wfi' in ARM mode`.  However, is should.

So, I created the `isrRegs_t` structure to be as follows:

```
//
// -- This is the order of the registers on the stack
//    -----------------------------------------------
typedef struct isrRegs_t {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	uint32_t r13;
	uint32_t r14;
	uint32_t r15;
} isrRegs_t;
```

This will certainly change, but it takes care of the basic requirements of the interrupt for ARM.  Also, I had to add the following to the `Tuprules.tup` file to handle the `wfi` instruction:

```
ifneq ($(ARCH),rpi2b)
CFLAGS += -mno-red-zone
else
CFLAGS += -mcpu=cortex-a7
endif
```

`kInit.cc` now compiles.

On to `HeapAlloc.cc`.  I need a constant for `BYTE_ALIGNMENT`.  That was a quick fix.

In `HeapInit.cc`, there are references to `pageTable_t` which is incorrect for rpi2b.  This reference is coming from `mmu.h`.

This is creating a problem between the loader and the kernel.  Each has its own `mmu.h` file and I need to keep them separate.  So, I will eliminate completely the `mmu.h` create either `mmu-loader.h` and `mmu-kernel.h` and everything `inc/$(ARCH)/arch-*.h` fill be renamed to `inc/$(ARCH)/arch-*-prevalent.h` -- not that the contents are common to all architectures but that the contents are used by multiple modules.

I will work on this now and get the other modules to compile -- beore I tackle `mmu.h` from the kernel.

---

### 2018-Nov-21

I spent the first part of the day organizing the kernel so that all the `.o` files compile.  I know I am going to have a bunch of issues linking the resulting `kernel.elf` file.  There are several things I am going to have to re-build when I try to actually link the `kernel.elf`.  And as I expected:

```
/home/adam/opt/cross/lib/gcc/arm-eabi/6.3.0/../../../../arm-eabi/bin/ld: warning: cannot find entry symbol _start; defaulting to 0000000080000000
/home/adam/workspace/century-os/obj/kernel/rpi2b/CpuTssInit.o: In function `CpuTssInit()':
/home/adam/workspace/century-os/modules/kernel/src/cpu/CpuTssInit.cc:29: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/modules/kernel/src/cpu/CpuTssInit.cc:35: undefined reference to `Ltr'
/home/adam/workspace/century-os/obj/kernel/rpi2b/FrameBufferClear.o: In function `FrameBufferClear()':
/home/adam/workspace/century-os/modules/kernel/src/frame-buffer/FrameBufferClear.cc:36: undefined reference to `kMemSetW'
/home/adam/workspace/century-os/obj/kernel/rpi2b/FrameBufferDrawChar.o: In function `FrameBufferDrawChar(char)':
/home/adam/workspace/century-os/modules/kernel/src/frame-buffer/FrameBufferDrawChar.cc:79: undefined reference to `systemFont'
/home/adam/workspace/century-os/obj/kernel/rpi2b/FrameBufferParseRGB.o: In function `FrameBufferParseRGB(char const*)':
/home/adam/workspace/century-os/modules/kernel/src/frame-buffer/FrameBufferParseRGB.cc:42: undefined reference to `kStrLen'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapAlloc.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapAlloc.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapAlloc.o: In function `HeapAlloc(unsigned int, bool)':
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapAlloc.cc:57: undefined reference to `DisableInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapAlloc.cc:76: undefined reference to `RestoreInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapAlloc.cc:91: undefined reference to `RestoreInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapAlloc.cc:112: undefined reference to `RestoreInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapAlloc.cc:123: undefined reference to `RestoreInterrupts'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapError.o: In function `HeapError(char const*, char const*)':
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapError.cc:32: undefined reference to `DisableInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapError.cc:34: undefined reference to `Halt'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapFree.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapFree.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapFree.o: In function `HeapFree(void*)':
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapFree.cc:50: undefined reference to `DisableInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapFree.cc:78: undefined reference to `RestoreInterrupts'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapInit.o: In function `HeapInit()':
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapInit.cc:76: undefined reference to `MmuUnmapPage(unsigned long)'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapInit.cc:77: undefined reference to `MmuMapToFrame(unsigned long, unsigned long, int)'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapInit.cc:83: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/obj/kernel/rpi2b/MessageReceive.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/MessageReceive.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/MessageReceive.o: In function `MessageReceive(Message_t*)':
/home/adam/workspace/century-os/modules/kernel/src/ipc/MessageReceive.cc:59: undefined reference to `kMemMove'
/home/adam/workspace/century-os/obj/kernel/rpi2b/MessageSend.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/MessageSend.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/MessageSend.o: In function `MessageSend(unsigned long, Message_t*)':
/home/adam/workspace/century-os/modules/kernel/src/ipc/MessageSend.cc:51: undefined reference to `kMemMove'
/home/adam/workspace/century-os/obj/kernel/rpi2b/PmmAllocFrame.o: In function `PmmAllocFrame()':
/home/adam/workspace/century-os/modules/kernel/src/pmm/PmmAllocFrame.cc:34: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessCreate.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessCreate.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessCreate.o: In function `ProcessCreate(char const*, unsigned long, unsigned long, ...)':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessCreate.cc:57: undefined reference to `Halt'
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessCreate.cc:66: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessCreate.cc:104: undefined reference to `GetCr3'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessEnd.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessEnd.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessEnd.o: In function `ProcessEnd()':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessEnd.cc:90: undefined reference to `Halt'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessHold.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessHold.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessInit.o: In function `ProcessInit()':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessInit.cc:53: undefined reference to `GetCr3'
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessInit.cc:62: undefined reference to `GetCr3'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessNewPID.o: In function `ProcessNewPID()':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessNewPID.cc:51: undefined reference to `Halt'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReady.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReady.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReady.o: In function `ProcessReady(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessReady.cc:107: undefined reference to `ProcessSwitch'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessRelease.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessRelease.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessRelease.o: In function `ProcessRelease(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessRelease.cc:73: undefined reference to `ProcessSwitch'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReschedule.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReschedule.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReschedule.o: In function `ProcessReschedule()':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessReschedule.cc:74: undefined reference to `Halt'
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessReschedule.cc:83: undefined reference to `ProcessSwitch'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessTerminate.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessTerminate.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessWait.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessWait.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/SerialInit.o: In function `SerialInit()':
/home/adam/workspace/century-os/modules/kernel/src/serial/rpi2b/SerialInit.cc:33: undefined reference to `BusyWait(unsigned long)'
/home/adam/workspace/century-os/modules/kernel/src/serial/rpi2b/SerialInit.cc:35: undefined reference to `BusyWait(unsigned long)'
/home/adam/workspace/century-os/obj/kernel/rpi2b/TimerEoi.o: In function `TimerEoi(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerEoi.cc:29: undefined reference to `outb'
/home/adam/workspace/century-os/obj/kernel/rpi2b/TimerInit.o: In function `TimerInit(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:42: undefined reference to `DisableInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:45: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:46: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:47: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:48: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:49: undefined reference to `outb'
/home/adam/workspace/century-os/obj/kernel/rpi2b/TimerInit.o:/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:50: more undefined references to `outb' follow
/home/adam/workspace/century-os/obj/kernel/rpi2b/TimerInit.o: In function `TimerInit(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:60: undefined reference to `IsrRegister(unsigned char, void (*)(isrRegs_t*))'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:62: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:63: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:64: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:68: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:69: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:71: undefined reference to `RestoreInterrupts'
/home/adam/workspace/century-os/obj/kernel/rpi2b/kInit.o: In function `kInit':
/home/adam/workspace/century-os/modules/kernel/src/kInit.cc:94: undefined reference to `PmmStart(Module_t*)'
/home/adam/workspace/century-os/modules/kernel/src/kInit.cc:99: undefined reference to `EnableInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/kInit.cc:105: undefined reference to `kMemSetB'
```

With this list, a couple of things stand out to me right away:
* My abstractions are not clean since rpi2b is still looking for `out()` and should have failed at compiling the source to a `.o`.
* I am not compiling everything since `kMemSetB()` is implemented and should be found.
* The function `TssInit()` should not exist for rpi2b -- right at the top of the list.

So, I'm going to first take care of the top error (about `_start`).  For i686, the `_start` symbol is in the `loader.s` file in the `$(ARCH)` folder.  The `rpi2b` folder is conspicuously empty and quite a number of my problems will be addressed by putting the right functions in that folder.

I copied nearly all the files from the loader to the kernel for the rpi2b architecture.  This cleaned up several errors but there is still quite a bit to solve.

I also corrected the existence of `in()`, `out()`, and `GetCr3()` in the rpi2b architecture.  This caused several more functions to fail compile, which of course meant they needed to be moved into respective architecture specific folders.

Now, I can get into the architcture-specific functions I need to learn how to properly implement -- starting with `EnableInterrupts()`.  Why?  Because it's the last thing on the list of errors at this point, of course!

---

The ARM CPU has a Current Program Status Register (CPSR) that contains several bits that indicate what exceptions can be fed through to the CPU.  There are 3 such bits: A, I, and F, such that:
* CPSR:A is bit 8 and controls imprecice data aborts
* CPSR:I is bit 7 and controls IRQ interrupts
* CPSR:F is bit 6 and controls FIQ interrupts

A value of `1` in any of these 3 bits disables interrupts of that type.  Therefore, to enable interrupts, I will need to read the CPSR, clear bits 6:8, and then write the CPSR.

* The opcode `mrs` can read the CPSR in the form (I think, anyway) `mrs r0,cpsr`.
* The opcode `msr` can write the CPSR in the form (I think, anyway) `msr cpsr,r0`.
* Therefore, the only thing I need to do in between is `and r0,~0x01c0` to clear the proper bits.

With this, I should have enough to implement the `EnableInterrupts()` function for rpi2b.

Using `EnableInterrupts()` as a template, I can create `DisableInterrupts()` which will return the state of the interrupts flags only  (which is a difference from the x86 implementation) and `RestoreInterrupts()` which will restore the state of the interrupt flags only (which is a difference from the x86 implementation).

Those all compile and the missing references have gone away.

The next thing to look at is Spinlocks.  There are a lot of referenced to those functions and once those are cleaned up, I might actually have a small set of remaining functions I can actually stub out to start some testing.

---

ARM has an opcode `strex` which is an atomic operation similar to the x86 `lock cmpxchg` operation.  Section 1.3.2 of this manual has a sample for implementing a mutex: http://infocenter.arm.com/help/topic/com.arm.doc.dht0008a/DHT0008A_arm_synchronization_primitives.pdf.  This link has the standards for implementing a stack frame: https://thinkingeek.com/2014/05/11/arm-assembler-raspberry-pi-chapter-18/.

The first function I want to implement is `SpinlockCmpXchg()`, but that is i686 specific naming -- I need to clean that up.

And now with `SpinlockAtomicLock()` and `SpinlockClear()` both written, here is my current error list:

```
/home/adam/workspace/century-os/obj/kernel/rpi2b/CpuTssInit.o: In function `CpuTssInit()':
/home/adam/workspace/century-os/modules/kernel/src/cpu/CpuTssInit.cc:35: undefined reference to `Ltr'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapInit.o: In function `HeapInit()':
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapInit.cc:76: undefined reference to `MmuUnmapPage(unsigned long)'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapInit.cc:77: undefined reference to `MmuMapToFrame(unsigned long, unsigned long, int)'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReady.o: In function `ProcessReady(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessReady.cc:107: undefined reference to `ProcessSwitch'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessRelease.o: In function `ProcessRelease(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessRelease.cc:73: undefined reference to `ProcessSwitch'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReschedule.o: In function `ProcessReschedule()':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessReschedule.cc:83: undefined reference to `ProcessSwitch'
/home/adam/workspace/century-os/obj/kernel/rpi2b/TimerVars.o:(.data.rel+0x0): undefined reference to `TimerInit(unsigned long)'
/home/adam/workspace/century-os/obj/kernel/rpi2b/TimerVars.o:(.data.rel+0x4): undefined reference to `TimerEoi(unsigned long)'
/home/adam/workspace/century-os/obj/kernel/rpi2b/kInit.o: In function `kInit':
/home/adam/workspace/century-os/modules/kernel/src/kInit.cc:90: undefined reference to `ProcessInit()'
/home/adam/workspace/century-os/modules/kernel/src/kInit.cc:94: undefined reference to `PmmStart(Module_t*)'
/home/adam/workspace/century-os/modules/kernel/src/kInit.cc:98: undefined reference to `TimerInit(unsigned long)'
```

The function `CpuTssInit()` should not exist for the rpi2b architecture.  I will deal with that next.

---

I finally have a compiled `kernel.elf`.  With this, I now have a kernel module I can try to find from the loader.  But that will start tomorrow.  It's a late night.

---

This morning, I am realizing that I am not getting any information about the modules back from `rpi-boot`.  I am not sure why -- if it is a limitation in `rpi-boot` to pass this information back or if it is just not loading the modules.  To get there, I will need to add some debugging code to determine if there is a concern with actually getting the module info or not.  This will be in the MB1 parser.

With this block of code:

```C
        SerialPutS("Module information present\n");

        for (m = (Mb1Mods_t *)mb1Data->modAddr, i = 0; i < mb1Data->modCount; i ++) {
            SerialPutS("   Found Module: ");
            SerialPutS(m[i].modIdent);
            SerialPutS("\n");
            AddModule(m[i].modStart, m[i].modEnd, m[i].modIdent);
        }
```

I can tell that the module information block is present but there is no data in it.  I can confirm that this works for i686:

```
Module information present
   Found Module: kernel
   Found Module: pmm
```

But for rpi2b, I only get this:

```
Module information present
```

So, it looks like I'm going to have to look at the `rpi-boot` code to see what the difference might be.  In `rpi-boot`, the code is there so I should see:

```C
	module_add(address, address + (uint32_t)bytes_read, name);

	printf("MODULE: %s loaded\n", name);
```

Let's go to the logs to see what is there.  There's nothing about a MODULE being present:

```
ARM system type is c43
EMMC: bcm_2708_power_off(): property mailbox did not return a valid response.
EMMC: BCM2708 controller did not power cycle successfully
EMMC: vendor 24, sdversion 1, slot_status 0
EMMC: WARNING: old SDHCI version detected
SD: found a valid version 1.0 and 1.01 SD card
MBR: found valid MBR on device emmc0
EXT2: found an ext2 filesystem on emmc0_0
MBR: found total of 1 partition(s)
MAIN: device list: emmc0_0(ext2)
MAIN: Found bootloader configuration: /boot/rpi_boot.cfg
MULTIBOOT: loaded kernel /boot/loader.elf
BOOT: multiboot load
Serial port initialized!
Setting basic memory information
/boot/loader.elf
Module information present
Setting memory map data
```

I would expect the MODULE line to be between the MULTIBOOT line and the BOOT line.  Now, before I get too crazy on this, I need to determine if I have an image that is being updated or not.  I had that problem before... and I'm not going to waste a ton of time debugging something without verifying that first.  The easiest way to check is to break the MULTIBOOT line so the `loader.elf` will not load.  And I am still having a problem (imagine my expletives here!).

Ok, here is what is odd to me: the image has the correct `grub.cfg` file.

```
[adam@os-dev grub]$ cat grub.cfg
multiboot /boot/loader.elfx
module /boot/kernel.elf kernel
boot
```

Do I have an old test file left over in my `sysroot`?  I have to.  The `rpi_boot.cfg` is taking precidence.  And as a matter fo fact, I see it in the logs above.

```
[adam@os-dev boot]$ ls
total 346
drwxr-xr-x. 2 root root   1024 Nov 22 07:53 grub
-rwxr-xr-x. 1 root root 166292 Nov 22 07:53 kernel.elf
-rwxr-xr-x. 1 root root 101388 Nov 22 07:53 loader.elf
-rwxr-xr-x. 1 root root  73732 Nov 22 07:53 loader.img
-rw-r--r--. 1 root root     32 Nov 22 07:53 rpi_boot.cfg
```

And after clearing out the `sysroot/rpi2b` folder, I now get this:

```
Welcome to Rpi bootloader
Compiled on Nov 16 2018 at 15:43:38
ARM system type is c43
EMMC: bcm_2708_power_off(): property mailbox did not return a valid response.
EMMC: BCM2708 controller did not power cycle successfully
EMMC: vendor 24, sdversion 1, slot_status 0
EMMC: WARNING: old SDHCI version detected
SD: found a valid version 1.0 and 1.01 SD card
MBR: found valid MBR on device emmc0
EXT2: found an ext2 filesystem on emmc0_0
MBR: found total of 1 partition(s)
MAIN: device list: emmc0_0(ext2)
MAIN: Found bootloader configuration: /boot/grub/grub.cfg
MULTIBOOT: loaded kernel /boot/loader.elf
MODULE: cannot load file kernel
cfg_parse: module failed with -1
```

I'm picking up the correct boot config file.  But I now have a problem with the MODULE line.  Going back to the `rpi-boot` code, the reason is that the file cannot be found:

```C
	// Load a module
	FILE *fp = fopen(name, "r");
	if(!fp)
	{
		printf("MODULE: cannot load file %s\n", name);
		return -1;
	}
```

This time it is a bug in `rpi-boot`.  The value of `name` is `kernel` whereas the value of `file` is `/boot/kernel.elf`.  I will make this change and write a commit for `rpi-boot`.  The pull request is here: https://github.com/jncronin/rpi-boot/pull/22.

Now, with that change, I am getting the kernel module loaded.  The loader is also trying to map the kernel into upper memory.  I am getting one failure for a page already mapped:

```
Initializing Modules:
kernel
   Starting Address: 0x0011c000
   FileSize = 0x0000743b; MemSize = 0x0000743b; FileOffset = 0x00001000
      Attempting to map page 0x80000000 to frame 0x0000011d
Attempting to map already mapped address 0x80000000
      Attempting to map page 0x80001000 to frame 0x0000011e
      Attempting to map page 0x80002000 to frame 0x0000011f
      Attempting to map page 0x80003000 to frame 0x00000120
      Attempting to map page 0x80004000 to frame 0x00000121
      Attempting to map page 0x80005000 to frame 0x00000122
      Attempting to map page 0x80006000 to frame 0x00000123
      Attempting to map page 0x80007000 to frame 0x00000124
   FileSize = 0x0001b20c; MemSize = 0x00051838; FileOffset = 0x00009000
      Attempting to map page 0x80008000 to frame 0x00000125
      Attempting to map page 0x80009000 to frame 0x00000126
      Attempting to map page 0x8000a000 to frame 0x00000127
      Attempting to map page 0x8000b000 to frame 0x00000128
```

However, I am not sure what is mapped there.  I do not see anything that is explicitly mapping that location.   Maybe if I report the frame it is mapped to I can have a clue on where to look.  It's already mapped to frame 0.  That's no help.  I also noticed that I am getting some odd frame numbers in when setting up the `.bss` section.

```
      Attempting to map page 0x80024000 to frame 0x00020002
      Attempting to map page 0x80025000 to frame 0x0002000d
      Attempting to map page 0x80026000 to frame 0x0002000e
      Attempting to map page 0x80027000 to frame 0x0002000f
      Attempting to map page 0x80028000 to frame 0x00020010
```

...  and when I look at the map for the `kernel.elf`, I am seeting lots of space that need not be there:

```
Disassembly of section .bss:

80024000 <heapLock>:
	...

80025000 <heapMemoryBlock>:
	...

80035000 <fixedList>:
	...

80039000 <fixedListUsed>:
80039000:	00000000 	andeq	r0, r0, r0

80039004 <_ZL5_heap>:
	...

80039024 <currentPID>:
80039024:	00000000 	andeq	r0, r0, r0

80039028 <procs>:
```

The binary sizes are not horrible (232K for i686 and 162K for rpi2b).  But `readelf -a bin/rpi2b/boot/kernel.elf` is interesting!

```
Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .text             PROGBITS        80000000 001000 005f10 00  AX  0   0  8
  [ 2] .rodata           PROGBITS        80006000 007000 00143b 00   A  0   0  4
  [ 3] .stab             PROGBITS        80008000 009000 01af51 04  WA  0   0  4
  [ 4] .data             PROGBITS        80023000 024000 00020c 00  WA  0   0  4
  [ 5] .bss              NOBITS          80024000 02420c 035838 00  WA  0   0 4096
  [ 6] .ARM.attributes   ARM_ATTRIBUTES  00000000 02420c 000039 00      0   0  1
  [ 7] ._text            PROGBITS        00000000 024248 000004 00      0   0  4
  [ 8] .symtab           SYMTAB          00000000 02424c 0034a0 10      9 712  4
  [ 9] .strtab           STRTAB          00000000 0276ec 00109c 00      0   0  1
  [10] .shstrtab         STRTAB          00000000 028788 000051 00      0   0  1
```

There is a leftover `._text` section that is added after the `.bss` section.  And more importantly it's a `PROGBITS` type.

But other than that, the size of `.bss` is actually very similar between i686 and rpi2b.

So, now, back to my problem.  Something is mapping address `0x80000000` or making the system think that the address is already mapped.  So, I need to re-enable all that debugging code and then take a pedantic walk through the results.

With all my debugging code turned on, I see this:

```
Initializing Modules:
kernel
   Starting Address: 0x0011d000
   FileSize = 0x0000743b; MemSize = 0x0000743b; FileOffset = 0x00001000
      Attempting to map page 0x80000000 to frame 0x0000011e
Mapping address 0x80000000 to frame 0x0000011e
  Ttl1 index is: 0x0b000000[0x00000800]
Creating a new TTL2 table for address 0x80000000
  The new frame is 0x0000b008
  The base ttl2 1K location is 0x0002c020
  The ttl1 index is 0x00000800
  Set the TTL1 table index 0x00000800 to 1K location 0x0002c020
  Set the TTL1 table index 0x00000801 to 1K location 0x0002c021
  Set the TTL1 table index 0x00000802 to 1K location 0x0002c022
  Set the TTL1 table index 0x00000803 to 1K location 0x0002c023
  The management address for this Ttl2 table is 0xffe00000
    The base location is 0xffc00000
    The table offset is  0x00200000
    The entry offset is  0x00000000
  The TTL1 management index for this address is 0x00000ffe
< Completed the table creation for 0x80000000
  Ttl2 location is: 0x0b008000[0x00000000]
Attempting to map already mapped address 0x80000000 (mapped to: 0x00000000); Fault is: 0x00000003
```

This makes me wonder if the `kMemSetB()` function is working properly.  I think I will replace it with a C version and see if I get different results.  And it works.  This tells me that my assembly language versions are not correct and need to be debugged.

---

Lots of debugging to the serial port later and I think it is not the `kMemSetB()` function but my calculations for mapping the pages that is the problem somewhere.  Or more to the point, something that is overwriting something.

---

I stopped the loader after initializing the TTL1 table with `kMemSetB()` and then investigated the memory with `gdb`.  What I saw is this:

```
(gdb) x/xw 0x0b000000
0xb000000:      0x000000ff
(gdb)
0xb000004:      0x00000000
(gdb)
0xb000008:      0x00000000
(gdb)
0xb00000c:      0x00000000
(gdb)
```

In addition, when I looked at each TTL2 table, the first byte was always 0xff.  So, my `kMemSetB()` function is somehow skipping the first byte.

And there it is!  Right in front of my face the whole time!

```
    strb    r1,[r0]                                 @@ store the value in r1 to the mem at addr r0
    add     r1,#1                                   @@ increment the address
    sub     r2,#1                                   @@ decrement the numebr of bytes
    b       kMemSetB                                @@ loop
```

I was incrementing the value to store, not the address in which to store the value!

---

This morning I am going to start researching why I have no memory available until I get way high in the frame count.  My concern is this:

```
      Attempting to map page 0x80021000 to frame 0x0000013e
      Attempting to map page 0x80022000 to frame 0x0000013f
      Attempting to map page 0x80023000 to frame 0x00000140
Checking from frame 0x00000144
      Attempting to map page 0x80024000 to frame 0x00020002
Checking from frame 0x00000144
      Attempting to map page 0x80025000 to frame 0x0002000d
Checking from frame 0x00000144
      Attempting to map page 0x80026000 to frame 0x0002000e
Checking from frame 0x00000144
      Attempting to map page 0x80027000 to frame 0x0002000f
Checking from frame 0x00000144
      Attempting to map page 0x80028000 to frame 0x00020010
Checking from frame 0x00000144
```

At this point, we should be finding frames at frame number 144, not up in about 20000.  The means my PMM initiialization has gone wrong and I will be adding code there to debug that.

I realized that the `start` for the PMMBitmap had wrapped around `0x00000000` back to high memory, so I added the following code to keep it in lower memory:

```
    if (start) {                            // -- if we are not dealing with an ebda (and therefore not x86)
        start -= PmmFrameToLinear(pages);
    }
```

This has caused the loader to lock up now.  I definitely have some problems here and will need to continue to debug for rpi2b -- and pray I don't break i686!

For rpi2b, I moved the bitmap from frame 0 to frame 1 and that worked.  It appears there is something that does not like frame 0 in some way.  I'm Ok with leaving that unusable if is helps with `NULL` pointer assignments.  And, at this point, I am getting all the proper allocations from the PMM.  But to be certain, I am going to continue to check the rest of this since I was not able to verify modules before I had a compiling kernel.

I might be writing off the end of the bitmap for the frame buffer:

```
Upper memory limit: 0x0c000000
0x00000001 pages are needed to hold the PMM Bitmap
   these pages will start at 0x00004000
PMM Bitmap cleared and ready for marking unusable space
Grub Reports available memory at 0x00000000 for 0x0000c000 frames
Marking frames 0 and 1 used
Marking the loader space as used starting at frame 0x00000100 for 0x00000018 frames.
Marking the frame buffer space as used starting at frame 0x0c100000 for 0x00000180 frames.
Marking the module kernel space as used starting at frame 0x0000011d for 0x00000029 frames.
Finally, marking the stack, hardware communication area, kernel heap, and this bitmap frames as used.
Phyiscal Memory Manager Initialized
```

I think the rpi2b does not report that memory at all.  My PMM functions do not check bounds, I a need to protect that in `PmmInit()`.  Also, I want to call out that I am starting to add debugging code `#define`s in the individual files like this:

```C
#ifndef DEBUG_PMM
#   define DEBUG_PMM 0
#endif

// --- snip ---

#if DEBUG_PMM == 1
// Some debugging output
#endif
```

This way I can turn debugging on more globally as a compile time option or in the individual files.

OK, I am now finally to the point where I can enable paging properly.  This will set the TTLB0 register and set the number of bits to 0 (meaning everything is in the one paging table and there is no user-space table).  Later in the kernel when I initialize the process structures, I will change this to 1 bit and use 2 tables.

I feel like I am relatively close to buttoning up the last of the loader.  I am not yet able to jump to the kernel so I have no clue where I am at there yet -- but I'm sure it's bad.

---

### 2018-Nov-24

Today my plan is to try to get the MMU enabled for the rpi2b archetecture.  Well, more to the point: research how to get the MMU enabled.  Since this archetecture is rather foreign to me, it takes a lot of research for me to understand what is required.

I know that I need to interact with Co-Processor 15 (`cp15`) and I will use `mcr` and `mrc` opcodes to interact with the registers.  There are several registers I will need to interact with for various reasons.
* Register 0 is a read-only register for TLB information.  I am not interested in this register for setting up the MMU.
* Register 1 contains the M bit (whether the MMU is enabled or disabled).  The M bit is bit 0.  I assume the M is set to 0 before I get to it.  I will need to code a test to investigate this.
* Register 2 contains the Translation Table Base registers and control.  There are 3 sub-registers of this:
    * 0 is base 0 -- the one will hold the user-space table mappings.  It will be the same as the kernel-space mappings for the purposes of the loader and late kernel initialization.  All other flags I will set to 0 for the moment, as I am not going to enable caching.
    * 1 is base 1, which will hold the kernel base table.  I will populate this in the loader.  All other flags I will set to 0 for the moment, as I am not going to enable caching.
    * And, 2 is the control register.  In the loader, this will be set to 1 bit, which means that if the most significant bit is 0, TTBR0 will be used and if that bit is set to 1, TTBR1 will be used.  Since they are the same, they will be the same tables.
* Register 3 is Domain Access Control, wihch I am not using.  This will be unchanged.
* Register 4 is reserved, so I will not touch it.
* Register 5 is a Fault Status Register.  It contains 2 sub-registers, neither of which I am using at this point.
* Register 6 is a Fault Address Register (which feels a little like `cr2` from x86 family).  I am not using that yet.
* Register 7 does not exist.
* Register 8 controls the TLB and has several functions.  I will leave all this at the default for now.
* Register 9 does not exist.
* Register 10 is the TLB Lockdown.  Again, I am not controlling this explicitly so I will leave this at the default.
* Register 11 does not exist.
* Register 12 does not exist.
* Register 13 contains the Process ID registers.  These need to be updated with a process change, so this will need to change with the TTBR0 register when swapping processes.  There are 2 sub-registers here, the Process ID and the Context ID.  More on these later.

So, to enable the MMU, I need to perform the following steps:
1. Check the current state of the MMU (Register 1[M]).  If it is already enabled, report so.  Should I disable before moving on?  Probably not since we will probably fault and lock up.
1. Write the TTLR0 value into Register 2/opcode 0.
1. Write the TTLR1 value into Register 2/opcode 1.
1. Write the number of bits to consider for the TTLR evaluation into Register 2/opcode 2 (this will be 1 bit).
1. Enable paging by writing a `1` to Register 1[M].
1. plan for the worst, but hope for the best.

---

I got everything written, and the the system locked up.  It's about what I figured would happen, but now I have to figure out how to debug this since I am not able to output much to the screen.

Step debugging with `gdb` gives me some results with registers:

```
(gdb) info reg
r0             0xc50079 12910713
r1             0xa      10
r2             0xa      10
r3             0xb000000        184549376
r4             0x105000 1069056
r5             0x0      0
r6             0x0      0
r7             0x0      0
r8             0x0      0
r9             0x118000 1146880
r10            0x2d227  184871
r11            0x7b4    1972
r12            0x7a0    1952
sp             0x7a8    0x7a8
lr             0x102fc8 1060808
pc             0x100e78 0x100e78 <MmuEnablePaging+24>
cpsr           0x600001d3       1610613203
fpscr          0x0      0
fpsid          0x410430f0       1090793712
fpexc          0x0      0
(gdb) stepi
^C
Thread 1 received signal SIGINT, Interrupt.
0x0000000c in ?? ()
(gdb) info reg
r0             0xc50079 12910713
r1             0xa      10
r2             0xa      10
r3             0xb000000        184549376
r4             0x105000 1069056
r5             0x0      0
r6             0x0      0
r7             0x0      0
r8             0x0      0
r9             0x118000 1146880
r10            0x2d227  184871
r11            0x7b4    1972
r12            0x7a0    1952
sp             0x0      0x0
lr             0x10     16
pc             0xc      0xc
cpsr           0x600001d7       1610613207
fpscr          0x0      0
fpsid          0x410430f0       1090793712
fpexc          0x0      0
```

First, `pc` has a crazy offset, probably dereferencing a `NULL` value.  `lr` and `sp` also have addresses that appear to be relative to `NULL`.

For the moment, I think the paging tables are built properly.  I did, however, make some assumptions about the TTBR0 and TTBR1 registers.  I will start there.

I reviewed the Domain setup and the default Domain (0b00) access is to throw faults.  So, I will try to correct that and see where that gets me.  And while it needed to be done, it did not solve all my problems.  Now I am looking at the flags for each table level.

---

I can finally confirm that I have been able to enable the MMU and I am executing code after that is complete.  However, the system is still locking up.  I was able to trace that to a stack operation which locks up the system.  The stack is broken on rpi2b.

---

### 2018-Nov-25

I took care of the stack, which had been hard-coded in the loader to be at address `0x800`.  Now I have debugged again and I have problems with the MMIO locations.  These are in user-space starting at address `0x3f...` and really need to be mapped up above `0x80000000`, but also identity mapped for the loader to finish up.

I need to find the proper address range for the MMIO locations, and determine a place to put the IO ports for the kernel.  The addresses for SoC peripherals is from `0x3f000000` to `0x3fffffff`.  I think for the rpi2b kernel, I will map this space into `0xfa000000`.

With that, I am able to get to the point where I am jumping to the kernel.  I am not sure if I make it there without a fault, but everything in the loader appears to be working properly at this point.  Certainly I am able to get the MMU enabled and continue processing, which was a big deal to accomplish.

I know I am making it to the kernel, but I am not getting any kind of greeting message from the kernel to the screen or to the serial port.  I was able to step-debug to confirm this fact.  I am going to commit this code now, since quite a bit has changed.  Then I can focus on completing the kernel for rpi2b.

---

Now, with that out of the way, the first order of business is going to be to get some output from `kprintf()`.  This is both the first thing that needs to take place and the only debugging output apparatus I have for the kernel.

The problem I am starting with is that I am now being driven into a situation where the rpi2b hardware is loader-speciific and I will have another version for the kernel for the kernel.  I do not want to duplicate this work, so I will have to build an abstraction here that works for both the loader and the kernel.  At the same time, I also found a file name duplication for `serial.h` that I need to correct.  Since the exercise is nearly the same, I will take them both on together.

For output to the serial port, I commented out the serial port greeting and was able to get the frame buffer to change colors and write the greeting.  So, I just need to figure out what is happening with the serial port output.  I know that the loader was writing to the serial port properly, but the kernel is trying to write to an adjusted address.

Silly me!  I had the same issue with `==` vs `!=` in the kernel `SerialPutChar()` that I had in the loader and I did not correct both instances.  I am now getting `kprintf()` output.

I am getting a bit of output, but the system still locks.  I know I have lots of stuff stubbed out that all need to get fixed.  The first thing that `kinit()` does is build the IDT for x86.  There is an equivalent process to get a fault handler built for rpi2b and get that installed.  This is what I will need to take on next.  I need to have visibility into the errors when they happen.  There are several things that will all need to take place to get this initialized.  There is an interrupt controller and a number of fault handlers that all need to be figured out and coded.

---

Reading up on how interrupts (exceptions) work, I have figured out that the actual `pc` value can indicate which exception has occurred if the table has not been set up.  In particular, based on the value in `pc`, it can be determined what exception has occurred:
* `0x00` -- reset
* `0x04` -- unused
* `0x08` -- supervisor call
* `0x0c` -- prefetch abort
* `0x10` -- data abort
* `0x14` -- unused
* `0x18` -- IRQ interrupt
* `0x1c` -- FIQ interrupt

These locations can only be at base `0x00000000` or `0xffff0000` and are virtual addresses.  Since we will be using upper addresses for the OS (`0x80000000` and greater), code will need to be moved and mapped to this upper location.  To make matters worse, the only thing that can be done is a single word-sized instruction, which leaves a 24-bit offet to the actual target code.  So, this code must be somewhere about `0xff800000` but no farther away.  Ultimately, I am going to need a specially built page mapped to that address (`0xffff0000`) that contains a table of addresses just a little higher in memory for jumping to the kernel proper.  This will be in the form: `ldr pc,[pc,#0x20]`.  Now, the problem is that this location is buried in the 'pseudo-recursive' MMU tables.  If it is not obviuos, I will need to rewrite the loader's MMU initialization -- and I need to take this task on now before I get into the interrupt/exception initialization.

At the same time, I need to revisit the virtual memory map as I would like to be able to align the 32-bit architectures better than I have at this point.  I have created discrepancies in the memory map from above and the one in README.md.  The target on README.md takes precedence, but it will take some time to refactor the x86 architecture to properly align.  I indend this to be completed in v0.2.1 and will create the Redmine version now to accomplish this.

---

After making several changes to create constants for the hard-coded values, I was able to change the location of the TTL1/TTL2 tables to `0x80400000` and `0x80000000` respectively.

This is a change since the TTL1 tables now come *after* the TTL2 tables whereas this was originally programmed to be *before*.  However, a test shows that this still works by only changing the constant values.  This is a good thing.  I need to do more of this!  This now will free up the memory required to set up the interrupt table.

I want to call out here that I believe I will be coding something that makes no sense and need to call it out.  The ARM ARM section 5.2.2 calls out that if `r15` or `pc` is used as the base register for loading an address, the actual value used in `pc + 8`, or "the address of the instruction plus 8".

---

### 2018-Nov-26

My UPS just ate itself.  It took a while to reboot everything, but so far it has all come back up properly.

Now, to get back to the interrupt table.  I need to determine if I am going to build this statically and relocate it or build it dynamically in the proper location.  Each of the `ldr` instructions would be the same, so it would not be too much of a stretch to build it during initialization.  I would just need to determine the actual instruction bits.  This should be able to be done with a simple `.s` file.

This quick file gave me the results I was looking for:

```
intReset:
    ldr     pc,[pc,#(8-8)]
intAbort:
    ldr     pc,[pc,#(8-8)]

resetTarget:
    .word   kInit
    .word   kInit
```

That is to say the `ldr` opcodes were loading the proper addresses.

```
80005f0c <intReset>:
80005f0c:	e59ff000 	ldr	pc, [pc]	; 80005f14 <resetTarget>

80005f10 <intAbort>:
80005f10:	e59ff000 	ldr	pc, [pc]	; 80005f18 <resetTarget+0x4>

80005f14 <resetTarget>:
80005f14:	800056a0 	andhi	r5, r0, r0, lsr #13
80005f18:	800056a0 	andhi	r5, r0, r0, lsr #13
```

It just so happens that `#(8-8)` is what I was playing with for this test, but the real structure will be something like `#(0x20-8)`.  Or, much more to the point: `0xe59ff018`.

---

I wrote functions to handle the MMU mappings.  The problem with them at this point is that I am only dealing with the kernel space at the moment -- not the user space mappings from `0x00000000` to `0x7fffffff`.  Well, they are all put into the kernel space maps.

I was also be able to write and document the `InterruptVector_t` structure.

---

### 2018-Nov-27

I am short on time today, so the task today is to determine how to relocate the interrupt vector table from `0x00000000` to `0xffff0000`.  This has something to do with a control register, and I presume `cp15` before I even start reading.  But, that is the research task of the day.  Sorry, it's called an "exception vector table".  I have been writing it wrong.

I see that the SCTLR register, bit 13 controls the Vector Base Address Register value.  The SCTLR is cp15,c1.  When this bit is set to 0, the address is `0x00000000` and software can remap the location using the VBAR.  On the other hand, when this value is 1, the address is `0xffff0000` and software cannot remap this value.  Hmmm...  am I missing something?

It looks like I can read the VBAR register with the following opcode:

```
mrc p15,0,r0,c12,c0,0
```

writing this register back out with `mcr` should allow me to set a fixed address for the exception vector table.

So, I asked this question on `freenode#osdev` and got the following response:

```
[19:55] <eryjus> For those who know the RPi, am I reading the documentation right for the exception vector table (VBAR)?  If I leave p15:c1:13 clear, I can set p15:c12 to any address I want and I am not limited to 0x0 and 0xffff0000?
[19:55] <geist> eryjus: yes
```

This means that I am actually able to move the TTL tables back to `0xffc00000` (less 4 frames) and I can put the exception vector table in the same location as the IDT for x86.  This will allow me to move the heap as well back to `0x80000000` and the MMIO addresses to the top of the kernel data location.  This far better aligns with much of the existing x86 architecture and I will not have to perform major surgery to the x86 code to get things aligned properly.

So, with that said, tomorrow I will make the attempt to write to those registers to see if I can get the addresses set proeprly.

---

### 2018-Nov-28

Going back to setting this exception vectory table address, the base virtual address of the GDT/TSS/IDT in the x86 family is `0xff401000`.  All 3 structures are located in this same frame in x86.  I want to use that same address for the exception vector table for rpi2b.  So this is what I will attempt to set up and then create a fault to test it.

Well, when I set the registers, there was no error.  I then set up for a test to generate a fault by reading a bad address: `x = *(uint32_t *)0x89723984;`, and this generates a fault, but the address I want to see in the `pc` register is not correct.

My function to set this up looks like this:

```
IdtSetAddr:
    mrc     p15,0,r0,c1,c0,0
    and     r0,r0,#(~(1<<13))
    mrc     p15,0,r0,c1,c0,0

    mov     r0,#0xff40
    lsl     r0,#12
    add     r0,#0x1000
    mrc     p15,0,r0,c12,c0,0

    mov     pc,lr
```

Of course, I confused the `mrc` and `mcr` opcodes.  Correcting those solved my issue.

```
(gdb) info reg
r0             0xff41000        267653120
r1             0xa      10
r2             0xa      10
r3             0x89723984       -1989002876
r4             0x80009000       -2147446784
r5             0x0      0
r6             0x0      0
r7             0x0      0
r8             0x0      0
r9             0x11a000 1155072
r10            0x2d227  184871
r11            0x80024fc4       -2147332156
r12            0x118f98 1150872
sp             0x0      0x0
lr             0xff41010        267653136
pc             0xff4100c        0xff4100c
cpsr           0x600001d7       1610613207
fpscr          0x0      0
fpsid          0x410430f0       1090793712
fpexc          0x0      0
```

---

I found an endian-ness problem with my hard-coded jump instruction.  I got that fixed and now the faulting `lr` is `0xff042004`.  This means I managed to execute something, but not sure how much and what took place.

---

It's getting late and I'm not getting anywhere.  I'm going to call it a night and try again tomorrow.

---

### 2018-Nov-29

I have been thinking about tooling.  I am wondering if there are any better (free) ARM emulators.

After asking on `freenode#osdev`, I now have access to the debugger (`-monitor stdio`) and I found that I have an endian-ness problem.  I switched it back.  The behavior is the same, but I get farther down the code before I run into problems.  Here is the output from the qemu logs:

```
0x80003060:  e8bd8830  pop      {r4, r5, fp, pc}

Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x9600003f
...with DFSR 0x5 DFAR 0x89723984
----------------
IN:
0xff401010:  e59ff018  ldr      pc, [pc, #0x18]

----------------
IN:
0x80002c54:  e92d4800  push     {fp, lr}
0x80002c58:  e28db004  add      fp, sp, #4
0x80002c5c:  e59f300c  ldr      r3, [pc, #0xc]
0x80002c60:  e08f3003  add      r3, pc, r3
0x80002c64:  e1a00003  mov      r0, r3
0x80002c68:  eb000e45  bl       #2147509636

Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x9600007f
...with DFSR 0x805 DFAR 0xfffffff8
```

This tells me that the Exception Vector Table worked and my Data fault handler really did get control.  However, there appears to be no stack set up.  I have more reading to do, but no time tonight.  But I do think the best thing is to take on getting a proper register dump written so I can debug better.

The Stack Pointer (`r13`) is indeed clear.  I need to determine why.

---

### 2018-Nov-30

I started searching last night and found it today.  The stack register (`sp` or `r13`) is replaced on an abort.  This table of banked registers is important to keep in mind -- it is in the ARM ARM on page A2-5 and is Figure A2-1.

In trying to get access to the `cspr` and `sp` from the exception, I realize that I am in supervisor mode and probably want to be in system mode.  The reason for this is that I will want access to the registers from the interrupt.  Since the registers are banked, each mode has its own register.  When I am handling an interrupt, I probably want to switch into supervisor (svc) mode.

The following link provides an example:  https://github.com/littlekernel/lk/blob/master/arch/arm/arm/exceptions.S#L75.

I am able to get a dump of the registers, but I cannot guarantee I have all the right registers.  I need to continue to work with my code and the instructions with the example above.

---

### 2018-Dec-01

At the moment, I am changing into system mode as soon as the kernel gets control.  That is a bit too early and I really want to be in svc mode for most of the initialization, changing to system mode right at the end of iniitializaiton, before assuming the butler role.

I also do not have a ton of time today, so all I am able to get done is to stub out the rest of the exception handlers to that they all print registers and lock the system to review.

Christmas party for about 100 people today....

---

### 2018-Dec-02

This morning, I am calling my `IdtBuild()` function complete.  Well, mostly.  There will be some changes as I get other things up and running (the timer is a prime example).  But I am not going to take those changes on now.

Also, there is no TSS to initialize with rpi2b, so that is done.  The rest of phase 1 initialization is related to greeting the user and updating the frame buffer.  Those are done as well.  So, I can call phase 1 initialization complete.

The next thing up is to initialize the process structures for rpi2b.  I expect that there will be some changes needed for the architecture -- in particular the things that need to be saved on the stack.

I commited this code.

---

My current i686 process structure looks like this:

```C
//
// -- This is a process structure
//    ---------------------------
typedef struct Process_t {
    regval_t esp;                       // This is the process current esp value (when not executing)
    regval_t ss;                        // This is the process ss value
    regval_t cr3;                       // This is the process cr3 value
    PID_t pid;                          // This is the PID of this process
    ptrsize_t ssAddr;                   // This is the address of the process stack
    size_t ssLength;                    // This is the length of the process stack
    char command[MAX_CMD_LEN];          // The identifying command, includes the terminating null
    size_t totalQuantum;                // This is the total quantum used by this process
    ProcStatus_t status;                // This is the process status
    ProcPriority_t priority;            // This is the process priority
    int quantumLeft;                    // This is the quantum remaining for the process (may be more than priority)
    bool isHeld;                        // Is this process held and on the Held list?
    struct Spinlock_t lock;             // This is the lock needed to read/change values
    ListHead_t::List_t stsQueue;        // This is the location on the current status queue
    ListHead_t lockList;                // This is the list of currently held locks (for the butler to release)
    ListHead_t messages;                // This is the queue of messages for this process
    void *prevPayload;                  // This is the previous payload in case the process did not allocate enough
} __attribute__((packed)) Process_t;
```

The first 3 fields in this structure are the specific to the x86 architecture.  They have little to no bearing on the rpi2b architecture.  The `esp` could be renamed to the `stackPointer` and then it would be relevant.  `cr3` could be renamed to `pageTables` and this would allow me to store the `TTBR0` value.  `ss` is totally irrelevant.  But I also can't help thinking that there are additional fields I need for the rpi2b.  I remember that there is a "process ID" register (or something like it) that needs to get updated at the same time as the `TTBR0`, and I will need to store that somewhere as well.

I'm going to start by renaming those 2 fields and comiling to address the fall-out from that change.  Note, I am not changing the position of any field.  Just the name.

---

It appears (though I am not totally convinced) the the Context ID registers in `cp15` are related to the Fast Context Switch Extension (FCSE) and are optional to what I am going to be doing.  So, I should not really need to save any other registers in the `Process_t` type.  It was an easy task to get the `ProcessInit()` function written.  It merely establishes the structures but I do not yet have enough of the system built to actually exercise these structures and work out any issues.  I think I am going to have to make an effort to organize the pushes properly to make sure I have the `cpsr` on the stack and room to get this value in the rgisters.  I will have to take this on once I get the Timer initialized.

Now, with that said, I have no clue what the Timer looks like in rpi2b.

---

I now know I need to read the CBAR to determine the base address register.  This is located in `cp15` in register `c15`.  Then, at offset `0x1000` I can then get access to the GIC.  Once I have the GIC, I can use that to configure the interrupts from the timer.

---

### 2018-Dec-03

Well, this is the last day before I take vacation.  I am not sure what I will be able to do while I am on vacation since my wife is one of those that measures the success of a vacation based on how tired we are when we get back....

For the short time I have, I will be looking at the Generic Interrupt Controller (GIC).  I think the first thing to do is to confirm I have a proper GIC.  This should be able to be done by reading `(HW_BASE + 0x1000 + 0x008)` or the `GICD_IIDR`.  The value in this read-only register should be `0x0100143b`.

---

Hmmm....  I'm not finding the GIC...  I notice the following the the TRM:

> Memory regions used for these registers must be marked as Device or Strongly-ordered in the translation tables.

> Memory regions marked as Normal Memory cannot access any of the GIC registers, instead access caches or external memory as required.

This means I will need to update my MMU initialization to set this to be device memory.  I have 2 unused parameters that I can leverage for this, so it should be a simple update to make this change.

---

So reading further, it appears that the BCM2836 SoC does not have a GIC included with it.  Well, that makes a mess.  I need to go back through and figure out how I will be handling interrupts.

---

### 2018-Dec-04

Not going to be able to do much tonight.  However, I am going to try to work on getting the timer working and firing interrupts.  I really do not expect too much in the way of accomplishments.

Another thing I might work on is the initialization sequence in the loader, which is not really 100% correct.

---

## 2018-Dec-05

This morning I was able to narrow this down to a timer problem, not an interrupt problem.  I did this by printing the polled status of the interrupt pending register.  I will start with that assumption and clean that up first.

---

I was able to finally get an interrupt.  I was able to get the interrupt by enabling every interrupt possible:

```C
    MmioWrite(INT_IRQENB0, 0xffffffff);
    MmioWrite(INT_IRQENB1, 0xffffffff);
    MmioWrite(INT_IRQENB2, 0xffffffff);
```

From here I am going to use the process of elimination to determine which interrupt it is.

I find that the bit that enables the IRQ is in (INT_IRQENB2|1<<25), or INTENB2IRQ53.  This is i2c_int.

Well, I have the interrupt firing....  Now, I want to make sure I get the registers processes properly so that I can output them properly.

Comparing the output of the qemu logs to that of what is output to the serial port, the qemu logs show:

```
IN:
0x80002d58:  f10e01d3  cpsid    aif, #0x13

R00=60000013 R01=02000000 R02=02000000 R03=f200b214
R04=8000a000 R05=00000000 R06=00000000 R07=00000000
R08=00000000 R09=0011b000 R10=0002d227 R11=80028fcc
R12=00119f98 R13=00000000 R14=8000017c R15=80002d58
PSR=60000192 -ZC- A NS irq32
```

... and the serial logs show:

```
IRQ:
 R0: 0x60000013   R1: 0x02000000   R2: 0x02000000
 R3: 0xf200b214   R4: 0x8000a000   R5: 0x00000000
 R6: 0x00000000   R7: 0x00000000   R8: 0x00000000
 R9: 0x0011b000  R10: 0x0002d227  R11: 0x80028fcc
R12: 0x00119f98   SP: 0x00000000   LR: 0x80028fb4
CPSR: 0x00000000
```

This shows that only the `lr` and `cpsr` registers are incorrect, and the `pc` register is not captured.

---

There are a few things I need to make sure I get in line so that I am coding properly.  For the record, I am referring to this example code to make sure I get things in line: https://github.com/littlekernel/lk/blob/master/arch/arm/arm/exceptions.S.

From this data, I am compiling a table of the what is located where so that I can build the stack properly going in and restore the system state properly departing.

| Position |   FIQ    |   IRQ    | Data Abort | Pref Abort |   SVC    |   UND    |
|:--------:|:--------:|:--------:|:----------:|:----------:|:--------:|:--------:|
|   +4c    | spsr_fiq | spsr_irq |  spsr_dta  |  spsr_pre  | spsr_svc | spsr_und |
|   +48    | lr_fiq-4 | lr_irq-4 |  lr_dta-8  |  lr_pre-4  |  lr_svc  |  lr_und  |
|   +44    | fiq_type | irq_type |  dta_type  |  pre_type  | svc_type | und_type |
|   +40    |    r0    |    r0    |     r0     |     r0     |    r0    |    r0    |
|   +3c    |    r1    |    r1    |     r1     |     r1     |    r1    |    r1    |
|   +38    |    r2    |    r2    |     r2     |     r2     |    r2    |    r2    |
|   +34    |    r3    |    r3    |     r3     |     r3     |    r3    |    r3    |
|   +30    |    r4    |    r4    |     r4     |     r4     |    r4    |    r4    |
|   +2c    |    r5    |    r5    |     r5     |     r5     |    r5    |    r5    |
|   +28    |    r6    |    r6    |     r6     |     r6     |    r6    |    r6    |
|   +24    |    r7    |    r7    |     r7     |     r7     |    r7    |    r7    |
|   +20    |    r8^   |    r8    |     r8     |     r8     |    r8    |    r8    |
|   +1c    |    r9^   |    r9    |     r9     |     r9     |    r9    |    r9    |
|   +18    |   r10^   |   r10    |    r10     |    r10     |   r10    |   r10    |
|   +14    |   r11^   |   r11    |    r11     |    r11     |   r11    |   r11    |
|   +10    |   r12^   |   r12    |    r12     |    r12     |   r12    |   r12    |
|    +c    |    sp^   |    sp    |     sp     |     sp     |    sp    |    sp    |
|    +8    |  lr_svc  |  lr_svc  |   lr_svc   |   lr_svc   |  lr_svc  |  lr_svc  |
|    +4    |  sp_usr  |  sp_usr  |   sp_usr   |   sp_usr   |  sp_usr  |  sp_usr  |
|    r0    |  lr_usr  |  lr_usr  |   lr_usr   |   lr_usr   |  lr_usr  |  lr_usr  |

Given time tomorrow, I will start to commit this to code.  I will need both the entry and the exit coded as the goal is to be able to have the timer IRQ fire and then be able to continue processing.

---

### 2018-Dec-06

Well, I was able to get the interrupt to fire repeatedly and dump the contents of the registers to the serial port with each iteration.  I am not totally convinced that the contents are 100% corect, but it works for now.  I need to copy the logic to the other exception handlers, but the core logic works.  It's also important to note that the above table is not 100% accurate already.

---

Well, it looks like the exception handlers are degrading into several data exceptions.  So, I will need to get to the bottom of this -- I may not have had that as well dialed in as I thought.  Well, it turns out I was not setting r0 to be the address of the registers on the stack, a simple fix.

I have built out the IRQ dispatch table for the rpi2b.  I need to register a timer callback function and then I should be able to get the scheduler started.

---

I have added the `TimerCallback()` function to the `IsrHandler` table and ended up with a Data Fault, with the following data:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
The CBAR reports the base address as 0x3f000000
Setting the scalar value to 0x83126e for frequency 0xfa
Data Exception:
At address: 0x8002af3c
 R0: 0xfffffe3f   R1: 0xfffffe3f   R2: 0x00000035
 R3: 0x80041024   R4: 0x8000a000   R5: 0x00000000
 R6: 0x00000000   R7: 0x00000000   R8: 0x00000000
 R9: 0x0011b000  R10: 0x0002d227  R11: 0x8002afac
R12: 0x00119f98   SP: 0x800036ec   LR_ret: 0x8000678c
SPSR_ret: 0xf800011f     type: 0x17

Additional Data Points:
User LR: 0x00000000  User SP: 0x00000000
Svc LR: 0x8002af4c
```

So, I am back to debugging.  Starting with the return `lr`, the function is `RestoreInterrupts()`.  So far, the only place I think this is called in when registering an ISR handler.  I want to make sure I am getting through that function properly.  The fault is generated from `RestoreInterrupts()`, so I need to clean that up.

---

### 2018-Dec-09

My IRQ handler appears to be getting multiple interrupts at once, or I am not really clearing out the interrupt.  Tracing the interrupts that are left over, I have something from register 2 (bit 9) and something from the uart (bit 19).  However, what is clear is that I need to ensure that I am comparing one bit at a time rather than expecting only a single bit to be set.  I also believe I am having trouble with actually acknowledging an interrupt (or in x86 work issuing an End of Ineterrupt).

The first order of business is to change `IsrHandler()` from being driven by a `switch` statment to a number of `if-then-else` statements.

---

At the moment my timer has stopped firing, and the uart is firing with every character written, even though I think I have asked it not to and even masked it out.

---

### 2018-Dec-10

I am still trying to get the timer working.  I am still on vacation and working remotely on this.  I fly home tomorrow and will get into some really hard debugging when I return.  However, in the meantime, I am working on trying to identify the proper location for this timer in the qemu source code.

---

### 2018-Dec-11

Well, it appears that I have been reading the incorrect documentation.  I need to be looking at the BCM2836 SoC and instead I have been reading the BCM2835 SoC docs.

So, with that said, I will need to revisit the `TimerInit()` function to make sure I am using the correct locations to manage the timer.

---

And after trying again with the new (proper?) register locations, I am still not getting anywhere.  I need to sleep on this again.

---

### 2018-Dec-12

Well, still no success here.  I did reach out on `#osdev` for some help....

With no quick reply, I am going back to basics.  I believe that I am working without a GIC.  I need to confirm this first.

OK, so I read and updated what I thought was the GIC Distributor enable bit and read back the results.  This came back as `0x00`.  So, this tells me I am updating the wrong address or there is no GIC.  I still need to research a bit more.  Instead, I will read the `GICD_IIDR` register and report its results and is a read only register.

So, the CBAR (Configuration Base Address Register) holds the location of the GIC (`0x3f000000`), and the Distributor is located at offset `0x1000` (`0x3f001000`), I can conclude that I am reading the correct location (this is identity mapped still).  So, I am able to conclude that I do not have a GIC in this system, which is what I thought.

So, in the BCM2836 documentation, there is a timer prescalar which has a reset value of 0.  When that is 0 there is in effect a divide-by-zero situation that can occur.  I am not sure how the CPU handles this condition, but the reset value is reported as 0.  So, I will try setting a value for this register.  I wrote 1, which made no difference in behavior.

---

Reviewing the qemu source, I see the following code for initializing the `raspi2` machine (which I am using):

```C
        /* Connect timers from the CPU to the interrupt controller */
        qdev_connect_gpio_out(DEVICE(&s->cpus[n]), GTIMER_PHYS,
                qdev_get_gpio_in_named(DEVICE(&s->control), "cntpnsirq", n));
        qdev_connect_gpio_out(DEVICE(&s->cpus[n]), GTIMER_VIRT,
                qdev_get_gpio_in_named(DEVICE(&s->control), "cntvirq", n));
        qdev_connect_gpio_out(DEVICE(&s->cpus[n]), GTIMER_HYP,
                qdev_get_gpio_in_named(DEVICE(&s->control), "cnthpirq", n));
        qdev_connect_gpio_out(DEVICE(&s->cpus[n]), GTIMER_SEC,
                qdev_get_gpio_in_named(DEVICE(&s->control), "cntpsirq", n));
```

This is happening for each core.  Therefore, I looked to see what the `cntpsirq` meant.  The ARM TRM states:

> Secure physical timer event

... while the `cntpnsirq` is the non-secure version.  So, I think I need to set up to use this timer -- at least for qemu.

---

So, I wrote the following debug code and have some results from it:

```C
    // -- this results on no change
    kprintf("Set the control register from %p\n", MmioRead(TMR_BASE + 0x00));
    MmioWrite(TMR_BASE + 0x00, 0x00);

    // -- I do not believe the documetation is correct on this
    kprintf("Set the prescalar register from %p\n", MmioRead(TMR_BASE + 0x08));
    MmioWrite(TMR_BASE + 0x08, 0x80000000);

    // -- Shows as 0 (unimplemented?)
    kprintf("The core timer is currently %p : %p\n", MmioRead(TMR_BASE + 0x20), MmioRead(TMR_BASE + 0x1c));

    // -- Set the GPU routing flags -- results in no change
    kprintf("Set the GPU Routing from %p\n", MmioRead(TMR_BASE + 0x0c));
    MmioWrite(TMR_BASE + 0x0c, 0x00);

    // -- Here we set the routing for the timer
    kprintf("Set the timer routing from %p\n", MmioRead(TMR_BASE + 0x40));
    MmioWrite(TMR_BASE + 0x40, 0x00000020);

    // -- Additional routing for the timer -- results on no change
    kprintf("Route the local timer to core 0/IRQ changed from %p\n", MmioRead(TMR_BASE + 0x24));
    MmioWrite(TMR_BASE + 0x24, 0x00);

    // -- Set the core interrupt sources
    kprintf("Set the core 0 interrupt sources from %p\n", MmioRead(TMR_BASE + 0x60));
    MmioWrite(TMR_BASE + 0x60, 0x00000020);

    // -- Enable the timer and interrupt (reload value of 0x100)
    kprintf("Enable the timer/interrupt from %p\n", MmioRead(TMR_BASE + 0x34));
    MmioWrite(TMR_BASE + 0x34, 0x30000100);

    // -- Reload and reset timer
    kprintf("Reload and reset timer from %p\n", MmioRead(TMR_BASE + 0x38));
    MmioWrite(TMR_BASE + 0x38, 0xc0000000);


    kprintf("The core timer is currently %p : %p\n", MmioRead(TMR_BASE + 0x20), MmioRead(TMR_BASE + 0x1c));
```

... resulting in

```
Set the control register from 0x00000000
Set the prescalar register from 0x00000000
The core timer is currently 0x00000000 : 0x00000000
Set the GPU Routing from 0x00000000
Set the timer routing from 0x00000000
Route the local timer to core 0/IRQ changed from 0x00000000
Set the core 0 interrupt sources from 0x00000000
Enable the timer/interrupt from 0x00000000
Reload and reset timer from 0x00000000
The core timer is currently 0x00000000 : 0x00000000
```

One thing I notice from this is that all the registers are starting at 0.  This may or may not be a bad thing, but I need to check that my updates are being maintained.

Checking my updates results in the following:

```
Set the control register from 0x00000000
Set the prescalar register from 0x00000000
... checking the update: 0x00000000
The core timer is currently 0x00000000 : 0x00000000
Set the GPU Routing from 0x00000000
Set the timer routing from 0x00000000
... checking the update: 0x00000020
Route the local timer to core 0/IRQ changed from 0x00000000
Set the core 0 interrupt sources from 0x00000000
... checking the update: 0x00000000
Enable the timer/interrupt from 0x00000000
... checking the update: 0x00000000
Reload and reset timer from 0x00000000
The core timer is currently 0x00000000 : 0x00000000
```

The only thing that is properly updated is the 'Core0 Interrupt Source` register.  The rest appear to be unimplemented by qemu.

---

### 2018-Dec-13

OK, I have some progress today!  Not sure if I can turn that into results yet, but I have figured out where I should be looking.  I think.

I started looking into the Raspbian source.  I found 2 files that are directly linked to the BCM2836 chip and more to the point to the timer I am trying to get running.  These are:
* https://github.com/raspberrypi/linux/blob/3b01f059d2ef9e48aca5174fc7f3b5c40fe2488c/include/linux/irqchip/irq-bcm2836.h
* https://github.com/raspberrypi/linux/blob/3b01f059d2ef9e48aca5174fc7f3b5c40fe2488c/drivers/irqchip/irq-bcm2836.c

Both of these files together demonstrate that that I am looking in the correct spot for implementing this timer stuff.  So, I started wondering about qemu and if it is supported or not.  Well the file `hw/intc/bcm2836_control.c` shows that it is implemented.

So, this is a per-core timer and I just need to set it up properly to get the results I am looking for....  Easy, right???!!

The first thing that Raspbian does is initialize the timer frequency.  This is to register LOCAL_PRESCALAR or offset `0x008`.  However, there does not appear to be a relevant register in the qemu emulation.  I will want to do this on real hardware, but the subsequent read will not return anything of value.  This agrees with my results from yesterday.

Further on the setting up of the frequency, the calculation as documented in the Raspbian code is: `timer_freq = input_freq * (2 ^ 31) / prescalar`.  The `input_freq` is 19.2MHz.  Raspbian sets this to a 1:1 against the crystal clock, resulting in 19.2 MHz, and an increment of 1.  I believe (but have not confirmed) that the qemu frequency is set to the emulation loop frequency only.

After that, Raspbian then goes through the trouble to register the IRQs for each of the 4 timer events: Hypervisor physical timer event, Non-secure physical timer event, Secure physical timer event, and Virtual timer event... plus the GPU Fast IRQ and PMU Fast IRQ (which I am not dealing with at the moment).  I do not believe this really does much with the hardware but instead registers handlers in the internal kernel structures.  At which point the initialization is complete.

Now, there are 2 functions `bcm2836_arm_irqchip_mask_per_cpu_irq()` and `bcm2836_arm_irqchip_unmask_per_cpu_irq()` which do some bit twiddling for the `LOCAL_TIMER_INT_CONTROL0` register.  But the point here is that there is really nothing fancy going on here.  This equates to the routing for the timer at offset `0x40` which is implemented in qemu and is able to be read back properly (as I saw yesterday).

---

So, I am wondering if I don't have some other problem like a permissions thing with the paging tables.  I may have to dig a bit deeper into the qemu code to see what might be preventing this from firing.  I might even have to write some debugging output into qemu and recompile it.

---

It dawns on me that the paging can be eliminated as a cause for problems by writing a special test into the loader -- before paging is enabled.  In this test, I would need to set up the irq table and duplicate some of that setup.  This would all be temporary of course.  Or, better yet, maybe I need a special purpose program that will exercise the timer.  I will sleep on this thought and pick this back up in the morning.

---

### 2018-Dec-14

I thought a lot about this last night and today during my day job.  If I want do to bare metal, I will need to actually start up a new archetecture that bypasses `rpi-boot`.  However, I think my problems lie in the MMU layer.  If this is the case, all I need to do is create a new module that is rpi2b only and create a binary for that module.  I can add that to the `grub.cfg` file and then boot to that module easy enough for testing.  I will start with the approach so that I can try to get a working timer.  Now, how will I know if I do?  I do not have an act led I can flash in QEMU and I am completely unprepared for real hardware at this point.

I have routines that I can copy from the loader to output to the serial port.  This will also generate IRQs so that I can check that the IRQs are working properly as well.  With this, I think there is enough to set a basic plan for working with hardware without the MMU getting in the way.

A couple of things for the record:
* I will be committing this module as well
* `rpi-timer` will be the module's name

---

My first obstacle is that the `rpi-boot` tools does not support menuing.  Therefore, I cannot have this selected by menu option at boot time.  I could hack `rpi-boot` to allow this, but not today.  The alternative is to make changes in `bin/rpi2b/boot/grub/Tupfile` to allow the alternative and for now comment and uncomment these steps.

---

Ok, my first attempt to get the timer to fire has not worked.  I really did not expect it to, but I was hopeful.  The code is very simple, with most of it related to setting up the serial port for debugging.  The first thing I want to do for debugging is to set up the UART to generate interrupts so that I can test whether my IVT is configured right.

This test yielded no interrupts.  So, I can only conclude I have a problem with my IVT setup somehow.

So, I am getting an IRQ.

```
Taking exception 5 [IRQ]
...from EL1 to EL1
...with ESR 0x13/0x4e000000
----------------
IN:
0x00000018:  e59ff018  ldr      pc, [pc, #0x18]
```

This is good.  But then right after this:

```
----------------
IN:
0xe24ee004:  00000000  andeq    r0, r0, r0

----------------
IN:
0xe24ee008:  00000000  andeq    r0, r0, r0

----------------
IN:
0xe24ee00c:  00000000  andeq    r0, r0, r0
```

The code runs off to nowhere and executes a bunch of `0x00000000` values!  What I need now is a .map of my `rpi-timer.elf` code.  Well, I had it, created automatically.

---

The problem appears to have been in the packing of the IVT.  Adding that (and one other change to types) cleared up the problem and I am getting IRQs.  Now, with the serial port generating IRQs and the timer (possibly) generating IRQs, I need to start filtering this down so that I know where everything is coming from.  First, I know where the UART IRQ is coming from, so I will start by disabling that right after enabling it.

It looks like I am really getting a timer IRQ.

Here is the code that is enabling the IRQs:

```C
    // -- Enable all IRQs
    MmioWrite(INT_IRQENB0, 0xffffffff);
    MmioWrite(INT_IRQENB1, 0xffffffff);
    MmioWrite(INT_IRQENB2, 0xffffffff);
    MmioWrite(INT_IRQDIS2, 0x02000000);             // immediately disable the serial IRQ
```

So, let's narrow down which one really needs to be enabled.  Starting by eliminating `INT_IRQENB2`.  And eliminating that stopped my interrupts.  But I am still getting all my debugging output:

```
Serial port initialized!
Interrupt Vector Table is set up
Timer is initialized -- interrupts should be happening
```

So, the interrupt I am looking for is in `INT_IRQENB2`.  I believe that this coming from IRQ54, based on some previous experiments.  I'm going to check that first.  But that does not enable those interrupts.  Double checking my previous test, though, I confirmed that the interrupt is coming from `INT_IRQENB2` somewhere (also meaning there is no point in investigating `INT_IRQENB0` or `INT_IRQENB1` any further).

Now on the other hand, I am not getting the final message written, so I may not be actually getting to the end of the code and really really getting a timer to fire.  This could really be recursive UART interrupts.  The solution to this is to figure out and filter out any UART interrupts and not perform a `SerialPutChar()` function for a UART interrupt.

Yes, as a matter of fact, when I filter out the IRQ for the UART, I was only getting a false sense of security on the timer interrupt.

Well, hang on!  My debugging output is still missing the last line:

```
Serial port initialized!
Interrupt Vector Table is set up
```

Could it be that I am just burying the qemu emulator in interrupts and it cannot get to the end point??  I can test this by changing the scalar value to something very low, say 0x100 and should be able to at least see the final line of code.  No, that's not the answer.  I need to go back to look at the UART IRQ.  I need to disable that again.  That got me to the end.  I believe that my problem is that I am not resetting that interrupt request, meaning that I am caught in a recursive interrupt -- the interrupt is handled but as soon as interrupts are enabled again there is another one to handle because there has not been a reset.  I will focus on getting that addressed.

I finally had to clear the interrupt pending flags in `UART_BASE + UART_ICR` to get the flooding to stop.  I am now getting to the end of my debugging output again, but I still have a lower presacalar value.  The higher value has no impact.

So, this means I am no longer getting the UART IRQs and I am not getting any timer interrupts...  even though everything is enabled.

Let me think this through (sticking my registers to core 0 only and typing all the addresses as offsets from `0x40000000`):
* `0x40` is routes the individual timers to the proper IRQ or FIQ for each of the 4 timer classes.  I am setting this in my code.
* `0x60` is the IRQ source for the core.  This is not being set in my code, and I believe it should be.
* `0x34` enables the timer and it is not being set in my code.  It probably should be.
* `0x24` sets up the local timer routing to IRQ/FIQ for which core.  The default is to route to IRQ on core 0 which is what I want.  I do not believe there is a need to set this register.

So, I have 2 registers I can try to set up.  I wil take them in turn, starting with `0x34`.  This did not work, so I will disable this line and try register `0x60`.  This did not work either, so now to try them both.  And that did not work either.

---

OK, so with this advice from `#osdev`:

```
[19:17] <mrvn> have you tried using an actual RPi2?
[19:17] <mrvn> make sure your code works before trying to fiddle with the emulator.
```

I am now trying to get a working bootable NOOBS image for my real hardware.  The problem here is what is in the root directory and what to replace with my code.  A directory listing shows this:

```
[adam@adamlt usb]$ ls
total 34782
drwxr-xr-x. 5 root root    16384 Dec 14 20:16 .
drwxr-xr-x. 4 root root     4096 Dec 14 20:05 ..
-rwxr-xr-x. 1 root root    23315 Nov 13 09:09 bcm2708-rpi-0-w.dtb
-rwxr-xr-x. 1 root root    22812 Nov 13 09:09 bcm2708-rpi-b.dtb
-rwxr-xr-x. 1 root root    23071 Nov 13 09:09 bcm2708-rpi-b-plus.dtb
-rwxr-xr-x. 1 root root    22589 Nov 13 09:09 bcm2708-rpi-cm.dtb
-rwxr-xr-x. 1 root root    24115 Nov 13 09:09 bcm2709-rpi-2-b.dtb
-rwxr-xr-x. 1 root root    25311 Nov 13 09:09 bcm2710-rpi-3-b.dtb
-rwxr-xr-x. 1 root root    25574 Nov 13 09:09 bcm2710-rpi-3-b-plus.dtb
-rwxr-xr-x. 1 root root    24087 Nov 13 09:09 bcm2710-rpi-cm3.dtb
-rwxr-xr-x. 1 root root    52116 Nov 13 09:09 bootcode.bin
-rwxr-xr-x. 1 root root      303 Nov 13 09:09 BUILD-DATA
drwxr-xr-x. 3 root root     2048 Nov 13 09:09 defaults
-rwxr-xr-x. 1 root root     2356 Nov 13 09:09 INSTRUCTIONS-README.txt
drwxr-xr-x. 2 root root     2048 Nov 13 09:09 os
drwxr-xr-x. 2 root root    12288 Nov 13 09:08 overlays
-rwxr-xr-x. 1 root root  3060512 Nov 13 09:09 recovery7.img.sav
-rwxr-xr-x. 1 root root       85 Dec 31  1979 recovery.cmdline
-rwxr-xr-x. 1 root root   677988 Nov 13 09:09 recovery.elf
-rwxr-xr-x. 1 root root        0 Nov 13 09:09 RECOVERY_FILES_DO_NOT_EDIT
-rwxr-xr-x. 1 root root  2995120 Nov 13 09:09 recovery.img
-rwxr-xr-x. 1 root root 28569600 Nov 13 09:09 recovery.rfs
-rwxr-xr-x. 1 root root     9728 Nov 13 09:09 riscos-boot.bin
```

Notice I have renamed the `recovery7.img` file to `recovery7.img.sav`.  NOOBS no longer boots, so I should be able to replace the `recovery7.img` file with my own version of that and be able to test.

---

OK, I'm wrapping up for the night.  I was able to find several samples from an old git repo (https://github.com/dwelch67/raspberrypi.git) that was already on my system.  None of them appear to implement a timer like I want, but there was a UART program to blast characters so that I could debug the hardware setup.  I finally an able to confirm that my hardware and the `screen` command are working -- I can get the characters on the screen.  The bit rate is 115200, so I will have to do some monkeying around with my code to get it set up properly.  However, I have a model I can work against to try to get my little program working for real hardware.  I am not working on my kernel mind, just the `rpi-timer` app.

So I just realized that the UART program I was just testing with is using the Aux UART (a little mini thing that has limited capability) and the one I have coded against is the full UART.  That could make a difference!

---

### 2018-Dec-15

I think I have made the decision to re-document some of this Rpi archetecture into my own format.  There are just so many errors, I really have no idea what works and what does not.  I'm finding errata all over the place and I think it will be good to consolidate it into 1 proper document I can refer to.

To do this, I will use my work computer and MS OneNote.  I have basket installed on my development workstation, but that does not provide the clean interface I want if I am going to take this on.  On the other hand, we have Office365 and I might be able to create a more global notebook and access it from the web.  Let me look at this.

---

I was able to get the UART documented and clean up some of the errata associated with that feature.  This was done with OneNote and is stored online at the following link (well, at least for now): https://andersondubose-my.sharepoint.com/:o:/g/personal/aclark_anderson-dubose_com/EvZDPp9tqlREs9Zj-pu_OEcBFz8DWkDGP89V2LAm_l4FZg?e=XlGPT1.  I may need to find a new home for this, but for now it will suffice.  Currently, this does not appear to be visible to anyone without an Office365 account.

My next task tomorrow it so lay this new documentation side-by-side to determine the differences between the PL011 UART and the Aux Mini-UART.

---

### 2018-Dec-16

Today I found the PL011 Technical Reference Manual.  It will clear up a few things.

I have been debating a bit over whether to use the mini-UART (which is proven to work on real hardware) or the PL011 UART.  I think in my timer test, I want to go with something that is guaranteed to work since the point is to get the timer working properly and fire an IRQ.  So, with that, I will change the addressing to be the mini-UART.

Despite my best efforts today, I am still not getting any output on the serial line to `screen`.  I tested again the `uart01.bin` program to make sure everything is wired properly and it is -- I get the proper output.  So, I am back to an exhaustive debugging cycle to get the mini-UART working properly -- starting by commenting out all the code that is not related to the UART.

---

No matter what me efforts, I am still not able to get serial output to happen.  However, when I test the `uart01.bin`, it works just fine.  At this point, I am going to comment out all my code and just copy in the C code from `uart01.c` and see what happens.

I'm honestly debating at this point if my code is ever getting control.  I am also wondering if using the C++ compiler is creating a problem.

---

I recompiled the `uart01.bin` file with my cross-compiler.  It does not work.  So, my cross-compiler is suspect.

---

3 recompile attempts later....  Well, I'm having trouble getting `crosstools-ng` to drop the cross-compiler in the right location.

---

Make that 6 attempts to make the cross-compiler and I think I finally have something more viable.  Instead of working with the `arm-eabi-*` toolchain, I am going to have to change over to use the `armv7-rpi2-linux-gnueabihf-*` toolchain.  This is going to require that I change all the `Tupfile`s and update all my compile commands.

I got those changes made and I went right back to my `rpi-timer` test to see what I could get to work -- nothing.  I was a bit too ambitious.  Tomorrow....

---

### 2018-Dec-17

Today I will take a look at my code to make sure I am getting serial output properly.  This will be my primary focus.

---

So, no matter what I do, the `rpi-timer.img` test does not work.  However, the `uart01.bin` test does.  There are some differences here:
* Compiler parameters
* C vs C++ compilers
* Extra code

What I need to do is start by eliminating the differences (the more prevalent right now are the compiler parameters) and see if I can get anything to run.

---

OK, I am going to stop working on `rpi-timer` for now and create a `uart01` built into my project.  It will start as a direct copy of the sameple code just to integrate it into the the build system.

---

OK, my first test did not work.  I wanted to capture the commands that do work, which are my target at the moment:

```
[adam@os-dev uart01]$ make
armv7-rpi2-linux-gnueabihf-as vectors.s -o vectors.o
armv7-rpi2-linux-gnueabihf-gcc -Wall  -O2 -nostdlib -nostartfiles -ffreestanding -c uart01.c -o uart01.o
armv7-rpi2-linux-gnueabihf-ld vectors.o uart01.o -T memmap -o uart01.elf
armv7-rpi2-linux-gnueabihf-objdump -D uart01.elf > uart01.list
armv7-rpi2-linux-gnueabihf-objcopy uart01.elf -O ihex uart01.hex
armv7-rpi2-linux-gnueabihf-objcopy uart01.elf -O binary uart01.bin
```

The `tup` commands are:

```
[adam@os-dev century-os]$ tup bin/rpi2b/boot/uart01.img
[ tup ] [0.000s] Scanning filesystem...
[ tup ] [0.014s] Reading in new environment variables...
[ tup ] [0.015s] No Tupfiles to parse.
[ tup ] [0.015s] No files to delete.
[ tup ] [0.015s] Executing Commands...
 1) [0.024s] obj/uart01/rpi2b: armv7-rpi2-linux-gnueabihf-as /home/adam/workspace/century-os/modules/uart01/src/rpi2b/vectors.s -o vectors.o
 2) [0.082s] obj/uart01/rpi2b: armv7-rpi2-linux-gnueabihf-gcc -Wall -O2 -nostdlib -nostartfiles -ffreestanding -c /home/adam/workspace/century-os/modules/uart01/src/rpi2b/uart01.c -o uart01.o
 3) [0.022s] bin/rpi2b/boot: armv7-rpi2-linux-gnueabihf-ld /home/adam/workspace/century-os/obj/uart01/rpi2b/uart01.o /home/adam/workspace/century-os/obj/uart01/rpi2b/vectors.o -T /home/adam/workspace/century-os/modules/uart01/src/rpi2b/memmap -o uart01.elf
 4) [0.011s] bin/rpi2b/boot: armv7-rpi2-linux-gnueabihf-objcopy uart01.elf -O binary uart01.img
 [    ] 100%
[ tup ] [0.151s] Partial update complete: skipped 1 commands.
```

Taking the `tup` commands and removing the path information, I end up with:

```
[adam@os-dev century-os]$ tup bin/rpi2b/boot/uart01.img
armv7-rpi2-linux-gnueabihf-as vectors.s -o vectors.o
armv7-rpi2-linux-gnueabihf-gcc -Wall -O2 -nostdlib -nostartfiles -ffreestanding -c uart01.c -o uart01.o
armv7-rpi2-linux-gnueabihf-ld uart01.o vectors.o -T memmap -o uart01.elf
armv7-rpi2-linux-gnueabihf-objcopy uart01.elf -O binary uart01.img
```

Laying these commands on top of each other and aligning for spacing, I get the following:

```
armv7-rpi2-linux-gnueabihf-as vectors.s -o vectors.o
armv7-rpi2-linux-gnueabihf-as vectors.s -o vectors.o

armv7-rpi2-linux-gnueabihf-gcc -Wall  -O2 -nostdlib -nostartfiles -ffreestanding -c uart01.c -o uart01.o
armv7-rpi2-linux-gnueabihf-gcc -Wall  -O2 -nostdlib -nostartfiles -ffreestanding -c uart01.c -o uart01.o

armv7-rpi2-linux-gnueabihf-ld vectors.o uart01.o -T memmap -o uart01.elf
armv7-rpi2-linux-gnueabihf-ld uart01.o vectors.o -T memmap -o uart01.elf

armv7-rpi2-linux-gnueabihf-objcopy uart01.elf -O binary uart01.bin
armv7-rpi2-linux-gnueabihf-objcopy uart01.elf -O binary uart01.img
```

Materially they are the same, with the only exceptions being the extension of the target name (`.bin` vs. `.img`) and the order of the object files being linked.  Since this is a binary with not specifiied entry point, the first instruction is supposed to be the start of the file.  I belive that this is my problem.

And it was.  The order of the modules in the binary file are not in the right order and the entry point was not the first byte in the file.  That was a painful lesson I hope I don't forget soon.

---

### 2018-Dec-18

I have a renewed sense of hope today getting into the coding.  At least I what I was doing wrong in getting the basics in line.  Such a stupid mistake, I'm actually embarrassed.

Today I will move back to the `rpi-timer` code and see what I can do there.  I feel much more confident in getting the code running now that I concretely know what the problem was.  This simple change allowed me to run the uart code to blast characters.  Now, I should be able to go back to my original purpose-built test -- which of course did not work!

I went for it all.  I am going to start by aligning the gcc parameters and see where that get me.

---

OK, things are compiling and running better now.  But I am still in the C compiler and I need to get to C++.  This is the next step.  This went relatively easily.  Next is to get the system to compile again.  And that is done.

So, now I want to start working on getting timer working.  This is still in the `rpi-timer` code, but is disabled at the moment.  I will start by enabling each bit at a time and testing to see where we end up.

---

Now, I'm getting somewhere.  I am trying to write the IVT data, starting with address `0x00000000`.  This assignment is not completing, or more to the point the next debugging line is not being written to the serial port.

I moved the interrupts to 1MB and the IVT initialization completes.   However, when interrupts are enabled, I am not getting anything.  I did program the timer, but have not checked if I am getting incrementing values.  I will enable that next.

OK, so everything works now up until enabling interrupts.  One last test on that since having added the infinite loop.

---

That still did not work.  So, enabling interrupts is killing my little program.  I am probably getting an interrupt and it is not working properly.  I will need to figure out how to sort that, but tomorrow.

---

### 2018-Dec-19

OK, so today I am going to work on getting the IVT table configured properly, since that is the most likely cause for interrupts locking up the system.  To do this, I need to figure out what where to check and manage HIVECS.

---

The location of the bit that set HIVECS is actually `SCTLR.V`.  Now with Cortex A7, I should be able to set the `VBAR` (or Vector Base Address Register) to be whatever I want.  I currently have it set at 1MB (`0x100000`).  However, I am not getting anything to collect the interrupt properly.

So, my next step is going to be in inquire on the different register addresses and output them to the serial port before enabling interrupts.  This means I will need my `SerialPutHex()` function.

OK, so here is where my test is at:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
Timer is initialized -- interrupts should be happening
```

The key things here is that the `VBAR` is set correctly and the `SCTLR.V` bit is clear.

Adding in the VBAR table dump, I get the following results:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xeafffe7d
Timer is initialized -- interrupts should be happening
```

THis finds something interesting, as the jump (or technically the `b` instruction) appears to be a relative offset.  I am not totally 100% sure of the `nop` opcodes, but they are all consistent so that odds are in my favor.  I will decode those anyway to be sure.

Adding in the code to make sure I get to the correct location from the Vector Table (and enabling interrupts again), I am getting the following:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xe51f0000
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008054
```

Notice I am not getting the initialization message.  So, something is still very wrong.  This config looks correct.  Let me see if I can force an undefined instruction with interrupts disabled.

The undefined instruction behavior is no different than the IRQ behavior.  Maybe the code that backs this is not working right anyway.

---

### 2018-Dec-20

Ok, I really need to figure out some way to test my code here....  I think I have a few options:
* Write an asm function with a jump to the IVT location.  I should be able to get the `'#'` character to the serial port once.
* Run this in qemu, fixing everything to have the proper offsets to be loaded by the qemu emulator (the entry point is different).  I should be able to debug a little bit from here.

I think I am going to start with the first option since I do not trust qemu to emulate anything rpi properly at the moment.

---

OK, with my first test, I am jumping to the location of the IVT in code (not the target location).  This test results in what I am looking for: a xingle `'#'` character printed.

```
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xe51f0000
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008058
#
```

Now for test #2: jump to location `0x100000`, or 1MB.  This test also worked, yielding the same results as above.

So, what does this tell me?  Let's see here:
1. The code is sound under the normal execution stream.
1. When I have some kind of exception, the system locks up.
1. My target location at 1MB is a good one and the code is being set properly there.
1. `IRQHandler()` sets a reasonable stack location and I can use it properly under a normal execution stream.

I made a change where the `IRQHandler()` function puts the CPU into supervisor mode.  This did not have a good result as the rpi locked up as soon as interrupts were enabled.  So, I think I am going to have to try with qemu to see what I can find.

---

OK, it looks like qemu emulated the program fine, but never enabled interrupts (or at least the timer does not fire).  On real hardware, an appear to be getting some exception that is crashing the system that I cannot trace.

---

### 2018-Dec-21

Once again, I am debating whether to pursue working on qemu (where I am not getting the same issues as on real hardware, but different things are breaking) or to pursue debugging on heal hardware (where my ability to actually do debugging and investigation is almost totally eliminated).  On one hand, I cannot duplicate the problem at hand and on the other I cannot debug the problem.  Not a fun choice at all.

Well, I have proven that the actual code I have written for the Interrupt Vectors and the target will work.  I get the `'#'` character.  So, something else must be happening at the system level, or the interrupts are not being routed properly.  I need to check that code carefully, as my checks have all be surface checks so far.  I am not sure exactly how much code or debugging I will really get done today....  I expect today to look much more like research than anything.

---

### 2018-Dec-22

Today I am continunig the work on documenting the Interrupt Controller (IC) and checking my code against the specification.

In the interest of being 100% accurate in my code, I did add in the following into the timer initialization today:

```C
    // -- for good measure, disable the FIQ
    MmioWrite(INT_FIQCONTROL, 0x0);
```

This disables the FIQ interrupt path.  I honestly do not expect this to make a difference in my test (and therefore will not test it).

The other thing I notice is that I am enabling the world, which is not the real intent here.  I really need to just enable the 1 interrupt for the timer.  So, I will disable everything and then go back and enable only the ARM Timer.

---

Now that I have some changes to test, I am getting really odd characters to the screen.  I am not sure if this is due to the changes I made or if the OS needs a reboot to clean something up.  To test this, I am going to run an older executable to see what results I get.  The older binary worked perfectly, so there is a problem with my code.  The results I am getting are just garbage to the screen:

```
'ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
ï¿½Äï¿½ï¿½ï¿½ï¿½ï¿½ï¿½:
ï¿½94aï¿½Òºhï¿½Iï¿½8s#=tï¿½\F\Xu`ï¿½c
?ï¿½ï¿½,ï¿½ï¿½/Kï¿½]ï¿½Iï¿½Uï¿½YVï¿½Hï¿½>Jï¿½`Hpï¿½d&ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½~ï¿½<;hï¿½ï¿½(ï¿½ï¿½{XNï¿½ï¿½ï¿½ï¿½fL1kï¿½ï¿½,ï¿½Ó¥ï¿½Fï¿½ï¿½ï¿½q}ÚRa2ï¿½ÍNYNï¿½ï¿½ï¿½,ï¿½ï¿½IlVï¿½Xbï¿½ï¿½ï¿½ï¿½,ï¿½Fï¿½ï¿½nï¿½ï¿½wï¿½ï¿½ï¿½sï¿½ï¿½ï¿½o$ï¿½ï¿½ï¿½>ï¿½Aï¿½ï¿½-AkË±ï¿½ï¿½Cï¿½=ï¿½ï¿½ï¿½ï¿½Zï¿½gï¿½Tï¿½ï¿½-&uï¿½ï¿½[ï¿½0ï¿½ï¿½ï¿½,ï¿½Kï¿½ï¿½*Uï¿½;Nï¿½ï¿½ï¿½ï¿½ï¿½C%ï¿½/Ô°ï¿ï¿½ï¿½ï¿½zï¿½m(Åï¿½ï¿½^*ï¿½/ï¿½z[ï¿½"#ï¿½ï¿½4rï¿½Kï¿½yï¿½^ï¿½q?4ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½(ï¿½ï¿½ï¿½!ï¿½ï¿½_ï¿½ï¿½Cmï¿½ï¿½ï¿½ï¿½YHï¿½B\ï¿½ï¿½jï¿½oï¿½zï¿½ï¿½srï¿½Bï¿½ï¿½ï¿½" ï¿½$Lï¿½/ï¿½ï¿½ï¿½_ï¿½jE>ï¿½8ï¿½ï¿½ï¿½0ï¿½Îdï¿½
```

... and lots of it.  I am not sure if interrupts are enabled when I get control.  I'm going to start with an explicit disable and if that does not change things, I'm going to comment out enabling the interrupts.  I want to check the output of the debugging code I have added.

Hmmm...  as I'm thinking about this and fixing up my code, I am wondering if I might be having an IRQ routed to another core, waking it up somehow.  I guess it's possible but I have no clue how it will behave.

I explicitly disabled interrupts and commented out the `EnableInterrupts()` function call and I am still getting garbage to the screen -- however based on the timing of the garbage and the relative length of the garbage, I believe it is getting to the end of the program whereas before commenting out the `EnableInterrupts()` call it was not.

I am going to start my troubleshooting by commenting out the 5 lines of code I added.  These are here:

```C
    // -- for good measure, disable the FIQ
//    MmioWrite(INT_FIQCONTROL, 0x0);

    // -- Disable all IRQs -- write to clear, anything that is high will be pulled low
//    MmioWrite(INT_IRQDIS0, 0xffffffff);
//    MmioWrite(INT_IRQDIS1, 0xffffffff);
//    MmioWrite(INT_IRQDIS2, 0xffffffff);

    // -- Now I should be able to set up the timer, which will be done locally
    MmioWrite(TMR_BASE + 0x00, 0x00);               // this register is not emulated by qemu
    MmioWrite(TMR_BASE + 0x08, 0x80000000);         // this register is not emulated by qemu
    MmioWrite(TMR_BASE + 0x40, 0x00000020);         // select as IRQ for core 0
    MmioWrite(TMR_BASE + 0x60, 0xfff);              // enable IRQs from all devices from local timer down
    MmioWrite(TMR_BASE + 0x34, 0x8 | (1<<28) | (1<<29));  // set up the counter for the timer and start it
    MmioWrite(TMR_BASE + 0x38, (1<<30) | (1<<31));  // clear and reload the timer

    // -- Now, enable the ARM Timer interrupt only
//    MmioWrite(INT_IRQENB0, 1);
```

I will run this test to see if I can get proper output again....  Still garbage!!

OK, there were 3 lines I removed as well.  These enabled all interrupts no matter what they were.  I will try to put them back in.  And still garbage!!

I did find that the program was linked to address `0x10000` still for qemu.  Made this change and will now test again.  OK, that restored it again.  It's the stupid things that get you sometimes.

I'm going to reinstate all my code and try again.  This worked and I have been able to determine the the IRQ Enable bit can be read.  Now, to try again with interrupts enabled.  This is still locking up and not getting to the end.  I really did not expect much of a different result.

So, now I need to look more into the BCM2836 chip related to multiple cores and see that I need to do to get that routes properly.  I found that I had the `nCNTPNSIRQ` interrupt routed to the FIQ (which is of course disabled).  So, I have made that change but I'm not ready to test yet.  I also wrote the register to route the local timer interrupt to core 0 IRQ.

Now for a test....  No change in behavior.

---

OK, to try to think this through...  I have 4 tests that I can try.  These tests are controlled by which of the following lines are commented:

```C
//    IvtFunc();                      // This is an explicit jump to the start of the Interrupt Vectors -- works
//    Undef();                        // This will generate an undefined exception -- does not work
//    EnableInterrupts();             // This enables interrupts and the timer should start firing -- does not work
```

My `IRQHandler()` function is a one-shot handler that just prints a `'#'` character to the serial port and then `Halt()`s the CPU.  When the `IRQHandler()` function is called, I do not expect the cpu to return to perform any additional processing.

I am setting the location of the Interrupt Vector Table with the following code as soon as my code gets control and all cores except 0 are quieted down:

```
initialize:
    mov     r0,#0x100000                @@ The location of the vector base address register (1MB)
    mcr     p15,0,r0,c12,c0,0           @@ Write this location to the Vector Base Address Register
```

#### Test 1

All of the lines are commented out.  This will drop the code into a loop that reads the timer value to ensure that it is incrementing.  This will print the following to the serial port:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xe51f0000
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008078
  The Basic Interrupt register is : 0x00000001
Timer is initialized -- interrupts should be happening
The timer value is: 0x00a4a5ad
The timer value is: 0x013a59d9
The timer value is: 0x01d00e2b
The timer value is: 0x0265c368
The timer value is: 0x02fb77fe
The timer value is: 0x03912c8e
The timer value is: 0x0426e2ea
The timer value is: 0x04bc9729
The timer value is: 0x05524bea
The timer value is: 0x05e801b6
```

The actual values of the timer value will be slightly different, but all-in-all, this is the result and it is consistent.  These are the expected results, so this test passes.


#### Test 2

This test is performed by uncommenting the following line:

```C
    IvtFunc();                      // This is an explicit jump to the start of the Interrupt Vectors -- works
```

The code that performs this jump to the first Interrupt Vector is:

```
IvtFunc:
    mov     r0,#0x100000                @@ The location of the vector base address register (1MB)
    mov     pc,r0                       @@ and jump to that location
```

In this function, there is an explicit jumpt to the first interrupt vector.  In this test, I am expecting a `'#'` character to be printed and I should not see the line reading `Timer is initialized -- interrupts should be happening`.

My results look like this:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xe51f0000
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008078
  The Basic Interrupt register is : 0x00000001
#
```

These are the expected results and this test passes.


#### Test 3

This test is performed ny uncommenting the following line:

```C
    Undef();                        // This will generate an undefined exception -- does not work
```

In this function, there is a jump to an assembly function that executes an undefined instruction.  This instruction looks like this from the disassembly:

```
00008074 <Undef>:
    8074:	ffffffff 			; <UNDEFINED> instruction: 0xffffffff
```

I am expecting the exact same results as Test 2, where I see a `'#'` character on the screen.  The results are the same as Test 2 without the `'#'` character, as shown below:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xe51f0000
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008078
  The Basic Interrupt register is : 0x00000001
```

This test fails as the `'#'` character is not written to the serial port.


#### Test 4

This test is performed ny uncommenting the following line:

```C
    EnableInterrupts();             // This enables interrupts and the timer should start firing -- does not work
```

In this function, interrupts are enabled and I expect the timer to fire an IRQ almost immediately.  `IRQHandler()` will get control and write a `'#'` character to the serial port and then stop processing.

The results of this test are the exact same as with Test 3:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xe51f0000
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008078
  The Basic Interrupt register is : 0x00000001
```

This test fails as the `'#'` character is not written to the serial port.


#### Conclusion

The conclusion I can draw from this is that there is something witht the state of the cpu that is causing a problem on an exception/interrupt.  Whether that is something with the `cpsr` or the registers, I am not sure.

However, with that said, I am not disabling the interrupts when switching modes, so I might be having a recursive interrupt problem...  Let me test that; setting up for test 2....  That test failed.

At this point, I am going to commit these changes to github so that I can ask for help....

---

Well, I have been working with a really old version of the ARM ARM and I only just now realized it.  Shame on me!

I had a great conversation on `freenode#osdev` about this, captured here:

```
on real rpi hardware.  I believe I have this narrowed down to a cpu state problem and would like to ask if someone can review my work.  My analysis of the situation is here: https://github.com/eryjus/century-os/blob/master/JOURNAL.md -- go to the bottom and look for today's date.  I had a problem with the link locaiton which is documented first for the day and then this
[13:59] <eryjus> analysis starts.
[13:59] <eryjus> The source for this test is here: https://github.com/eryjus/century-os/tree/master/modules/rpi-timer
[14:00] <eryjus> thanks in advance.
[14:00] <geist> why do you think it's a cpu state problem?
[14:01] <eryjus> i have 4 tests documented -- and an explitit jump to the first interrupt vector works, but when i force an undefined excpetion the cpu locks
[14:01] <geist> also depens on what you mean by 'cpu state'
[14:02] <geist> ah. good.i was going to stay, trigger an undefined opcode
[14:02] <eryjus> cspr or registers
[14:02] <geist> then the question is does it make it to your code or is the vbar not correct
[14:02] <geist> point me at your exception handling code, the table and the assembly
[14:03] <eryjus> the vbar is set correctly or at least I can read back what i wrote and get the results I expect
[14:03] <geist> what was the value?
[14:03] <eryjus> ivt: https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/entry.s#L77
[14:04] <eryjus> moves it to the correct locaiton: https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/TimerMain.cc#L233
[14:04] <geist> the nop sled is cute
[14:04] <geist> what value was put in vbar?
[14:04] <eryjus> the actual handler code: https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/IRQTarget.s#L30
[14:05] <eryjus> load of vbar: https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/entry.s#L57
[14:05] <geist> what was the value put in the vbar?
[14:05] <geist> the numeric value
[14:05] <eryjus> the value loaded is 0x100000 and that is what I am getting back when i read it later
[14:05] <geist> okay
[14:06] <geist> what is this patching of the IVT thing?
[14:07] <eryjus> not sure I follow your question..
[14:07] <geist> https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/TimerMain.cc#L234
[14:07] <geist> what is that all about
[14:07] <eryjus> I am relocating it to 0x100000 in my initialization
[14:08] <geist> is the I and D cache initialized at this point?
[14:08] <geist> if so, you can have a cache coherency problem
[14:08] <eryjus> this may help  https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/TimerMain.cc#L225
[14:08] <geist> I & D caches are not synchronized on ARM. if you write out code and expect it to run you *must* flush the D and I caches
[14:08] <geist> generally you just point the vbar at the vector table inside your binary, no need to copy it
[14:09] <eryjus> no code to init any caches..
[14:09] <geist> only constraint there is making sure it aligned
[14:09] <geist> in that cas eyou dont know the state of the caches
[14:09] <geist> so it's possible the I&D cache is enabled, in which case the copyying of the vector table wont work
[14:10] <geist> there are a few other problems
[14:10] <geist> assume the cache is okay
[14:10] <geist> the instruction you're using in the IVT is a ldr reg, =value
[14:11] <geist> the =value part means it may emit a PC relative load to a hidden .word
[14:11] <geist> which you're probably not copying
[14:11] <eryjus> true -- but when i to an explicit jump to the first vector the code works
[14:12] <geist> explicit jump in the copied version?
[14:12] <geist> or in the in text version?
[14:12] <eryjus> when this line is uncommented: https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/TimerMain.cc#L301
[14:12] <eryjus> https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/entry.s#L73
[14:12] <eryjus> This works
[14:12] <geist> *shrug* dunno then
[14:12] <geist> you have about 5 layersof hacks here, any of which can go wrong
[14:13] <geist> the ldr = stuff is at best extremely fragile
[14:13] <geist> i'd recommend something like....
[14:14] <geist> hmm, i dont have it here. my LK code is using the vector table in place in the .text segment, so it can get away with a simple relative branch
[14:14] <geist> https://github.com/littlekernel/lk/blob/master/arch/arm/arm/start.S#L28
[14:14] <geist> but i've absolutley written code to put a ldr pc, <some label just after the table>
[14:15] <geist> and then after ldrs it has a table of .word
[14:15] <geist> and then copy all of that
[14:15] <geist> that way it is completely position independent
[14:16] <geist> but assuming this all works, what is at IRQTarget?
[14:16] <geist> I dont see that code
[14:17] <eryjus> IRQTarget: https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/IRQTarget.s#L30
[14:18] <eryjus> I forgot I pulled IRQHandler out of the mix to try to get a simple case working.
[14:18] <geist> can you post the dissassembly of the entire binary?
[14:18] <eryjus> stand by
[14:20] <eryjus> https://github.com/eryjus/century-os/blob/master/maps/rpi2b/rpi-timer.map
[14:21] <geist> https://github.com/eryjus/century-os/blob/master/maps/rpi2b/rpi-timer.map#L521 is the thing I was talking about, btw
[14:21] <geist> see how the instruction at 874c references the hidden .word at 8754?
[14:22] <geist> anyway, i'd start off by getting rid of that whole copy stuff, simplify it. align the IVT on a 64 byte or so boundary (i forget the requirements) and just point vbar directly at it
[14:22] <geist> then you'll have removed a big pile of complexium
[14:23] <eryjus> I do and if you look at the code here, I have accounted for that by copying an extra .word https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/TimerMain.cc#L234
[14:23] <eryjus> 7 nop -- 1 ldr -- 1 mov -- 1 hidden word
[14:23] <eryjus> or at least i tried
[14:23] <eryjus> ok, will give that a try.
[14:24] <geist> yah, but again that's all just fragile stuff. start by removing the stuff and simplifying
[14:24] <geist> so that there's less variables in flight. i'm looking up the alignment requirements of the vector table now
[14:24] <geist> i think it's something like 64 bytes
[14:26] <eryjus> im searching as well..
[14:26] <geist> looks like 32 bytes. the bottom 5 bits of the register are ignored
[14:27] <geist> which makes sense, since that's basically the size of the vector table. the hardware almost assurednly just ORs in the offset when computing an address
[14:27] <eryjus> geist, where can i find that?
[14:27] <geist> if you toss in a .balign 32 or something just in front of it
[14:27] <geist> i just found it in the ARM ARM, in the description of the VBAR
[14:28] <eryjus> interesting...  my ARMARM does not have a reference to VBAR.
[14:28] <geist> then if you use the direct vbar you can simplify your vector table with a series of branches
[14:28] <geist> yes i'm looking in the armv8 one
[14:28] <geist> but in this case it's backwards compatible
[14:28] <geist> armv8 defines much easier to understand hard names for all these old control registers, which is where VBAR comes from
[14:28] <eryjus> i gotta get a better manual -- mine is (c) 2005
[14:29] <geist> yes. that *wayyyy* predates vbar existing
[14:29] <eryjus> that could be part of my problem
[14:29] <geist> 2005 is armv5 era, the thing you're dealing with here is armv7
[14:29] <geist> which itself is already about 8 years out of date, since armv8 is the current standard
[14:30] <eryjus> this might explain why I am fighting so hard and getting nowhere....
[14:30] <eryjus> not sure where I got that dinosaur from....
[14:30] <geist> go to arms site and find them directly
[14:31] <geist> if you're ust googling for random crap,you're going to get it
[14:31] <geist> https://developer.arm.com/products/architecture/cpu-architecture/r-profile/docs/ddi0406/latest/arm-architecture-reference-manual-armv7-a-and-armv7-r-edition seems to be a good starting point for the armv7-a spec
[14:32] <geist> https://developer.arm.com/products/architecture/cpu-architecture/r-profile/docs/ddi0487/latest/arm-architecture-reference-manual-armv8-for-armv8-a-architecture-profile is the armv8-a
[14:32] <geist> but it'll be a lot more complicated for what you're doing, so i'd start with the v7-a
[14:35] <eryjus> i have them both now.  Thank you!!! -- I'm sure things will start looking better from here.
[14:35] <eryjus> I'll call that an early Christmas present.  thanks again
[14:37] <geist> yay
[14:37] <geist> and i'm not trying to bust your chops about this stuff, you just have to make sure all the details are right
[14:38] <geist> there's a certain amount of hackery you can do when first getting started, but it quickly topples over. if you have too many layeres of hacks to try to get something working then it's hard to see at what level things are broken
[14:38] <geist> so it makes sense to go and knock out some of the complexity to try to simplify the problem
[14:39] <geist> i do this all the time when doing initial bringup. hack together some stuff to prototype that it works, then go back and clean it up. but sometimes the hacks get too deep prior to getting something working, and you have to declare bankruptcy and start building a better foundation
[14:39] <geist> which is of course hard if you dont know how to make it go
[14:40] <eryjus> geist, i totally agree -- this is a purpose built test to for real hardware to get the timer irq to fire -- where I can go back to qemu and figure out what is emulated and what is not...  none of this was going to survive into a kernel without a real hard look.  I knew the copy of the table was risky at best.
[14:41] <eryjus> we're saying the same thing -- but thanks for the counsel.
[14:41] <geist> yah
[14:42] <eryjus> i might be able to actually navigate now with an accurate map
[14:46] <geist> yah a rpi2 is a cortex-a7, which is a armv7-a implementation
```

So, the next thing here is to read, and read some more, and then get the VBAR set to the code location, properly aligned.

---

I set up a test to determine which mode I am in.  I come back with a value of `0x1a` whereas I am expecting to be in `0x13`.  What the hell is mode `0x1a`??  mode `0x1a` is `hyp` mode and I am expecting `svc`.

I am trying to make sure I am in `svc` mode as the first instruction I execute.  However, the ARM ARM states in section B9.1.2:

> Is not an exception return instruction, and is executed in Hyp mode, and attempts to set CPSR.M to a value other than '11010', the value for Hyp mode.

This means that my little instruction `cps #0x13` does absolutely nothing (and I believe it is treated as a `nop`).

I tries to use this little trick to not get into `hyp` mode in the first place: https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=98904&p=864376#p864376.  However, the pi will not boot.

I have the right instruction now and I am trying to boot.  Now nothing happens...  and I'm out of time for the night....

---

Well, I did have some more time tonight.  I was able to force the processor into `svc` mode!!  I used some code from this web site (well the thinking anyway): https://github.com/raspberrypi/linux/blob/rpi-4.1.y/arch/arm/include/asm/assembler.h#L319.  The result in my code is:

```
_start:
    mrs     r0,cpsr                     @@ get the current program status register
    and     r0,#0x1f                    @@ and mask out the mode bits
    cmp     r0,#0x1a                    @@ are we in hyp mode?
    beq     hyp                         @@ if we are in hyp mode, go to that section
    cpsid   iaf,#0x13                   @@ if not switch to svc mode, ensure we have a stack for the kernel; no ints
    b       cont                        @@ and then jump to set up the stack

@@ -- from here we are in hyp mode so we need to exception return to the svc mode
hyp:
    mrs     r0,cpsr                     @@ get the cpsr again
    and     r0,#~0x1f                   @@ clear the mode bits
    orr     r0,#0x013                   @@ set the mode for svc
    orr     r0,#1<<6|1<<7|1<<8          @@ disable interrupts as well
    msr     spsr_cxsf,r0                @@ and save that in the spsr

    ldr     r0,=cont                    @@ get the address where we continue
    msr     elr_hyp,r0                  @@ store that in the elr register

    eret                                @@ this is an exception return

@@ -- everyone continues from here
cont:
```

My test forced an undefiend exception and the results were:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
The processor mode is: 0x00000013
  VBAR: 0x000087a0
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe320f000
  The code at VBAR[1] is: 0xe320f000
  The code at VBAR[2] is: 0xe320f000
  The code at VBAR[3] is: 0xe320f000
  The code at VBAR[4] is: 0xe320f000
  The code at VBAR[5] is: 0xe320f000
  The code at VBAR[6] is: 0xe320f000
  The code at VBAR[7] is: 0xe59f0004
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x000087a0
  The Basic Interrupt register is: 0x00000001
#
```

This is the expected results.  One more test tonight and I am going to call it a night.  This time to see if I can get the timer interrupt to fire and get the `'#'` character to the screen.

That test worked.  I'm going to end my night on this success.

---

### 2018-Dec-23

This morning I started with by putting all my IRQ code back and compiled a test -- but I forgot to save my changes to `IRQTarget.cc`.  Another test and I'm hopeful.

After this test, the interrupt is not firing (or more to the point I am not getting any output).

After stripping out all the extra stuff from the `IRQTarget()` function, this is working now.  Here is my output:

```
Ready to enable interrupts!
This is the system configuration:
The processor mode is: 0x00000013
  VBAR: 0x00008780
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe320f000
  The code at VBAR[1] is: 0xe320f000
  The code at VBAR[2] is: 0xe320f000
  The code at VBAR[3] is: 0xe320f000
  The code at VBAR[4] is: 0xe320f000
  The code at VBAR[5] is: 0xe320f000
  The code at VBAR[6] is: 0xe320f000
  The code at VBAR[7] is: 0xe59f0004
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008780
  The Basic Interrupt register is: 0x00000001
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
```

What I see now is that I am burying the CPU in IRQs so it cannot process anything more.  This should be an easy fix....  But I am having some trouble with it.  It looks like I am getting into a loop of some kind there the `'!'` character is printed as fast as possible.  I am unable to slow it down.  So I have capped the duration of the test and I then disable interrupts.

---

So it dawns on me that I am not clearing the timer interrupt when it fires.  Therefore, as soon as interrupts are enabled again after handling that interrupt, the interrupt gets re-triggered.  Let's see here, that should be as simple as clearing the timer flag for the Basic Pending register.

No matter what I try, I am not able to clear the interrupt and therefore I am still getting the cpu buried in the first interrupt.  I will have to do some research online.

I found the problem -- There is a timer reset which has an interrupt clear register that needs to be written.  This fixed the problem and now I have a program the works the way I want it.  Except for the timer frequency.

I am close and getting the interrupts to fire on a regular basis.  The `'!'` character denoted a timer interrupt:

```
Serial port initialized!
Ready to enable interrupts!
This is the system configuration:
The processor mode is: 0x00000013
  VBAR: 0x00008780
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe320f000
  The code at VBAR[1] is: 0xe320f000
  The code at VBAR[2] is: 0xe320f000
  The code at VBAR[3] is: 0xe320f000
  The code at VBAR[4] is: 0xe320f000
  The code at VBAR[5] is: 0xe320f000
  The code at VBAR[6] is: 0xe320f000
  The code at VBAR[7] is: 0xe59f0004
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008780
  The Basic Interrupt register is: 0x00000001
Timer is initialized -- interrupts should be happening
!The timer value is: 0x00a56249
!The timer value is: 0x013b106e
!The timer value is: 0x01d0bd08
!The timer value is: 0x026676d1
!The timer value is: 0x02fc30af
!!The timer value is: 0x0391eaf1
!The timer value is: 0x0427a3fb
!The timer value is: 0x04bd5196
!The timer value is: 0x0552ff00
!The timer value is: 0x05e8bb0f
```

Now, I want to try to get the interrupts to fire at about 100/second (x86 is set to 250/second, but with this slow cpu it might bury it again).

I now have the timer firing at a resaonable rate.  I might need to tune that a bit later, but at the moement the hardware for `rpi-timer` is working properly.  Now to commit this code and then switch back over to the timer on qemu.

---

OK, I have some sorting out to do...  In the middle of all this I broke the rpi2b build and the i686 build.  rpi2b has no valid loader and i686 triple faults on jumping to the kernel.  I believe my problem is going to be in `Tuprules.tup`, but I'm not sure.

OK, so the problem with the rpi2b not running was that the page size when linking was set to be too large.  Changing that resolved that problem.  Now for the reason the i686 triple faults...

---

So, for the i686, I did find a bug in my `kMemMove()` function.  This has not changed in some time and I am wondering how it never surfaced??!!

I still have a problem with a page fault in the kernel which I need to fix....  And this looks like an address for the rpi2b!  What was I thinking???

Once I found that, it was also an easy fix.  So, where do I stand now?
* i686 is functioning again
* rpi2b boots and dies with an undefined instruction

So, I will go find that and clean it up as well.  Well, the fault is coming out of `ProcessInit()` or the results of `ProcessInit()`:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
Undefined Instruction:
At address: 0x80031f78
 R0: 0x00009c0c   R1: 0x800321c0   R2: 0x80032200
 R3: 0x00000000   R4: 0x80032140   R5: 0x00000001
 R6: 0x00000000   R7: 0x00000000   R8: 0x00000000
 R9: 0x0011c000  R10: 0x0002d207  R11: 0x00028668
R12: 0x80069c0c   SP: 0x800029bc   LR_ret: 0x800029e0
SPSR_ret: 0x600001d3     type: 0x1b
```

...  or maybe not....

Something is not right....  I am still getting the undefined instruction and I think I'm running off to nowhere...  With this code:

```C
    kprintf("Initializing the butler lists\n");
    ListInit(&butler.stsQueue);
    butler.lockList.lock = {0};
    butler.messages.lock = {0};
    ListInit(&butler.lockList.list);
    ListInit(&butler.messages.list);
```

... I get this result:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
Attempting to initialize the idle and butler processes
Idle Process initialized
Initializing the butler lists
Undefined Instruction:
At address: 0x80031f70
 R0: 0x0000001e   R1: 0x800321c0   R2: 0xf2201000
 R3: 0x800058e2   R4: 0x80032111   R5: 0x800320c0
 R6: 0x80049c0c   R7: 0x00000001   R8: 0x00000000
 R9: 0x0011c000  R10: 0x0002d207  R11: 0x00028668
R12: 0xff000000   SP: 0x80002a1c   LR_ret: 0x80002a20
SPSR_ret: 0x600001d3     type: 0x1b

Additional Data Points:
User LR: 0x00000000  User SP: 0x00000000
Svc LR: 0x80031f80
```

Then when I add some additional code to determine which line is creating the problem, such as this:

```C
    kprintf("Initializing the butler lists\n");
    ListInit(&butler.stsQueue);
    kprintf("a");
    butler.lockList.lock = {0};
    kprintf("b");
    butler.messages.lock = {0};
    kprintf("c");
    ListInit(&butler.lockList.list);
    kprintf("d");
    ListInit(&butler.messages.list);
```

... I get this result:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
Undefined Instruction:
At address: 0x80031f70
 R0: 0x00005870   R1: 0xfb009c50   R2: 0x00000325
 R3: 0x00000002   R4: 0x00002140   R5: 0x00000000
 R6: 0x00000000   R7: 0x00000000   R8: 0x00000000
 R9: 0x0011c000  R10: 0x0002d207  R11: 0x00028668
R12: 0xfb00a140   SP: 0x800040f4   LR_ret: 0x8000291c
SPSR_ret: 0x600001d3     type: 0x1b

Additional Data Points:
User LR: 0x00000000  User SP: 0x00000000
Svc LR: 0x80031f80
```

... where the line about Initializing the butler lists is missing.  So, it's a qemu log again:

```
IN:
0x8000290c:  e3050870  movw     r0, #0x5870
0x80002910:  e92d41f0  push     {r4, r5, r6, r7, r8, lr}
0x80002914:  e3024140  movw     r4, #0x2140
0x80002918:  ed2d8b02  vpush    {d8}

R00=00000000 R01=fb009c50 R02=00000325 R03=00000002
R04=80032218 R05=00000000 R06=00000000 R07=00000000
R08=00000000 R09=0011c000 R10=0002d207 R11=00028668
R12=fb00a140 R13=80031fd8 R14=800040f4 R15=8000290c
PSR=600001d3 -ZC- A NS svc32
Taking exception 1 [Undefined Instruction]
...from EL1 to EL1
...with ESR 0x7/0x1fe0000a
```

Notice the `vpush` instruction.

---

I was able to determine that this is a cross-compiler problem.  Not only that but the cross-compiler for qemu needs to be configured differently than the cross-compiler for the real rpi2b hardware.  This means I will have to set up a new architecture for qemu.  This task I am going to add to Redmine to address later and move on to get the timer working properly (http://eryjus.ddns.net:3000/issues/373).

At this point, I am past the timer initialization and I am getting a Data Exception as a result:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
Idle Process initialized
Butler Process initialized
Setting the scalar value to 0x25800 for frequency 0xfa
IsrHandler registered
Timer is initialized
Data Exception:
At address: 0x80030f70
 R0: 0x80033000   R1: 0x00000000   R2: 0x00000001
 R3: 0x802000cc   R4: 0x80034000   R5: 0xd0000000
 R6: 0x80043000   R7: 0x80031000   R8: 0x00000000
 R9: 0x0011b000  R10: 0x0002d207  R11: 0x00028668
R12: 0x30200000   SP: 0x80000e10   LR_ret: 0x80001e94
SPSR_ret: 0x80000113     type: 0x17

Additional Data Points:
User LR: 0x00000000  User SP: 0x00000000
Svc LR: 0x80030f80
```

This Data Exception is happening in `MmuUnmapPage()`:

```
80001e88 <_Z12MmuUnmapPagem>:
80001e88:	e1a03620 	lsr	r3, r0, #12
80001e8c:	e1a03103 	lsl	r3, r3, #2
80001e90:	e2833102 	add	r3, r3, #-2147483648	; 0x80000000
80001e94:	e5d30000 	ldrb	r0, [r3]
80001e98:	e2100003 	ands	r0, r0, #3
80001e9c:	13a02000 	movne	r2, #0
80001ea0:	15930000 	ldrne	r0, [r3]
80001ea4:	15832000 	strne	r2, [r3]
80001ea8:	17f30650 	ubfxne	r0, r0, #12, #20
80001eac:	e12fff1e 	bx	lr
```

---

It dawned on me while I was away that the `MmuUnmapPage()` funtion call is from `HeapInit()`, which I am not debugging at the moment.  So, I am going to add a loop and some output to make sure that I am getting a timer emulated from qemu.  But, I am back at my original problem, in qemu the timer will not fire.

Or maybe it is firing and I'm not able to determine the IRQ number.  Nope...  I would see it.

---

I think I am going to have to come to terms with the fact that qemu just does not cut it for an emulator for rpi2b.  I am going to have to work on real hardware.  In order to do that, I am going to have to move my development system to my laptop.  Which means I need to build a number of cross compilers.  I have used `ct-ng` to do this and the config files should be available to migrate over.  I will need to get a commit of those with the code and write some instructions for building each of the cross compilers anyway.

At the same time, I am going to use a tool mrvn has released called `raspbootin`, which will allow a serial connection and load the kernel directly.  This should save a significant number of write cycles on the microSD drive.


