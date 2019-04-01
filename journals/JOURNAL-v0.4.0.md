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


