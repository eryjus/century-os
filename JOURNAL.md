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

