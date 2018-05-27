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

