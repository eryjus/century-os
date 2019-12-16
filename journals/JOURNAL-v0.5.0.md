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


