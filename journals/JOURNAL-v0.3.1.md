# The Century OS Development Journal - v0.3.1

## Version 0.3.1

This version will replace the PMM in the kernel with a stack implementation. The current implementation is a bitmap, which is slow and costly in memory.

At the same time, this change will also take care of alignment concerns when allocating frames. At the same time, the free of a frame needs to remain trivial. So the following features are included in this version:

* Re-implement the PMM as a stack, where a stack entry is a frame number and number of frames free in this block
* Allocation of a single frame is either decrementing the number of free frames and incrementing the starting frame and returning the result
* Freeing a single frame is putting the frame into a scrubbing queue
* The butler process will be responsible for pulling a frame from the scrubbing queue, clearing it, and then adding it to the PMM stack (aggregating/merging blocks properly)
* Maintain a stack of memory in the <1MB range for DMA
* Add the ability to allocate contiguous frames with the proper alignment

The goals for this version are tracked here: http://eryjus.ddns.net:3000/versions/8

---

### 2019-Mar-10

So, I start this version with some thinking and design work here in the journal.  I do not want to keep track of a list of individual frames.  That was very short-sighted on my part in the past.  I want to actually keep track of a number of frame + length combinations.

So, I am going to need a structure like this to keep track of my addresses:

```C
struct {
    ListHead_t::List_t list;
    frame_t frame;
    size_t count;
};
```

The `list` member will be used to locate the frame on the stack, or to locate the frame on the freed frame queue waiting to be scrubbed.

The overall management of the physical memory will be held by the following structure:

```C
struct {
    ListHead_t freeFrames;          // most allocations come from this stack; contains its own lock
    ListHead_t freeFramesLow;       // used for DMA and low-memory allocations; contains its own lock
    ListHead_t scrubQueue;          // these are freed frames that need to be scrubbed; contains its own lock
};
```

There are also a number of operations that I need to define for the PMM:

```C
frame_t PmmAllocateFrame(void);             // new function name to make sure I clean up properly
frame_t PmmAllocateLowFrame(void);          // new function name to make sure I clean up properly
void PmmAllocAlignedFrames(const size_t count, const int bitAlignment);
void PmmAllocAlignedLowFrames(const size_t count, const int bitAlignment);
void PmmDeallocateFrame(const frame_t frame);                               // also a new function
void PmmDeallocateFrameRange(const frame_t frame, const size_t count);      // to help with initialization
void PmmScrubFrame(const frame_t frame);
```

Finally, I need to work through how I am going to store the stack element structure.  I can use the heap, but that will make things difficult for the early initialization.  On the other hand, I can set up a slab and allocate from that.  I can also, of course, use a more hybrid approach, where I use a slab in early initialization where I have exceptionally low fragmentation and then augment later from the heap once it has been initialized.

I think I will want to revisit the early frame allocator in order to resolve this concern.

Another thing I think I want to do is to revisit the `lists.h` file and define explicit `Queue_t` and `Stack_t` types and explicit operations thereof.  These changes should make the code more readable.

The key to success here is going to be to move the heap initialization to prior to the `PmmInit()` call in `LoaderMain()`.  For this to work, I think I am going to need to extend the use of `NextEarlyFrame()` just a little longer.  This is risky and I'm certain I will want to address that at some point as well so I do not run out of space.  But the order would end up being:

```C
MmuInit();
HeapInit();
PmmInit();
```

I think I have enough of a plan to start coding.

---

### 2019-Mar-11

I am working on coding.

I did discover an opportunity for a deadlock (which is not very likely but can happen).  I have captured this in [Redmine #405](http://eryjus.ddns.net:3000/issues/405).  I do not plan to address this concern in this release at this time, but will at some point in the future -- probably after I clean up the scheduler.  Yes, I have thoughts already for version 0.3.2.

---

### 20109-Mar-12

I was able to complete the working function to allocate a block.  What is left is to redo the `PmmInit()` function and then replace all the function calls.

I am concerned about initialization performance.  What I am looking at so far is an exhaustive search of all frames to see whether they fall into used or free.  This will be 3-5 checks per frame iterated over all the frames on the system.  In 64-bit memory, this could take way too long to complete.  Now, with that said, I think I can simply iterate through the MB memory map and remove the first 4MB of memory (which is all that is used up to this point).  The challenge with that is going to be the memory between the last module loaded and the last early frame that was allocated.  I will defer that to the cleanup after the boot.

---

OK, the kernel is booting and does not crash.  However, I am not sure that everything is working.  I need to work on getting the butler process to scrub frames and once that is working properly I will need to architect some tests.

---

For early init, I should be able to remove the entire block of frames and initialize them as a block -- which might take a long time before I can allocate a frame.  On the other hand, if I add them a few frames at a time, I will need to make sure I am able to remove fragmentation in the stacks, which will take some time to execute with a spinlock held.

So, what we need to do is loop through all the nodes in the stack and determine if the block can be added to one of the existing blocks in the following manner:

```C
if (frame == block->frame - 1) {
    block->frame --;
    block->count ++;
    return;
} else if (frame == block->frame + block->count) {
    block->count ++;
    return;
} else {
    // move on to the next block
}
```

---

### 2019-Mar-13

I have realized that we are actually clearing all the memory on init.  Is that something we really want to do?  In other words, what is the point of scrubbing a frame?  Is it for security reasons?  Or is it to pre-zero all of the memory in the hopes to save some time when we have a bss section to allocate to?  Well, it really needs to be the former since my OCD requires that I initialize everything on allocation anyway.

With that in mind, there is no reason during `PmmInit()` to put everything into the `scrubStack` -- it just becomes wasted CPU cycles.

---

So, this appears to be working now.  My next task is going to be to write some tests to make sure I am allocating and freeing data properly.  It is also going to need a function to dump the PMM state in a manner where it is readable and can be used for debugging.

My internal debate for the day is whether I want to start some sort of interactive kernel debugging at this point.  This debugger is envisioned to be through the serial port where I can issue commands to that port, pause execution, and evaluate several structures in the kernel.  At this point, I am not ready to take that on.  However, what I do want to do is make sure I am planning for that in my functions.  Therefore, I do want to make this `PmmDumpState()` function self-contained so it can be called from the debugger.

I think I also want to make sure I put some additional counters/variables for some general stats like the number of frames in each stack.  I will start with these stats.

---

For the first iteration of checking the state on x86-pc, I have the following output:

```
=============================================================================================

Dumping the current state of the PMM structures
  (Note: these data elements may be in motion at the time the state is being dumped)
------------------------------------------------------------------------------------

The number of frames in each stack are:
  Scrub.: 0x0
  Normal: 0xffffffec
  Low...: 0x0


=============================================================================================
```

The problem is the number of Normal frames, which would put the total memory more than 4GB, which is not possible on 32-bits.

However, when initializing the PMM, the following is reported:

```
Releasing block of memory from frame 0x400 for a count of 0xbfbe0 frames
```

Which ended up being an easy fix in `PmmInit()`.

---

From this point, I think I have a working PMM.  So, I am going to commit my changes.

---

