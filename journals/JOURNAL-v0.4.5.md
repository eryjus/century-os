# The Century OS Development Journal - v0.4.5

## Version 0.4.5

The 0.4.x series of versions will enable SMP on both architectures.  This is all new things for me, so I do expect this to take a lot of work to get dialled in.

There is a roadmap I have developed:
* v0.4.0 -- enable caches and branch prediction on the arm cpu; and debug the resulting mess
* v0.4.1 -- replace the 8259 PIC with the Local APIC on x86-pc arch
* v0.4.2 -- implement atomic integers and replace the foundation of `Spinlock_t` implementation with `AtomicInt_t`
* v0.4.3 -- implement inter-process communication (IPC) in near-compliance with Posix -- these will be synchronization primitives
* v0.4.4 -- implement messaging in near-compliance with Posix
* v0.4.5 -- implement inter-processor interrupts and CPU-specific memory/structures
* v0.4.6 -- finally enable the APs; cleanup the resulting mess

---

### 2019-Jun-07

So, the next task in the list is to develop IPI and cpu-specific data elements and structures.  The first task here is to be able to determine at runtime what the cpu number is any task is executing on.  I think I will start there and then start to define a structure for all the CPU-related master-level data and the per-core data that will be defined in an array.

Once again, I am going to work on the rpi2b arch first.

---

### 2019-Jun-08

So, I have to update `pi-bootloader` to be able to transfer control to the kernel along with the boot processor.  At this point, nothing is left dependent on `pi-bootloader` code and that memory area can now be compeletly reclaimed.

---

So, with that, I now need to be able to build the arm functions to perform inter-processor interrupts.  In the rpi platform, there are 16 mailboxes for communicating between processors.  These are labeled for Core 0 (4 registers), Core 1 (4 registers), Core 2 (4 registers), and Core 3 (4 registers).  Each of these sets of 4 registers are labeled as Mailbox 0 through Mailbox 3.  It seems to me that if I want to interrupt Core 0, I need to send a message to a mailbox for Core 0, and the source Core will indicate which of the 4 mailboxes I send to.

So, for any given core, I think I want to use Mailbox 0 to indicate messages that come from Core 0, Mailbox 1 for Core 1, etc..  In this manner, I can identify from where a message comes and if a response is required (I have no clue what that might be!), I know where to send it back to.

`lk` is only using Mailbox 0 for each core, and is using a shifted `1` to determine what the request is.  There is no directed messaging, but just broadcasts.

So, it makes me wonder if I just need to be able to send an interrupt as a broadcast.

`x86` allows for up to 256 (well, maybe 240) interrupts to be passed to the processors -- each message becomes its own interrupt vector.  `arm` allows a full 32-bits of messages.

---

I have the sending side written for both archetectures, but not the receiving sides.  However, I am not yet certain what I need to be able to send (and more to the point to receive).  So, as much as I want to get this working fully before I move on, I am not sure what more I can really do until I have a purposeful use case.  After much internal debate, I think I am going to commit now and fill in any leftover pieces later.







