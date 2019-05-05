#####################################################################################################################
##
##  Makefile -- This is the core makefile for creating the Century OS for any of the supported architectures
##
##        Copyright (c)  2017-2019 -- Adam Clark; See LICENSE.md
##
##  The basic folder layout here is going to be as follows:
##
##  +- century-os
##    +- arch           // this will include all the architecture-specific code
##    |  +- arm         // for the arm cpus
##    |  |  +- inc      // include files for the architecture
##    |  +- x86         // for the x86 cpus
##    |     +- inc      // include files for the architecture
##	  +- inc            // include files common to all architectures -- for inclusion into modules
##    +- kernel         // this is the kernel code
##    | +- inc          // include files internal to the kernel
##    | +- ld           // linker scripts for the kernel
##    | +- src          // source files internal to the kernel
##    +- modules        // source files common to all architectures
##    | +- libc         // this is the kernel interface runtime libraries
##    | | +- inc        // include files internal to libc
##    | | +- ld         // linker scripts
##    | | +- src        // source files internal to libc
##    | +- module1      // this is executable module #1
##    | | +- inc        // internal includes
##    | | +- ld         // linker scripts
##    | | +- src        // internal source
##    | +- module2      // this is executable module #2
##    |   +- inc        // internal includes
##    |   +- ld         // linker scripts
##    |   +- src        // internal source
##    +- platform       // platform source
##    | +- bcm2836      // the Broadcom bcm2836 SoC
##    | +- inc          // common include files
##    | +- pc           // pc chipset
##    +- sysroot        // this is the root files system for each architecture
##    | +- rpi2b        // this is code that is specific to the Raspberry Pi 2B
##    | +- x86          // this is code specific to the intel x86 32-bit processor
##    +- targets        // this is where the build targets are dropped
##      +- rpi2b        // the architecture + platform called rpi2b
##      | +- bin        // this will be the bin folder of sysroot
##      | | +- boot
##      | | +- lib
##      | +- obj        // this is the object folder for each module
##      |   +- kernel   // kernel objects
##      |   +- module1  // module1 objects
##      |   +- module2  // module2 objects
##      +- x86-pc       // the architecture + platform
##        +- bin        // this will be the bin folder of sysroot
##        | +- boot
##        | +- lib
##        +- obj        // this is the object folder for each module
##          +- kernel   // kernel objects
##          +- module1  // module1 objects
##          +- module2  // module2 objects
##
##  So, I have had a rather large shift in direction with my build system.  I have started to use `tup` to
##  execute the core of my build, and then use make to do more scripting things.  The reason for the shift
##  is simple: `tup` is FAR easier to maintain than makefiles.  One of the key reasons for this is that the
##  Tupfile is located in the target directory.  Therefore, if there is something that is needed to satisfy
##  a dependency, `tup` only needs to look in the directory where that dependency should be and if it is not
##  there, read the Tupfile on how to create that object.
##
##  However, there is also give and take with `tup`.  Since I do not want to clutter up my sysroot folders with
##  a bunch of Tupfiles that would end up on the .iso or .img, I have implemnted the parts that would copy
##  these files into the sysroot folders in this makefile.  The result is MUCH simpler.  I have a default
##  'all' target that simply calls `tup` to refresh the build.  Any of the other specialized targets (.iso,
##  or running QEMU) depend on the all target and then run the steps needed to complete the script.
##
##  The only key function I am giving up is the ability to build an architecture or a module independently.
##  For the moment, I think I can live with that by creating stub functions when needed.
##
##  For the general `make` commands, we will basically operate on targets.  These targets are the combination
##  of an architecture and a platform.  The following are the supported targets so far (with their respective)
##  architectures and platforms:
##  * rpi2b -- arm + bcm2836
##  * x86-pc -- x86 + pc
##
##  A simple `make` (with no explitit targets) will compile everything.  However, you can narrow this down
##  by specifying a target (`make x86-pc` or `make rpi2b`) which will also make a bootable image for the target.
##  Finally, there are verbs that can be used with each target as well, such as `make run-rpi2b` and
##  `make debug-x86-pc`.  These commands, however, have less strict meanings depending on the target.
##
## -----------------------------------------------------------------------------------------------------------------
##
##     Date      Tracker  Version  Pgmr  Description
##  -----------  -------  -------  ----  ---------------------------------------------------------------------------
##  2017-Mar-26  Initial   0.0.0   ADCL  Initial version
##  2017-May-10            0.0.0   ADCL  Gut this file in favor of `tup` and some short scripts
##  2018-May-23  Initial   0.1.0   ADCL  Pull this file from `century` into `century-os`
##  2019-Feb-08            0.3.0   ADCL  Redo the source tree structure
##
#####################################################################################################################


.SILENT:

ARMV7-LIB = $(shell armv7-rpi2-linux-gnueabihf-gcc  -mlittle-endian  -mcpu=cortex-a7  -mfpu=neon-vfpv4  -mfloat-abi=hard  --print-libgcc-file-name)
X86-LIB = $(shell i686-elf-gcc --print-libgcc-file-name)


##
## -- This is the default rule, to compile everything
##    -----------------------------------------------
.PHONY: all
all: init
	tup


##
## -- This rule will make sure that up is initialized and that we have created all the proper variants
##    ------------------------------------------------------------------------------------------------
.PHONY: init
init: tuprules.inc
	if [ ! -f .tup/db ]; then `tup init`; fi;


##
## -- Rebuild the toolchain is requested
##    ----------------------------------
.PHONY: toolchain
toolchain:
	cd util && ./toolchain.sh


##
## -- we need to know the current base folder
##    ---------------------------------------
tuprules.inc: Makefile
	echo WS = `pwd` > $@
	echo ARMV7_LDFLAGS = $(dir $(ARMV7-LIB)) >> $@
	echo X86_LDFLAGS = $(dir $(X86-LIB)) >> $@


## ==================================================================================================================


##
## == These rules make the rpi2b architecture
##    =======================================


##
## -- This is the rule to build the rpi2b bootable image
##    --------------------------------------------------
.PHONY: rpi2b
rpi2b: init
	tup targets/$@/*
	rm -fR img/rpi2b.*
	rm -fR sysroot/rpi2b/*
	mkdir -p sysroot/rpi2b img
	cp -fR targets/rpi2b/bin/* sysroot/rpi2b/
	find sysroot/rpi2b -type f -name Tupfile -delete
	dd if=/dev/zero of=img/rpi2b.img count=20 bs=1048576
	echo 'type=83' | sfdisk img/rpi2b.img
	dd if=/dev/zero of=img/rpi2b.p1 count=38912 bs=512
	mkfs.ext2 img/rpi2b.p1 -d sysroot/rpi2b/
	dd if=img/rpi2b.p1 of=img/rpi2b.img seek=2048 count=38912
	rm -f img/rpi2b.p1


##
## -- Run the rpi2b executable on real hardware
##    -----------------------------------------
.PHONY: run-rpi2b
run-rpi2b: rpi2b
	pbl-server /dev/ttyUSB0 sysroot/rpi2b/boot/grub/cfg-file


##
## -- Run the rpi2b target on qemu, setting up for debugging
##    ------------------------------------------------------
.PHONY: debug-rpi2b
debug-rpi2b: rpi2b
	qemu-system-arm -m 1024 -machine raspi2 -cpu cortex-a7 -smp 4 -dtb util/bcm2709-rpi-2-b.dtb -serial mon:stdio -kernel ~/bin/kernel-qemu.img --hda img/rpi2b.img -S


##
## -- Run the rpi2b target as a qemu emulation target
##    -----------------------------------------------
.PHONY: qemu-rpi2b
qemu-rpi2b: rpi2b
	qemu-system-arm -m 1024 -machine raspi2 -cpu cortex-a7 -smp 4 -dtb util/bcm2709-rpi-2-b.dtb -serial stdio -kernel ~/bin/kernel-qemu.img --hda img/rpi2b.img


## ==================================================================================================================


##
## == These rules make the x86-pc target
##    ==================================


##
## -- This is the rule to build the x86-pc bootable image
##    ---------------------------------------------------
.PHONY: x86-pc
x86-pc: init
	tup targets/$@/*
	rm -fR img/x86-pc.iso
	rm -fR sysroot/x86-pc/*
	mkdir -p sysroot/x86-pc img
	cp -fR targets/x86-pc/bin/* sysroot/x86-pc/
	find sysroot/x86-pc -type f -name Tupfile -delete
	grub2-mkrescue -o img/x86-pc.iso sysroot/x86-pc


##
## -- Run the x86-pc on qemu
##    ----------------------
.PHONY: run-x86-pc
run-x86-pc: x86-pc
	qemu-system-i386 -m 3584 -serial stdio -cdrom img/x86-pc.iso


##
## -- Run the x86-pc on bochs
##    -----------------------
.PHONY: bochs-x86-pc
bochs-x86-pc: x86-pc
	bochs -f .bochsrc -q


##
## -- Debug the x86-pc on qemu
##    ------------------------
.PHONY: debug-x86-pc
debug-x86-pc: x86-pc
	qemu-system-i386 -no-reboot -no-shutdown -m 3584 -serial mon:stdio -cdrom img/x86-pc.iso -S


##
## -- Write the .iso image to a USB stick (sdb)
##    -----------------------------------------
.PHONY: write-x86-pc
write-x86-pc: x86-pc
	sudo umount /dev/sdb1 || true
	sudo dd bs=4M if=img/x86-pc.iso of=/dev/sdb


