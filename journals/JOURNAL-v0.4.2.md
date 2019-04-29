# The Century OS Development Journal - v0.4.2

## Version 0.4.2

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

### 2019-Apr-28

Now, it is time to take on Atomic Integer operations.  These will be the basis for all locks and synchronization primitives.  Additionally, these will be access across all cores, so the I will have to be very cautious in the implementation.

So what operations do I need to support for Atomic Integers?  Well....
* For Spinlocks:
    * Assign 0 to 1
    * Assign 1 to 0
    * Read value

* For Semaphores
    * Decrement
    * Increment
    * Read value
Note, I am not talking about the `wait()` and `signal()` APIs, but what will be needed at the Atomic Integer level.

* For Mutexes
    * Assign 0 to 1
    * Assign 1 to 0
    * Read value
Note, I am not talking about the `acquire()` and `release()` APIs, but that will be needed at the Atomic Integer level.

Now, for the 0 to 1 API, this is really a specialization of the Increment API and likewise for 1 to 0 and Decrement APIs.

There needs to be a low-level driver for these APIs to perform a swap in memory from the value that is in the location to a desired value returning the value that was stored in the location originally where a higher-level function can determine success and reset if necessary.

Now, one would ask why I should not use a spinlock to lock the Semaphore or Mutex before making a change?  I may yet in order to prevent race conditions.  But I will still use the Atomic Integers at its core for both.

[This web site](https://notes.shichao.io/lkd/ch10/) has a good discussion on the atomic integer operations and the basics that are provided in all kernel operations.

So lifting from this site, I should support the following:

```C
ATOMIC_INIT(int i);
int atomic_read(atomic_t *v);
void atomic_set(atomic_t *v, int i);
void atomic_add(int i, atomic_t *v);
void atomic_sub(int i, atomic_t *v);
void atomic_inc(atomic_t *v);
void atomic_dec(atomic_t *v);
int atomic_sub_and_test(int i, atomic_t *v);
int atomic_add_negative(int i, atomic_t *v);
int atomic_add_return(int i, atomic_t *v);
int atomic_sub_return(int i, atomic_t *v);
int atomic_inc_return(int i, atomic_t *v);
int atomic_dec_return(int i, atomic_t *v);
int atomic_dec_and_test(atomic_t *v);
int atomic_inc_and_test(atomic_t *v);
```

So, how does this fix my spinlock issue where I need to set and return the previous value?  I should be able to also implement an `atomic_set()` function that returns the previous value -- or change the function above to return an `int`.

Now, I think I will be able to start putting code down for this.  Once again, I will work on the arm code first since it is a little more foreign (and will be harder to implement due to the RISC architecture).

---

This has been written.  Now I need to perform some basic tests.  I will not be completely sure that these work until I get into SMP.

---

Looking at this and testing, I can see that the inline assembly is clobbering registers.  I think I am going to have to re-implement in straight assembly.

---

OK, I have been able to distill this down to 2 functions implemented in assembly, and then the rest calling these 2 functions.  These are `AtomicSet()` and `AtomicAdd()`.  Both targets have been tested and they work (well, to the best of my ability to test).

So, the next step is going to be to replace the details of the implementation of `Spinlock_t` with `AtomicInt_t`.  In the meantime, I commit my changes (but I'm not going to push them just yet).

---

