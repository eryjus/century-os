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

### 2019-Feb-07

The journal file was getting to be a bit unwieldy.  `vscode` was not able to keep up with my (slow) typing and would peg the CPU trying to spell check this file.  As a result, today I split my journals into several 'chapters', by version.  The original Prologue is above and will remain intact.  In fact, I was careful to split the journals properly and not lose any of my commentary.

---

* [Original Century Journal](JOURNAL-century.md)
* [Version 0.1.0 Journal](JOURNAL-v0.1.0.md)
* [Version 0.2.0 Journal](JOURNAL-v0.2.0.md)
* [Version 0.3.0 Journal](JOURNAL-v0.3.0.md)
* [Version 0.3.1 Journal](JOURNAL-v0.3.1.md)
* [Version 0.3.2 Journal](JOURNAL-v0.3.2.md)
* [Version 0.4.0 Journal](JOURNAL-v0.4.0.md)
* [Version 0.4.1 Journal](JOURNAL-v0.4.1.md)
* [Version 0.4.2 Journal](JOURNAL-v0.4.2.md)