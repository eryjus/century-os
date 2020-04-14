# The Century OS Development Journal - v0.6.1

This version will start to build the messaging foundations.

## Version 0.6.1a

This micro-version will implement the `msgget` system interface.

---

### 2020-Apr-08

The Posix XSI interfaces specification indicates that a filename must exist as part of the requirements to generate a `key_t` which is used to get to a unique message queue.  The function of building this key value resides in the user-space C-library -- this is not a kernel function.  But a kernel that uses these queues also needs to be able to build these `kKey_t` values.  I will use hard-coded values for now.  I may come back and revisit this later.

So, the first thing I need to do is to define the type `key_t`.  `glib` makes a mess of this by all kinds of `#defines` and intermediate `typedef`s.  I'm not going to get that fancy.

Posix says that this prototype is defined in `sys/msg.h`.  However, this is for user space.  I need a kernel space function to handle this work.  The function name will not be the same.  It's also clear that the actual implementation is up to me -- I only need to be able to provide the proper interface to user-space.

So, with that, I really only need to get started with the code.  The key things to keep in mind is that the interface functions:

```c++
int       msgctl(int, int, struct msqid_ds *);
int       msgget(key_t, int);
ssize_t   msgrcv(int, void *, size_t, long, int);
int       msgsnd(int, const void *, size_t, int);
```

OK, I think I have the header file all worked out.

---

### 2020-Apr-09

I was thinking last night about ownership of the message queue.  I am going to need a field to describe the owner at some point.  But I will add that when I get there.  For now, I will be just assuming that when the number of processes referencing a message queue, it is available to be deleted.

Now, one process can reference many message queues, and one message queue can be referenced by many processes.  This is a many-to-many relationship and that makes a mess of things.  To resolve this, I am going to need a structure to resolve these references.  Or...  can I just get away with a reference count?  Nope....  If something dies, I need to be able to clean up its mess.  I need specific references.

With that, I think I have all I need to start my `MsgqInit()` function, and then the `MsgqCreate()` function.

When releasing the reference to a resource, the trick is to find both the Process Reference and the Resource Reference and remove both.  For safety sake, they need to be checked.  The problem is the many-to-many relationship and the list implementation: I have to search both lists for a match.  This is ***ugly!!***  I'm thinking O(n<sup>2</sup>).  The only thing I can think of is to add the `Process_t` address and the resource address to the `Reference_t` structure to make checking easier.  With that, I should only need to check the process's list of references and if that does not exist I can just call it and move on.

---

I was able to get all the messaging kernel functionality written.  I do not yet have the Posix SYSCALL interface ready and that will not come for some time.  Everything compiles.  I will have to construct a couple of test processes to see this working.

And both processes block on a MSGW status.  So, this can be likely one of a few things:
* The message does not get queued properly
* The process is not added to the waiting queue properly
* A message is not being identified to be received properly
* A process is not getting readied properly when a message arrives

I cannot find anything rubber-ducking the code.  I'm going to have to build some debugging functions for message queues.

```
+---------------------------+--------+----------+----------+------------+-----------------------+
| Command                   | PID    | Priority | Status   | Address    | Time Used             |
+---------------------------+--------+----------+----------+------------+-----------------------+
| kInit                     | 0      | OS       | DLYW     | 0x90000040 | 0x00000000 0x00d12ec8 |
| Idle Process              | 1      | IDLE     | READY    | 0x900000e0 | 0x00000000 0x7bc03ca0 |
| Idle Process              | 2      | IDLE     | RUNNING  | 0x90000180 | 0x00000000 0x7bb87470 |
| Idle Process              | 3      | IDLE     | RUNNING  | 0x90000220 | 0x00000000 0x7bb04a98 |
| Idle Process              | 4      | IDLE     | RUNNING  | 0x900002c0 | 0x00000000 0x7bb288d0 |
| kInitAp(1)                | 5      | OS       | TERM     | 0x90000360 | 0x00000000 0x00009088 |
| kInitAp(2)                | 6      | OS       | TERM     | 0x90000400 | 0x00000000 0x000088b8 |
| kInitAp(3)                | 7      | OS       | TERM     | 0x900004a0 | 0x00000000 0x000088b8 |
| Process A                 | 8      | OS       | MSGW     | 0x90000648 | 0x00000000 0x000007d0 |
| Process B                 | 9      | OS       | MSGW     | 0x900006e8 | 0x00000000 0x00000bb8 |
| Kernel Debugger           | 10     | OS       | RUNNING  | 0x900007bc | 0x00000000 0xa4fd6958 |
+---------------------------+--------+----------+----------+------------+-----------------------+
sched :>
 (allowed: show,status,run,ready,list,exit)
```

As you can see, `Process A` and `Process B` are not getting any CPU once they block on the message queue for the first time.

What kinds of things do I need to see from a debugging function for message queues?  Well, I think I need to show a list of them, with the number of messages queued, the number of processes waiting, and the status of the queue overall.  I may have to take that on tomorrow.

---

### 2020-Apr-10

I am working today on getting some debugging code written to view the message queues.  I will have 2 commands so far:
1. `status` -- which will give the overall status of the queues
1. `show` -- which will give lots of details about one of the queues in question

Implementing the first one will be relatively easy.  The second one, on the other hand, is going to require either some name or ordinal queue number to work, which requires updating my structures a bit.  Let me start with some status code.

OK, interesting:

```
- :> msgq
msgq :> stat
+------------+--------+--------------+------------+--------------+------------+
| Queue Addr | Status | #Msgs Stated | #Msgs Calc | #Wait Stated | #Wait Calc |
+------------+--------+--------------+------------+--------------+------------+
| 0x90000540 | ALLOC  | 1            | 0          | 0            | 0          |
| 0x900005c4 | ALLOC  | 1            | 1          | 0            | 0          |
+------------+--------+--------------+------------+--------------+------------+
msgq :>
 (allowed: status,show,exit)
```

I have a message that was pulled out of the queue but the count was not updated properly.  Also, the number of waiting tasks is 0 for both queues yet both processes are waiting in a `MSGW` status.

And I found a bug when I block waiting for a message -- it was not queueing the process into the waiting queue.

So, after testing, I think it is time to commit and merge this micro-version.  The next step will end up being the post-boot cleanup, and turning the `kInit()` process into the `Butler()` process to maintain the system.

---

## Version 0.6.1b

In this micro-version, I will create the butler process.  I believe that cleaning up the startup code and memory is going to expose several problems.  I will take this slowly perform several commits along the way.

First, let's assume the correct name.  Well, the name and the priority:

```
+---------------------------+--------+----------+----------+------------+-----------------------+
| Command                   | PID    | Priority | Status   | Address    | Time Used             |
+---------------------------+--------+----------+----------+------------+-----------------------+
| Butler                    | 0      | LOW      | MSGW     | 0x90000040 | 0x00000000 0x00004a38 |
| Idle Process              | 1      | IDLE     | RUNNING  | 0x900000e0 | 0x00000000 0x00a4d350 |
| Idle Process              | 2      | IDLE     | READY    | 0x90000180 | 0x00000000 0x00a57b48 |
| Idle Process              | 3      | IDLE     | RUNNING  | 0x90000220 | 0x00000000 0x00a47d60 |
| Idle Process              | 4      | IDLE     | RUNNING  | 0x900002c0 | 0x00000000 0x00a490e8 |
| kInitAp(1)                | 5      | OS       | TERM     | 0x90000360 | 0x00000000 0x00016b48 |
| kInitAp(2)                | 6      | OS       | TERM     | 0x90000400 | 0x00000000 0x00013880 |
| kInitAp(3)                | 7      | OS       | TERM     | 0x900004a0 | 0x00000000 0x00016760 |
| Process A                 | 8      | OS       | MSGW     | 0x90000648 | 0x00000000 0x00002af8 |
| Process B                 | 9      | OS       | DLYW     | 0x900006e8 | 0x00000000 0x000003e8 |
| Kernel Debugger           | 10     | OS       | RUNNING  | 0x900007bc | 0x00000000 0x00db21a8 |
+---------------------------+--------+----------+----------+------------+-----------------------+
sched :>
 (allowed: show,status,run,ready,list,exit)
```

I now have the basic process loop waiting for something to do.  None of the proper clean-up has been done yet.  So, what is first??  I think I will start with the low memory.  There is a lot to free there that might be needed for drivers that will get loaded soon.  But are also a number of land-mines to navigate.

The notes I took [here](http://eryjus.ddns.net:3000/projects/century-os/wiki/Low_Memory_Usage_Map) are critical to this reclamation.  Some of this is, however, arch-dependent.  For x86, I have several structures that need to remain in this memory.  For armv7, this is all fair game.

I think the best thing to do is to create an arch-specific function that will accept a frame number and return whether the frame is valid to free.  For arm, this will be trivial -- true.  For x86, this will need to be implemented as a function with some real logic.  First, met me make sure I have the EBDA properly bounded and the starting address recorded.  Which it is not.

How about the GDT frame count?  Am I logging that?  I think I am only using 1 frame.  It is only 1 frame.

OK, so I have a few functions written and have reorganized a bit of code.  The first thing I am going to do is unmap the memory below 1MB.  Well my first test triple faults reading the EBDA.  The page would be for frame 0, which I am not mapping.

OK, I think I have the low memory cleaned up.  Time for a commit.

---

### 2020-Apr-11

Looking at the linker script, I have several blocks of memory that need to be reclaimed.
* The multi-boot entry point -- This block of memory is located at 1MB.  The concern is that this is also the block of memory that contains the location positions.  These will need to be copied out to at least the stack.
* The loader -- This memory performed much of the initialization of the system.  This is located at `0x80000000`.  Hopefully, everything I need to manage the system is moved out of this block of memory.
* AP Trampoline code -- This memory is initially located as part of the kernel code, tacked onto the end.  It was copied to physical address `0x8000` and that will be its permanent location.  This is only 1 page.

I have that all written, but there is a problem with releasing memory.  I end up with a deadlock on the rpi -- there is no fault reported, making it difficult to debug.  So, I will comment out a bunch of code and add it back in slowly.

Even after commenting the code, I managed to get an exception:

```
Data Exception:
.. Data Fault Address: 0x01000000list,exit)
.. Data Fault Status Register: 0x00000807
.. Fault status 0x7: Translation fault (Second level)
.. Fault occurred because of a write
At address: 0xff800f68
 R0: 0x00000000   R1: 0x00000005   R2: 0x01000000
 R3: 0x01000000   R4: 0x00000034   R5: 0x9000038c
 R6: 0x810005e8   R7: 0x80803f18   R8: 0x90003ca0
 R9: 0xff800fec  R10: 0x00000000  R11: 0x90003ca0
R12: 0x90003cd4   SP: 0x81000848   LR_ret: 0x81000850
SPSR_ret: 0x600001f3     type: 0x17

Additional Data Points:
User LR: 0xefefcfbf  User SP: 0xefffeef9
Svc LR: 0xff800f78
```

This fault does not make sense.

Disabling all the code I just added and pedantically adding code back in, the following line appears to be the problem:

```c++
    PmmReleaseFrame(krnSmpPhys >> 12);
```

Now, what I am not certain of is whether this is from the function call or from the frame itself being freed.  Also, I am not certain I am getting consistent results.

What the heck??!??!?!:

```
!!! ASSERT FAILURE !!!
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessUnlockAndSchedule.cc(31) AtomicRead(&scheduler.postponeCount) > 0 postponeCount out if sync

Prefetch Abort:
```

... and then it deadlocks.

Interestingly enough, increasing the built-in delay when performing the TLB flush IPI, seems to help with this problem.  It makes me wonder what's going on.  Do I need a soft barrier?

Well, adding `SoftwareBarrier()` into the code also breaks it -- causing it to loop.  Now, if I remove the delay, the code locks hopelessly!  I'm betting the code is horribly broken to begin with.  I just have no clue where to look or how to debug it.

Hmmmmm...  x86 works really well.  This makes me wonder if I have a flaw in the IPIs on rpi2b.  The Mailbox functions for the bcm2836 will stop the interrupt once the value is 0.  So, am I doing this right??  It feels like not all the CPUs are engaged for the IPI.

Ok, this may be a problem:

```c++
EXTERN_C EXPORT KERNEL
void _PicBroadcastIpi(PicDevice_t *dev, int ipi)
{
    if (!dev) return;
    if (!dev->ipiReady) return;

    for (int i = 0; i < cpus.cpusRunning; i ++) {
        if (i != thisCpu->cpuNum) {
            MmioWrite(IPI_MAILBOX_BASE + (0x10 * i), (archsize_t)ipi);
        }
    }
}
```

There is nothing to keep this process from migrating during the entire IPI.  This may be a big problem.  More to the point, the process waiting for the IPI ack may be migrated to another CPU before the IPI sending is complete.

Hmmm...  still problems..  I think I need to write something into the debugger to check the TLB Flusher IPI.  I think the first part is going to be to replace the name of the command.  Well, that didn't work.

Let's see here...  is there anything I can do with the tools I have?  I can see the running processes on each CPU.  I may be able to create a new status just for IPI handling to see what is waiting for what.....

So, I put an extra control into the rpi2b to wait before returning from the `PicBroadcastIpi()`, making certain all the cores are engaged before moving on to another task....  I am testing this now.  The problem is that it was not consistent, so success looks like the absence of a problem.

5 rpi executions, and no issues.  I was about 50% problem rate.  I have high enough hopes I am going to add back some more code.

OK, so far so good.  Nothing major is majorly broken.  Now to clean up the PMM frames.  This will be a longer process to complete since I will be zeroing nearly all the memory.

This is not taking much time at all....  Time to review the `PmmInit()` function to see what I did there, and maybe create some debugging functions.  OK, I put everything from 4MB to the end of memory on the normal stack.  So, the time to clear the few frames is correct.  On the other hand, there is a huge hole between 1MB and 4MB where memory should be freed.  I can probably change the `LowMemCheck()` function to handle all memory up to 4MB -- or moreover, encapsulate that function in a `MemCheck()` function that calls the `LowMemCheck()` function for anything < 1MB.

OK, when I try to clear the free PMM frames between 1MB and 4MB, I am getting a triple fault.  Something in my checking is not working out.

---

### 2020-Apr-12

OK, I am still faulting trying to clear the first 4MB of physical memory.  Also, it is in the first part of the 1MB memory range.  I realized I was shifting the wrong direction to convert a frame to an address.  I must have been tired last night!!

OK, so x86 is working well now.  But rpi is deadlocking.  More than likely, I have some element somewhere that is not mapped to the correct section.  Since x86 is working properly, I am betting it's going to be either in the values provided by the linker or in the arch- or platform-specific code.

I wonder if I am properly relocating the IVT address into virtual memory..??  OK, for rpi, `ExceptionInit()` is not actually called so the IVT is never moved to its proper virtual memory location.

Wait a minute!!!  This is not right:

```
Found something to do for real; the block is at address 0x9000ae6c
.. The block starts at frame 0x100 and has 0x1 frames
Found something to do for real; the block is at address 0x9000ae38
.. The block starts at frame 0x101 and has 0x1 frames
Found something to do for real; the block is at address 0x9000ae04
.. The block starts at frame 0x111 and has 0x1 frames
```

So I am trying to clear frame `0x111`, but from `readelf`:

```
Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .entry            PROGBITS        00100000 001000 001000 00  AX  0   0 16
  [ 2] .loader           PROGBITS        80000000 002000 003000 00  AX  0   0 4096
  [ 3] .syscall          PROGBITS        80400000 005000 001000 00 WAX  0   0  4
  [ 4] .text             PROGBITS        80800000 006000 00c000 00  AX  0   0  8
  [ 5] .data             PROGBITS        81000000 012000 005000 00  WA  0   0  8
  [ 6] .stab             PROGBITS        81005000 017000 0bc000 00   A  0   0  4
  [ 7] .ARM.attributes   ARM_ATTRIBUTES  00000000 0d3000 000039 00      0   0  1
  [ 8] .symtab           SYMTAB          00000000 0d303c 009530 10      9 2072  4
  [ 9] .strtab           STRTAB          00000000 0dc56c 0029b1 00      0   0  1
  [10] .shstrtab         STRTAB          00000000 0def1d 000055 00      0   0  1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  y (purecode), p (processor specific)

There are no section groups in this file.

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x00100000 0x00100000 0x01000 0x01000 R E 0x1000
  LOAD           0x002000 0x80000000 0x00101000 0x03000 0x03000 R E 0x1000
  LOAD           0x005000 0x80400000 0x00104000 0x01000 0x01000 RWE 0x1000
  LOAD           0x006000 0x80800000 0x00105000 0x0c000 0x0c000 R E 0x1000
  LOAD           0x012000 0x81000000 0x00111000 0xc1000 0xc1000 RW  0x1000
```

This is part of the `.data` section!  Well no wonder shit's failing.

OK, wait a minute!  rpi does not have or need an SMP section.  So, it has the same addresses as the section that follows it.  And that would be the `.data` section!

OK, so I need to check the size of the section to be certain.  That's it.  This now works.

I am going to commit the code at this point.  The cleanup is complete at this point.  All that's left if to change the butler to respond to messages when a process terminates and when a PMM frame is released.

---

OK, so the first thing to do is to set up the initial messages the Butler will respond to.  So far, I have 2:
* ProcessTerminate
* SanitizePmmFrame

Interesting....  I am now running out of memory:

```
PANIC: unable to allocate memory for freeing a frame


At address: 0xff800f60
 R0: 0x8080a1bc   R1: 0x00000000   R2: 0x00000000
 R3: 0xfffff034   R4: 0x00000000   R5: 0x00000001
 R6: 0x00000336   R7: 0x00100000   R8: 0x00111000
 R9: 0x0000c000  R10: 0x00105000  R11: 0x01007000
R12: 0x00000000   SP: 0x80805028   LR_ret: 0x80805028
SPSR_ret: 0x000001d3     type: 0x80805028
```

This is happening on both archs.  However, I have also written the heap to expand itself.  So I need to get some quick stats from the heap into the debugger.

OK, I am running out of heap space!!  I had made a change when I was working on another version, but I had abandoned that code.  Let me see if I can recover that code....  Nope.  I removed that branch.

OK, now [this Redmine](http://eryjus.ddns.net:3000/issues/405) is catching me -- I have a deadlock between a lock on the PMM, and needing to expand the heap, and expanding the heap needing to get to the PMM again.  To get around this for now, I am going to pre-allocate some more heap in the Butler initialization.  TO be clear, I am not solving this problem.  I am kicking the can down the road.

Hmmmm...  I think I have a really bad race condition.  I am not going to be able to cheat my code.  I have to solve this now.  The problem is that there are a lot of things that require a PMM frame, including messaging and MMU and just about anything that could want memory.  Even the PMM can want more memory from the heap, which will want frames from the PMM.

Long story show, #405 is absolutely correct but it is bigger than just the heap/pmm relationship.  I'm going to have to think on this a bit.

I might be able to get around this with the following changes:
* Add a `SpinLockTry()` function, where I can bail out immediately when I cannot get a lock
* Change the `PMM` structure to also include a number of pre-allocated frames (protected by its own lock as well) -- call it a reserved frame list
* Change the `PmmAllocateFrame()` function to first check for a normal lock and if unable to get that in a reasonable amount of time or if there are not other normal frames left, get a frame from the pre-allocated list
* Change the `PmmScrubFrame()` function to first check if it needs to replenish the re-allocated frame list and if so get that lock -- this should also be a try for a short period of time

I could also change this to look at the low memory if the upper memory could not be locked.

There may still be a risk for deadlock, but those risks are severely reduced.  I need to think on this a bit.

```
[14:01:37] <eryjus_> crap!  rookie mistake:  the pmm is dependent on the heap and the heap is dependent on the pmm...  deadlock!
[14:25:25] <geist> eryjus_: yah i'd suggest having the pmm depend on nothing at all
[14:25:31] <geist> can depend on itself
```

I can think of 3 ways to get this done....
1. Have the `pmm` be its own process, with its own heap.  This is a flawed design as I just add an extra abstraction to the deadlock.
1. Have the `pmm` allocate a slab of memory and use that for managing the stack.  Still worst case is that I will need 768K to be able to manage everything.  That's a lot of overhead.
1. Actually use the frames themselves to hold the stack, keeping only the top of the stack mapped so that I can read the stack and number of contiguous frames.  This completely eliminates the dependencies on the heap, but will slow down the allocations with managing the paging tables (and resulting TLB flushes across cores).

So, I have 2 choices to get through this roadblock.  One option is a half-assed non-solution that kicks the can down the road.  The other a bigger but sustainable and permanent solution.  Hmmmmm....

Seriously, there is not really a choice.  The kernel has enough features in it that everything I do from here is completely dependent on what I have done.  I have to redo the PMM design.

I want to keep the same basic design -- a structure with the frame number (the current one that be being read) and the count of the number of frames in that block, and I would have to add a `next frame` element to the structure rather than the list.  I would keep that frame mapped until all the frames were consumed.  On the side where I re-stack sanitized frames (and try to tie blocks together), I would have another page I could use the check the stack after looking at the top of the stack.  So, instead of 3 lists (Low, Normal, Scrub), I would have 5 pages to map:
1. Low
2. Normal
2. Scrub
2. Search
4. Insertion

Each page would be protected by its own spinlock.

It looks like I have 5 pages starting at `0xff40b000` on both archs that are available for the above 5 pages.  However, if I require a 6th page I will have to separate the pages or move them together to the `0xff42x000` block.

OK, I have backed out enough changes to get the kernel to run again.  At this point I am going to commit the code and tickle the micro-version for the PMM rewrite.

---

## Version 0.6.1c

OK, for this version, I will begin the rewrite of the PMM.  I needed a solid roll-back point before starting this major change.  `v0.6.1b` is the rollback point.

### 2020-Apr-13

Yesterday I managed to complete the rewrite of all but 2 files to the new PMM.  Today, I will wrap up the last 2 files (I hope anyway).

Hmmm...  I also want to be able to pull a block out of the middle of the stack.  To do this effectively, I need a doubly linked list.  However, this will not be from `ListHead_t::List_t`, but will be simply a typical implementation.

---

Hmmm....  as I go through this, I am treating `MmuIsMapped()` as an atomic function.  I need to confirm that I have locks where I need locks.

This is working.  I'm surprised, only 1 bug to sort out.  I will commit this code here and tickle the micro-version.

---



