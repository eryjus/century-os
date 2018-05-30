***The Century OS Development Journal***

***Prologue***

I found some old code that I want to reuse.  The most successful iteration I have had was the second 32-bit version.  I thought I had lost that code (see below).  So I am going to try to resurrect it with this iteration.  Plus, I will clean up after myself a bit.

My goals from the previous iteration are all still valid, I am just hoping to move this along a little faster.  I plan on combining code from several sources here.


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


**2018-05-23**

The first orders of business are the basic structure and the build system.  First, I would like to structure to be relatively flat, but broken up into modules.  The recovered code from Century32 is a monolithic kernel, so everything was in the main directory.  On the other hand, several things are going to be hardware-specific and need to be placed into their own folders.  I may not be able to keep the goal to be flat as cleanly as I would prefer.  Back to that in a bit...

As for the build system, I prefer `tup`.  Is is very fast and have never let me down and forcing me to do a total rebuild.  However, there are just some things that `tup` requires that I do not like as well, such as forcing build definition files into the target folders.  Well, we can use `make` to manage `tup`, and that is the approach I will take.  For more information, check out the [readme](./README.md) file.

So, back to the directory structure...  This might sound a little odd, but I think the starting point should be the `Makefile` from century.  That `Makefile` is optimized for `tup` already and already takes care of some of the management I was referring to above.  It will be an easy clean-up from that start, so I will go and copy it.


**2018-05-24**

Today I am still working on getting the structure aligned.  I am continuing to follow the structure laid out in `Makefile`.

I also have another thought I want to work through...  NASM vs. GAS.  One is a dedicated assembler (NASM) while the other is part of the GNU compiler suite and therefore more integrated.  The problem with GAS is that it uses AT&T syntax -- which I believe leads to bugs.  I personally prefer the NASM syntax.  However, if I am recalling properly there is something I have to do when I link...  Stay tuned on that one, but I will use `nasm`.

At this point, I am starting to write `lists.cc`.  I wanted to take a moment to talk about the high-level language I am using.  I am writing my code largely in C.  However, I am going to make use of several features of the C++ compiler (inline functions as an example).  My call-out here is that you should expect to be reading Procedure Oriented code.  This does, however, set up a situation of name mangling with assembly.  To overcome this, any asm function will also be declared as `extern "C"`.

Another thought about organization is the number of functions to put into a single file.  Compiles run much faster with multiple functions in a single file (reducing the number of files to compile).  However, it is easier to maintain and read/understand the entire system with a single or few functions in each file.  Also, I believe it will aid portability.  So I am going to change back to a single function in each file.


**2018-05-25**

Today I will continue laying out the basic of the kernel support functions.  In this case, lists. 

I have decided I will not be using `doxygen` for the kernel (at least not at this point).  Therefore, I am pulling out all the tags from the migrated code.  In the meantime, I am also working on thinking through the layout of the source and build system.  As an example, do I really need to compile asm from an architecture-independent folder and C from an architecture-specific folder?  Well, I have several things to work out as I lay in this first bit of code.

So I have finished the lists, changing the entire interface into inline functions.  At this point, I am going to perform a local interim commit to hold the state.

I have also been able to get the kernel to boot and immediately lock up (which is the intent -- no triple fault).  This is a rather big milestone, so I will locally commit again.

So, now after the commit, what is next?  Well I need to get data to the screen.  Let's face it: any sort of debugging is going to be impossible without that ability.  Century32 has a number of kScreen functions that outputs to the screen.  However, the rpi2b does not have a text mode.  For the moment, I am going to ignore that fact and start in text mode.  I will revisit this decision when I start the rpi2b version.

Also, another note is that I am going to start putting my TODO list into my own internal Redmine bug tracker.  However, one feature of century32 that I want to leverage into century-os is the status bar on the bottom of the screen.  This was a good tool to determine what was going on for the session -- a feature I lifted from the AS/400 green screens.

As I get into the early screen functions, I am struck by the amount of code duplication I had to deal with in century32.  I recall that this bothered me then as well and I never really addressed it.  The concern is that I had `kprintf` and a `ttyprintf` functions and the majority of the body of both functions were similar.  I was able to leverage some of the `kprintf` code, but that was not good enough.  So, I would rather write a `kputs` function that writes early messages to the screen and keep the bulk of the `kprintf` stuff really optimized for the tty functions.  I will be giving up the ability to print numbers and other things using `%` tags, but I think that is OK for the moment.  Just like `puts`, `kputs` will append a newline after writing the string.


**2018-05-26**

Today I will start to bring over early console functionality.  This will also drag over several supporting functions and I will make the effort to bring over all the similar functions at the same time.  I will start with a `ConsolePutChar` function and then build from there.


**2018-05-27**

Today I have finished up copying in several functions and getting my `ConsolePutS()` implementation working.  I now have a greeting on the screen.  This is a milestone so I will commit these changes locally.

So, since I am working on the screen, I think the next thing to do is to write the status bar.  This way the screen is actually 'complete'.  The status bar will have to be blank for now since I don't have too much to actually paint into the status bar, but I will start laying the foundation.  What I am thinking about is the Redmine (my internal bug tracker, sorry for the loss of visibility on that) I have put in for the `ConsolePutChar()` to be more architecture independent.  I will think on this a bit....

So, I did go ahead and make the `ConsolePaintChar()` function, as I had identified in Redmine #347.

The 2 key functions calls that I have ahead of what has been implemented so far is the function to set the cursor type and the other to clear the screen.  I think those are the next 2 I will be implementing.  This way, I will be able to look forward rather than backward.


**2018-05-28**

Today I started to wrap up the rest of the early console functions.  The last thing is the cursor geometry function -- and I do not see the value in it at the moment.  I may pull it in later.  So, I will commit locally at this point and move on to CPU initialization.

Actually, I pushed the commits to GitHub since I figured there was enough working to make complete an interim push....

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


**2018-05-29**

So century put the GDT at address 0x00000000.  It was also set prior to leaving the assembly language loader.  I want to duplicate that here, but I am not convinced that the address is really the best thing to use due to null pointer assignments.  Now, on the other hand, since I will be using paging, I can keep that page unmapped and be relatively safe.  So I think I will keep that setup.

The other thing to make sure I bring up is the BIOS Data Area (BDA) and the Multiboot info.  These really should work in conjunction with the `cpuid` instruction.  At some point, I will need to implement those into the CpuGetCapabilities function.

So I am not starting to build out the Interrupt Descriptor Table (IDT).  In reading, not every interrupt will clear the interrupt flag.  However, if the interrupt is an exception, the CPU (i686) will clear the IF.  Therefore, I will `cli` myself just to be sure.  Now, since they are all interrupts, the `iret` opcode will also restore the previous flags value.

The problem is where to put the IDT.  For 256 interrupts, the table will be 256*8 bytes long, or 2K, or 1/2 of a page frame.  The GDT is 128 bytes long in its current configuration.  I think it might make sense to put them into the same 4K frame, with the IDT being in the last 2K if this frame.  That would be at address 0x00000800 and a limit of 0x000007ff.  Like the GDT, I am going to load this in the in the loader before call `kInit()`.

The final thing to do tonight as I wrap up for the evening is to build the actual IDT able in place (at address 0x800) as part of the initialization.  That will be a task for tomorrow.


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