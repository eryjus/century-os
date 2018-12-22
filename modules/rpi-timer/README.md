Raspberry Pi Timer Test
---

This module is a "closer-to-bare-metal" test module that is intended to be booted from `rpi-boot`, but eliminates the MMU from the equation.  This is done in an effort to be able find code for qemu that will start the timer and get the IRQs to fire.  Eventually, I hope to be in a position to run this on real hardware as well.

Much of this module is a point-in-time copy of the loader to set the basic needed to get into the actual test.
