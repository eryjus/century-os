# Physical Memory Manager

This file is intended to document the Physical Memory Manager for the CenturyOS hobby operating system.  There are a few design decisions that have been made already about the architecture of this driver and how it will integrate into the kernel.  These are:

* The PMM is managed by a bitmap
* The PMM driver will run as a user-space process
* The PMM will be built into the kernel binary
* The PMM sections will be separated from the kernel
* The PMM will initialize ahead of the heap, so anything requiring the heap will not be available during initialization

I want to note that at this point, since the PMM is only a single threaded process and will only respond to messages in sequence, there is no need to manage a lock on the PMM bitmap.  By definition, one and only one process will every want to get access to this structure at any given time.  Multiple requests will be queued and serviced in the order they are received.

I have coded this so that a frame can be freed multiple times without any concerns, but only allocated once.

The PMM will respond to the following messages, all of which are in the msg field:
* PMM_FREE_FRAME -- Free the frame in `Message_t.parm1`.  `Message_t.parm2` is unused.
* PMM_ALLOC_FRAME -- Allocate the frame in `Message_t.parm1`.  `Message_t.parm2` is unused.
* PMM_FREE_RANGE -- Free a range of frames starting in `Message_t.parm1` and continuing for a total of `Message_t.parm2` blocks.
* PMM_ALLOC_RANGE -- Allocate a range of frames starting in `Message_t.parm1` and continuing for a total of `Message_t.parm2` blocks.
* PMM_NEW_FRAME -- Allocate a new frame.
