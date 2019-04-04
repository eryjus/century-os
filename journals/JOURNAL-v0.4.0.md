# The Century OS Development Journal - v0.4.0

## Version 0.4.0

The 0.4.x series of versions will enable SMP on both architectures.  This is all new things for me, so I do expect this to take a lot of work to get dialled in.

There is a roadmap I have developed:
* v0.4.0 -- enable caches and branch prediction on the arm cpu; and debug the resulting mess
* v0.4.1 -- replace the 8259 PIC with the Local APIC on x86-pc arch
* v0.4.2 -- implement atomic integers and replace the foundation of `Spinlock_t` implementation with `AtomicInt_t`
* v0.4.3 -- implement inter-process communication (IPC) in near-compliance with Posix
* v0.4.4 -- implement messaging in near-compliance with Posix
* v0.4.5 -- implement inter-processor interrupts and CPU-specific memory/structures
* v0.4.6 -- finally enable the APs; cleanup the resulting mess

---

### 2019-Mar-31

I am spending a lot of time today reading.  I am starting with Branch Predictor.

There are 3 operations that happen with Branch Predictor (kind-of in order of the size of the hammer):
1. `BPIMVA` which invalidates the branch predictor for a specified address
1. `BPIALL` which invalidates all branch predictors
1. `BPIALLIS` which invalidates the branch predictor for all inner shareable locations (CPUs in the same inner shareable domain)

So, which hammer is the right hammer for the job?  Well, there are a few things that require this Branch Predictor maintenance:
* Enabling the MMU -- Since this is by core, a `BPIALL` should be the right sized hammer.
* Mapping and Unmapping pages in the MMU -- these structures are shared across all CPUs, so `BPIALLIS` is the right operation.
* Updating TTBR0, TTBR1, or TTBCR -- these are per core operations (as the registers are duplicated), so `BPIALL` will work.
* Changes to VTTBR or VCTLR -- these are unused in Century.

This, then, also gives me a scope of the changes I need to make.

The last thing to research is how to enable the Branch Predictor.  The bit `SCTLR.Z` controls Branch Prediction.  Currently, I am not sure if it is enabled by default or not.  I guess I need to write some code to find out.

```
The current value of SCTLR is 0x00c50879
```

So, the 3 bits I am interested in with this version series are the `C` (bit 2), `I` (bit 12), and `Z` (bit 11):
* bit 2 (`C`) is off
* bit 11 (`Z`) is on
* bit 12 (`I`) is off

I am a little shocked to find that Branch Prediction is already turned on by default.  Hmmm... can it be turned off?

```
The current value of SCTLR is 0x00c50879
The current value of SCTLR is 0x00c50879
```

OK, so it cannot be disabled.  However, this feels like something I should be able to control through `#define`s -- something like:

```C
#ifdef ENABLE_BRANCH_PREDICTION
    // -- do something here
#endif
```

... and then add it into the compile options for rpi2b so that (for now) it is always included.

---

I have the Branch Predictor enabled (well it was already) and I have the required maintenance set and ready to go now.  A quick rpi2b test shows no changes in behavior (which is good since it worked well before I did this).  Since I made changes to the build rules, I really need to verify x86 as well.  Once that is done, I can commit this quick change.

---

There is a sequence of things to do (registers to read) to determine what caches are available on the arm processor.  These involve reading registers.

So, I am going to start reading these registers and dumping the results.

```
The CTR has the value 0x84448003
```

This means:
* armv7 format
* Cache Write-back Granule is 2^4, so the max size that can be written back is 16 bytes (aligned to 16 bytes)
* Exclusives Reservation Granule is 2^4, so the max reservation granule is 16 bytes (aligned to 16 bytes)
* Min Data Cache Line is 2^4, or 16 bytes (aligned)
* L1 Instruction Policy is 0b10, or Virtual Index, Physical Tag (VIPT)
* Min Instruction Cache Line is 2^3, 8 bytes (aligned to 8 bytes)

```
The CLIDR has the value 0x0a200023
```

This means:
* LoUU is `0b001`
* LoC is `0b010`
* LoUIS is `0b001`
* CType1 is `0b011` (Separate Instruction and Data Caches)
* CType2 is `0b100` (Unified Cache)
* All other CTypes are not present

---

### 2019-Apr-01

I asked about `dsb` and `dmb` today.  Quite a lot was offered on this:

```
[15:26] <eryjus> armv7, summarizing dsb and dmb...  dmb will halt (for want of a better term) processing on the core until all other reads/updates to memory are complete; dsb will halt processing until a change has been written/synchronized to memory (does this assume or require write-through?)
[15:28] <geist> well, it's a bit more complicated
[15:28] <geist> as usual. dmb doesn't ensure it's written to memory, dmb ensures that it is out of the cpu and into the cache heirarchy, and that everything before it has
[15:29] <geist> so dmb gets it out into the L1 cache at least. it dumps the pipeline and any pending write buffers
[15:29] <geist> if you want to ensure someting is out to memory you have to actually go through a full cache clean operation, and *then* dmb
[15:29] <geist> well, dsb actually
[15:29] <geist> dsb is more powerful, it is a dmb but also pulls in other things like making sure outstanding cache transctions and TLB flushes are written
[15:30] <geist> in a nutshell, there are additional parts there, and in armv8 there are lots more variants of dmb and dsb
[15:31] <eryjus> hmmmm....
[15:31] <geist> so dmb isn't so much a halt as a memory barrier (which is exactly what the name means). as in stuff before and after happen before and after, and allthe stuff before has at least made it out into the cache after the instrruction has completed
[15:31] <eryjus> that makes more sense
[15:32] <doug16k> imagine store barriers meaning, make sure that other cpus can see every store before the barrier, before it can see any store after the barrier
[15:32] <geist> dsb is an upgraded dmb that picks up additional non memory things, basically
[15:32] <geist> like TLB flushes, outstanding cache stuff
[15:33] <geist> https://github.com/littlekernel/lk/blob/master/arch/arm/arm/cache-ops.S#L308 is a good example
[15:33] <geist> arch_clean_cache_range causes the cpu to writeback data for a range of virtual memory
[15:33] <geist> basically that routine walks through the address a cache line at a time (usually 32 or 64)
[15:34] <geist> and then, once it's done it does a DSB, which waits for the cache to complete
[15:34] <geist> since even the cache operations are asynchronous and weakly ordered
[15:37] <geist> the best description i've heard is that in a weakly ordered memory system like that, if you were externally observe the memory transactions on the 'bus' as the cpu does it, they can appear in any order, or not at all
[15:37] <geist> the cpu itself promises to appear in order relative to itself, so it wont load before store and that sort of thing, but externally the cpu is free to reorder whatever it wants
[15:38] <geist> the cpus are cache coherent relative to each other, but only once the memoryt transactions have made it out to the cache
[15:38] <geist> so they're snooping each other and whatnot, but the order is completely undefined
[15:39] <doug16k> here's how I explain it: If I call a constructor on a freshly allocated memory block and I set 3 int members in the constructor and I then take that pointer and stick it in a shared location usable from other cpus, is that okay?
[15:39] <geist> sicne derpy in order cores like cortex-a7 or a53 are not that out of order, their external memory accesses are probably roughly in order, so you aren't likely to see as many problems
[15:39] <geist> right, and in doug16k case that is absolutely not okay
[15:39] <doug16k> it's not okay. there is no guarantee that the stores to the 3 int members will be globally visible before I put that pointer in the shared place
[15:40] <doug16k> so, there, I need to enforce "make SURE that all the stores before here are globally visible, before you let anything past here be globally visible"
[15:40] <doug16k> that's a store barrier
[15:40] <geist> note that in general you dont see as many prob;ems in user space softare that you think because you usually use locks and other suck constructs to keep this from happening
[15:40] <doug16k> you can accomplish that with a store release storing that shared pointer
[15:40] <geist> and those all have barriers built in
[15:40] <geist> but if you're the person actually implementing the locks...
[15:41] <aalm> ...fun...
[15:41] <eryjus> ok, then i have to go back to swizzling cpus...  if process A is running on core 1 and I move it to core 2, I have to set a barrier before I make that change.
[15:42] <aalm> pretty sure you need more than one barrier there, atleast unless you're going to special case everything like that
[15:42] <geist> right, note that spinlocks always have a barrier built in, so generally speaking grabbing locks to fiddle with cross cpu data structures accomplishes the barrier
[15:42] <geist> also in the fine print of the architecture, both x86 and arm define an exception (or syscall) as an implicit barrier
[15:43] <geist> so if you context switch via syscall or interrupt, barrier accomplished
[15:43] <aalm> good point
[15:43] <eryjus> that tidbit will be good to remember
[15:44] <aalm> i suppose you've got interrupt handlers like timers working already?
[15:44] <eryjus> i do
[15:45] <eryjus> aalm -- im working on the pre-smp cleanup before I start the APs
[15:46] <aalm> cool:]
[15:46] <eryjus> all new territory for me, so I may yet have one more question....
[15:46] <eryjus> thanks for the info.
```

The prototype for the function offered by Geist is at: https://github.com/littlekernel/lk/blob/17598a284534bad32a5da91b734e4de18f3ed0f0/include/arch/ops.h#L62.

So, there is one big takeaway I have with this conversation.  Everything in the scheduler needs to be synchronized for all CPUs to see.  This means I will need to take great care when updating this data.  It also means that (since I will be working with C functions), I really need to find an x86 equivalent to these functions.  There is `wbinvd` which will write the cache back in flush the cache.

Another point is that the `lk` example is using `DCCMVAC`, meaning that it is operating on an address to a point of coherency.

I think I am at a point where I can write some code.

---

OK, I think I have cache enabled on both x86 and armv7.  I have not done any updates yet to the code.  I want to run through some basic testing here to make sure that I did not break anything.

I also want to point out that I will be focusing updates on the scheduler and heap.  The other things I need to worry about (locks, etc) are going to be developed or redeveloped still.

And the rpi did nothing....  The x86-pc looks good, but I am not running on real hardware yet.

I'm wondering if the MMU setup is correct.  I'm betting the flags are no longer set properly for having data caching enabled.

---

### 2019-Apr-02

Ok, yesterday was a bad joke day for debugging.  Today I have worked up a `SerialEarlyPurChar()` function and confirmed it works.  I am now going to sprinkle this through the `entry.s` file to see at least where I have the problem with the cache.

I have this narrowed down to this point of failure:

```
@ this works
mov     r0,#'*'
bl      SerialEarlyPutChar

    mrc     p15,0,r1,c1,c0,0            @@ This gets the cp15 register 1 and puts it in r0
    orr     r1,#1                       @@ set bit 0
    mcr     p15,0,r1,c1,c0,0            @@ Put the cp15 register 1 back, with the MMU enabled

@ this does not work
mov     r0,#'*'
bl      SerialEarlyPutChar
```

So, things work right up to the point I enable the MMU.  So, my suspicion yesterday was correct.

So, it looks like something is creating problems with `kprintf()`.  I am not able to get that to work at the moment.  Now, there is a function `EarlyInit()` which will call `MmuEarlyInit()` that should prepare everything to work and there is a problem.

I have tried everything I can think of and have reached out to my buddies on `freenode#osdev`.  While I am waiting, I am going to write loader-address-space exception handlers.

---

I was able to work out that I am dealing with a pre-fetch abort.

---

### 2019-Apr-03

The task to start the day today is to figure out what kind of pre-fetch abort I am dealing with.  I'm hoping the codes I pull from the registers will give me a clue.

---

### 2019-Apr-04

Today I need to take a step back and see if I can describe my problem.  Hopefully, if I describe the problem properly either I can find something to research further or I can paste this into a gist and ask for more help.  To prepare for this, I will commit my code.





