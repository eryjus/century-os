***The Century OS Development Journal***

***Prologue***

I found some old code that I want to reuse.  The most successful iteration I have had was the second 32-bit version.  I thought I had lost that code (see below).  So I am going to try to resurrect it with this iteration.  Plus, I will clean up after myself a bit.

My goals from the previous iteration are all still valid, I am just hoping to move this along a little faster.  I plan on combining code from several sources here.


---
---

***Prologue from Century***

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
---

**2018-05-23**

The first orders of business are the basic structure and the build system.  First, I would like to structure to be relatively flat, but broken up into modules.  The recovered code from Century32 is a monolithic kernel, so everything was in the main directory.  On the other hand, several things are going to be hardware-specific and need to be placed into their own folders.  I may not be able to keep the goal to be flat as cleanly as I would prefer.  Back to that in a bit...

As for the build system, I prefer `tup`.  Is is very fast and have never let me down and forcing me to do a total rebuild.  However, there are just some things that `tup` requires that I do not like as well, such as forcing build definition files into the target folders.  Well, we can use `make` to manage `tup`, and that is the approach I will take.  For more information, check out the [readme](./README.md) file.

So, back to the directory structure...  This might sound a little odd, but I think the starting point should be the `Makefile` from century.  That `Makefile` is optimized for `tup` already and already takes care of some of the management I was referring to above.  It will be an easy clean-up from that start, so I will go and copy it.


---
---

**2018-05-24**

Today I am still working on getting the structure aligned.  I am continuing to follow the structure laid out in `Makefile`.

I also have another thought I want to work through...  NASM vs. GAS.  One is a dedicated assembler (NASM) while the other is part of the GNU compiler suite and therefore more integrated.  The problem with GAS is that it uses AT&T syntax -- which I believe leads to bugs.  I personally prefer the NASM syntax.  However, if I am recalling properly there is something I have to do when I link...  Stay tuned on that one, but I will use `nasm`.

At this point, I am starting to write `lists.cc`.  I wanted to take a moment to talk about the high-level language I am using.  I am writing my code largely in C.  However, I am going to make use of several features of the C++ compiler (inline functions as an example).  My call-out here is that you should expect to be reading Procedure Oriented code.  This does, however, set up a situation of name mangling with assembly.  To overcome this, any asm function will also be declared as `extern "C"`.

Another thought about organization is the number of functions to put into a single file.  Compiles run much faster with multiple functions in a single file (reducing the number of files to compile).  However, it is easier to maintain and read/understand the entire system with a single or few functions in each file.  Also, I believe it will aid portability.  So I am going to change back to a single function in each file.


---
---

**2018-05-25**

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
---

**2018-05-26**

Today I will start to bring over early console functionality.  This will also drag over several supporting functions and I will make the effort to bring over all the similar functions at the same time.  I will start with a `ConsolePutChar` function and then build from there.


---
---

**2018-05-27**

Today I have finished up copying in several functions and getting my `ConsolePutS()` implementation working.  I now have a greeting on the screen.  This is a milestone so I will commit these changes locally.

---

So, since I am working on the screen, I think the next thing to do is to write the status bar.  This way the screen is actually 'complete'.  The status bar will have to be blank for now since I don't have too much to actually paint into the status bar, but I will start laying the foundation.  What I am thinking about is the Redmine (my internal bug tracker, sorry for the loss of visibility on that) I have put in for the `ConsolePutChar()` to be more architecture independent.  I will think on this a bit....

So, I did go ahead and make the `ConsolePaintChar()` function, as I had identified in Redmine #347.

The 2 key functions calls that I have ahead of what has been implemented so far is the function to set the cursor type and the other to clear the screen.  I think those are the next 2 I will be implementing.  This way, I will be able to look forward rather than backward.


---
---

**2018-05-28**

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
---

**2018-05-29**

So century put the GDT at address 0x00000000.  It was also set prior to leaving the assembly language loader.  I want to duplicate that here, but I am not convinced that the address is really the best thing to use due to null pointer assignments.  Now, on the other hand, since I will be using paging, I can keep that page unmapped and be relatively safe.  So I think I will keep that setup.

The other thing to make sure I bring up is the BIOS Data Area (BDA) and the Multiboot info.  These really should work in conjunction with the `cpuid` instruction.  At some point, I will need to implement those into the CpuGetCapabilities function.

So I am not starting to build out the Interrupt Descriptor Table (IDT).  In reading, not every interrupt will clear the interrupt flag.  However, if the interrupt is an exception, the CPU (i686) will clear the IF.  Therefore, I will `cli` myself just to be sure.  Now, since they are all interrupts, the `iret` opcode will also restore the previous flags value.

The problem is where to put the IDT.  For 256 interrupts, the table will be 256*8 bytes long, or 2K, or 1/2 of a page frame.  The GDT is 128 bytes long in its current configuration.  I think it might make sense to put them into the same 4K frame, with the IDT being in the last 2K if this frame.  That would be at address 0x00000800 and a limit of 0x000007ff.  Like the GDT, I am going to load this in the in the loader before call `kInit()`.

The final thing to do tonight as I wrap up for the evening is to build the actual IDT able in place (at address 0x800) as part of the initialization.  That will be a task for tomorrow.


---
---

**2018-05-30**

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
---

**2018-05-31**

I am working today on bringing over the heap functions.


---
---

**2018-06-01**

OK, I have been able to bring over all the heap functions.  Having done that, I think I may be running into a problem where the source folder is going to get quite full of files.  One one hand, I am not too concerned about it since the file is named the same as the function and all names start with the module (except maybe CPU).  On the other hand, it would be much easier for anyone else to find what they are looking for if all the heap functions and files were in a heap folder.  For now, I will leave it as it is but I will likely revisit this as I grow the kernel.

In the meantime, I commit and push since this is a rather significant milestone.

---

Now that the commit os done, I really need to be able to identify how much heap I really can have.  For the moment I have hard-coded the upper limit.  I was thinking I should get the upper memory limit from MBI, but that is not really a good solution since I will have paging enabled and will have a virtual limit.  I need to think this through but for the moment, I will keep it hard-coded.

The next thing to work through is paging.  However, I also need to set up the kernel into upper memory, which was not done in Century32.  It was in Century64 (assembly) and Century (multi-architecture).  This will be the hardest part of the port so far.  Tomorrow....


---
---

**2018-06-02**

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
---

**2018-06-03**

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
---

**2018-06-04**

Today I start with organizing the include files a little better.  I have 2 types that I need to maintain: 1) the internal kernel ones and 2) the ones required by the standalone OS programs.  The former should never make it to the output image and are expected to only be used for the loader, libk, and kernel modules.  The latter will need to be written to the /include folder on the resulting boot image.  Note that the loader, libk, and kernel will pull from both locations.


---
---

**2018-06-05**

I am working on getting the memory map pulled out so I can map out the physical memory.  More work on this today.


---
---

**2018-06-06**

I have the Multiboot 1 Information structure detailed out.  Now, I need to figure out how to get output and debugging information.  I have 2 choices at this point: 1) output to the screen, and 2) output to the serial port.  On one hand, I may want to output to the screen before the screen is ready for output and on the other hand the rpi serial port is not perfect with the cable I have been using.  A third potential option would be to output the log to a fixed point in memory and use a debugger to read it.  I think the best bet is going to be the serial port since I will be able to use that consistently with the emulators.  This will be only for the loader as the kernel will be able to write logs to files for debugging.

I will work on a UART driver for the loader.


---
---

**2018-06-07**

OK, I have the most basic serial output enabled for the loader, which will allow for the most basic debugging.  I am only able to output strings (no numbers, pointers, or hex), but I will add that as I need them.

I do need to go back and revisit the structure (which has not been checked in) and I have several files I need to properly comment.

With that complete, I will commit these changes at this point publicly as there are so many changes in structure and code, but not functionality.

---

Now, with that done, I can continue to parse the multiboot information structures.


---
---

**2018-06-08**

I am pretty happy with where I have gotten with the MB1 parsing.  It is not complete but I have enough to begin the physical memory manager initialization.

Next is to implement the same things with the MB2 Information structure.

And now that this is complete, it is time to set up the first 128MB of the PMM.  For this, I will need a bitmap and a location to put the data.

Actually, the PMM initialization I have written for century will initialize the entire physical memory space into the bitmap, and I will copy that to century-os.  That is important to note is that the PMM bitmap is placed just prior to either the EBDA or the 640K low memory cap, whichever is lower.  I have not yet found the EBDA, so I need to go after that first.


---
---

**2018-06-09**

OK, now to pull data from the BDA, which will include identifying the EBDA location.


---
---

**2018-06-10**

This morning I have been able to complete the code that reads the BDA and stores its contents in the internal hardware communication area.  Included in this is the location of the EBDA, which if it exists will serve as the upper boundary for the PMM bitmap in physical memory (alternatively, it will be the traditional 640K low memory limit).

So, returning to the top-down-ish approach for the loader, I can resume development on the PMM initialization.


---
---

**2018-06-11**

Today I am working on completing the PMM initialization.


---
---

**2018-06-12**

I have completed the fundamental PMM initialization given all the information I have mined so far.  I still have the video buffer and the modules to map, but I am not yet prepared for this.  I am going to commit these changes to the public repo and figure out what to work on next.

---

I think the next order of business will be to print out the greeting.  Since I am already in graphics mode from the boot loader, I will need to "paint" the characters on the screen.  I have already included the system font in the loader binary, so I just need to know the frame buffer address.  Thankfully, I have the frame buffer data in the multiboot data.  So, I need to extract this data from the loader, and allocate the frames in the PMM as part of the PMM initialization.  Finally, I will be able to paint a greeting on the screen (one pixel at a time).


---
---

**2018-06-13**

Today I was able to get the frame buffer to clear, setting the background to blue.  The next things to collect are the functions that will write the greeting on the screen and output that greeting.

The greeting is completed, so I will commit publicly again.


---
---

**2018-06-17**

So, the last 2 major things to do to get the minimal loader operational it to A) Enable paging, and B) parse the ELF file of the kernel proper (and other modules) and move them to the final location.

So, I think the best thing to start with is getting the ELF modules copied into the proper locations in memory (I need to map that memory for the PMM anyway).  So, this is not something that I have every fully implemented before, so it might take some time to get it right.  I cannot just copy the code from another OS attempt into this one.  This means that I might just have to implement some additional functions to output hex values and the like -- I'm likely going to need them for debugging.  I will build them as I go, but I do expect that this will be rather quickly once I get something that will execute.

The other things is where to write the debugging output.  The best place at this point I think will be the serial port as that is maintained on the console with a decent scroll-back buffer.  Not to mention I can easily redirect the output to a file if I need to.

Now, there are 2 standards I can look at implementing..  a 32-bit and a 64-bit ELF standard.  I am not yet familiar with them enough to discern the difference.  So, reading it is...

It looks like if I am able to write the ELF data types properly, I should be able to merge the 2 specifications together cleanly.  The 64-bit spec is supposed to be a minimal extension of the 32-bit spec.  So, I start with merging the data types and the program header structure.

So, I managed to get the ELF File header structure written and now need to identify the modules that were loaded by multiboot and figure out what I am going to do about them.  At the moment, I am parsing the multiboot information prior to initializing the PMM.  This means I will need to capture the module information as part of the multiboot parsing, then allocate the memory that is already in use by these modules in the PMM, and then finally parse each module's ELF information to extract the program and data and prepare each module for execution.

So, I go and get the module information from both MB1 and MB2....


---
---

**2018-06-23**

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
---

**2018-06-24**

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
---

**2018-06-25**

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
---

**2018-06-26**

Today I realize that I am going to need to debug the paging table maintenance.  In particular, I need to write a hex number to the serial port.  I will start working on that feature today.

I managed to get a bunch of debugging done today.  I will need to comment out all the changes I have made, but I am not sure I have all that in me this evening.  I will start tonight and probably finish this work tomorrow.


---
---

**2018-06-27**

OK, I have managed to get everything documented the way I want (I think anyway).  I now need to initialize all the rest of the kernel mappings, including the loader which will have to be de-allocated by the kernel.  There are also a couple of additional thing I need to map into a final location as well, so the virtual memory map from 25-Jun will change.


---
---

**2018-06-28**

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
---

**2018-06-30**

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
---

**2018-07-01**

So, the first order of business today is to get this code committed.  I need to work through all the files I have modified and make sure the comments are proper and up-to-date.

---

With that commit pushed to github, on to the next item on the list.  That is going to be to get the kernel to launch and to print a second greeting on the console.  This will require that paging is enabled which is going to require a lot of debugging.  The problem is going to be that I do not have a Page Fault handler that I have written or installed, so any issues at the moment are going to triple fault the PC and cause it to reboot, with no indication of where the problem is.  For now, I will start with the `MmuDumpTables()` function to check on things, but I have a feeling I will need a rudimentary Page Fault handler to dump data to the serial port.  I will write it when I just cannot tolerate not having it anymore, which will be far later than I really should write it.

But before I get into that, I am a formatting change I want to make to this journal file.  I am going to go back an put horizontal lines between the different days to aid in reading.  I am not going to make any changes in the content (I'm not even going to correct any typos).

And,...  as I predicted,... I have a triple fault as soon as I enable paging.  So I have some debugging to do.

OK, so the stack was not mapped into the paging tables.  It needed to be frame-allocated and then identity-mapped.  Once I got that completed, I was able to get paging enabled and not have a triple fault.  The key here is that the stack is also hard-coded at the frame right below 2MB and that will eventually get stepped on by the loaded modules.  I have entered a Redmine for that to fix and will likely be built into the loaded executable (loader.elf) at some point.  For now, I can get away with it.

So, the final thing is going to be to jump into the actual kernel code and acknowledge the arrival somehow.

Just to make a note of where I am at this point, I am jumping to a kernel location (which I think is a good location) but the hardware structure is not available to the kernel.  As a result, I am not able to confirm it with what happens on the screen.  I mistakenly moved `hw-disc.cc` to libk but it really needs to be copied to a good location in low memory by the loader and then pointed to in the kernel.

OK, I was not able to get the frame buffer to work, but I was able to product serial output that demonstrates that I am executing the kernel.  I do have some things to work out, but the video driver will probably move the frame buffer if possible.  So, I am going to call this objective complete (the kernel is being executed) and commit this the code at this point.  I am also going to zip up an .iso image that can be run from a command line and drop that on the github project as well.


