# The Century OS Development Journal - v0.2.0

## Version 0.2.0

This version bring the Centuty-OS kernel to run on a rpi2b.  There is no added features here -- just a parallel move to the Raspberry Pi 2B.

### 2018-Nov-11

So, now with that commit, I have a decision to make: Do I develop wide first and add an architecture?  Or do I develop deep first and add functionality for more satisfaction?

I asked this question on the `#osdev` IRC channel and had this exchange:

```
[15:54] <eryjus> poll for those who have been there: I am happy with my v0.1 of my microkernel for now and ultimately my plan is to support other architectures -- would you recommend going wide and add an architecture first, or deep and build out other fucntionality first?
[15:54] <geist> probably adding at least one more is useful
[15:55] <geist> it tends to point out where your architecture abstractions are weak
[15:55] <eryjus> I'm certain there is a ton of opportunity there....
[16:01] <eryjus> Building deep on top of flaws like that does not seem like a good idea...
```

I think I am going to take that advice and work on the `rpi2b` architecture.  This is so significantly different than the x86 architecture that if there are any abstraction issues they will be evident quickly.  There will be lots of problems.

The first thing I need to do is to refresh my `rpi-boot` project to be able to boot an RPi2b emulator.  This was an easy recompile and copy the resulting `kernel-qemu.img` to `~/bin`.  To compile this, I also need to `export CROSS_COMPILE=arm-eabi-`.

I already have a cross-compiler for the rpi2b architecture, so all I really need to do is get into it.

---

I think it is going to be important to discuss the differences between the architectures as I work through the issues.  As a result, I will spend a little time discussing each file and the issues in encounter with each.  I expect to be bouncing around a bit as I work my way through the issues.  I am generally working on the last file that failed, so a lot is going to depend on what the last one is.

I started with creating the 3 `arch-*.h` files and moving what was appropriate to those files.  With that, I am working on the `LoaderMain.cc` file.  Here are the problems I am dealing with at the moment:
* The `loader` file `mmu.h` is poorly named.  It contains several architecture-dependent function prototypes.
* The x86 architecture uses register `cr3` to control the top of the paging tables.  However, the rpi2b architecture uses several CP15 registers to control the MMU.
* As a result of the issue above, the function `MmuSwitchPageDir()` is architecture dependent.
* The Framebuffer address is hard-coded and should be defined in an architecture-dependent header.
* The HW discovery structure location that is sent to the kernel is hard-coded and should be defined in an architecture-dependent header.

---

After the `LoaderMain.cc`, it took quite a while to get the `Tupfile`s worked out so that I could continue to try to get a full executable image ready for debugging.  So, with that said, `tup` is trying to link the kernel first.  The file `kinit.cc` appears to be the one reporting the errors last at this point.  But most of the errors are happening on `idt.h`, or at least the ones that are still left on the screen.  Certainly the Interrupt Descriptor Table (or IDT) is an intel-specific thing, so I will need to refactor that name and everything that touches it.  Once I rename this file to be `interrupt.h` all kinds of things break.

I now have the kernel's `MmuMapToFrame()` function that is coming up.  This is clearly going to be very architecture dependent.  But the questions now becomes how to organize it.  Do I create a folder with-in each sub-module for each architecture?  Or a sub-module within each architecture?  Tonight I am leaning toward the latter, but we will see how I feen about it tomorrow.

---

### 2018-Nov-12

Today I spent a bit of time looking at what would be the best way to handle the architecture question with `tup`.  There are several solutions but nothing that would make things super easy to handle.  So, I will create an architecture folder within each sub-module.  Starting with the MMU.

I am going to focus on getting the loader for rpi2b processed.  There are going to be enough issues with that to keep me busy for a while.

I was able to find my loader's `entry.s` from century and copied it in.  However, this is written in AT&T syntax while `nasm` is Intel syntax.  Moreover, `nasm` does not support anything other than x86, which really sucks.  So, it's back to GNU Asm....  However, there is a directive I can include in the source to switch to Intel syntax: `.intel_syntax noprefix`.  I will start by adding that to the rpi2b asm files and then retrofit that back into the i686 files once I have a working rpi2b architecture.  I'm not sure how much will break while I am transitioning this code.

---

Well, it looks like my cross-compiler was not built to be able to support intel syntax.  I will have to rebuild it at some point.  But for now, I am going to just suffer through the AT&T syntax -- after all it is not an Intel CPU I am assembling for!!   Duh!!!

Ok, `entry.s` is assembling.  But I have this error when trying to link that reads:

```
/home/adam/opt/cross/lib/gcc/arm-eabi/6.3.0/libgcc.a(pr-support.o): In function `_Unwind_GetTextRelBase':
/home/adam/devkit/build-gcc/arm-eabi/libgcc/../../../gcc-6.3.0/libgcc/config/arm/pr-support.c:384: undefined reference to `abort'
```

What bothers me is that I should not be including anything.  I am not sure where this is coming from and I need to try to dig that out.

Well, after a **lot** of digging and reading about issues on the inter-webs, I finally found the 2 options I needed: `-fno-unwind-tables -fno-excpetions`.  I have added that to CFLAGS and the loader compiles properly.  Well, as properly as it can while still missing functions that need to be ported.  My i686 loader still compiles properly as well.

---

### 2018-Nov-13

Today I am going to tackle the serial port for debugging.  There is a key difference between the x86 and ARM architectures where the x86 uses IO ports to read and write data for a device and to control the devices whereas the ARM uses Memory Mapped IO (MMIO) to do the same thing.  So, with MMIO, configuring and writing to the serial port is as easy as writing to memory locations.  I believe that later versions of x86-family have increasing MMIO abilities but leave the IO ports in place as well for backward compaitibility.

So, my current error list when trying to compile my `loader.elf` binary are:

```
[adam@os-dev century-os]$ tup bin/rpi2b/boot/loader.elf
 [ETA~=<1s Remaining=0 ] 100%
* 1) bin/rpi2b/boot: arm-eabi-gcc -T /home/adam/workspace/century-os/modules/loader/src/rpi2b/loader.ld -g -ffreestanding -O2 -nostdlib -L /home/adam/workspace/century-os/lib/rpi2b -z max-page-size=0x1000 -o loader.elf /home/adam/workspace/century-os/obj/loader/rpi2b/FrameBufferInit.o /home/adam/workspace/century-os/obj/loader/rpi2b/LoaderMain.o /home/adam/workspace/century-os/obj/loader/rpi2b/ModuleInit.o /home/adam/workspace/century-os/obj/loader/rpi2b/PmmInit.o /home/adam/workspace/century-os/obj/loader/rpi2b/PmmNewFrame.o /home/adam/workspace/century-os/obj/loader/rpi2b/entry.o /home/adam/workspace/century-os/obj/loader/rpi2b/hw-disc.o /home/adam/workspace/century-os/obj/loader/rpi2b/mb1.o /home/adam/workspace/century-os/obj/loader/rpi2b/mb2.o /home/adam/workspace/century-os/lib/rpi2b/libk.a /home/adam/workspace/century-os/modules/loader/src/rpi2b/loader.ld -lgcc -lk;
/home/adam/workspace/century-os/obj/loader/rpi2b/LoaderMain.o: In function `LoaderMain':
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:39: undefined reference to `SerialInit()'
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:40: undefined reference to `HwDiscovery()'
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:46: undefined reference to `MmuInit()'
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:51: undefined reference to `SetMmuTopAddr()'
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:56: undefined reference to `kMemMove'
/home/adam/workspace/century-os/obj/loader/rpi2b/ModuleInit.o: In function `ModuleInit()':
/home/adam/workspace/century-os/modules/loader/src/modules/ModuleInit.cc:73: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/modules/loader/src/modules/ModuleInit.cc:125: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/modules/loader/src/modules/ModuleInit.cc:137: undefined reference to `MmuMapToFrame(unsigned long, unsigned long, unsigned long, bool, bool)'
/home/adam/workspace/century-os/modules/loader/src/modules/ModuleInit.cc:158: undefined reference to `cr3'
/home/adam/workspace/century-os/obj/loader/rpi2b/PmmInit.o: In function `PmmInit()':
/home/adam/workspace/century-os/modules/loader/src/pmm/PmmInit.cc:66: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/obj/loader/rpi2b/PmmNewFrame.o: In function `PmmNewFrame()':
/home/adam/workspace/century-os/modules/loader/src/pmm/PmmNewFrame.cc:35: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/obj/loader/rpi2b/mb1.o: In function `AddModule(unsigned long long, unsigned long long, char*)':
/home/adam/workspace/century-os/inc/hw-disc.h:244: undefined reference to `kStrCpy'
/home/adam/workspace/century-os/lib/rpi2b/libk.a(FrameBufferClear.o): In function `FrameBufferClear()':
/home/adam/workspace/century-os/modules/libk/src/frame-buffer/FrameBufferClear.cc:36: undefined reference to `kMemSetW'
/home/adam/workspace/century-os/lib/rpi2b/libk.a(SerialPutChar.o): In function `SerialPutChar(char)':
/home/adam/workspace/century-os/modules/libk/src/SerialPutChar.cc:27: undefined reference to `inb'
/home/adam/workspace/century-os/modules/libk/src/SerialPutChar.cc:29: undefined reference to `outb'
/home/adam/workspace/century-os/modules/libk/src/SerialPutChar.cc:30: undefined reference to `serialPort'
collect2: error: ld returned 1 exit status
 *** tup errors ***
 *** Command ID=3975 failed with return value 1
tup error: Expected to write to file 'loader.elf' from cmd 3975 but didn't
 [ ] 100%
 *** tup: 1 job failed.
[adam@os-dev century-os]$ tup bin/i686/boot/loader.elf
 [  ETA~=<1s Remaining=0    ] 100%
 skipped 138 commands.
 ```

So, looking at this list, all of the Serial port implementations will need to be pulled into an architecture folder.  But the prototypes will all need to be consistent.  This is a good small subsystem to take on for a first exercise.

First, the `serial.h` file in in `libk`.  Since my goal is to eliminate the `libk` module, I will move that into the loader includes and copy it into the kernel includes at the same time -- yes, 2 copies of the same file.  I will have to take on some de-dup effort.

In the meantime, I have created a `serial` submodule with architecture folders therein.  I have moved the `Serial*()` functions from `libk` to the loader.  And I have started looking at the MMIO inlines that will be used to interact with the uart on rpi2b (Do I really need to type the architecture every time now? -- I guess I better get used to it...).  I started to put the MMIO functions into the `arch-cpu.h` file, but realized that with the APIC I will want these for the x86 family as well.  So, into `cpu.h` they go.  For now.

I was able to extract all the relevant function lines from `uart-dev.c` from century to match the configuration for i686.  The serial port is set for 38400 baud, 8-N-1 for both architectures now.  rpi2b `SerialInit()` function requires a `BusyWait()` function, which in turn required a read of a low level system timer.  I had to bring those all over as well.

I want to note that the low level system timer is not emulated properly in the `qemu` version I am running, so I had to put in a patch to work around it.  I believe that we should be able to pull that patch out at some point.  But, today is not the day.

When I try to link, I have a bunch of references to `SerialPutS()` now.  I will take that on next, along with `SerialPutChar()` since there is a strong dependency there.

Actually, looking at it, `SerialPutS()` is hardware independent!  I get a freebee!  Actually, so is `SerialPutHex()`.  So, that really only leaves `SerialPutChar()`.

That was not too bad....

I did get this message on linking:

```
/home/adam/workspace/century-os/modules/loader/src/rpi2b/SysTimerCount.s:28: undefined reference to `ST_CLO'
```

This is directly related to this function:

```asy
SysTimerCount:
	ldr		r0,=ST_CLO                  @@ load the base address of the system timer
	ldrd	r0,r1,[r0]                  @@ Get the 64-bit timer "count" into r1:r0
	mov		pc,lr						@@ return
```

The problem is that the assembler is not throwing an error on undefined references.  I will work on sorting that out next.  Well, it looks like `as` assumes that all unknowns are defined in another file and there is no way to change that behavior.  At least that's I found.

---

I think I will take on `kMemSetB()` next.  It will give me a reason to refresh my ARM assembler...  It dawns on me that I have not really actually done much ARM assembly coding.  I have written a few fuctions that get and return a value, but nothing that actually takes a parameter and does something with it.

---

I managed to get both `kMemSetB()` and `kMemSetW()` written.  I have no clue how successful I was in getting the memory block right...   I guess I will have to debug some when I can get it to compile.

I am down to a few functions left.  `kStrCpy()` is the next one I will tackle.

Ok, I have all the lower level memory functions done.  I have only a few issues left to tackle:

```
/home/adam/workspace/century-os/obj/loader/rpi2b/LoaderMain.o: In function `LoaderMain':
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:40: undefined reference to `HwDiscovery()'
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:46: undefined reference to `MmuInit()'
/home/adam/workspace/century-os/modules/loader/src/LoaderMain.cc:51: undefined reference to `SetMmuTopAddr()'
/home/adam/workspace/century-os/obj/loader/rpi2b/ModuleInit.o: In function `ModuleInit()':
/home/adam/workspace/century-os/modules/loader/src/modules/ModuleInit.cc:137: undefined reference to `MmuMapToFrame(unsigned long, unsigned long, unsigned long, bool, bool)'
/home/adam/workspace/century-os/modules/loader/src/modules/ModuleInit.cc:158: undefined reference to `cr3'
```

These should all be C functions and some will be quite complicated (I'm dreading `MmuInit()`!).

---

### 2018-Nov-14

I started the day by taking care of everything but the MMU for the rpi2b architecture.  Before I go on, I want to pull everything out of the rpi2b build except for the loader.  This way I can actually test the loader sooner.  It may not do much now, but I have a lot of reading to do before I can even attempt to set up the rpi2b MMU.

Here are some notes as I read through the documentation:
* The TTL1 must be 16K aligned and is 16K in length
* There are 4096 TTL1 entries, each is 4 bytes long
* Each TTL1 Entry can look at 1MB of physical memory
* A word to ARM is 4 bytes long (a half-word is 2 bytes)
* A TTL2 table is 1K in length, which makes a mess of the 4K frame unless we put multiple in a single frame
* There are 256 TTL2 entries in a single 1K table
* There are 1K X 4096 or 4MB of TTL2 Tables that would need to be mapped
* If I was to map the tables, I would need TTL1 entries 4095, 4094, 4093, and 4092 to accomplish the TTL2 table mappings
* This would also leave 4 additional TTL1 mappings I would also need to build out to access the TTL1 table
* TTL2 tables would be from addresses `0xffc00000` to `0xffffffff`; TTL1 tables would be from `0xffbfc000` to `0xffbfffff`

---

After checking on #osdev, I learned that there is a TTLB0 and TTLB1 register pair.  TTLB0 is intended to be used for user processes and TTLB1 is for system processes.  I can set the number of bits to distinguish between the 2 tables to be 1 bit.  This means that addresses from `0x00000000` to `0x7fffffff` will refer to TTLB0 and addresses from `0x80000000` to `0xffffffff` will refer to TTLB1.  The key here is that when I perform a task switch, I only have to be concerned with updating the TTLB0 for the new target process.  The TTLB1 is static.

Now, the drawback is that the address space for my processes just got reduced (or at least if I plan to have them remain the same between x86 and rpi2b).

If I am reading the documentation correctly, the TTLB1 address must be 16K in length and 16K aligned (but 1 for the entire OS) while the TTLB0 address would only need to be 8K aligned and 8K in length (1 per non-kernel process).

So, how then to manage the mappings to the table structures.  I think the original thoughts still apply: the top 2M + 16K (only half the addresses are relevant, so only half the TTL2 tables are needed) are used to map the kernel.  And then for processes, use the the top 2M + 8K for the map, up to address `0x7fffffff`.

I think I have a memory layout and a design to start with.  The good news is I only have to start with the kernel mapping in the loader since all I need is kernel mappings for the loader.

When I come back, I think it's time to put some code down to start the implementation.

---

Ok, so, I need to map this out...  starting with the kernel:
* `kTTL1` will be located at `0xffbfc000`.  The address of that table will be static.
* The index `n1` into `kTTL1` will be calculated as `vAddr >> 20` (0 to 4095)
* `kTTL2` tables will begin at `0xffc00000`.  This starting address will be static.
* The `kTTL2` table that backs `kTTL1[n1]` will be located at `kTTL2 + (1024 * n1)`.
* The index `n2` into `kTTL2` will be calculated as `(vAddr >> 12) & 0xff`

---

Well, crap!!!  I just lost an uncommitted copy of `Tuprules.tup`!!!  I'm depressed.

---

### 2018-Nov-15

OK, I need to start building out the `Tuprules.tup` file again.  What is really bothering me is that I really have no clue what happened and how to prevent it again.  I know I can stage the changes in `git` more often and I should have done that 5 minutes before it happened.  But I didn't and here I am.  Just in case, I am going to start with a reboot today.

---

Well, I was able to get everything to compile again.  I was able to stage the files and when I tried to make this command automatically overwrite the target image filesystem, it resets Tuprules.tup!

```
	sudo mkfs.ext2 /dev/mapper/loop0p1
```

I was able to recover my file from `git`.  But I will have to deal with the following prompt for a while:

```
20+0 records in
20+0 records out
20971520 bytes (21 MB, 20 MiB) copied, 0.0155924 s, 1.3 GB/s
mke2fs 1.44.2 (14-May-2018)
/dev/mapper/loop0p1 contains a ext2 file system
        last mounted on /home/adam/workspace/century-os/p1 on Thu Nov 15 15:18:08 2018
Proceed anyway? (y,N) y
Discarding device blocks: done
Creating filesystem with 18432 1k blocks and 4608 inodes
Filesystem UUID: 78feccc7-54a2-4b7c-9057-4d0e1ddc2d3e
Superblock backups stored on blocks:
        8193

Allocating group tables: done
Writing inode tables: done
Writing superblocks and filesystem accounting information: done
```

Actually I figured it out!!  I forgot I had added the following to the `Makefile`:

```
Tuprules.tup: Makefile
	echo "WS = `pwd`" > $@
	echo "II686 = \$$(WS)/bin/i686/usr/include/*" >> $@
```

I removed that part of the code.

So, running the rpi2b loader, and I am now getting these errors and is now the subject of my debugging efforts.

```
Welcome to Rpi bootloader
Compiled on Nov 11 2018 at 16:12:41
ARM system type is c43
EMMC: bcm_2708_power_off(): property mailbox did not return a valid response.
EMMC: BCM2708 controller did not power cycle successfully
EMMC: vendor 24, sdversion 1, slot_status 0
EMMC: WARNING: old SDHCI version detected
SD: found a valid version 1.0 and 1.01 SD card
MBR: found valid MBR on device emmc0
EXT2: not a valid ext2 filesystem on emmc0_0
MBR: found total of 1 partition(s)
MAIN: device list:
VFS: unable to determine device name when parsing /boot/rpi_boot.cfg
VFS: unable to determine device name when parsing /boot/rpi-boot.cfg
VFS: unable to determine device name when parsing /boot/grub/grub.cfg
MAIN: No bootloader configuration file found
```

I have mounted the image and I am able to confirm that `/boot/grub/grub.cfg` exists.

So, I need to back up a bit.  The EXT2 error message is certainly a problem.  So, is the Welcome message coming from `rpi-boot`?  It is; I found the following in the main.cc file:

```C
    printf("Welcome to Rpi bootloader\n");
	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("ARM system type is %x\n", arm_m_type);
```

So, let's go see what the problem is with EXT2.  This is the check that is being made:

```C
	// Confirm its ext2
	if(*(uint16_t *)&sb[56] != 0xef53)
	{
		printf("EXT2: not a valid ext2 filesystem on %s\n", parent->device_name);
		return -1;
	}
```

So, this is something I can check in the file system.  Let's see how to get this done...

```
[adam@os-dev century-os]$ sudo kpartx -as iso/rpi2b.img
[adam@os-dev century-os]$ sudo debugfs /dev/mapper/loop0p1
debugfs:  stats
Filesystem volume name:   <none>
Last mounted on:          /home/adam/workspace/century-os/p1
Filesystem UUID:          1a506bae-56cf-4450-9589-3d60b6691d00
Filesystem magic number:  0xEF53
...
```

So, there is a discrepancy between what the superblock is reporting and what `rpi-boot` is reading.  This all looks to be correct.  However, I am left wondering if the disk image that `rpi-boot` is expecting is un-partitioned.

---

So, I want to document what I have found so far.  I have been digging through the `rpi-boot` code and I belive the problem is the formation of my disk.  `rpi-boot` assumes that the EXT2 partition actually starts in sector 2 (0-based) while I am building my partition from 1MB.  Rather than take on fixing the `rpi-boot` code (which I think I may do at some point), I will see if I can get the `parted` script to build the disk right.

For example, when I create the disk image using this command, I get:
```
parted --script iso/rpi2b.img mklabel msdos mkpart p ext2 4s 100% set 1 boot on

MBR: partition number 0 (emmc0_0) of type 83, start sector 4, sector count 40956, p_offset 1be
EXT2: looking for a filesytem on emmc0_0
block_read: performing multi block read (2 blocks) from block 2 on emmc0_0
```

```
parted --script iso/rpi2b.img mklabel msdos mkpart p ext2 3s 100% set 1 boot on

MBR: partition number 0 (emmc0_0) of type 83, start sector 3, sector count 40957, p_offset 1be
EXT2: looking for a filesytem on emmc0_0
block_read: performing multi block read (2 blocks) from block 2 on emmc0_0
```

```
parted --script iso/rpi2b.img mklabel msdos mkpart p ext2 1 100% set 1 boot on

MBR: partition number 0 (emmc0_0) of type 83, start sector 2048, sector count 38912, p_offset 1be
EXT2: looking for a filesytem on emmc0_0
block_read: performing multi block read (2 blocks) from block 2 on emmc0_0
```

The short story here is that no matter how the disk is partitioned, `rpi-boot` is reading from block #2 every time.  It is not taking the partition table and its start offset into account.  This is the offending line:

```C
int r = block_read(parent, sb, 1024, 1024 / parent->block_size);
```

The `block_read` prototype looks like this:

```C
size_t block_read(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t starting_block);
```

The line that prints the debug code is:

```C
printf("block_read: performing multi block read (%i blocks) from "
			"block %i on %s\n", buf_size / dev->block_size, starting_block,
			dev->device_name);
```

This is telling.  It explains that I ahve a specific request to read from block 2 (passed in).  The buffer size / block size is reporting back as 2 blocks (or really 2 sectors) -- since this read is specifically looking at the superblock.  I have a feeling that the concern is complicated by the use of the term block when is should be sector.

So, I have tested through starting sector 8 and nothing will boot.

---

Well, it looks like I'm going to have to debug the `rpi-boot` solution.  It is not loading the correct sectory for me.  I am struggling because this is not a task I wanted to have to take on.  I have one more thing to try -- which is loading my loader directly.  And that did not work easily either.

On the other hand...

```
adam@os-dev century-os]$ sudo kpartx -uv iso/rpi2b.img
add map loop61p1 (253:60): 0 40958 linear 7:61 2
[adam@os-dev century-os]$ sudo kpartx -uv iso/rpi2b.img
add map loop62p1 (253:61): 0 40958 linear 7:62 2
[adam@os-dev century-os]$ sudo kpartx -uv iso/rpi2b.img
add map loop63p1 (253:62): 0 40958 linear 7:63 2
[adam@os-dev century-os]$ sudo kpartx -anv iso/rpi2b.img
add map loop64p1 (253:63): 0 40958 linear 7:64 2
[adam@os-dev century-os]$ sudo kpartx -anv iso/rpi2b.img
add map loop65p1 (253:64): 0 40958 linear 7:65 2
```

Each execution of kpartx is creating a new device map.  I'm too tired for this!

---

### 2018-Nov-16

It's amazing how a little sleep can help.  I recalled that I had the build working at one point -- the one I am currently having trouble with.  I was able to dig out the `Makefile` from an earlier version of the code: https://github.com/eryjus/century-old/blob/master/Makefile#L117.

The difference with the github version is that I have several statements in a couple of parenthetical statements -- executed as a group.  It is this grouping that I think I am missing now.  I'm going to set it up and then reboot to clean my system and then give it a try.  Or a couple of tries.

---

Ok, after the reboot, my `/dev/mapper` folder has the following:

```
[adam@os-dev ~]$ cd /dev/mapper
[adam@os-dev mapper]$ ls -al
total 0
drwxr-xr-x.  2 root root     120 Nov 16 14:37 .
drwxr-xr-x. 20 root root    3900 Nov 16 14:38 ..
crw-------.  1 root root 10, 236 Nov 16 14:37 control
lrwxrwxrwx.  1 root root       7 Nov 16 14:37 fedora-home -> ../dm-2
lrwxrwxrwx.  1 root root       7 Nov 16 14:37 fedora-root -> ../dm-0
lrwxrwxrwx.  1 root root       7 Nov 16 14:37 fedora-swap -> ../dm-1
```

So, everthing is cleaned up -- I am no longer at loop70p1....  Now, for a test compile.  With 2 test compiles, I end up with this:

```
[adam@os-dev mapper]$ ls -al
total 0
drwxr-xr-x.  2 root root     160 Nov 16 14:50 .
drwxr-xr-x. 20 root root    3980 Nov 16 14:50 ..
crw-------.  1 root root 10, 236 Nov 16 14:37 control
lrwxrwxrwx.  1 root root       7 Nov 16 14:37 fedora-home -> ../dm-2
lrwxrwxrwx.  1 root root       7 Nov 16 14:37 fedora-root -> ../dm-0
lrwxrwxrwx.  1 root root       7 Nov 16 14:37 fedora-swap -> ../dm-1
lrwxrwxrwx.  1 root root       7 Nov 16 14:50 loop0p1 -> ../dm-3
lrwxrwxrwx.  1 root root       7 Nov 16 14:50 loop1p1 -> ../dm-4
```

So, I still have the same problem.  This has got to be the first thing I tackle.

---

After a few reboots and some line-by-line testing, I have finally settled on the following:

```
#
# -- This rule and the following recipe is used to build a disk image that can be booted:
#    * create a disk image, size = 20MB
#    * make the partition table, partition it, and set it to bootable
#    * map the partitions from the image file
#    * write an ext2 file system to the first partition
#    * create a temporary mount point
#    * Mount the filesystem via loopback
#    * copy the files to the disk
#    * unmount the device
#    * unmap the image
#
#    In the event of an error along the way, the image is umounted and the partitions unmapped.
#    Finally, if the error cleanup is completely suffessful, then false is called to fail the
#    recipe.
#    ------------------------------------------------------------------------------------------------
rpi2b-iso: all
	rm -fR iso/rpi2b.img
	cp -fR bin/rpi2b/* sysroot/rpi2b/
	find sysroot/rpi2b -type f -name Tupfile -delete
	mkdir -p ./p1
	(																						\
		dd if=/dev/zero of=iso/rpi2b.img count=20 bs=1048576;								\
		parted --script iso/rpi2b.img mklabel msdos mkpart p ext2 1 20 set 1 boot on; 		\
		sudo losetup -v -L -P /dev/loop0 iso/rpi2b.img;										\
		sudo mkfs.ext2 /dev/loop0p1;														\
		sudo mount /dev/loop0p1 ./p1;														\
		sudo cp -R sysroot/rpi2b/* p1/;														\
		sudo umount ./p1;																	\
		sudo losetup -v -d /dev/loop0;														\
	) || (																					\
		sudo umount ./p1;																	\
		sudo losetup -v -d /dev/loop0;														\
	) || false
```

This appears to be working.

Now I need to back out all the changes to `rpi-boot` and start testing all over again.  I cannot assume that anything I was seeing before was not related to the mal-formed disk image I kept reading.  I get all kinds of new and different stuff!

```
SD: read() card ready, reading from block 2048
SD: issuing command CMD18
SD: multi block transfer, awaiting block 0 ready
SD: block 0 transfer complete
SD: multi block transfer, awaiting block 1 ready
SD: block 1 transfer complete
SD: command completed successfully
SD: data read successful
MULTIBOOT: no valid multiboot header found in /boot/loader.elf
cfg_parse: multiboot failed with -1
```

I certainly need to back out all the debugging `#define`s so that I can get a cleaner boot and try to get to the root of the problem now.  And I get cleaner messages:

```
Welcome to Rpi bootloader
Compiled on Nov 16 2018 at 15:43:38
ARM system type is c43
EMMC: bcm_2708_power_off(): property mailbox did not return a valid response.
EMMC: BCM2708 controller did not power cycle successfully
EMMC: vendor 24, sdversion 1, slot_status 0
EMMC: WARNING: old SDHCI version detected
SD: found a valid version 1.0 and 1.01 SD card
MBR: found valid MBR on device emmc0
EXT2: found an ext2 filesystem on emmc0_0
MBR: found total of 1 partition(s)
MAIN: device list: emmc0_0(ext2)
MAIN: Found bootloader configuration: /boot/rpi_boot.cfg
MULTIBOOT: no valid multiboot header found in /boot/loader.elf
cfg_parse: multiboot failed with -1
```

This is so much better!!

So, this is relatively easy to track down.  `readelf -a bin/boot/loader.elf` yields the following:

```
Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x010000 0x00100000 0x00100000 0x12004 0x13334 RWE 0x10000
```

THe offset must be in the first 4K for the Multiboot specification.  I am at 256K.  A simple flag addition solved this problem.

```
LDFLAGS += -z max-page-size=0x1000
```

So, now I am booting and getting nothing done!  I cannot say for sure just how far I have gotten.  But it is huge progress over last night.

```
Welcome to Rpi bootloader
Compiled on Nov 16 2018 at 15:43:38
ARM system type is c43
EMMC: bcm_2708_power_off(): property mailbox did not return a valid response.
EMMC: BCM2708 controller did not power cycle successfully
EMMC: vendor 24, sdversion 1, slot_status 0
EMMC: WARNING: old SDHCI version detected
SD: found a valid version 1.0 and 1.01 SD card
MBR: found valid MBR on device emmc0
EXT2: found an ext2 filesystem on emmc0_0
MBR: found total of 1 partition(s)
MAIN: device list: emmc0_0(ext2)
MAIN: Found bootloader configuration: /boot/rpi_boot.cfg
MULTIBOOT: loaded kernel /boot/loader.elf
BOOT: multiboot load
```

But at this point, I need to commit the code.  I am able to boot properly into the loader but it locks up.

---

Let's get into determining where the problem is...  I know I am passing all the edits to get to the loader.  Let's start with addresses.  I want to make sure that the address the loader is being loaded at is a good address.  To do this, I can compare to century where I was able to boot with `qemu` at one point.

The addresses look good.  So, I'm going to take a step back....  Thinking this through, the `rpi-boot` code is able to write to the serial port and it shows up on the command line as output.  This is what I want to do with my code.  `rpi-boot` works; mine does not.  One thing I am wondering is if `rpi-boot` uses the mini-UART or the PL011 UART.  I need to compare the 2 solutions to see what the differences are.

`rpi-boot` uses the following base UART address:

```C
#define UART0_BASE			0x20201000
```

I use:

```C
#define HW_BASE             (0x3f000000)
#define UART_BASE           (HW_BASE+0x201000)          // The UART base register
```

So, I am using the incorrect address.  However, changing the base hardware address did nothing.  I changed it back.

A line-by-line comparison shows that I had an inequality comparison as an equality check, fixed here:

```C
void SerialPutChar(char byte)
{
    while ((MmioRead(UART_BASE + UART_FR) & UARTFR_TXFF) != 0) { }

    MmioWrite(UART_BASE + UART_DR, byte);
}
```

Changing this check results in lots of additional debugging info:

```
BOOT: multiboot load
Serial port initialized!
Setting basic memory information
/boot/loader.elf
Module information present
Setting memory map data

Frame Buffer is at: 0x0c100000; The pitch is: 0x00000500; The height is: 0x000001e0
```

However, I am not getting a lot of information from `rpi-boot` about the system.  The good thing is that every rpi2b has basically the same config and this can be hard coded.  For the record, it looks like I am getting the following information from `rpi-boot1`:
* Basic Memory info
* Command line
* Module Info
* Memory Map
* Boot loader name (most likely, but blank)
* Frame buffer info

This covers the critical information for rpi2b (particularly the middle 2 points for century).  The next thing to look at with loader is the temporary PMM initialization.  The next step is to add some debugging statements to figure out where the problem is.

This check came up empty:

```C
    // -- pages now holds the bitmap aligned to 4K right up to the EBDA or 640K boundary; set to no available memory
    SerialPutS("Getting ready to init the bitmap\n");
    kMemSetB((void *)start, 0, PmmFrameToLinear(pages));
    SerialPutS("Done clearing the bitmap\n");
```

I got both lines.  However, I never get here:

```C
    // -- Allocate the loaded modules
    SerialPutS("About to allocate the module locations\n");
    if (HaveModData()) {
```

I do get past this point:

```C
       PmmFreeFrameRange(frame, length);
    }
    SerialPutS("Past the Memory Map\n");
```

So, in short, my problem is somewhere in these lines:

```C
    SerialPutS("Past the Memory Map\n");

    // -- The GDT is at linear address 0 and make it unavailable
    PmmAllocFrame(0);

    // -- Page Directory is not available
    PmmAllocFrame(1);

    // -- The area between the EBDA and 1MB is allocated
    PmmAllocFrameRange(PmmLinearToFrame(GetEbda()), 0x100 - PmmLinearToFrame(GetEbda()));

    // -- now that all our memory is available, set the loader space to be not available; _loader* already aligned
    frame_t ls = PmmLinearToFrame((ptrsize_t)_loaderStart);
    frame_t le = PmmLinearToFrame((ptrsize_t)_loaderEnd);
    PmmAllocFrameRange(ls, le - ls);        // _loaderEnd is already page aligned, so no need to add 1 page.

    // -- Allocate the Frame Buffer
    PmmAllocFrameRange(PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr()), 1024 * 768 * 2);

    // -- Allocate the loaded modules
    SerialPutS("About to allocate the module locations\n");
```

Which makes me wonder if the EBDA is a problem, since the rpi2b doesn't have one.  Well, a quick test begs to differ...

```C
    SerialPutS("Allocating EBDA\n");
    PmmAllocFrameRange(PmmLinearToFrame(GetEbda()), 0x100 - PmmLinearToFrame(GetEbda()));
    SerialPutS("Allocated EBDA\n");
```

... where I get both lines out output.  It may not be right, but they both execute.  Finally the following debug line executes, so the problem must be in the FrameBuffer allocation since the module locations debug locations do not print:

```C
    SerialPutS("Allocated EBDA\n");

    // -- now that all our memory is available, set the loader space to be not available; _loader* already aligned
    frame_t ls = PmmLinearToFrame((ptrsize_t)_loaderStart);
    frame_t le = PmmLinearToFrame((ptrsize_t)_loaderEnd);
    PmmAllocFrameRange(ls, le - ls);        // _loaderEnd is already page aligned, so no need to add 1 page.
    SerialPutS("Loader allocated\n");

    // -- Allocate the Frame Buffer
    PmmAllocFrameRange(PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr()), 1024 * 768 * 2);

```

I was finally able to whittle this down the the following frame buffer operation where it was allocating a huge number of frames:

```C
    SerialPutS("Frame Buffer Address: "); SerialPutHex((ptrsize_t)GetFrameBufferAddr()); SerialPutChar('\n');
    SerialPutS("Number of frames to allocate: "); SerialPutHex(1024 * 768 * 2); SerialPutChar('\n');
    PmmAllocFrameRange(PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr()), (1024 * 768 * 2) >> 12);
```

The bug was that the number of frames was not adjusted -- it was just a byte count.  With this change my boot sequence looks like this:

```
BOOT: multiboot load
Serial port initialized!
Setting basic memory information
/boot/loader.elf
Module information present
Setting memory map data

Frame Buffer is at: 0x0c100000; The pitch is: 0x00000500; The height is: 0x000001e0
Getting ready to init the bitmap
Done clearing the bitmap
Past the Memory Map
Loader allocated
Frame Buffer Address: 0x0c100000
Number of frames to allocate: 0x00180000
About to allocate the module locations
Phyiscal Memory Manager Initialized
Unable to locate Kernel...  Halting!
```

This is good news -- everything is executing and it is looking for the kernel which is not present.  I cleaned up the debugging code.  However, with that said, the screen is not clearing and I'm certain I did not complete the MMU initialization.  I am getting a message on the screen, but the screen is not clearing first.

I will look into the FrameBuffer tomorrow as it is the next thing in order in `LoaderInit()`.

---

### 2018-Nov-17

So, I was thinking last night...  Of the tracable portions that happen in the `LoaderInit()` function, the FrameBuffer is giving me trouble.  But, it's not the frame buffer.  I am getting the greeting message properly shown.  The problem is that the screen is not clearing properly.  This clear screen function is written in assembly and is using the registers from the ABI...  both of which are likely wrong.  I coudld re-implement these functions in C and bypass both problems, but I really need to make sure I have this dialed in.

Well, the ABI understanding was correct.  This is a good thing.  However, the assembly instruction was not correct.  I was trying to auto-increment an address:

```
    strh    r1,[r0]!                                @@ store the value in r1 to the mem at addr r0 and update r0
```

For whatever reason, this was creating a problem, and likely an alignment data abort that my software did not pick up.  I changed the code to this:

```
    strh    r1,[r0]                                 @@ store the value in r1 to the mem at addr r0
    add     r0,#2                                   @@ move to the next address
```

... and this worked.

This now brings me to `MmuInit()` -- the next big thing.  But first a commit.  Oh, did I mention I finally got all of libk eliminated?

---

### 2018-Nov-18

This morning I am going to use what time I have to put some details into the MMU initialization.  Right now, the function is merely a stub so that I could get the loader to compile and I could try to run it to make sure I had all my basics right.  Good thing I did since I had a couple days of some pretty big issues to work through for the rpi2b build system.

I will start by allocating 16K for the kernel TTL1 table.  Recall from **2018-Nov-14** that I will be splitting the address space in half for the user (where bit 31 == 0) and kernel (where bit 31 == 1).

So, I will allocate this table and set it up.

As I write the first lines of code, I realize that I really have no clue what the rpi2b memory map looks like.  Since my PMM allocates by specific frame or from the last frame we know of, this makes it difficult to allocate memory without knowing where things are.  So I will print the memory map to the serial port.  I get only 1 block of physical memory:

```
Upper memory limit: 0x0c000000
Grub Reports available memory at 0x00000000 for 0x0000c000 frames
```

Now, where to put the TTL1 table.  I think the best thing to do is to put the table up high, well out fo the way.  Let me start allocating at frame number 0xb000 or more to the point: `GetUpperMemLimit() - 0x1000000`.

Ok, now I have a design decision to make.  The TTL2 table is 1K, and I can fit 4 of them into a single frame.  The question is this: do I map the 4 TTL2 tables to be consecutive such that a request to set up a TTL2 table results in `% 4 == 0`, `% 4 == 1`, `% 4 == 2`, and `% 4 == 3` all being mapped?  Or do I just map the singular TTL2 table and leave the remaining 3 frames available for additional TTL2 tables to be mapped at a later time?  Here's the concern: I am planning on putting the TTL2 tables in order at specific memory addresses so that I can mimic the recursive mappings from x86.  Since 4 of them fit into a frame, this requires me to have them in order.  I would rather save on memory, but I think the management method I have chosen will trump that desire.

So, what to I need to implement?  Let's start with an `MmuMapToFrame()` function.  Taking a top-down approach, this will drive the additional functions I will need to implement.  I will follow the same parameter convention I have with x86.

---

Looking at `MmuMakeTtl2Table()`, this is quite a bit more work without the native "recurisve mapping" trick.  I have to handle the tables to access the tables separately and explicitly.  It's a bit more work but is very doable.

So, I have the function written.  It's recursive and I have one of those feelings I have written a function that will generate a stack overflow.  Testing will tell, but note that I have my doubts already.

I got the loader to compiler and I am now going to set up to complete the mapping for the TTL1 table into the management area.  This will be at address 0xffbfc000 for 4 pages.

Well, my first execution was anti-climactic:

```
Creating a new TTL2 table for address 0xffbfc000
Creating a new TTL2 table for address 0xffbfd000
Creating a new TTL2 table for address 0xffbfe000
Creating a new TTL2 table for address 0xffbff000
Unable to locate Kernel...  Halting!
```

So I commented out this line:

```
    if (addr >= 0xffbfc000 && addr < 0xffc00000) return;
```

... and I now have these results:

```
Creating a new TTL2 table for address 0xffbfc000
  The new frame is 0x0000b004
Creating a new TTL2 table for address 0xffc00000
  The new frame is 0x0000b005
Creating a new TTL2 table for address 0x3fc00000
  The new frame is 0x0000b006
Attempting do map already mapped address 0xffc00000
Creating a new TTL2 table for address 0x7fc00000
  The new frame is 0x0000b007
Attempting do map already mapped address 0xffc00000
Attempting do map already mapped address 0x3fc00000
Creating a new TTL2 table for address 0xbfc00000
  The new frame is 0x0000b008
Attempting do map already mapped address 0xffc00000
Attempting do map already mapped address 0x3fc00000
Attempting do map already mapped address 0x7fc00000
< Completed the table creation for 0xbfc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0x7fc00000
Attempting do map already mapped address 0x7fc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0x3fc00000
Attempting do map already mapped address 0x3fc00000
Attempting do map already mapped address 0x7fc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0xffc00000
Attempting do map already mapped address 0xffc00000
Attempting do map already mapped address 0x3fc00000
Attempting do map already mapped address 0x7fc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0xffbfc000
Creating a new TTL2 table for address 0xffbfd000
  The new frame is 0x0000b009
Attempting do map already mapped address 0xffc00000
Attempting do map already mapped address 0x3fc00000
Attempting do map already mapped address 0x7fc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0xffbfd000
Creating a new TTL2 table for address 0xffbfe000
  The new frame is 0x0000b00a
Attempting do map already mapped address 0xffc00000
Attempting do map already mapped address 0x3fc00000
Attempting do map already mapped address 0x7fc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0xffbfe000
Creating a new TTL2 table for address 0xffbff000
  The new frame is 0x0000b00b
Attempting do map already mapped address 0xffc00000
Attempting do map already mapped address 0x3fc00000
Attempting do map already mapped address 0x7fc00000
Attempting do map already mapped address 0xbfc00000
< Completed the table creation for 0xffbff000
Attempting do map already mapped address 0xffbff000
Unable to locate Kernel...  Halting!
```

This is neither the stack overflow I expected nor the result I really want.

So, let's pick apart the first bits of this:
1. A request to map address 0xffbfc000 -- which is the first page for the TTL1 table.
1. A new frame is allocated: 0xb004.
1. A level 2 request to map address 0xffc00000 -- which is the first location for virtual addresses 0x00000000 to 0x00400000.  *This is incorrect.*
1. A new frame is allocated: 0xb005.
1. A level 3 request to map address 0x3fc00000.  *No way this is right.*
1. There is not point in continuing the analysis.

I found a shifting problem with my calculations.

```
MmuMapToFrame(ttl1, 0xffc00000 + ((i << 20) * 1024), ttl2Frame, true, true);
```

The `<< 20` portion was removed and I am now getting the recursion problem I was expecting:

```
Mapping address 0xffbfc000 to frame 0x0000b000
Creating a new TTL2 table for address 0xffbfc000
  The new frame is 0x0000b004
  The base ttl2 index is 0x00000ff8
Mapping address 0xffffe000 to frame 0x0000b004
Creating a new TTL2 table for address 0xffffe000
  The new frame is 0x0000b005
  The base ttl2 index is 0x00000ffc
Mapping address 0xfffff000 to frame 0x0000b005
Creating a new TTL2 table for address 0xfffff000
  The new frame is 0x0000b006
  The base ttl2 index is 0x00000ffc
Mapping address 0xfffff000 to frame 0x0000b006
Creating a new TTL2 table for address 0xfffff000
```

---

### 2018-Nov-19

I need to start today by finishing up my thoughts from yesterday.  I am mapping the first address to the first frame of the TTL1 for management.  This is address 0xffbfc000.  This address will not yet have an associated TTL2 table.  I will get this table and map it into the TTL1 entry for address 0xffbfc000.  It will also need to be mapped into the address for the TTL2 management address.  The index into the TTL1 table is 0xffb, and the address for the TTL2 table for managing this frame is (0xffc00000 + (0xffb * 1024)), or 0xffffec00.  So, something is not right -- either my calculations or the code.

It looks like I am confusing the mapping for the management portion of the tables.  I need to back that recursive call back out and rethink that.

The TTL1 table has 4096 * 4-byte entries (16K).  Each entry therefore controls access to 1M of memory.

There are 4096 TTL2 tables, each 1K in length.  Each table contains 256 entries.  All of these tables will take up 1024 frames, or 4MB, and will have a total of 1048576 entries.  These tables will start at 0xffc00000 and will be contiguous.

Each entry in the TTL2 table could be indexed by the address from 0x00000 to 0xfffff, or 20 bits or vAddr >> 12.  And then to get the address of the TTL2 entry for a TTL2 table in the management addresses, it would be (((vAddr >> 12) * 4) + 0xffc00000).

I now see that I am working with frames and need to be working with 1K physical addresses, so I added a shift here and renamed the variable accordingly:

```C
    frame_t ttl2PAddr = allocFrom << 2;         // Adjust to 1K accuracy
```

Taking this step by step in the code, I finally think I have something that works.  It certainly does what I expect.

```C
    // Here we need to get the TTL1 entry for the management address.
    ptrsize_t mgmtTtl2Addr = 0xffc00000 + ((addr >> 12) * 4);
    int mgmtTtl1Index = mgmtTtl2Addr >> 20;
    Ttl1_t *mgmtTtl1Entry = &ttl1Table[mgmtTtl1Index];

    SerialPutS("  The TTL1 management index for this address is "); SerialPutHex(mgmtTtl1Index); SerialPutChar('\n');

    // If the TTL1 Entry for the management address is faulted; create a new TTL2 table
    if (mgmtTtl1Entry->fault == 0b00) {
        MmuMakeTtl2Table(ttl1, mgmtTtl2Addr);
    }
```

The resuls are:

```
Mapping address 0xffbfc000 to frame 0x0000b000
Creating a new TTL2 table for address 0xffbfc000
  The new frame is 0x0000b004
  The base ttl2 base addr is 0x0002c010
  The base ttl2 index is 0x00000ff8
  The TTL1 management index for this address is 0x00000fff
Creating a new TTL2 table for address 0xffffeff0
  The new frame is 0x0000b005
  The base ttl2 base addr is 0x0002c014
  The base ttl2 index is 0x00000ffc
  The TTL1 management index for this address is 0x00000fff
< Completed the table creation for 0xffffeff0
< Completed the table creation for 0xffbfc000
Mapping address 0xffbfd000 to frame 0x0000b400
Creating a new TTL2 table for address 0xffbfd000
  The new frame is 0x0000b006
  The base ttl2 base addr is 0x0002c018
  The base ttl2 index is 0x00000ff8
  The TTL1 management index for this address is 0x00000fff
< Completed the table creation for 0xffbfd000
Mapping address 0xffbfe000 to frame 0x0000b800
Creating a new TTL2 table for address 0xffbfe000
  The new frame is 0x0000b007
  The base ttl2 base addr is 0x0002c01c
  The base ttl2 index is 0x00000ff8
  The TTL1 management index for this address is 0x00000fff
< Completed the table creation for 0xffbfe000
Mapping address 0xffbff000 to frame 0x0000bc00
Creating a new TTL2 table for address 0xffbff000
  The new frame is 0x0000b008
  The base ttl2 base addr is 0x0002c020
  The base ttl2 index is 0x00000ff8
  The TTL1 management index for this address is 0x00000fff
< Completed the table creation for 0xffbff000
```

This should allow me to initialize the MMU from `MmuInit()`.

---

I am misusing the word 'frame' in all my rpi2b architecture... because it was grandfathered in from the x86 code in the form `MmuMapToFrame()`.  I need to rename this function to be architecture agnostic.  Later.

---

So, here's the problem I am having: I am just not getting my head around the fact that the second level table is only 1K in size.  In my head, I am not keeping track of where I am looking for an ordinal frame number and a 1K TTL2 table.  for example, the following block of code is horribly wrong:

```C
    frame_t ttl1 = allocFrom;

    // ---- snip ----

    // -- Map the TTL1 table to location 0xffbfc000
    MmuMapToFrame(ttl1, 0xffbfc000, ttl1, true, true);
    MmuMapToFrame(ttl1, 0xffbfd000, ttl1 + 1024, true, true);
    MmuMapToFrame(ttl1, 0xffbfe000, ttl1 + 2048, true, true);
    MmuMapToFrame(ttl1, 0xffbff000, ttl1 + 3072, true, true);
```

The reason is that the variable `ttl1` is a `frame_t` type and the very next frame should be the next one in the TTL1 table.

---

### 2018-Nov-20

Let me see if I can keep this straight today as I'm working through all this code....  I have some hope, but not high hopes.

---

OK, I whittled down the address I was mapping to just one and focused in on getting all my calculations consistent with the requirements.  I finally have a good page mapped (or at least consistently wrong if it is wrong).  The output with all my debugging code is:

```
Set up the TTL1 management table
Mapping address 0xffbfc000 to frame 0x0b000000
  Ttl1 index is: 0x0b000000[0x00000ffb]
Creating a new TTL2 table for address 0xffbfc000
  The new frame is 0x0000b004
  The base ttl2 1K location is 0x0002c010
  The ttl1 index is 0x00000ff8
  Set the TTL1 table index 0x00000ff8 to 1K location 0x0002c010
  Set the TTL1 table index 0x00000ff9 to 1K location 0x0002c011
  Set the TTL1 table index 0x00000ffa to 1K location 0x0002c012
  Set the TTL1 table index 0x00000ffb to 1K location 0x0002c013
  The management address for this Ttl2 table is 0xffffeff0
    The base location is 0xffc00000
    The table offset is  0x003fec00
    The entry offset is  0x000003f0
  The TTL1 management index for this address is 0x00000fff
Creating a new TTL2 table for address 0xffffeff0
  The new frame is 0x0000b005
  The base ttl2 1K location is 0x0002c014
  The ttl1 index is 0x00000ffc
  Set the TTL1 table index 0x00000ffc to 1K location 0x0002c014
  Set the TTL1 table index 0x00000ffd to 1K location 0x0002c015
  Set the TTL1 table index 0x00000ffe to 1K location 0x0002c016
  Set the TTL1 table index 0x00000fff to 1K location 0x0002c017
  The management address for this Ttl2 table is 0xfffffff8
    The base location is 0xffc00000
    The table offset is  0x003ffc00
    The entry offset is  0x000003f8
  The TTL1 management index for this address is 0x00000fff
< Completed the table creation for 0xffffeff0
< Completed the table creation for 0xffbfc000
  Ttl2 location is: 0x0b004c00[0x000000fc]
Checking our work

MmuDumpTables: Walking the page tables for address 0xffbfc000
Level  Tabl-Addr     Index        Entry Addr    Next PAddr    fault
-----  ----------    ----------   ----------    ----------    -----
TTL1   0x0b000000    0x00000ffb   0x0b003fec    0x0b004c00     01
TTL2   0x0b004c00    0x000000fc   0x0b004ff0    0x00000000     10
```

I have checked all the math and it all looks good for now.  No stack overflow.  Now I will start to put in the additional addresses I need to map.

I have removed a number of debugging statements using `#if 0` preprocessor directives, so you as can see what the heck I had to go through to get this code debugged.

With that, I have all the major components of the loader done except the module initialization.  For that to work, I need a kernel to compile.  So, I'm back to architecture abstraction.

---

For `interrupt.h`, nearly everything in this file is specific to i686, so I have moved the contents to `arch-interrupt.h`.  Done.

Now, for compiling kInit.cc, there is a structure `isrRegs_t` which is not defined on the rpi2b architecture.  I will need to get that defined.  Which means I need to figure out what the architecture really does.  Additionally, I am getting a message: `Error: selected processor does not support 'wfi' in ARM mode`.  However, is should.

So, I created the `isrRegs_t` structure to be as follows:

```
//
// -- This is the order of the registers on the stack
//    -----------------------------------------------
typedef struct isrRegs_t {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	uint32_t r13;
	uint32_t r14;
	uint32_t r15;
} isrRegs_t;
```

This will certainly change, but it takes care of the basic requirements of the interrupt for ARM.  Also, I had to add the following to the `Tuprules.tup` file to handle the `wfi` instruction:

```
ifneq ($(ARCH),rpi2b)
CFLAGS += -mno-red-zone
else
CFLAGS += -mcpu=cortex-a7
endif
```

`kInit.cc` now compiles.

On to `HeapAlloc.cc`.  I need a constant for `BYTE_ALIGNMENT`.  That was a quick fix.

In `HeapInit.cc`, there are references to `pageTable_t` which is incorrect for rpi2b.  This reference is coming from `mmu.h`.

This is creating a problem between the loader and the kernel.  Each has its own `mmu.h` file and I need to keep them separate.  So, I will eliminate completely the `mmu.h` create either `mmu-loader.h` and `mmu-kernel.h` and everything `inc/$(ARCH)/arch-*.h` fill be renamed to `inc/$(ARCH)/arch-*-prevalent.h` -- not that the contents are common to all architectures but that the contents are used by multiple modules.

I will work on this now and get the other modules to compile -- beore I tackle `mmu.h` from the kernel.

---

### 2018-Nov-21

I spent the first part of the day organizing the kernel so that all the `.o` files compile.  I know I am going to have a bunch of issues linking the resulting `kernel.elf` file.  There are several things I am going to have to re-build when I try to actually link the `kernel.elf`.  And as I expected:

```
/home/adam/opt/cross/lib/gcc/arm-eabi/6.3.0/../../../../arm-eabi/bin/ld: warning: cannot find entry symbol _start; defaulting to 0000000080000000
/home/adam/workspace/century-os/obj/kernel/rpi2b/CpuTssInit.o: In function `CpuTssInit()':
/home/adam/workspace/century-os/modules/kernel/src/cpu/CpuTssInit.cc:29: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/modules/kernel/src/cpu/CpuTssInit.cc:35: undefined reference to `Ltr'
/home/adam/workspace/century-os/obj/kernel/rpi2b/FrameBufferClear.o: In function `FrameBufferClear()':
/home/adam/workspace/century-os/modules/kernel/src/frame-buffer/FrameBufferClear.cc:36: undefined reference to `kMemSetW'
/home/adam/workspace/century-os/obj/kernel/rpi2b/FrameBufferDrawChar.o: In function `FrameBufferDrawChar(char)':
/home/adam/workspace/century-os/modules/kernel/src/frame-buffer/FrameBufferDrawChar.cc:79: undefined reference to `systemFont'
/home/adam/workspace/century-os/obj/kernel/rpi2b/FrameBufferParseRGB.o: In function `FrameBufferParseRGB(char const*)':
/home/adam/workspace/century-os/modules/kernel/src/frame-buffer/FrameBufferParseRGB.cc:42: undefined reference to `kStrLen'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapAlloc.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapAlloc.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapAlloc.o: In function `HeapAlloc(unsigned int, bool)':
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapAlloc.cc:57: undefined reference to `DisableInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapAlloc.cc:76: undefined reference to `RestoreInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapAlloc.cc:91: undefined reference to `RestoreInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapAlloc.cc:112: undefined reference to `RestoreInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapAlloc.cc:123: undefined reference to `RestoreInterrupts'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapError.o: In function `HeapError(char const*, char const*)':
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapError.cc:32: undefined reference to `DisableInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapError.cc:34: undefined reference to `Halt'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapFree.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapFree.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapFree.o: In function `HeapFree(void*)':
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapFree.cc:50: undefined reference to `DisableInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapFree.cc:78: undefined reference to `RestoreInterrupts'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapInit.o: In function `HeapInit()':
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapInit.cc:76: undefined reference to `MmuUnmapPage(unsigned long)'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapInit.cc:77: undefined reference to `MmuMapToFrame(unsigned long, unsigned long, int)'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapInit.cc:83: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/obj/kernel/rpi2b/MessageReceive.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/MessageReceive.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/MessageReceive.o: In function `MessageReceive(Message_t*)':
/home/adam/workspace/century-os/modules/kernel/src/ipc/MessageReceive.cc:59: undefined reference to `kMemMove'
/home/adam/workspace/century-os/obj/kernel/rpi2b/MessageSend.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/MessageSend.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/MessageSend.o: In function `MessageSend(unsigned long, Message_t*)':
/home/adam/workspace/century-os/modules/kernel/src/ipc/MessageSend.cc:51: undefined reference to `kMemMove'
/home/adam/workspace/century-os/obj/kernel/rpi2b/PmmAllocFrame.o: In function `PmmAllocFrame()':
/home/adam/workspace/century-os/modules/kernel/src/pmm/PmmAllocFrame.cc:34: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessCreate.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessCreate.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessCreate.o: In function `ProcessCreate(char const*, unsigned long, unsigned long, ...)':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessCreate.cc:57: undefined reference to `Halt'
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessCreate.cc:66: undefined reference to `kMemSetB'
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessCreate.cc:104: undefined reference to `GetCr3'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessEnd.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessEnd.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessEnd.o: In function `ProcessEnd()':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessEnd.cc:90: undefined reference to `Halt'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessHold.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessHold.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessInit.o: In function `ProcessInit()':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessInit.cc:53: undefined reference to `GetCr3'
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessInit.cc:62: undefined reference to `GetCr3'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessNewPID.o: In function `ProcessNewPID()':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessNewPID.cc:51: undefined reference to `Halt'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReady.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReady.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReady.o: In function `ProcessReady(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessReady.cc:107: undefined reference to `ProcessSwitch'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessRelease.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessRelease.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessRelease.o: In function `ProcessRelease(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessRelease.cc:73: undefined reference to `ProcessSwitch'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReschedule.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReschedule.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReschedule.o: In function `ProcessReschedule()':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessReschedule.cc:74: undefined reference to `Halt'
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessReschedule.cc:83: undefined reference to `ProcessSwitch'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessTerminate.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessTerminate.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessWait.o: In function `SpinlockLock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:64: undefined reference to `SpinlockCmpXchg'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessWait.o: In function `SpinlockUnlock(Spinlock_t*)':
/home/adam/workspace/century-os/modules/kernel/inc/spinlock.h:73: undefined reference to `SpinlockClear'
/home/adam/workspace/century-os/obj/kernel/rpi2b/SerialInit.o: In function `SerialInit()':
/home/adam/workspace/century-os/modules/kernel/src/serial/rpi2b/SerialInit.cc:33: undefined reference to `BusyWait(unsigned long)'
/home/adam/workspace/century-os/modules/kernel/src/serial/rpi2b/SerialInit.cc:35: undefined reference to `BusyWait(unsigned long)'
/home/adam/workspace/century-os/obj/kernel/rpi2b/TimerEoi.o: In function `TimerEoi(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerEoi.cc:29: undefined reference to `outb'
/home/adam/workspace/century-os/obj/kernel/rpi2b/TimerInit.o: In function `TimerInit(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:42: undefined reference to `DisableInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:45: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:46: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:47: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:48: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:49: undefined reference to `outb'
/home/adam/workspace/century-os/obj/kernel/rpi2b/TimerInit.o:/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:50: more undefined references to `outb' follow
/home/adam/workspace/century-os/obj/kernel/rpi2b/TimerInit.o: In function `TimerInit(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:60: undefined reference to `IsrRegister(unsigned char, void (*)(isrRegs_t*))'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:62: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:63: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:64: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:68: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:69: undefined reference to `outb'
/home/adam/workspace/century-os/modules/kernel/src/timer/TimerInit.cc:71: undefined reference to `RestoreInterrupts'
/home/adam/workspace/century-os/obj/kernel/rpi2b/kInit.o: In function `kInit':
/home/adam/workspace/century-os/modules/kernel/src/kInit.cc:94: undefined reference to `PmmStart(Module_t*)'
/home/adam/workspace/century-os/modules/kernel/src/kInit.cc:99: undefined reference to `EnableInterrupts'
/home/adam/workspace/century-os/modules/kernel/src/kInit.cc:105: undefined reference to `kMemSetB'
```

With this list, a couple of things stand out to me right away:
* My abstractions are not clean since rpi2b is still looking for `out()` and should have failed at compiling the source to a `.o`.
* I am not compiling everything since `kMemSetB()` is implemented and should be found.
* The function `TssInit()` should not exist for rpi2b -- right at the top of the list.

So, I'm going to first take care of the top error (about `_start`).  For i686, the `_start` symbol is in the `loader.s` file in the `$(ARCH)` folder.  The `rpi2b` folder is conspicuously empty and quite a number of my problems will be addressed by putting the right functions in that folder.

I copied nearly all the files from the loader to the kernel for the rpi2b architecture.  This cleaned up several errors but there is still quite a bit to solve.

I also corrected the existence of `in()`, `out()`, and `GetCr3()` in the rpi2b architecture.  This caused several more functions to fail compile, which of course meant they needed to be moved into respective architecture specific folders.

Now, I can get into the architcture-specific functions I need to learn how to properly implement -- starting with `EnableInterrupts()`.  Why?  Because it's the last thing on the list of errors at this point, of course!

---

The ARM CPU has a Current Program Status Register (CPSR) that contains several bits that indicate what exceptions can be fed through to the CPU.  There are 3 such bits: A, I, and F, such that:
* CPSR:A is bit 8 and controls imprecice data aborts
* CPSR:I is bit 7 and controls IRQ interrupts
* CPSR:F is bit 6 and controls FIQ interrupts

A value of `1` in any of these 3 bits disables interrupts of that type.  Therefore, to enable interrupts, I will need to read the CPSR, clear bits 6:8, and then write the CPSR.

* The opcode `mrs` can read the CPSR in the form (I think, anyway) `mrs r0,cpsr`.
* The opcode `msr` can write the CPSR in the form (I think, anyway) `msr cpsr,r0`.
* Therefore, the only thing I need to do in between is `and r0,~0x01c0` to clear the proper bits.

With this, I should have enough to implement the `EnableInterrupts()` function for rpi2b.

Using `EnableInterrupts()` as a template, I can create `DisableInterrupts()` which will return the state of the interrupts flags only  (which is a difference from the x86 implementation) and `RestoreInterrupts()` which will restore the state of the interrupt flags only (which is a difference from the x86 implementation).

Those all compile and the missing references have gone away.

The next thing to look at is Spinlocks.  There are a lot of referenced to those functions and once those are cleaned up, I might actually have a small set of remaining functions I can actually stub out to start some testing.

---

ARM has an opcode `strex` which is an atomic operation similar to the x86 `lock cmpxchg` operation.  Section 1.3.2 of this manual has a sample for implementing a mutex: http://infocenter.arm.com/help/topic/com.arm.doc.dht0008a/DHT0008A_arm_synchronization_primitives.pdf.  This link has the standards for implementing a stack frame: https://thinkingeek.com/2014/05/11/arm-assembler-raspberry-pi-chapter-18/.

The first function I want to implement is `SpinlockCmpXchg()`, but that is i686 specific naming -- I need to clean that up.

And now with `SpinlockAtomicLock()` and `SpinlockClear()` both written, here is my current error list:

```
/home/adam/workspace/century-os/obj/kernel/rpi2b/CpuTssInit.o: In function `CpuTssInit()':
/home/adam/workspace/century-os/modules/kernel/src/cpu/CpuTssInit.cc:35: undefined reference to `Ltr'
/home/adam/workspace/century-os/obj/kernel/rpi2b/HeapInit.o: In function `HeapInit()':
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapInit.cc:76: undefined reference to `MmuUnmapPage(unsigned long)'
/home/adam/workspace/century-os/modules/kernel/src/heap/HeapInit.cc:77: undefined reference to `MmuMapToFrame(unsigned long, unsigned long, int)'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReady.o: In function `ProcessReady(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessReady.cc:107: undefined reference to `ProcessSwitch'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessRelease.o: In function `ProcessRelease(unsigned long)':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessRelease.cc:73: undefined reference to `ProcessSwitch'
/home/adam/workspace/century-os/obj/kernel/rpi2b/ProcessReschedule.o: In function `ProcessReschedule()':
/home/adam/workspace/century-os/modules/kernel/src/process/ProcessReschedule.cc:83: undefined reference to `ProcessSwitch'
/home/adam/workspace/century-os/obj/kernel/rpi2b/TimerVars.o:(.data.rel+0x0): undefined reference to `TimerInit(unsigned long)'
/home/adam/workspace/century-os/obj/kernel/rpi2b/TimerVars.o:(.data.rel+0x4): undefined reference to `TimerEoi(unsigned long)'
/home/adam/workspace/century-os/obj/kernel/rpi2b/kInit.o: In function `kInit':
/home/adam/workspace/century-os/modules/kernel/src/kInit.cc:90: undefined reference to `ProcessInit()'
/home/adam/workspace/century-os/modules/kernel/src/kInit.cc:94: undefined reference to `PmmStart(Module_t*)'
/home/adam/workspace/century-os/modules/kernel/src/kInit.cc:98: undefined reference to `TimerInit(unsigned long)'
```

The function `CpuTssInit()` should not exist for the rpi2b architecture.  I will deal with that next.

---

I finally have a compiled `kernel.elf`.  With this, I now have a kernel module I can try to find from the loader.  But that will start tomorrow.  It's a late night.

---

This morning, I am realizing that I am not getting any information about the modules back from `rpi-boot`.  I am not sure why -- if it is a limitation in `rpi-boot` to pass this information back or if it is just not loading the modules.  To get there, I will need to add some debugging code to determine if there is a concern with actually getting the module info or not.  This will be in the MB1 parser.

With this block of code:

```C
        SerialPutS("Module information present\n");

        for (m = (Mb1Mods_t *)mb1Data->modAddr, i = 0; i < mb1Data->modCount; i ++) {
            SerialPutS("   Found Module: ");
            SerialPutS(m[i].modIdent);
            SerialPutS("\n");
            AddModule(m[i].modStart, m[i].modEnd, m[i].modIdent);
        }
```

I can tell that the module information block is present but there is no data in it.  I can confirm that this works for i686:

```
Module information present
   Found Module: kernel
   Found Module: pmm
```

But for rpi2b, I only get this:

```
Module information present
```

So, it looks like I'm going to have to look at the `rpi-boot` code to see what the difference might be.  In `rpi-boot`, the code is there so I should see:

```C
	module_add(address, address + (uint32_t)bytes_read, name);

	printf("MODULE: %s loaded\n", name);
```

Let's go to the logs to see what is there.  There's nothing about a MODULE being present:

```
ARM system type is c43
EMMC: bcm_2708_power_off(): property mailbox did not return a valid response.
EMMC: BCM2708 controller did not power cycle successfully
EMMC: vendor 24, sdversion 1, slot_status 0
EMMC: WARNING: old SDHCI version detected
SD: found a valid version 1.0 and 1.01 SD card
MBR: found valid MBR on device emmc0
EXT2: found an ext2 filesystem on emmc0_0
MBR: found total of 1 partition(s)
MAIN: device list: emmc0_0(ext2)
MAIN: Found bootloader configuration: /boot/rpi_boot.cfg
MULTIBOOT: loaded kernel /boot/loader.elf
BOOT: multiboot load
Serial port initialized!
Setting basic memory information
/boot/loader.elf
Module information present
Setting memory map data
```

I would expect the MODULE line to be between the MULTIBOOT line and the BOOT line.  Now, before I get too crazy on this, I need to determine if I have an image that is being updated or not.  I had that problem before... and I'm not going to waste a ton of time debugging something without verifying that first.  The easiest way to check is to break the MULTIBOOT line so the `loader.elf` will not load.  And I am still having a problem (imagine my expletives here!).

Ok, here is what is odd to me: the image has the correct `grub.cfg` file.

```
[adam@os-dev grub]$ cat grub.cfg
multiboot /boot/loader.elfx
module /boot/kernel.elf kernel
boot
```

Do I have an old test file left over in my `sysroot`?  I have to.  The `rpi_boot.cfg` is taking precidence.  And as a matter fo fact, I see it in the logs above.

```
[adam@os-dev boot]$ ls
total 346
drwxr-xr-x. 2 root root   1024 Nov 22 07:53 grub
-rwxr-xr-x. 1 root root 166292 Nov 22 07:53 kernel.elf
-rwxr-xr-x. 1 root root 101388 Nov 22 07:53 loader.elf
-rwxr-xr-x. 1 root root  73732 Nov 22 07:53 loader.img
-rw-r--r--. 1 root root     32 Nov 22 07:53 rpi_boot.cfg
```

And after clearing out the `sysroot/rpi2b` folder, I now get this:

```
Welcome to Rpi bootloader
Compiled on Nov 16 2018 at 15:43:38
ARM system type is c43
EMMC: bcm_2708_power_off(): property mailbox did not return a valid response.
EMMC: BCM2708 controller did not power cycle successfully
EMMC: vendor 24, sdversion 1, slot_status 0
EMMC: WARNING: old SDHCI version detected
SD: found a valid version 1.0 and 1.01 SD card
MBR: found valid MBR on device emmc0
EXT2: found an ext2 filesystem on emmc0_0
MBR: found total of 1 partition(s)
MAIN: device list: emmc0_0(ext2)
MAIN: Found bootloader configuration: /boot/grub/grub.cfg
MULTIBOOT: loaded kernel /boot/loader.elf
MODULE: cannot load file kernel
cfg_parse: module failed with -1
```

I'm picking up the correct boot config file.  But I now have a problem with the MODULE line.  Going back to the `rpi-boot` code, the reason is that the file cannot be found:

```C
	// Load a module
	FILE *fp = fopen(name, "r");
	if(!fp)
	{
		printf("MODULE: cannot load file %s\n", name);
		return -1;
	}
```

This time it is a bug in `rpi-boot`.  The value of `name` is `kernel` whereas the value of `file` is `/boot/kernel.elf`.  I will make this change and write a commit for `rpi-boot`.  The pull request is here: https://github.com/jncronin/rpi-boot/pull/22.

Now, with that change, I am getting the kernel module loaded.  The loader is also trying to map the kernel into upper memory.  I am getting one failure for a page already mapped:

```
Initializing Modules:
kernel
   Starting Address: 0x0011c000
   FileSize = 0x0000743b; MemSize = 0x0000743b; FileOffset = 0x00001000
      Attempting to map page 0x80000000 to frame 0x0000011d
Attempting to map already mapped address 0x80000000
      Attempting to map page 0x80001000 to frame 0x0000011e
      Attempting to map page 0x80002000 to frame 0x0000011f
      Attempting to map page 0x80003000 to frame 0x00000120
      Attempting to map page 0x80004000 to frame 0x00000121
      Attempting to map page 0x80005000 to frame 0x00000122
      Attempting to map page 0x80006000 to frame 0x00000123
      Attempting to map page 0x80007000 to frame 0x00000124
   FileSize = 0x0001b20c; MemSize = 0x00051838; FileOffset = 0x00009000
      Attempting to map page 0x80008000 to frame 0x00000125
      Attempting to map page 0x80009000 to frame 0x00000126
      Attempting to map page 0x8000a000 to frame 0x00000127
      Attempting to map page 0x8000b000 to frame 0x00000128
```

However, I am not sure what is mapped there.  I do not see anything that is explicitly mapping that location.   Maybe if I report the frame it is mapped to I can have a clue on where to look.  It's already mapped to frame 0.  That's no help.  I also noticed that I am getting some odd frame numbers in when setting up the `.bss` section.

```
      Attempting to map page 0x80024000 to frame 0x00020002
      Attempting to map page 0x80025000 to frame 0x0002000d
      Attempting to map page 0x80026000 to frame 0x0002000e
      Attempting to map page 0x80027000 to frame 0x0002000f
      Attempting to map page 0x80028000 to frame 0x00020010
```

...  and when I look at the map for the `kernel.elf`, I am seeting lots of space that need not be there:

```
Disassembly of section .bss:

80024000 <heapLock>:
	...

80025000 <heapMemoryBlock>:
	...

80035000 <fixedList>:
	...

80039000 <fixedListUsed>:
80039000:	00000000 	andeq	r0, r0, r0

80039004 <_ZL5_heap>:
	...

80039024 <currentPID>:
80039024:	00000000 	andeq	r0, r0, r0

80039028 <procs>:
```

The binary sizes are not horrible (232K for i686 and 162K for rpi2b).  But `readelf -a bin/rpi2b/boot/kernel.elf` is interesting!

```
Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .text             PROGBITS        80000000 001000 005f10 00  AX  0   0  8
  [ 2] .rodata           PROGBITS        80006000 007000 00143b 00   A  0   0  4
  [ 3] .stab             PROGBITS        80008000 009000 01af51 04  WA  0   0  4
  [ 4] .data             PROGBITS        80023000 024000 00020c 00  WA  0   0  4
  [ 5] .bss              NOBITS          80024000 02420c 035838 00  WA  0   0 4096
  [ 6] .ARM.attributes   ARM_ATTRIBUTES  00000000 02420c 000039 00      0   0  1
  [ 7] ._text            PROGBITS        00000000 024248 000004 00      0   0  4
  [ 8] .symtab           SYMTAB          00000000 02424c 0034a0 10      9 712  4
  [ 9] .strtab           STRTAB          00000000 0276ec 00109c 00      0   0  1
  [10] .shstrtab         STRTAB          00000000 028788 000051 00      0   0  1
```

There is a leftover `._text` section that is added after the `.bss` section.  And more importantly it's a `PROGBITS` type.

But other than that, the size of `.bss` is actually very similar between i686 and rpi2b.

So, now, back to my problem.  Something is mapping address `0x80000000` or making the system think that the address is already mapped.  So, I need to re-enable all that debugging code and then take a pedantic walk through the results.

With all my debugging code turned on, I see this:

```
Initializing Modules:
kernel
   Starting Address: 0x0011d000
   FileSize = 0x0000743b; MemSize = 0x0000743b; FileOffset = 0x00001000
      Attempting to map page 0x80000000 to frame 0x0000011e
Mapping address 0x80000000 to frame 0x0000011e
  Ttl1 index is: 0x0b000000[0x00000800]
Creating a new TTL2 table for address 0x80000000
  The new frame is 0x0000b008
  The base ttl2 1K location is 0x0002c020
  The ttl1 index is 0x00000800
  Set the TTL1 table index 0x00000800 to 1K location 0x0002c020
  Set the TTL1 table index 0x00000801 to 1K location 0x0002c021
  Set the TTL1 table index 0x00000802 to 1K location 0x0002c022
  Set the TTL1 table index 0x00000803 to 1K location 0x0002c023
  The management address for this Ttl2 table is 0xffe00000
    The base location is 0xffc00000
    The table offset is  0x00200000
    The entry offset is  0x00000000
  The TTL1 management index for this address is 0x00000ffe
< Completed the table creation for 0x80000000
  Ttl2 location is: 0x0b008000[0x00000000]
Attempting to map already mapped address 0x80000000 (mapped to: 0x00000000); Fault is: 0x00000003
```

This makes me wonder if the `kMemSetB()` function is working properly.  I think I will replace it with a C version and see if I get different results.  And it works.  This tells me that my assembly language versions are not correct and need to be debugged.

---

Lots of debugging to the serial port later and I think it is not the `kMemSetB()` function but my calculations for mapping the pages that is the problem somewhere.  Or more to the point, something that is overwriting something.

---

I stopped the loader after initializing the TTL1 table with `kMemSetB()` and then investigated the memory with `gdb`.  What I saw is this:

```
(gdb) x/xw 0x0b000000
0xb000000:      0x000000ff
(gdb)
0xb000004:      0x00000000
(gdb)
0xb000008:      0x00000000
(gdb)
0xb00000c:      0x00000000
(gdb)
```

In addition, when I looked at each TTL2 table, the first byte was always 0xff.  So, my `kMemSetB()` function is somehow skipping the first byte.

And there it is!  Right in front of my face the whole time!

```
    strb    r1,[r0]                                 @@ store the value in r1 to the mem at addr r0
    add     r1,#1                                   @@ increment the address
    sub     r2,#1                                   @@ decrement the numebr of bytes
    b       kMemSetB                                @@ loop
```

I was incrementing the value to store, not the address in which to store the value!

---

This morning I am going to start researching why I have no memory available until I get way high in the frame count.  My concern is this:

```
      Attempting to map page 0x80021000 to frame 0x0000013e
      Attempting to map page 0x80022000 to frame 0x0000013f
      Attempting to map page 0x80023000 to frame 0x00000140
Checking from frame 0x00000144
      Attempting to map page 0x80024000 to frame 0x00020002
Checking from frame 0x00000144
      Attempting to map page 0x80025000 to frame 0x0002000d
Checking from frame 0x00000144
      Attempting to map page 0x80026000 to frame 0x0002000e
Checking from frame 0x00000144
      Attempting to map page 0x80027000 to frame 0x0002000f
Checking from frame 0x00000144
      Attempting to map page 0x80028000 to frame 0x00020010
Checking from frame 0x00000144
```

At this point, we should be finding frames at frame number 144, not up in about 20000.  The means my PMM initiialization has gone wrong and I will be adding code there to debug that.

I realized that the `start` for the PMMBitmap had wrapped around `0x00000000` back to high memory, so I added the following code to keep it in lower memory:

```
    if (start) {                            // -- if we are not dealing with an ebda (and therefore not x86)
        start -= PmmFrameToLinear(pages);
    }
```

This has caused the loader to lock up now.  I definitely have some problems here and will need to continue to debug for rpi2b -- and pray I don't break i686!

For rpi2b, I moved the bitmap from frame 0 to frame 1 and that worked.  It appears there is something that does not like frame 0 in some way.  I'm Ok with leaving that unusable if is helps with `NULL` pointer assignments.  And, at this point, I am getting all the proper allocations from the PMM.  But to be certain, I am going to continue to check the rest of this since I was not able to verify modules before I had a compiling kernel.

I might be writing off the end of the bitmap for the frame buffer:

```
Upper memory limit: 0x0c000000
0x00000001 pages are needed to hold the PMM Bitmap
   these pages will start at 0x00004000
PMM Bitmap cleared and ready for marking unusable space
Grub Reports available memory at 0x00000000 for 0x0000c000 frames
Marking frames 0 and 1 used
Marking the loader space as used starting at frame 0x00000100 for 0x00000018 frames.
Marking the frame buffer space as used starting at frame 0x0c100000 for 0x00000180 frames.
Marking the module kernel space as used starting at frame 0x0000011d for 0x00000029 frames.
Finally, marking the stack, hardware communication area, kernel heap, and this bitmap frames as used.
Phyiscal Memory Manager Initialized
```

I think the rpi2b does not report that memory at all.  My PMM functions do not check bounds, I a need to protect that in `PmmInit()`.  Also, I want to call out that I am starting to add debugging code `#define`s in the individual files like this:

```C
#ifndef DEBUG_PMM
#   define DEBUG_PMM 0
#endif

// --- snip ---

#if DEBUG_PMM == 1
// Some debugging output
#endif
```

This way I can turn debugging on more globally as a compile time option or in the individual files.

OK, I am now finally to the point where I can enable paging properly.  This will set the TTLB0 register and set the number of bits to 0 (meaning everything is in the one paging table and there is no user-space table).  Later in the kernel when I initialize the process structures, I will change this to 1 bit and use 2 tables.

I feel like I am relatively close to buttoning up the last of the loader.  I am not yet able to jump to the kernel so I have no clue where I am at there yet -- but I'm sure it's bad.

---

### 2018-Nov-24

Today my plan is to try to get the MMU enabled for the rpi2b archetecture.  Well, more to the point: research how to get the MMU enabled.  Since this archetecture is rather foreign to me, it takes a lot of research for me to understand what is required.

I know that I need to interact with Co-Processor 15 (`cp15`) and I will use `mcr` and `mrc` opcodes to interact with the registers.  There are several registers I will need to interact with for various reasons.
* Register 0 is a read-only register for TLB information.  I am not interested in this register for setting up the MMU.
* Register 1 contains the M bit (whether the MMU is enabled or disabled).  The M bit is bit 0.  I assume the M is set to 0 before I get to it.  I will need to code a test to investigate this.
* Register 2 contains the Translation Table Base registers and control.  There are 3 sub-registers of this:
    * 0 is base 0 -- the one will hold the user-space table mappings.  It will be the same as the kernel-space mappings for the purposes of the loader and late kernel initialization.  All other flags I will set to 0 for the moment, as I am not going to enable caching.
    * 1 is base 1, which will hold the kernel base table.  I will populate this in the loader.  All other flags I will set to 0 for the moment, as I am not going to enable caching.
    * And, 2 is the control register.  In the loader, this will be set to 1 bit, which means that if the most significant bit is 0, TTBR0 will be used and if that bit is set to 1, TTBR1 will be used.  Since they are the same, they will be the same tables.
* Register 3 is Domain Access Control, wihch I am not using.  This will be unchanged.
* Register 4 is reserved, so I will not touch it.
* Register 5 is a Fault Status Register.  It contains 2 sub-registers, neither of which I am using at this point.
* Register 6 is a Fault Address Register (which feels a little like `cr2` from x86 family).  I am not using that yet.
* Register 7 does not exist.
* Register 8 controls the TLB and has several functions.  I will leave all this at the default for now.
* Register 9 does not exist.
* Register 10 is the TLB Lockdown.  Again, I am not controlling this explicitly so I will leave this at the default.
* Register 11 does not exist.
* Register 12 does not exist.
* Register 13 contains the Process ID registers.  These need to be updated with a process change, so this will need to change with the TTBR0 register when swapping processes.  There are 2 sub-registers here, the Process ID and the Context ID.  More on these later.

So, to enable the MMU, I need to perform the following steps:
1. Check the current state of the MMU (Register 1[M]).  If it is already enabled, report so.  Should I disable before moving on?  Probably not since we will probably fault and lock up.
1. Write the TTLR0 value into Register 2/opcode 0.
1. Write the TTLR1 value into Register 2/opcode 1.
1. Write the number of bits to consider for the TTLR evaluation into Register 2/opcode 2 (this will be 1 bit).
1. Enable paging by writing a `1` to Register 1[M].
1. plan for the worst, but hope for the best.

---

I got everything written, and the the system locked up.  It's about what I figured would happen, but now I have to figure out how to debug this since I am not able to output much to the screen.

Step debugging with `gdb` gives me some results with registers:

```
(gdb) info reg
r0             0xc50079 12910713
r1             0xa      10
r2             0xa      10
r3             0xb000000        184549376
r4             0x105000 1069056
r5             0x0      0
r6             0x0      0
r7             0x0      0
r8             0x0      0
r9             0x118000 1146880
r10            0x2d227  184871
r11            0x7b4    1972
r12            0x7a0    1952
sp             0x7a8    0x7a8
lr             0x102fc8 1060808
pc             0x100e78 0x100e78 <MmuEnablePaging+24>
cpsr           0x600001d3       1610613203
fpscr          0x0      0
fpsid          0x410430f0       1090793712
fpexc          0x0      0
(gdb) stepi
^C
Thread 1 received signal SIGINT, Interrupt.
0x0000000c in ?? ()
(gdb) info reg
r0             0xc50079 12910713
r1             0xa      10
r2             0xa      10
r3             0xb000000        184549376
r4             0x105000 1069056
r5             0x0      0
r6             0x0      0
r7             0x0      0
r8             0x0      0
r9             0x118000 1146880
r10            0x2d227  184871
r11            0x7b4    1972
r12            0x7a0    1952
sp             0x0      0x0
lr             0x10     16
pc             0xc      0xc
cpsr           0x600001d7       1610613207
fpscr          0x0      0
fpsid          0x410430f0       1090793712
fpexc          0x0      0
```

First, `pc` has a crazy offset, probably dereferencing a `NULL` value.  `lr` and `sp` also have addresses that appear to be relative to `NULL`.

For the moment, I think the paging tables are built properly.  I did, however, make some assumptions about the TTBR0 and TTBR1 registers.  I will start there.

I reviewed the Domain setup and the default Domain (0b00) access is to throw faults.  So, I will try to correct that and see where that gets me.  And while it needed to be done, it did not solve all my problems.  Now I am looking at the flags for each table level.

---

I can finally confirm that I have been able to enable the MMU and I am executing code after that is complete.  However, the system is still locking up.  I was able to trace that to a stack operation which locks up the system.  The stack is broken on rpi2b.

---

### 2018-Nov-25

I took care of the stack, which had been hard-coded in the loader to be at address `0x800`.  Now I have debugged again and I have problems with the MMIO locations.  These are in user-space starting at address `0x3f...` and really need to be mapped up above `0x80000000`, but also identity mapped for the loader to finish up.

I need to find the proper address range for the MMIO locations, and determine a place to put the IO ports for the kernel.  The addresses for SoC peripherals is from `0x3f000000` to `0x3fffffff`.  I think for the rpi2b kernel, I will map this space into `0xfa000000`.

With that, I am able to get to the point where I am jumping to the kernel.  I am not sure if I make it there without a fault, but everything in the loader appears to be working properly at this point.  Certainly I am able to get the MMU enabled and continue processing, which was a big deal to accomplish.

I know I am making it to the kernel, but I am not getting any kind of greeting message from the kernel to the screen or to the serial port.  I was able to step-debug to confirm this fact.  I am going to commit this code now, since quite a bit has changed.  Then I can focus on completing the kernel for rpi2b.

---

Now, with that out of the way, the first order of business is going to be to get some output from `kprintf()`.  This is both the first thing that needs to take place and the only debugging output apparatus I have for the kernel.

The problem I am starting with is that I am now being driven into a situation where the rpi2b hardware is loader-speciific and I will have another version for the kernel for the kernel.  I do not want to duplicate this work, so I will have to build an abstraction here that works for both the loader and the kernel.  At the same time, I also found a file name duplication for `serial.h` that I need to correct.  Since the exercise is nearly the same, I will take them both on together.

For output to the serial port, I commented out the serial port greeting and was able to get the frame buffer to change colors and write the greeting.  So, I just need to figure out what is happening with the serial port output.  I know that the loader was writing to the serial port properly, but the kernel is trying to write to an adjusted address.

Silly me!  I had the same issue with `==` vs `!=` in the kernel `SerialPutChar()` that I had in the loader and I did not correct both instances.  I am now getting `kprintf()` output.

I am getting a bit of output, but the system still locks.  I know I have lots of stuff stubbed out that all need to get fixed.  The first thing that `kinit()` does is build the IDT for x86.  There is an equivalent process to get a fault handler built for rpi2b and get that installed.  This is what I will need to take on next.  I need to have visibility into the errors when they happen.  There are several things that will all need to take place to get this initialized.  There is an interrupt controller and a number of fault handlers that all need to be figured out and coded.

---

Reading up on how interrupts (exceptions) work, I have figured out that the actual `pc` value can indicate which exception has occurred if the table has not been set up.  In particular, based on the value in `pc`, it can be determined what exception has occurred:
* `0x00` -- reset
* `0x04` -- unused
* `0x08` -- supervisor call
* `0x0c` -- prefetch abort
* `0x10` -- data abort
* `0x14` -- unused
* `0x18` -- IRQ interrupt
* `0x1c` -- FIQ interrupt

These locations can only be at base `0x00000000` or `0xffff0000` and are virtual addresses.  Since we will be using upper addresses for the OS (`0x80000000` and greater), code will need to be moved and mapped to this upper location.  To make matters worse, the only thing that can be done is a single word-sized instruction, which leaves a 24-bit offet to the actual target code.  So, this code must be somewhere about `0xff800000` but no farther away.  Ultimately, I am going to need a specially built page mapped to that address (`0xffff0000`) that contains a table of addresses just a little higher in memory for jumping to the kernel proper.  This will be in the form: `ldr pc,[pc,#0x20]`.  Now, the problem is that this location is buried in the 'pseudo-recursive' MMU tables.  If it is not obviuos, I will need to rewrite the loader's MMU initialization -- and I need to take this task on now before I get into the interrupt/exception initialization.

At the same time, I need to revisit the virtual memory map as I would like to be able to align the 32-bit architectures better than I have at this point.  I have created discrepancies in the memory map from above and the one in README.md.  The target on README.md takes precedence, but it will take some time to refactor the x86 architecture to properly align.  I indend this to be completed in v0.2.1 and will create the Redmine version now to accomplish this.

---

After making several changes to create constants for the hard-coded values, I was able to change the location of the TTL1/TTL2 tables to `0x80400000` and `0x80000000` respectively.

This is a change since the TTL1 tables now come *after* the TTL2 tables whereas this was originally programmed to be *before*.  However, a test shows that this still works by only changing the constant values.  This is a good thing.  I need to do more of this!  This now will free up the memory required to set up the interrupt table.

I want to call out here that I believe I will be coding something that makes no sense and need to call it out.  The ARM ARM section 5.2.2 calls out that if `r15` or `pc` is used as the base register for loading an address, the actual value used in `pc + 8`, or "the address of the instruction plus 8".

---

### 2018-Nov-26

My UPS just ate itself.  It took a while to reboot everything, but so far it has all come back up properly.

Now, to get back to the interrupt table.  I need to determine if I am going to build this statically and relocate it or build it dynamically in the proper location.  Each of the `ldr` instructions would be the same, so it would not be too much of a stretch to build it during initialization.  I would just need to determine the actual instruction bits.  This should be able to be done with a simple `.s` file.

This quick file gave me the results I was looking for:

```
intReset:
    ldr     pc,[pc,#(8-8)]
intAbort:
    ldr     pc,[pc,#(8-8)]

resetTarget:
    .word   kInit
    .word   kInit
```

That is to say the `ldr` opcodes were loading the proper addresses.

```
80005f0c <intReset>:
80005f0c:	e59ff000 	ldr	pc, [pc]	; 80005f14 <resetTarget>

80005f10 <intAbort>:
80005f10:	e59ff000 	ldr	pc, [pc]	; 80005f18 <resetTarget+0x4>

80005f14 <resetTarget>:
80005f14:	800056a0 	andhi	r5, r0, r0, lsr #13
80005f18:	800056a0 	andhi	r5, r0, r0, lsr #13
```

It just so happens that `#(8-8)` is what I was playing with for this test, but the real structure will be something like `#(0x20-8)`.  Or, much more to the point: `0xe59ff018`.

---

I wrote functions to handle the MMU mappings.  The problem with them at this point is that I am only dealing with the kernel space at the moment -- not the user space mappings from `0x00000000` to `0x7fffffff`.  Well, they are all put into the kernel space maps.

I was also be able to write and document the `InterruptVector_t` structure.

---

### 2018-Nov-27

I am short on time today, so the task today is to determine how to relocate the interrupt vector table from `0x00000000` to `0xffff0000`.  This has something to do with a control register, and I presume `cp15` before I even start reading.  But, that is the research task of the day.  Sorry, it's called an "exception vector table".  I have been writing it wrong.

I see that the SCTLR register, bit 13 controls the Vector Base Address Register value.  The SCTLR is cp15,c1.  When this bit is set to 0, the address is `0x00000000` and software can remap the location using the VBAR.  On the other hand, when this value is 1, the address is `0xffff0000` and software cannot remap this value.  Hmmm...  am I missing something?

It looks like I can read the VBAR register with the following opcode:

```
mrc p15,0,r0,c12,c0,0
```

writing this register back out with `mcr` should allow me to set a fixed address for the exception vector table.

So, I asked this question on `freenode#osdev` and got the following response:

```
[19:55] <eryjus> For those who know the RPi, am I reading the documentation right for the exception vector table (VBAR)?  If I leave p15:c1:13 clear, I can set p15:c12 to any address I want and I am not limited to 0x0 and 0xffff0000?
[19:55] <geist> eryjus: yes
```

This means that I am actually able to move the TTL tables back to `0xffc00000` (less 4 frames) and I can put the exception vector table in the same location as the IDT for x86.  This will allow me to move the heap as well back to `0x80000000` and the MMIO addresses to the top of the kernel data location.  This far better aligns with much of the existing x86 architecture and I will not have to perform major surgery to the x86 code to get things aligned properly.

So, with that said, tomorrow I will make the attempt to write to those registers to see if I can get the addresses set proeprly.

---

### 2018-Nov-28

Going back to setting this exception vectory table address, the base virtual address of the GDT/TSS/IDT in the x86 family is `0xff401000`.  All 3 structures are located in this same frame in x86.  I want to use that same address for the exception vector table for rpi2b.  So this is what I will attempt to set up and then create a fault to test it.

Well, when I set the registers, there was no error.  I then set up for a test to generate a fault by reading a bad address: `x = *(uint32_t *)0x89723984;`, and this generates a fault, but the address I want to see in the `pc` register is not correct.

My function to set this up looks like this:

```
IdtSetAddr:
    mrc     p15,0,r0,c1,c0,0
    and     r0,r0,#(~(1<<13))
    mrc     p15,0,r0,c1,c0,0

    mov     r0,#0xff40
    lsl     r0,#12
    add     r0,#0x1000
    mrc     p15,0,r0,c12,c0,0

    mov     pc,lr
```

Of course, I confused the `mrc` and `mcr` opcodes.  Correcting those solved my issue.

```
(gdb) info reg
r0             0xff41000        267653120
r1             0xa      10
r2             0xa      10
r3             0x89723984       -1989002876
r4             0x80009000       -2147446784
r5             0x0      0
r6             0x0      0
r7             0x0      0
r8             0x0      0
r9             0x11a000 1155072
r10            0x2d227  184871
r11            0x80024fc4       -2147332156
r12            0x118f98 1150872
sp             0x0      0x0
lr             0xff41010        267653136
pc             0xff4100c        0xff4100c
cpsr           0x600001d7       1610613207
fpscr          0x0      0
fpsid          0x410430f0       1090793712
fpexc          0x0      0
```

---

I found an endian-ness problem with my hard-coded jump instruction.  I got that fixed and now the faulting `lr` is `0xff042004`.  This means I managed to execute something, but not sure how much and what took place.

---

It's getting late and I'm not getting anywhere.  I'm going to call it a night and try again tomorrow.

---

### 2018-Nov-29

I have been thinking about tooling.  I am wondering if there are any better (free) ARM emulators.

After asking on `freenode#osdev`, I now have access to the debugger (`-monitor stdio`) and I found that I have an endian-ness problem.  I switched it back.  The behavior is the same, but I get farther down the code before I run into problems.  Here is the output from the qemu logs:

```
0x80003060:  e8bd8830  pop      {r4, r5, fp, pc}

Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x9600003f
...with DFSR 0x5 DFAR 0x89723984
----------------
IN:
0xff401010:  e59ff018  ldr      pc, [pc, #0x18]

----------------
IN:
0x80002c54:  e92d4800  push     {fp, lr}
0x80002c58:  e28db004  add      fp, sp, #4
0x80002c5c:  e59f300c  ldr      r3, [pc, #0xc]
0x80002c60:  e08f3003  add      r3, pc, r3
0x80002c64:  e1a00003  mov      r0, r3
0x80002c68:  eb000e45  bl       #2147509636

Taking exception 4 [Data Abort]
...from EL1 to EL1
...with ESR 0x25/0x9600007f
...with DFSR 0x805 DFAR 0xfffffff8
```

This tells me that the Exception Vector Table worked and my Data fault handler really did get control.  However, there appears to be no stack set up.  I have more reading to do, but no time tonight.  But I do think the best thing is to take on getting a proper register dump written so I can debug better.

The Stack Pointer (`r13`) is indeed clear.  I need to determine why.

---

### 2018-Nov-30

I started searching last night and found it today.  The stack register (`sp` or `r13`) is replaced on an abort.  This table of banked registers is important to keep in mind -- it is in the ARM ARM on page A2-5 and is Figure A2-1.

In trying to get access to the `cspr` and `sp` from the exception, I realize that I am in supervisor mode and probably want to be in system mode.  The reason for this is that I will want access to the registers from the interrupt.  Since the registers are banked, each mode has its own register.  When I am handling an interrupt, I probably want to switch into supervisor (svc) mode.

The following link provides an example:  https://github.com/littlekernel/lk/blob/master/arch/arm/arm/exceptions.S#L75.

I am able to get a dump of the registers, but I cannot guarantee I have all the right registers.  I need to continue to work with my code and the instructions with the example above.

---

### 2018-Dec-01

At the moment, I am changing into system mode as soon as the kernel gets control.  That is a bit too early and I really want to be in svc mode for most of the initialization, changing to system mode right at the end of iniitializaiton, before assuming the butler role.

I also do not have a ton of time today, so all I am able to get done is to stub out the rest of the exception handlers to that they all print registers and lock the system to review.

Christmas party for about 100 people today....

---

### 2018-Dec-02

This morning, I am calling my `IdtBuild()` function complete.  Well, mostly.  There will be some changes as I get other things up and running (the timer is a prime example).  But I am not going to take those changes on now.

Also, there is no TSS to initialize with rpi2b, so that is done.  The rest of phase 1 initialization is related to greeting the user and updating the frame buffer.  Those are done as well.  So, I can call phase 1 initialization complete.

The next thing up is to initialize the process structures for rpi2b.  I expect that there will be some changes needed for the architecture -- in particular the things that need to be saved on the stack.

I commited this code.

---

My current i686 process structure looks like this:

```C
//
// -- This is a process structure
//    ---------------------------
typedef struct Process_t {
    regval_t esp;                       // This is the process current esp value (when not executing)
    regval_t ss;                        // This is the process ss value
    regval_t cr3;                       // This is the process cr3 value
    PID_t pid;                          // This is the PID of this process
    ptrsize_t ssAddr;                   // This is the address of the process stack
    size_t ssLength;                    // This is the length of the process stack
    char command[MAX_CMD_LEN];          // The identifying command, includes the terminating null
    size_t totalQuantum;                // This is the total quantum used by this process
    ProcStatus_t status;                // This is the process status
    ProcPriority_t priority;            // This is the process priority
    int quantumLeft;                    // This is the quantum remaining for the process (may be more than priority)
    bool isHeld;                        // Is this process held and on the Held list?
    struct Spinlock_t lock;             // This is the lock needed to read/change values
    ListHead_t::List_t stsQueue;        // This is the location on the current status queue
    ListHead_t lockList;                // This is the list of currently held locks (for the butler to release)
    ListHead_t messages;                // This is the queue of messages for this process
    void *prevPayload;                  // This is the previous payload in case the process did not allocate enough
} __attribute__((packed)) Process_t;
```

The first 3 fields in this structure are the specific to the x86 architecture.  They have little to no bearing on the rpi2b architecture.  The `esp` could be renamed to the `stackPointer` and then it would be relevant.  `cr3` could be renamed to `pageTables` and this would allow me to store the `TTBR0` value.  `ss` is totally irrelevant.  But I also can't help thinking that there are additional fields I need for the rpi2b.  I remember that there is a "process ID" register (or something like it) that needs to get updated at the same time as the `TTBR0`, and I will need to store that somewhere as well.

I'm going to start by renaming those 2 fields and comiling to address the fall-out from that change.  Note, I am not changing the position of any field.  Just the name.

---

It appears (though I am not totally convinced) the the Context ID registers in `cp15` are related to the Fast Context Switch Extension (FCSE) and are optional to what I am going to be doing.  So, I should not really need to save any other registers in the `Process_t` type.  It was an easy task to get the `ProcessInit()` function written.  It merely establishes the structures but I do not yet have enough of the system built to actually exercise these structures and work out any issues.  I think I am going to have to make an effort to organize the pushes properly to make sure I have the `cpsr` on the stack and room to get this value in the rgisters.  I will have to take this on once I get the Timer initialized.

Now, with that said, I have no clue what the Timer looks like in rpi2b.

---

I now know I need to read the CBAR to determine the base address register.  This is located in `cp15` in register `c15`.  Then, at offset `0x1000` I can then get access to the GIC.  Once I have the GIC, I can use that to configure the interrupts from the timer.

---

### 2018-Dec-03

Well, this is the last day before I take vacation.  I am not sure what I will be able to do while I am on vacation since my wife is one of those that measures the success of a vacation based on how tired we are when we get back....

For the short time I have, I will be looking at the Generic Interrupt Controller (GIC).  I think the first thing to do is to confirm I have a proper GIC.  This should be able to be done by reading `(HW_BASE + 0x1000 + 0x008)` or the `GICD_IIDR`.  The value in this read-only register should be `0x0100143b`.

---

Hmmm....  I'm not finding the GIC...  I notice the following the the TRM:

> Memory regions used for these registers must be marked as Device or Strongly-ordered in the translation tables.

> Memory regions marked as Normal Memory cannot access any of the GIC registers, instead access caches or external memory as required.

This means I will need to update my MMU initialization to set this to be device memory.  I have 2 unused parameters that I can leverage for this, so it should be a simple update to make this change.

---

So reading further, it appears that the BCM2836 SoC does not have a GIC included with it.  Well, that makes a mess.  I need to go back through and figure out how I will be handling interrupts.

---

### 2018-Dec-04

Not going to be able to do much tonight.  However, I am going to try to work on getting the timer working and firing interrupts.  I really do not expect too much in the way of accomplishments.

Another thing I might work on is the initialization sequence in the loader, which is not really 100% correct.

---

## 2018-Dec-05

This morning I was able to narrow this down to a timer problem, not an interrupt problem.  I did this by printing the polled status of the interrupt pending register.  I will start with that assumption and clean that up first.

---

I was able to finally get an interrupt.  I was able to get the interrupt by enabling every interrupt possible:

```C
    MmioWrite(INT_IRQENB0, 0xffffffff);
    MmioWrite(INT_IRQENB1, 0xffffffff);
    MmioWrite(INT_IRQENB2, 0xffffffff);
```

From here I am going to use the process of elimination to determine which interrupt it is.

I find that the bit that enables the IRQ is in (INT_IRQENB2|1<<25), or INTENB2IRQ53.  This is i2c_int.

Well, I have the interrupt firing....  Now, I want to make sure I get the registers processes properly so that I can output them properly.

Comparing the output of the qemu logs to that of what is output to the serial port, the qemu logs show:

```
IN:
0x80002d58:  f10e01d3  cpsid    aif, #0x13

R00=60000013 R01=02000000 R02=02000000 R03=f200b214
R04=8000a000 R05=00000000 R06=00000000 R07=00000000
R08=00000000 R09=0011b000 R10=0002d227 R11=80028fcc
R12=00119f98 R13=00000000 R14=8000017c R15=80002d58
PSR=60000192 -ZC- A NS irq32
```

... and the serial logs show:

```
IRQ:
 R0: 0x60000013   R1: 0x02000000   R2: 0x02000000
 R3: 0xf200b214   R4: 0x8000a000   R5: 0x00000000
 R6: 0x00000000   R7: 0x00000000   R8: 0x00000000
 R9: 0x0011b000  R10: 0x0002d227  R11: 0x80028fcc
R12: 0x00119f98   SP: 0x00000000   LR: 0x80028fb4
CPSR: 0x00000000
```

This shows that only the `lr` and `cpsr` registers are incorrect, and the `pc` register is not captured.

---

There are a few things I need to make sure I get in line so that I am coding properly.  For the record, I am referring to this example code to make sure I get things in line: https://github.com/littlekernel/lk/blob/master/arch/arm/arm/exceptions.S.

From this data, I am compiling a table of the what is located where so that I can build the stack properly going in and restore the system state properly departing.

| Position |   FIQ    |   IRQ    | Data Abort | Pref Abort |   SVC    |   UND    |
|:--------:|:--------:|:--------:|:----------:|:----------:|:--------:|:--------:|
|   +4c    | spsr_fiq | spsr_irq |  spsr_dta  |  spsr_pre  | spsr_svc | spsr_und |
|   +48    | lr_fiq-4 | lr_irq-4 |  lr_dta-8  |  lr_pre-4  |  lr_svc  |  lr_und  |
|   +44    | fiq_type | irq_type |  dta_type  |  pre_type  | svc_type | und_type |
|   +40    |    r0    |    r0    |     r0     |     r0     |    r0    |    r0    |
|   +3c    |    r1    |    r1    |     r1     |     r1     |    r1    |    r1    |
|   +38    |    r2    |    r2    |     r2     |     r2     |    r2    |    r2    |
|   +34    |    r3    |    r3    |     r3     |     r3     |    r3    |    r3    |
|   +30    |    r4    |    r4    |     r4     |     r4     |    r4    |    r4    |
|   +2c    |    r5    |    r5    |     r5     |     r5     |    r5    |    r5    |
|   +28    |    r6    |    r6    |     r6     |     r6     |    r6    |    r6    |
|   +24    |    r7    |    r7    |     r7     |     r7     |    r7    |    r7    |
|   +20    |    r8^   |    r8    |     r8     |     r8     |    r8    |    r8    |
|   +1c    |    r9^   |    r9    |     r9     |     r9     |    r9    |    r9    |
|   +18    |   r10^   |   r10    |    r10     |    r10     |   r10    |   r10    |
|   +14    |   r11^   |   r11    |    r11     |    r11     |   r11    |   r11    |
|   +10    |   r12^   |   r12    |    r12     |    r12     |   r12    |   r12    |
|    +c    |    sp^   |    sp    |     sp     |     sp     |    sp    |    sp    |
|    +8    |  lr_svc  |  lr_svc  |   lr_svc   |   lr_svc   |  lr_svc  |  lr_svc  |
|    +4    |  sp_usr  |  sp_usr  |   sp_usr   |   sp_usr   |  sp_usr  |  sp_usr  |
|    r0    |  lr_usr  |  lr_usr  |   lr_usr   |   lr_usr   |  lr_usr  |  lr_usr  |

Given time tomorrow, I will start to commit this to code.  I will need both the entry and the exit coded as the goal is to be able to have the timer IRQ fire and then be able to continue processing.

---

### 2018-Dec-06

Well, I was able to get the interrupt to fire repeatedly and dump the contents of the registers to the serial port with each iteration.  I am not totally convinced that the contents are 100% corect, but it works for now.  I need to copy the logic to the other exception handlers, but the core logic works.  It's also important to note that the above table is not 100% accurate already.

---

Well, it looks like the exception handlers are degrading into several data exceptions.  So, I will need to get to the bottom of this -- I may not have had that as well dialed in as I thought.  Well, it turns out I was not setting r0 to be the address of the registers on the stack, a simple fix.

I have built out the IRQ dispatch table for the rpi2b.  I need to register a timer callback function and then I should be able to get the scheduler started.

---

I have added the `TimerCallback()` function to the `IsrHandler` table and ended up with a Data Fault, with the following data:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
The CBAR reports the base address as 0x3f000000
Setting the scalar value to 0x83126e for frequency 0xfa
Data Exception:
At address: 0x8002af3c
 R0: 0xfffffe3f   R1: 0xfffffe3f   R2: 0x00000035
 R3: 0x80041024   R4: 0x8000a000   R5: 0x00000000
 R6: 0x00000000   R7: 0x00000000   R8: 0x00000000
 R9: 0x0011b000  R10: 0x0002d227  R11: 0x8002afac
R12: 0x00119f98   SP: 0x800036ec   LR_ret: 0x8000678c
SPSR_ret: 0xf800011f     type: 0x17

Additional Data Points:
User LR: 0x00000000  User SP: 0x00000000
Svc LR: 0x8002af4c
```

So, I am back to debugging.  Starting with the return `lr`, the function is `RestoreInterrupts()`.  So far, the only place I think this is called in when registering an ISR handler.  I want to make sure I am getting through that function properly.  The fault is generated from `RestoreInterrupts()`, so I need to clean that up.

---

### 2018-Dec-09

My IRQ handler appears to be getting multiple interrupts at once, or I am not really clearing out the interrupt.  Tracing the interrupts that are left over, I have something from register 2 (bit 9) and something from the uart (bit 19).  However, what is clear is that I need to ensure that I am comparing one bit at a time rather than expecting only a single bit to be set.  I also believe I am having trouble with actually acknowledging an interrupt (or in x86 work issuing an End of Ineterrupt).

The first order of business is to change `IsrHandler()` from being driven by a `switch` statment to a number of `if-then-else` statements.

---

At the moment my timer has stopped firing, and the uart is firing with every character written, even though I think I have asked it not to and even masked it out.

---

### 2018-Dec-10

I am still trying to get the timer working.  I am still on vacation and working remotely on this.  I fly home tomorrow and will get into some really hard debugging when I return.  However, in the meantime, I am working on trying to identify the proper location for this timer in the qemu source code.

---

### 2018-Dec-11

Well, it appears that I have been reading the incorrect documentation.  I need to be looking at the BCM2836 SoC and instead I have been reading the BCM2835 SoC docs.

So, with that said, I will need to revisit the `TimerInit()` function to make sure I am using the correct locations to manage the timer.

---

And after trying again with the new (proper?) register locations, I am still not getting anywhere.  I need to sleep on this again.

---

### 2018-Dec-12

Well, still no success here.  I did reach out on `#osdev` for some help....

With no quick reply, I am going back to basics.  I believe that I am working without a GIC.  I need to confirm this first.

OK, so I read and updated what I thought was the GIC Distributor enable bit and read back the results.  This came back as `0x00`.  So, this tells me I am updating the wrong address or there is no GIC.  I still need to research a bit more.  Instead, I will read the `GICD_IIDR` register and report its results and is a read only register.

So, the CBAR (Configuration Base Address Register) holds the location of the GIC (`0x3f000000`), and the Distributor is located at offset `0x1000` (`0x3f001000`), I can conclude that I am reading the correct location (this is identity mapped still).  So, I am able to conclude that I do not have a GIC in this system, which is what I thought.

So, in the BCM2836 documentation, there is a timer prescalar which has a reset value of 0.  When that is 0 there is in effect a divide-by-zero situation that can occur.  I am not sure how the CPU handles this condition, but the reset value is reported as 0.  So, I will try setting a value for this register.  I wrote 1, which made no difference in behavior.

---

Reviewing the qemu source, I see the following code for initializing the `raspi2` machine (which I am using):

```C
        /* Connect timers from the CPU to the interrupt controller */
        qdev_connect_gpio_out(DEVICE(&s->cpus[n]), GTIMER_PHYS,
                qdev_get_gpio_in_named(DEVICE(&s->control), "cntpnsirq", n));
        qdev_connect_gpio_out(DEVICE(&s->cpus[n]), GTIMER_VIRT,
                qdev_get_gpio_in_named(DEVICE(&s->control), "cntvirq", n));
        qdev_connect_gpio_out(DEVICE(&s->cpus[n]), GTIMER_HYP,
                qdev_get_gpio_in_named(DEVICE(&s->control), "cnthpirq", n));
        qdev_connect_gpio_out(DEVICE(&s->cpus[n]), GTIMER_SEC,
                qdev_get_gpio_in_named(DEVICE(&s->control), "cntpsirq", n));
```

This is happening for each core.  Therefore, I looked to see what the `cntpsirq` meant.  The ARM TRM states:

> Secure physical timer event

... while the `cntpnsirq` is the non-secure version.  So, I think I need to set up to use this timer -- at least for qemu.

---

So, I wrote the following debug code and have some results from it:

```C
    // -- this results on no change
    kprintf("Set the control register from %p\n", MmioRead(TMR_BASE + 0x00));
    MmioWrite(TMR_BASE + 0x00, 0x00);

    // -- I do not believe the documetation is correct on this
    kprintf("Set the prescalar register from %p\n", MmioRead(TMR_BASE + 0x08));
    MmioWrite(TMR_BASE + 0x08, 0x80000000);

    // -- Shows as 0 (unimplemented?)
    kprintf("The core timer is currently %p : %p\n", MmioRead(TMR_BASE + 0x20), MmioRead(TMR_BASE + 0x1c));

    // -- Set the GPU routing flags -- results in no change
    kprintf("Set the GPU Routing from %p\n", MmioRead(TMR_BASE + 0x0c));
    MmioWrite(TMR_BASE + 0x0c, 0x00);

    // -- Here we set the routing for the timer
    kprintf("Set the timer routing from %p\n", MmioRead(TMR_BASE + 0x40));
    MmioWrite(TMR_BASE + 0x40, 0x00000020);

    // -- Additional routing for the timer -- results on no change
    kprintf("Route the local timer to core 0/IRQ changed from %p\n", MmioRead(TMR_BASE + 0x24));
    MmioWrite(TMR_BASE + 0x24, 0x00);

    // -- Set the core interrupt sources
    kprintf("Set the core 0 interrupt sources from %p\n", MmioRead(TMR_BASE + 0x60));
    MmioWrite(TMR_BASE + 0x60, 0x00000020);

    // -- Enable the timer and interrupt (reload value of 0x100)
    kprintf("Enable the timer/interrupt from %p\n", MmioRead(TMR_BASE + 0x34));
    MmioWrite(TMR_BASE + 0x34, 0x30000100);

    // -- Reload and reset timer
    kprintf("Reload and reset timer from %p\n", MmioRead(TMR_BASE + 0x38));
    MmioWrite(TMR_BASE + 0x38, 0xc0000000);


    kprintf("The core timer is currently %p : %p\n", MmioRead(TMR_BASE + 0x20), MmioRead(TMR_BASE + 0x1c));
```

... resulting in

```
Set the control register from 0x00000000
Set the prescalar register from 0x00000000
The core timer is currently 0x00000000 : 0x00000000
Set the GPU Routing from 0x00000000
Set the timer routing from 0x00000000
Route the local timer to core 0/IRQ changed from 0x00000000
Set the core 0 interrupt sources from 0x00000000
Enable the timer/interrupt from 0x00000000
Reload and reset timer from 0x00000000
The core timer is currently 0x00000000 : 0x00000000
```

One thing I notice from this is that all the registers are starting at 0.  This may or may not be a bad thing, but I need to check that my updates are being maintained.

Checking my updates results in the following:

```
Set the control register from 0x00000000
Set the prescalar register from 0x00000000
... checking the update: 0x00000000
The core timer is currently 0x00000000 : 0x00000000
Set the GPU Routing from 0x00000000
Set the timer routing from 0x00000000
... checking the update: 0x00000020
Route the local timer to core 0/IRQ changed from 0x00000000
Set the core 0 interrupt sources from 0x00000000
... checking the update: 0x00000000
Enable the timer/interrupt from 0x00000000
... checking the update: 0x00000000
Reload and reset timer from 0x00000000
The core timer is currently 0x00000000 : 0x00000000
```

The only thing that is properly updated is the 'Core0 Interrupt Source` register.  The rest appear to be unimplemented by qemu.

---

### 2018-Dec-13

OK, I have some progress today!  Not sure if I can turn that into results yet, but I have figured out where I should be looking.  I think.

I started looking into the Raspbian source.  I found 2 files that are directly linked to the BCM2836 chip and more to the point to the timer I am trying to get running.  These are:
* https://github.com/raspberrypi/linux/blob/3b01f059d2ef9e48aca5174fc7f3b5c40fe2488c/include/linux/irqchip/irq-bcm2836.h
* https://github.com/raspberrypi/linux/blob/3b01f059d2ef9e48aca5174fc7f3b5c40fe2488c/drivers/irqchip/irq-bcm2836.c

Both of these files together demonstrate that that I am looking in the correct spot for implementing this timer stuff.  So, I started wondering about qemu and if it is supported or not.  Well the file `hw/intc/bcm2836_control.c` shows that it is implemented.

So, this is a per-core timer and I just need to set it up properly to get the results I am looking for....  Easy, right???!!

The first thing that Raspbian does is initialize the timer frequency.  This is to register LOCAL_PRESCALAR or offset `0x008`.  However, there does not appear to be a relevant register in the qemu emulation.  I will want to do this on real hardware, but the subsequent read will not return anything of value.  This agrees with my results from yesterday.

Further on the setting up of the frequency, the calculation as documented in the Raspbian code is: `timer_freq = input_freq * (2 ^ 31) / prescalar`.  The `input_freq` is 19.2MHz.  Raspbian sets this to a 1:1 against the crystal clock, resulting in 19.2 MHz, and an increment of 1.  I believe (but have not confirmed) that the qemu frequency is set to the emulation loop frequency only.

After that, Raspbian then goes through the trouble to register the IRQs for each of the 4 timer events: Hypervisor physical timer event, Non-secure physical timer event, Secure physical timer event, and Virtual timer event... plus the GPU Fast IRQ and PMU Fast IRQ (which I am not dealing with at the moment).  I do not believe this really does much with the hardware but instead registers handlers in the internal kernel structures.  At which point the initialization is complete.

Now, there are 2 functions `bcm2836_arm_irqchip_mask_per_cpu_irq()` and `bcm2836_arm_irqchip_unmask_per_cpu_irq()` which do some bit twiddling for the `LOCAL_TIMER_INT_CONTROL0` register.  But the point here is that there is really nothing fancy going on here.  This equates to the routing for the timer at offset `0x40` which is implemented in qemu and is able to be read back properly (as I saw yesterday).

---

So, I am wondering if I don't have some other problem like a permissions thing with the paging tables.  I may have to dig a bit deeper into the qemu code to see what might be preventing this from firing.  I might even have to write some debugging output into qemu and recompile it.

---

It dawns on me that the paging can be eliminated as a cause for problems by writing a special test into the loader -- before paging is enabled.  In this test, I would need to set up the irq table and duplicate some of that setup.  This would all be temporary of course.  Or, better yet, maybe I need a special purpose program that will exercise the timer.  I will sleep on this thought and pick this back up in the morning.

---

### 2018-Dec-14

I thought a lot about this last night and today during my day job.  If I want do to bare metal, I will need to actually start up a new archetecture that bypasses `rpi-boot`.  However, I think my problems lie in the MMU layer.  If this is the case, all I need to do is create a new module that is rpi2b only and create a binary for that module.  I can add that to the `grub.cfg` file and then boot to that module easy enough for testing.  I will start with the approach so that I can try to get a working timer.  Now, how will I know if I do?  I do not have an act led I can flash in QEMU and I am completely unprepared for real hardware at this point.

I have routines that I can copy from the loader to output to the serial port.  This will also generate IRQs so that I can check that the IRQs are working properly as well.  With this, I think there is enough to set a basic plan for working with hardware without the MMU getting in the way.

A couple of things for the record:
* I will be committing this module as well
* `rpi-timer` will be the module's name

---

My first obstacle is that the `rpi-boot` tools does not support menuing.  Therefore, I cannot have this selected by menu option at boot time.  I could hack `rpi-boot` to allow this, but not today.  The alternative is to make changes in `bin/rpi2b/boot/grub/Tupfile` to allow the alternative and for now comment and uncomment these steps.

---

Ok, my first attempt to get the timer to fire has not worked.  I really did not expect it to, but I was hopeful.  The code is very simple, with most of it related to setting up the serial port for debugging.  The first thing I want to do for debugging is to set up the UART to generate interrupts so that I can test whether my IVT is configured right.

This test yielded no interrupts.  So, I can only conclude I have a problem with my IVT setup somehow.

So, I am getting an IRQ.

```
Taking exception 5 [IRQ]
...from EL1 to EL1
...with ESR 0x13/0x4e000000
----------------
IN:
0x00000018:  e59ff018  ldr      pc, [pc, #0x18]
```

This is good.  But then right after this:

```
----------------
IN:
0xe24ee004:  00000000  andeq    r0, r0, r0

----------------
IN:
0xe24ee008:  00000000  andeq    r0, r0, r0

----------------
IN:
0xe24ee00c:  00000000  andeq    r0, r0, r0
```

The code runs off to nowhere and executes a bunch of `0x00000000` values!  What I need now is a .map of my `rpi-timer.elf` code.  Well, I had it, created automatically.

---

The problem appears to have been in the packing of the IVT.  Adding that (and one other change to types) cleared up the problem and I am getting IRQs.  Now, with the serial port generating IRQs and the timer (possibly) generating IRQs, I need to start filtering this down so that I know where everything is coming from.  First, I know where the UART IRQ is coming from, so I will start by disabling that right after enabling it.

It looks like I am really getting a timer IRQ.

Here is the code that is enabling the IRQs:

```C
    // -- Enable all IRQs
    MmioWrite(INT_IRQENB0, 0xffffffff);
    MmioWrite(INT_IRQENB1, 0xffffffff);
    MmioWrite(INT_IRQENB2, 0xffffffff);
    MmioWrite(INT_IRQDIS2, 0x02000000);             // immediately disable the serial IRQ
```

So, let's narrow down which one really needs to be enabled.  Starting by eliminating `INT_IRQENB2`.  And eliminating that stopped my interrupts.  But I am still getting all my debugging output:

```
Serial port initialized!
Interrupt Vector Table is set up
Timer is initialized -- interrupts should be happening
```

So, the interrupt I am looking for is in `INT_IRQENB2`.  I believe that this coming from IRQ54, based on some previous experiments.  I'm going to check that first.  But that does not enable those interrupts.  Double checking my previous test, though, I confirmed that the interrupt is coming from `INT_IRQENB2` somewhere (also meaning there is no point in investigating `INT_IRQENB0` or `INT_IRQENB1` any further).

Now on the other hand, I am not getting the final message written, so I may not be actually getting to the end of the code and really really getting a timer to fire.  This could really be recursive UART interrupts.  The solution to this is to figure out and filter out any UART interrupts and not perform a `SerialPutChar()` function for a UART interrupt.

Yes, as a matter of fact, when I filter out the IRQ for the UART, I was only getting a false sense of security on the timer interrupt.

Well, hang on!  My debugging output is still missing the last line:

```
Serial port initialized!
Interrupt Vector Table is set up
```

Could it be that I am just burying the qemu emulator in interrupts and it cannot get to the end point??  I can test this by changing the scalar value to something very low, say 0x100 and should be able to at least see the final line of code.  No, that's not the answer.  I need to go back to look at the UART IRQ.  I need to disable that again.  That got me to the end.  I believe that my problem is that I am not resetting that interrupt request, meaning that I am caught in a recursive interrupt -- the interrupt is handled but as soon as interrupts are enabled again there is another one to handle because there has not been a reset.  I will focus on getting that addressed.

I finally had to clear the interrupt pending flags in `UART_BASE + UART_ICR` to get the flooding to stop.  I am now getting to the end of my debugging output again, but I still have a lower presacalar value.  The higher value has no impact.

So, this means I am no longer getting the UART IRQs and I am not getting any timer interrupts...  even though everything is enabled.

Let me think this through (sticking my registers to core 0 only and typing all the addresses as offsets from `0x40000000`):
* `0x40` is routes the individual timers to the proper IRQ or FIQ for each of the 4 timer classes.  I am setting this in my code.
* `0x60` is the IRQ source for the core.  This is not being set in my code, and I believe it should be.
* `0x34` enables the timer and it is not being set in my code.  It probably should be.
* `0x24` sets up the local timer routing to IRQ/FIQ for which core.  The default is to route to IRQ on core 0 which is what I want.  I do not believe there is a need to set this register.

So, I have 2 registers I can try to set up.  I wil take them in turn, starting with `0x34`.  This did not work, so I will disable this line and try register `0x60`.  This did not work either, so now to try them both.  And that did not work either.

---

OK, so with this advice from `#osdev`:

```
[19:17] <mrvn> have you tried using an actual RPi2?
[19:17] <mrvn> make sure your code works before trying to fiddle with the emulator.
```

I am now trying to get a working bootable NOOBS image for my real hardware.  The problem here is what is in the root directory and what to replace with my code.  A directory listing shows this:

```
[adam@adamlt usb]$ ls
total 34782
drwxr-xr-x. 5 root root    16384 Dec 14 20:16 .
drwxr-xr-x. 4 root root     4096 Dec 14 20:05 ..
-rwxr-xr-x. 1 root root    23315 Nov 13 09:09 bcm2708-rpi-0-w.dtb
-rwxr-xr-x. 1 root root    22812 Nov 13 09:09 bcm2708-rpi-b.dtb
-rwxr-xr-x. 1 root root    23071 Nov 13 09:09 bcm2708-rpi-b-plus.dtb
-rwxr-xr-x. 1 root root    22589 Nov 13 09:09 bcm2708-rpi-cm.dtb
-rwxr-xr-x. 1 root root    24115 Nov 13 09:09 bcm2709-rpi-2-b.dtb
-rwxr-xr-x. 1 root root    25311 Nov 13 09:09 bcm2710-rpi-3-b.dtb
-rwxr-xr-x. 1 root root    25574 Nov 13 09:09 bcm2710-rpi-3-b-plus.dtb
-rwxr-xr-x. 1 root root    24087 Nov 13 09:09 bcm2710-rpi-cm3.dtb
-rwxr-xr-x. 1 root root    52116 Nov 13 09:09 bootcode.bin
-rwxr-xr-x. 1 root root      303 Nov 13 09:09 BUILD-DATA
drwxr-xr-x. 3 root root     2048 Nov 13 09:09 defaults
-rwxr-xr-x. 1 root root     2356 Nov 13 09:09 INSTRUCTIONS-README.txt
drwxr-xr-x. 2 root root     2048 Nov 13 09:09 os
drwxr-xr-x. 2 root root    12288 Nov 13 09:08 overlays
-rwxr-xr-x. 1 root root  3060512 Nov 13 09:09 recovery7.img.sav
-rwxr-xr-x. 1 root root       85 Dec 31  1979 recovery.cmdline
-rwxr-xr-x. 1 root root   677988 Nov 13 09:09 recovery.elf
-rwxr-xr-x. 1 root root        0 Nov 13 09:09 RECOVERY_FILES_DO_NOT_EDIT
-rwxr-xr-x. 1 root root  2995120 Nov 13 09:09 recovery.img
-rwxr-xr-x. 1 root root 28569600 Nov 13 09:09 recovery.rfs
-rwxr-xr-x. 1 root root     9728 Nov 13 09:09 riscos-boot.bin
```

Notice I have renamed the `recovery7.img` file to `recovery7.img.sav`.  NOOBS no longer boots, so I should be able to replace the `recovery7.img` file with my own version of that and be able to test.

---

OK, I'm wrapping up for the night.  I was able to find several samples from an old git repo (https://github.com/dwelch67/raspberrypi.git) that was already on my system.  None of them appear to implement a timer like I want, but there was a UART program to blast characters so that I could debug the hardware setup.  I finally an able to confirm that my hardware and the `screen` command are working -- I can get the characters on the screen.  The bit rate is 115200, so I will have to do some monkeying around with my code to get it set up properly.  However, I have a model I can work against to try to get my little program working for real hardware.  I am not working on my kernel mind, just the `rpi-timer` app.

So I just realized that the UART program I was just testing with is using the Aux UART (a little mini thing that has limited capability) and the one I have coded against is the full UART.  That could make a difference!

---

### 2018-Dec-15

I think I have made the decision to re-document some of this Rpi archetecture into my own format.  There are just so many errors, I really have no idea what works and what does not.  I'm finding errata all over the place and I think it will be good to consolidate it into 1 proper document I can refer to.

To do this, I will use my work computer and MS OneNote.  I have basket installed on my development workstation, but that does not provide the clean interface I want if I am going to take this on.  On the other hand, we have Office365 and I might be able to create a more global notebook and access it from the web.  Let me look at this.

---

I was able to get the UART documented and clean up some of the errata associated with that feature.  This was done with OneNote and is stored online at the following link (well, at least for now): https://andersondubose-my.sharepoint.com/:o:/g/personal/aclark_anderson-dubose_com/EvZDPp9tqlREs9Zj-pu_OEcBFz8DWkDGP89V2LAm_l4FZg?e=XlGPT1.  I may need to find a new home for this, but for now it will suffice.  Currently, this does not appear to be visible to anyone without an Office365 account.

My next task tomorrow it so lay this new documentation side-by-side to determine the differences between the PL011 UART and the Aux Mini-UART.

---

### 2018-Dec-16

Today I found the PL011 Technical Reference Manual.  It will clear up a few things.

I have been debating a bit over whether to use the mini-UART (which is proven to work on real hardware) or the PL011 UART.  I think in my timer test, I want to go with something that is guaranteed to work since the point is to get the timer working properly and fire an IRQ.  So, with that, I will change the addressing to be the mini-UART.

Despite my best efforts today, I am still not getting any output on the serial line to `screen`.  I tested again the `uart01.bin` program to make sure everything is wired properly and it is -- I get the proper output.  So, I am back to an exhaustive debugging cycle to get the mini-UART working properly -- starting by commenting out all the code that is not related to the UART.

---

No matter what me efforts, I am still not able to get serial output to happen.  However, when I test the `uart01.bin`, it works just fine.  At this point, I am going to comment out all my code and just copy in the C code from `uart01.c` and see what happens.

I'm honestly debating at this point if my code is ever getting control.  I am also wondering if using the C++ compiler is creating a problem.

---

I recompiled the `uart01.bin` file with my cross-compiler.  It does not work.  So, my cross-compiler is suspect.

---

3 recompile attempts later....  Well, I'm having trouble getting `crosstools-ng` to drop the cross-compiler in the right location.

---

Make that 6 attempts to make the cross-compiler and I think I finally have something more viable.  Instead of working with the `arm-eabi-*` toolchain, I am going to have to change over to use the `armv7-rpi2-linux-gnueabihf-*` toolchain.  This is going to require that I change all the `Tupfile`s and update all my compile commands.

I got those changes made and I went right back to my `rpi-timer` test to see what I could get to work -- nothing.  I was a bit too ambitious.  Tomorrow....

---

### 2018-Dec-17

Today I will take a look at my code to make sure I am getting serial output properly.  This will be my primary focus.

---

So, no matter what I do, the `rpi-timer.img` test does not work.  However, the `uart01.bin` test does.  There are some differences here:
* Compiler parameters
* C vs C++ compilers
* Extra code

What I need to do is start by eliminating the differences (the more prevalent right now are the compiler parameters) and see if I can get anything to run.

---

OK, I am going to stop working on `rpi-timer` for now and create a `uart01` built into my project.  It will start as a direct copy of the sameple code just to integrate it into the the build system.

---

OK, my first test did not work.  I wanted to capture the commands that do work, which are my target at the moment:

```
[adam@os-dev uart01]$ make
armv7-rpi2-linux-gnueabihf-as vectors.s -o vectors.o
armv7-rpi2-linux-gnueabihf-gcc -Wall  -O2 -nostdlib -nostartfiles -ffreestanding -c uart01.c -o uart01.o
armv7-rpi2-linux-gnueabihf-ld vectors.o uart01.o -T memmap -o uart01.elf
armv7-rpi2-linux-gnueabihf-objdump -D uart01.elf > uart01.list
armv7-rpi2-linux-gnueabihf-objcopy uart01.elf -O ihex uart01.hex
armv7-rpi2-linux-gnueabihf-objcopy uart01.elf -O binary uart01.bin
```

The `tup` commands are:

```
[adam@os-dev century-os]$ tup bin/rpi2b/boot/uart01.img
[ tup ] [0.000s] Scanning filesystem...
[ tup ] [0.014s] Reading in new environment variables...
[ tup ] [0.015s] No Tupfiles to parse.
[ tup ] [0.015s] No files to delete.
[ tup ] [0.015s] Executing Commands...
 1) [0.024s] obj/uart01/rpi2b: armv7-rpi2-linux-gnueabihf-as /home/adam/workspace/century-os/modules/uart01/src/rpi2b/vectors.s -o vectors.o
 2) [0.082s] obj/uart01/rpi2b: armv7-rpi2-linux-gnueabihf-gcc -Wall -O2 -nostdlib -nostartfiles -ffreestanding -c /home/adam/workspace/century-os/modules/uart01/src/rpi2b/uart01.c -o uart01.o
 3) [0.022s] bin/rpi2b/boot: armv7-rpi2-linux-gnueabihf-ld /home/adam/workspace/century-os/obj/uart01/rpi2b/uart01.o /home/adam/workspace/century-os/obj/uart01/rpi2b/vectors.o -T /home/adam/workspace/century-os/modules/uart01/src/rpi2b/memmap -o uart01.elf
 4) [0.011s] bin/rpi2b/boot: armv7-rpi2-linux-gnueabihf-objcopy uart01.elf -O binary uart01.img
 [    ] 100%
[ tup ] [0.151s] Partial update complete: skipped 1 commands.
```

Taking the `tup` commands and removing the path information, I end up with:

```
[adam@os-dev century-os]$ tup bin/rpi2b/boot/uart01.img
armv7-rpi2-linux-gnueabihf-as vectors.s -o vectors.o
armv7-rpi2-linux-gnueabihf-gcc -Wall -O2 -nostdlib -nostartfiles -ffreestanding -c uart01.c -o uart01.o
armv7-rpi2-linux-gnueabihf-ld uart01.o vectors.o -T memmap -o uart01.elf
armv7-rpi2-linux-gnueabihf-objcopy uart01.elf -O binary uart01.img
```

Laying these commands on top of each other and aligning for spacing, I get the following:

```
armv7-rpi2-linux-gnueabihf-as vectors.s -o vectors.o
armv7-rpi2-linux-gnueabihf-as vectors.s -o vectors.o

armv7-rpi2-linux-gnueabihf-gcc -Wall  -O2 -nostdlib -nostartfiles -ffreestanding -c uart01.c -o uart01.o
armv7-rpi2-linux-gnueabihf-gcc -Wall  -O2 -nostdlib -nostartfiles -ffreestanding -c uart01.c -o uart01.o

armv7-rpi2-linux-gnueabihf-ld vectors.o uart01.o -T memmap -o uart01.elf
armv7-rpi2-linux-gnueabihf-ld uart01.o vectors.o -T memmap -o uart01.elf

armv7-rpi2-linux-gnueabihf-objcopy uart01.elf -O binary uart01.bin
armv7-rpi2-linux-gnueabihf-objcopy uart01.elf -O binary uart01.img
```

Materially they are the same, with the only exceptions being the extension of the target name (`.bin` vs. `.img`) and the order of the object files being linked.  Since this is a binary with not specifiied entry point, the first instruction is supposed to be the start of the file.  I belive that this is my problem.

And it was.  The order of the modules in the binary file are not in the right order and the entry point was not the first byte in the file.  That was a painful lesson I hope I don't forget soon.

---

### 2018-Dec-18

I have a renewed sense of hope today getting into the coding.  At least I what I was doing wrong in getting the basics in line.  Such a stupid mistake, I'm actually embarrassed.

Today I will move back to the `rpi-timer` code and see what I can do there.  I feel much more confident in getting the code running now that I concretely know what the problem was.  This simple change allowed me to run the uart code to blast characters.  Now, I should be able to go back to my original purpose-built test -- which of course did not work!

I went for it all.  I am going to start by aligning the gcc parameters and see where that get me.

---

OK, things are compiling and running better now.  But I am still in the C compiler and I need to get to C++.  This is the next step.  This went relatively easily.  Next is to get the system to compile again.  And that is done.

So, now I want to start working on getting timer working.  This is still in the `rpi-timer` code, but is disabled at the moment.  I will start by enabling each bit at a time and testing to see where we end up.

---

Now, I'm getting somewhere.  I am trying to write the IVT data, starting with address `0x00000000`.  This assignment is not completing, or more to the point the next debugging line is not being written to the serial port.

I moved the interrupts to 1MB and the IVT initialization completes.   However, when interrupts are enabled, I am not getting anything.  I did program the timer, but have not checked if I am getting incrementing values.  I will enable that next.

OK, so everything works now up until enabling interrupts.  One last test on that since having added the infinite loop.

---

That still did not work.  So, enabling interrupts is killing my little program.  I am probably getting an interrupt and it is not working properly.  I will need to figure out how to sort that, but tomorrow.

---

### 2018-Dec-19

OK, so today I am going to work on getting the IVT table configured properly, since that is the most likely cause for interrupts locking up the system.  To do this, I need to figure out what where to check and manage HIVECS.

---

The location of the bit that set HIVECS is actually `SCTLR.V`.  Now with Cortex A7, I should be able to set the `VBAR` (or Vector Base Address Register) to be whatever I want.  I currently have it set at 1MB (`0x100000`).  However, I am not getting anything to collect the interrupt properly.

So, my next step is going to be in inquire on the different register addresses and output them to the serial port before enabling interrupts.  This means I will need my `SerialPutHex()` function.

OK, so here is where my test is at:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
Timer is initialized -- interrupts should be happening
```

The key things here is that the `VBAR` is set correctly and the `SCTLR.V` bit is clear.

Adding in the VBAR table dump, I get the following results:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xeafffe7d
Timer is initialized -- interrupts should be happening
```

THis finds something interesting, as the jump (or technically the `b` instruction) appears to be a relative offset.  I am not totally 100% sure of the `nop` opcodes, but they are all consistent so that odds are in my favor.  I will decode those anyway to be sure.

Adding in the code to make sure I get to the correct location from the Vector Table (and enabling interrupts again), I am getting the following:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xe51f0000
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008054
```

Notice I am not getting the initialization message.  So, something is still very wrong.  This config looks correct.  Let me see if I can force an undefined instruction with interrupts disabled.

The undefined instruction behavior is no different than the IRQ behavior.  Maybe the code that backs this is not working right anyway.

---

### 2018-Dec-20

Ok, I really need to figure out some way to test my code here....  I think I have a few options:
* Write an asm function with a jump to the IVT location.  I should be able to get the `'#'` character to the serial port once.
* Run this in qemu, fixing everything to have the proper offsets to be loaded by the qemu emulator (the entry point is different).  I should be able to debug a little bit from here.

I think I am going to start with the first option since I do not trust qemu to emulate anything rpi properly at the moment.

---

OK, with my first test, I am jumping to the location of the IVT in code (not the target location).  This test results in what I am looking for: a xingle `'#'` character printed.

```
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xe51f0000
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008058
#
```

Now for test #2: jump to location `0x100000`, or 1MB.  This test also worked, yielding the same results as above.

So, what does this tell me?  Let's see here:
1. The code is sound under the normal execution stream.
1. When I have some kind of exception, the system locks up.
1. My target location at 1MB is a good one and the code is being set properly there.
1. `IRQHandler()` sets a reasonable stack location and I can use it properly under a normal execution stream.

I made a change where the `IRQHandler()` function puts the CPU into supervisor mode.  This did not have a good result as the rpi locked up as soon as interrupts were enabled.  So, I think I am going to have to try with qemu to see what I can find.

---

OK, it looks like qemu emulated the program fine, but never enabled interrupts (or at least the timer does not fire).  On real hardware, an appear to be getting some exception that is crashing the system that I cannot trace.

---

### 2018-Dec-21

Once again, I am debating whether to pursue working on qemu (where I am not getting the same issues as on real hardware, but different things are breaking) or to pursue debugging on heal hardware (where my ability to actually do debugging and investigation is almost totally eliminated).  On one hand, I cannot duplicate the problem at hand and on the other I cannot debug the problem.  Not a fun choice at all.

Well, I have proven that the actual code I have written for the Interrupt Vectors and the target will work.  I get the `'#'` character.  So, something else must be happening at the system level, or the interrupts are not being routed properly.  I need to check that code carefully, as my checks have all be surface checks so far.  I am not sure exactly how much code or debugging I will really get done today....  I expect today to look much more like research than anything.

---

### 2018-Dec-22

Today I am continunig the work on documenting the Interrupt Controller (IC) and checking my code against the specification.

In the interest of being 100% accurate in my code, I did add in the following into the timer initialization today:

```C
    // -- for good measure, disable the FIQ
    MmioWrite(INT_FIQCONTROL, 0x0);
```

This disables the FIQ interrupt path.  I honestly do not expect this to make a difference in my test (and therefore will not test it).

The other thing I notice is that I am enabling the world, which is not the real intent here.  I really need to just enable the 1 interrupt for the timer.  So, I will disable everything and then go back and enable only the ARM Timer.

---

Now that I have some changes to test, I am getting really odd characters to the screen.  I am not sure if this is due to the changes I made or if the OS needs a reboot to clean something up.  To test this, I am going to run an older executable to see what results I get.  The older binary worked perfectly, so there is a problem with my code.  The results I am getting are just garbage to the screen:

```
'ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
ï¿½Äï¿½ï¿½ï¿½ï¿½ï¿½ï¿½:
ï¿½94aï¿½Òºhï¿½Iï¿½8s#=tï¿½\F\Xu`ï¿½c
?ï¿½ï¿½,ï¿½ï¿½/Kï¿½]ï¿½Iï¿½Uï¿½YVï¿½Hï¿½>Jï¿½`Hpï¿½d&ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½~ï¿½<;hï¿½ï¿½(ï¿½ï¿½{XNï¿½ï¿½ï¿½ï¿½fL1kï¿½ï¿½,ï¿½Ó¥ï¿½Fï¿½ï¿½ï¿½q}ÚRa2ï¿½ÍNYNï¿½ï¿½ï¿½,ï¿½ï¿½IlVï¿½Xbï¿½ï¿½ï¿½ï¿½,ï¿½Fï¿½ï¿½nï¿½ï¿½wï¿½ï¿½ï¿½sï¿½ï¿½ï¿½o$ï¿½ï¿½ï¿½>ï¿½Aï¿½ï¿½-AkË±ï¿½ï¿½Cï¿½=ï¿½ï¿½ï¿½ï¿½Zï¿½gï¿½Tï¿½ï¿½-&uï¿½ï¿½[ï¿½0ï¿½ï¿½ï¿½,ï¿½Kï¿½ï¿½*Uï¿½;Nï¿½ï¿½ï¿½ï¿½ï¿½C%ï¿½/Ô°ï¿ï¿½ï¿½ï¿½zï¿½m(Åï¿½ï¿½^*ï¿½/ï¿½z[ï¿½"#ï¿½ï¿½4rï¿½Kï¿½yï¿½^ï¿½q?4ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½(ï¿½ï¿½ï¿½!ï¿½ï¿½_ï¿½ï¿½Cmï¿½ï¿½ï¿½ï¿½YHï¿½B\ï¿½ï¿½jï¿½oï¿½zï¿½ï¿½srï¿½Bï¿½ï¿½ï¿½" ï¿½$Lï¿½/ï¿½ï¿½ï¿½_ï¿½jE>ï¿½8ï¿½ï¿½ï¿½0ï¿½Îdï¿½
```

... and lots of it.  I am not sure if interrupts are enabled when I get control.  I'm going to start with an explicit disable and if that does not change things, I'm going to comment out enabling the interrupts.  I want to check the output of the debugging code I have added.

Hmmm...  as I'm thinking about this and fixing up my code, I am wondering if I might be having an IRQ routed to another core, waking it up somehow.  I guess it's possible but I have no clue how it will behave.

I explicitly disabled interrupts and commented out the `EnableInterrupts()` function call and I am still getting garbage to the screen -- however based on the timing of the garbage and the relative length of the garbage, I believe it is getting to the end of the program whereas before commenting out the `EnableInterrupts()` call it was not.

I am going to start my troubleshooting by commenting out the 5 lines of code I added.  These are here:

```C
    // -- for good measure, disable the FIQ
//    MmioWrite(INT_FIQCONTROL, 0x0);

    // -- Disable all IRQs -- write to clear, anything that is high will be pulled low
//    MmioWrite(INT_IRQDIS0, 0xffffffff);
//    MmioWrite(INT_IRQDIS1, 0xffffffff);
//    MmioWrite(INT_IRQDIS2, 0xffffffff);

    // -- Now I should be able to set up the timer, which will be done locally
    MmioWrite(TMR_BASE + 0x00, 0x00);               // this register is not emulated by qemu
    MmioWrite(TMR_BASE + 0x08, 0x80000000);         // this register is not emulated by qemu
    MmioWrite(TMR_BASE + 0x40, 0x00000020);         // select as IRQ for core 0
    MmioWrite(TMR_BASE + 0x60, 0xfff);              // enable IRQs from all devices from local timer down
    MmioWrite(TMR_BASE + 0x34, 0x8 | (1<<28) | (1<<29));  // set up the counter for the timer and start it
    MmioWrite(TMR_BASE + 0x38, (1<<30) | (1<<31));  // clear and reload the timer

    // -- Now, enable the ARM Timer interrupt only
//    MmioWrite(INT_IRQENB0, 1);
```

I will run this test to see if I can get proper output again....  Still garbage!!

OK, there were 3 lines I removed as well.  These enabled all interrupts no matter what they were.  I will try to put them back in.  And still garbage!!

I did find that the program was linked to address `0x10000` still for qemu.  Made this change and will now test again.  OK, that restored it again.  It's the stupid things that get you sometimes.

I'm going to reinstate all my code and try again.  This worked and I have been able to determine the the IRQ Enable bit can be read.  Now, to try again with interrupts enabled.  This is still locking up and not getting to the end.  I really did not expect much of a different result.

So, now I need to look more into the BCM2836 chip related to multiple cores and see that I need to do to get that routes properly.  I found that I had the `nCNTPNSIRQ` interrupt routed to the FIQ (which is of course disabled).  So, I have made that change but I'm not ready to test yet.  I also wrote the register to route the local timer interrupt to core 0 IRQ.

Now for a test....  No change in behavior.

---

OK, to try to think this through...  I have 4 tests that I can try.  These tests are controlled by which of the following lines are commented:

```C
//    IvtFunc();                      // This is an explicit jump to the start of the Interrupt Vectors -- works
//    Undef();                        // This will generate an undefined exception -- does not work
//    EnableInterrupts();             // This enables interrupts and the timer should start firing -- does not work
```

My `IRQHandler()` function is a one-shot handler that just prints a `'#'` character to the serial port and then `Halt()`s the CPU.  When the `IRQHandler()` function is called, I do not expect the cpu to return to perform any additional processing.

I am setting the location of the Interrupt Vector Table with the following code as soon as my code gets control and all cores except 0 are quieted down:

```
initialize:
    mov     r0,#0x100000                @@ The location of the vector base address register (1MB)
    mcr     p15,0,r0,c12,c0,0           @@ Write this location to the Vector Base Address Register
```

#### Test 1

All of the lines are commented out.  This will drop the code into a loop that reads the timer value to ensure that it is incrementing.  This will print the following to the serial port:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xe51f0000
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008078
  The Basic Interrupt register is : 0x00000001
Timer is initialized -- interrupts should be happening
The timer value is: 0x00a4a5ad
The timer value is: 0x013a59d9
The timer value is: 0x01d00e2b
The timer value is: 0x0265c368
The timer value is: 0x02fb77fe
The timer value is: 0x03912c8e
The timer value is: 0x0426e2ea
The timer value is: 0x04bc9729
The timer value is: 0x05524bea
The timer value is: 0x05e801b6
```

The actual values of the timer value will be slightly different, but all-in-all, this is the result and it is consistent.  These are the expected results, so this test passes.


#### Test 2

This test is performed by uncommenting the following line:

```C
    IvtFunc();                      // This is an explicit jump to the start of the Interrupt Vectors -- works
```

The code that performs this jump to the first Interrupt Vector is:

```
IvtFunc:
    mov     r0,#0x100000                @@ The location of the vector base address register (1MB)
    mov     pc,r0                       @@ and jump to that location
```

In this function, there is an explicit jumpt to the first interrupt vector.  In this test, I am expecting a `'#'` character to be printed and I should not see the line reading `Timer is initialized -- interrupts should be happening`.

My results look like this:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xe51f0000
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008078
  The Basic Interrupt register is : 0x00000001
#
```

These are the expected results and this test passes.


#### Test 3

This test is performed ny uncommenting the following line:

```C
    Undef();                        // This will generate an undefined exception -- does not work
```

In this function, there is a jump to an assembly function that executes an undefined instruction.  This instruction looks like this from the disassembly:

```
00008074 <Undef>:
    8074:	ffffffff 			; <UNDEFINED> instruction: 0xffffffff
```

I am expecting the exact same results as Test 2, where I see a `'#'` character on the screen.  The results are the same as Test 2 without the `'#'` character, as shown below:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xe51f0000
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008078
  The Basic Interrupt register is : 0x00000001
```

This test fails as the `'#'` character is not written to the serial port.


#### Test 4

This test is performed ny uncommenting the following line:

```C
    EnableInterrupts();             // This enables interrupts and the timer should start firing -- does not work
```

In this function, interrupts are enabled and I expect the timer to fire an IRQ almost immediately.  `IRQHandler()` will get control and write a `'#'` character to the serial port and then stop processing.

The results of this test are the exact same as with Test 3:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
  VBAR: 0x00100000
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe1a00000
  The code at VBAR[1] is: 0xe1a00000
  The code at VBAR[2] is: 0xe1a00000
  The code at VBAR[3] is: 0xe1a00000
  The code at VBAR[4] is: 0xe1a00000
  The code at VBAR[5] is: 0xe1a00000
  The code at VBAR[6] is: 0xe1a00000
  The code at VBAR[7] is: 0xe51f0000
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008078
  The Basic Interrupt register is : 0x00000001
```

This test fails as the `'#'` character is not written to the serial port.


#### Conclusion

The conclusion I can draw from this is that there is something witht the state of the cpu that is causing a problem on an exception/interrupt.  Whether that is something with the `cpsr` or the registers, I am not sure.

However, with that said, I am not disabling the interrupts when switching modes, so I might be having a recursive interrupt problem...  Let me test that; setting up for test 2....  That test failed.

At this point, I am going to commit these changes to github so that I can ask for help....

---

Well, I have been working with a really old version of the ARM ARM and I only just now realized it.  Shame on me!

I had a great conversation on `freenode#osdev` about this, captured here:

```
on real rpi hardware.  I believe I have this narrowed down to a cpu state problem and would like to ask if someone can review my work.  My analysis of the situation is here: https://github.com/eryjus/century-os/blob/master/JOURNAL.md -- go to the bottom and look for today's date.  I had a problem with the link locaiton which is documented first for the day and then this
[13:59] <eryjus> analysis starts.
[13:59] <eryjus> The source for this test is here: https://github.com/eryjus/century-os/tree/master/modules/rpi-timer
[14:00] <eryjus> thanks in advance.
[14:00] <geist> why do you think it's a cpu state problem?
[14:01] <eryjus> i have 4 tests documented -- and an explitit jump to the first interrupt vector works, but when i force an undefined excpetion the cpu locks
[14:01] <geist> also depens on what you mean by 'cpu state'
[14:02] <geist> ah. good.i was going to stay, trigger an undefined opcode
[14:02] <eryjus> cspr or registers
[14:02] <geist> then the question is does it make it to your code or is the vbar not correct
[14:02] <geist> point me at your exception handling code, the table and the assembly
[14:03] <eryjus> the vbar is set correctly or at least I can read back what i wrote and get the results I expect
[14:03] <geist> what was the value?
[14:03] <eryjus> ivt: https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/entry.s#L77
[14:04] <eryjus> moves it to the correct locaiton: https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/TimerMain.cc#L233
[14:04] <geist> the nop sled is cute
[14:04] <geist> what value was put in vbar?
[14:04] <eryjus> the actual handler code: https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/IRQTarget.s#L30
[14:05] <eryjus> load of vbar: https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/entry.s#L57
[14:05] <geist> what was the value put in the vbar?
[14:05] <geist> the numeric value
[14:05] <eryjus> the value loaded is 0x100000 and that is what I am getting back when i read it later
[14:05] <geist> okay
[14:06] <geist> what is this patching of the IVT thing?
[14:07] <eryjus> not sure I follow your question..
[14:07] <geist> https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/TimerMain.cc#L234
[14:07] <geist> what is that all about
[14:07] <eryjus> I am relocating it to 0x100000 in my initialization
[14:08] <geist> is the I and D cache initialized at this point?
[14:08] <geist> if so, you can have a cache coherency problem
[14:08] <eryjus> this may help  https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/TimerMain.cc#L225
[14:08] <geist> I & D caches are not synchronized on ARM. if you write out code and expect it to run you *must* flush the D and I caches
[14:08] <geist> generally you just point the vbar at the vector table inside your binary, no need to copy it
[14:09] <eryjus> no code to init any caches..
[14:09] <geist> only constraint there is making sure it aligned
[14:09] <geist> in that cas eyou dont know the state of the caches
[14:09] <geist> so it's possible the I&D cache is enabled, in which case the copyying of the vector table wont work
[14:10] <geist> there are a few other problems
[14:10] <geist> assume the cache is okay
[14:10] <geist> the instruction you're using in the IVT is a ldr reg, =value
[14:11] <geist> the =value part means it may emit a PC relative load to a hidden .word
[14:11] <geist> which you're probably not copying
[14:11] <eryjus> true -- but when i to an explicit jump to the first vector the code works
[14:12] <geist> explicit jump in the copied version?
[14:12] <geist> or in the in text version?
[14:12] <eryjus> when this line is uncommented: https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/TimerMain.cc#L301
[14:12] <eryjus> https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/entry.s#L73
[14:12] <eryjus> This works
[14:12] <geist> *shrug* dunno then
[14:12] <geist> you have about 5 layersof hacks here, any of which can go wrong
[14:13] <geist> the ldr = stuff is at best extremely fragile
[14:13] <geist> i'd recommend something like....
[14:14] <geist> hmm, i dont have it here. my LK code is using the vector table in place in the .text segment, so it can get away with a simple relative branch
[14:14] <geist> https://github.com/littlekernel/lk/blob/master/arch/arm/arm/start.S#L28
[14:14] <geist> but i've absolutley written code to put a ldr pc, <some label just after the table>
[14:15] <geist> and then after ldrs it has a table of .word
[14:15] <geist> and then copy all of that
[14:15] <geist> that way it is completely position independent
[14:16] <geist> but assuming this all works, what is at IRQTarget?
[14:16] <geist> I dont see that code
[14:17] <eryjus> IRQTarget: https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/IRQTarget.s#L30
[14:18] <eryjus> I forgot I pulled IRQHandler out of the mix to try to get a simple case working.
[14:18] <geist> can you post the dissassembly of the entire binary?
[14:18] <eryjus> stand by
[14:20] <eryjus> https://github.com/eryjus/century-os/blob/master/maps/rpi2b/rpi-timer.map
[14:21] <geist> https://github.com/eryjus/century-os/blob/master/maps/rpi2b/rpi-timer.map#L521 is the thing I was talking about, btw
[14:21] <geist> see how the instruction at 874c references the hidden .word at 8754?
[14:22] <geist> anyway, i'd start off by getting rid of that whole copy stuff, simplify it. align the IVT on a 64 byte or so boundary (i forget the requirements) and just point vbar directly at it
[14:22] <geist> then you'll have removed a big pile of complexium
[14:23] <eryjus> I do and if you look at the code here, I have accounted for that by copying an extra .word https://github.com/eryjus/century-os/blob/master/modules/rpi-timer/src/rpi2b/TimerMain.cc#L234
[14:23] <eryjus> 7 nop -- 1 ldr -- 1 mov -- 1 hidden word
[14:23] <eryjus> or at least i tried
[14:23] <eryjus> ok, will give that a try.
[14:24] <geist> yah, but again that's all just fragile stuff. start by removing the stuff and simplifying
[14:24] <geist> so that there's less variables in flight. i'm looking up the alignment requirements of the vector table now
[14:24] <geist> i think it's something like 64 bytes
[14:26] <eryjus> im searching as well..
[14:26] <geist> looks like 32 bytes. the bottom 5 bits of the register are ignored
[14:27] <geist> which makes sense, since that's basically the size of the vector table. the hardware almost assurednly just ORs in the offset when computing an address
[14:27] <eryjus> geist, where can i find that?
[14:27] <geist> if you toss in a .balign 32 or something just in front of it
[14:27] <geist> i just found it in the ARM ARM, in the description of the VBAR
[14:28] <eryjus> interesting...  my ARMARM does not have a reference to VBAR.
[14:28] <geist> then if you use the direct vbar you can simplify your vector table with a series of branches
[14:28] <geist> yes i'm looking in the armv8 one
[14:28] <geist> but in this case it's backwards compatible
[14:28] <geist> armv8 defines much easier to understand hard names for all these old control registers, which is where VBAR comes from
[14:28] <eryjus> i gotta get a better manual -- mine is (c) 2005
[14:29] <geist> yes. that *wayyyy* predates vbar existing
[14:29] <eryjus> that could be part of my problem
[14:29] <geist> 2005 is armv5 era, the thing you're dealing with here is armv7
[14:29] <geist> which itself is already about 8 years out of date, since armv8 is the current standard
[14:30] <eryjus> this might explain why I am fighting so hard and getting nowhere....
[14:30] <eryjus> not sure where I got that dinosaur from....
[14:30] <geist> go to arms site and find them directly
[14:31] <geist> if you're ust googling for random crap,you're going to get it
[14:31] <geist> https://developer.arm.com/products/architecture/cpu-architecture/r-profile/docs/ddi0406/latest/arm-architecture-reference-manual-armv7-a-and-armv7-r-edition seems to be a good starting point for the armv7-a spec
[14:32] <geist> https://developer.arm.com/products/architecture/cpu-architecture/r-profile/docs/ddi0487/latest/arm-architecture-reference-manual-armv8-for-armv8-a-architecture-profile is the armv8-a
[14:32] <geist> but it'll be a lot more complicated for what you're doing, so i'd start with the v7-a
[14:35] <eryjus> i have them both now.  Thank you!!! -- I'm sure things will start looking better from here.
[14:35] <eryjus> I'll call that an early Christmas present.  thanks again
[14:37] <geist> yay
[14:37] <geist> and i'm not trying to bust your chops about this stuff, you just have to make sure all the details are right
[14:38] <geist> there's a certain amount of hackery you can do when first getting started, but it quickly topples over. if you have too many layeres of hacks to try to get something working then it's hard to see at what level things are broken
[14:38] <geist> so it makes sense to go and knock out some of the complexity to try to simplify the problem
[14:39] <geist> i do this all the time when doing initial bringup. hack together some stuff to prototype that it works, then go back and clean it up. but sometimes the hacks get too deep prior to getting something working, and you have to declare bankruptcy and start building a better foundation
[14:39] <geist> which is of course hard if you dont know how to make it go
[14:40] <eryjus> geist, i totally agree -- this is a purpose built test to for real hardware to get the timer irq to fire -- where I can go back to qemu and figure out what is emulated and what is not...  none of this was going to survive into a kernel without a real hard look.  I knew the copy of the table was risky at best.
[14:41] <eryjus> we're saying the same thing -- but thanks for the counsel.
[14:41] <geist> yah
[14:42] <eryjus> i might be able to actually navigate now with an accurate map
[14:46] <geist> yah a rpi2 is a cortex-a7, which is a armv7-a implementation
```

So, the next thing here is to read, and read some more, and then get the VBAR set to the code location, properly aligned.

---

I set up a test to determine which mode I am in.  I come back with a value of `0x1a` whereas I am expecting to be in `0x13`.  What the hell is mode `0x1a`??  mode `0x1a` is `hyp` mode and I am expecting `svc`.

I am trying to make sure I am in `svc` mode as the first instruction I execute.  However, the ARM ARM states in section B9.1.2:

> Is not an exception return instruction, and is executed in Hyp mode, and attempts to set CPSR.M to a value other than '11010', the value for Hyp mode.

This means that my little instruction `cps #0x13` does absolutely nothing (and I believe it is treated as a `nop`).

I tries to use this little trick to not get into `hyp` mode in the first place: https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=98904&p=864376#p864376.  However, the pi will not boot.

I have the right instruction now and I am trying to boot.  Now nothing happens...  and I'm out of time for the night....

---

Well, I did have some more time tonight.  I was able to force the processor into `svc` mode!!  I used some code from this web site (well the thinking anyway): https://github.com/raspberrypi/linux/blob/rpi-4.1.y/arch/arm/include/asm/assembler.h#L319.  The result in my code is:

```
_start:
    mrs     r0,cpsr                     @@ get the current program status register
    and     r0,#0x1f                    @@ and mask out the mode bits
    cmp     r0,#0x1a                    @@ are we in hyp mode?
    beq     hyp                         @@ if we are in hyp mode, go to that section
    cpsid   iaf,#0x13                   @@ if not switch to svc mode, ensure we have a stack for the kernel; no ints
    b       cont                        @@ and then jump to set up the stack

@@ -- from here we are in hyp mode so we need to exception return to the svc mode
hyp:
    mrs     r0,cpsr                     @@ get the cpsr again
    and     r0,#~0x1f                   @@ clear the mode bits
    orr     r0,#0x013                   @@ set the mode for svc
    orr     r0,#1<<6|1<<7|1<<8          @@ disable interrupts as well
    msr     spsr_cxsf,r0                @@ and save that in the spsr

    ldr     r0,=cont                    @@ get the address where we continue
    msr     elr_hyp,r0                  @@ store that in the elr register

    eret                                @@ this is an exception return

@@ -- everyone continues from here
cont:
```

My test forced an undefiend exception and the results were:

```
Serial port initialized!
Initializing the IVT:
Interrupt Vector Table is set up
Ready to enable interrupts!
This is the system configuration:
The processor mode is: 0x00000013
  VBAR: 0x000087a0
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe320f000
  The code at VBAR[1] is: 0xe320f000
  The code at VBAR[2] is: 0xe320f000
  The code at VBAR[3] is: 0xe320f000
  The code at VBAR[4] is: 0xe320f000
  The code at VBAR[5] is: 0xe320f000
  The code at VBAR[6] is: 0xe320f000
  The code at VBAR[7] is: 0xe59f0004
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x000087a0
  The Basic Interrupt register is: 0x00000001
#
```

This is the expected results.  One more test tonight and I am going to call it a night.  This time to see if I can get the timer interrupt to fire and get the `'#'` character to the screen.

That test worked.  I'm going to end my night on this success.

---

### 2018-Dec-23

This morning I started with by putting all my IRQ code back and compiled a test -- but I forgot to save my changes to `IRQTarget.cc`.  Another test and I'm hopeful.

After this test, the interrupt is not firing (or more to the point I am not getting any output).

After stripping out all the extra stuff from the `IRQTarget()` function, this is working now.  Here is my output:

```
Ready to enable interrupts!
This is the system configuration:
The processor mode is: 0x00000013
  VBAR: 0x00008780
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe320f000
  The code at VBAR[1] is: 0xe320f000
  The code at VBAR[2] is: 0xe320f000
  The code at VBAR[3] is: 0xe320f000
  The code at VBAR[4] is: 0xe320f000
  The code at VBAR[5] is: 0xe320f000
  The code at VBAR[6] is: 0xe320f000
  The code at VBAR[7] is: 0xe59f0004
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008780
  The Basic Interrupt register is: 0x00000001
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
```

What I see now is that I am burying the CPU in IRQs so it cannot process anything more.  This should be an easy fix....  But I am having some trouble with it.  It looks like I am getting into a loop of some kind there the `'!'` character is printed as fast as possible.  I am unable to slow it down.  So I have capped the duration of the test and I then disable interrupts.

---

So it dawns on me that I am not clearing the timer interrupt when it fires.  Therefore, as soon as interrupts are enabled again after handling that interrupt, the interrupt gets re-triggered.  Let's see here, that should be as simple as clearing the timer flag for the Basic Pending register.

No matter what I try, I am not able to clear the interrupt and therefore I am still getting the cpu buried in the first interrupt.  I will have to do some research online.

I found the problem -- There is a timer reset which has an interrupt clear register that needs to be written.  This fixed the problem and now I have a program the works the way I want it.  Except for the timer frequency.

I am close and getting the interrupts to fire on a regular basis.  The `'!'` character denoted a timer interrupt:

```
Serial port initialized!
Ready to enable interrupts!
This is the system configuration:
The processor mode is: 0x00000013
  VBAR: 0x00008780
  SCTLR.V: clear
  The code at VBAR[0] is: 0xe320f000
  The code at VBAR[1] is: 0xe320f000
  The code at VBAR[2] is: 0xe320f000
  The code at VBAR[3] is: 0xe320f000
  The code at VBAR[4] is: 0xe320f000
  The code at VBAR[5] is: 0xe320f000
  The code at VBAR[6] is: 0xe320f000
  The code at VBAR[7] is: 0xe59f0004
  The code at VBAR[8] is: 0xe1a0f000
  The code at VBAR[9] is: 0x00008780
  The Basic Interrupt register is: 0x00000001
Timer is initialized -- interrupts should be happening
!The timer value is: 0x00a56249
!The timer value is: 0x013b106e
!The timer value is: 0x01d0bd08
!The timer value is: 0x026676d1
!The timer value is: 0x02fc30af
!!The timer value is: 0x0391eaf1
!The timer value is: 0x0427a3fb
!The timer value is: 0x04bd5196
!The timer value is: 0x0552ff00
!The timer value is: 0x05e8bb0f
```

Now, I want to try to get the interrupts to fire at about 100/second (x86 is set to 250/second, but with this slow cpu it might bury it again).

I now have the timer firing at a resaonable rate.  I might need to tune that a bit later, but at the moement the hardware for `rpi-timer` is working properly.  Now to commit this code and then switch back over to the timer on qemu.

---

OK, I have some sorting out to do...  In the middle of all this I broke the rpi2b build and the i686 build.  rpi2b has no valid loader and i686 triple faults on jumping to the kernel.  I believe my problem is going to be in `Tuprules.tup`, but I'm not sure.

OK, so the problem with the rpi2b not running was that the page size when linking was set to be too large.  Changing that resolved that problem.  Now for the reason the i686 triple faults...

---

So, for the i686, I did find a bug in my `kMemMove()` function.  This has not changed in some time and I am wondering how it never surfaced??!!

I still have a problem with a page fault in the kernel which I need to fix....  And this looks like an address for the rpi2b!  What was I thinking???

Once I found that, it was also an easy fix.  So, where do I stand now?
* i686 is functioning again
* rpi2b boots and dies with an undefined instruction

So, I will go find that and clean it up as well.  Well, the fault is coming out of `ProcessInit()` or the results of `ProcessInit()`:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
Undefined Instruction:
At address: 0x80031f78
 R0: 0x00009c0c   R1: 0x800321c0   R2: 0x80032200
 R3: 0x00000000   R4: 0x80032140   R5: 0x00000001
 R6: 0x00000000   R7: 0x00000000   R8: 0x00000000
 R9: 0x0011c000  R10: 0x0002d207  R11: 0x00028668
R12: 0x80069c0c   SP: 0x800029bc   LR_ret: 0x800029e0
SPSR_ret: 0x600001d3     type: 0x1b
```

...  or maybe not....

Something is not right....  I am still getting the undefined instruction and I think I'm running off to nowhere...  With this code:

```C
    kprintf("Initializing the butler lists\n");
    ListInit(&butler.stsQueue);
    butler.lockList.lock = {0};
    butler.messages.lock = {0};
    ListInit(&butler.lockList.list);
    ListInit(&butler.messages.list);
```

... I get this result:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
Attempting to initialize the idle and butler processes
Idle Process initialized
Initializing the butler lists
Undefined Instruction:
At address: 0x80031f70
 R0: 0x0000001e   R1: 0x800321c0   R2: 0xf2201000
 R3: 0x800058e2   R4: 0x80032111   R5: 0x800320c0
 R6: 0x80049c0c   R7: 0x00000001   R8: 0x00000000
 R9: 0x0011c000  R10: 0x0002d207  R11: 0x00028668
R12: 0xff000000   SP: 0x80002a1c   LR_ret: 0x80002a20
SPSR_ret: 0x600001d3     type: 0x1b

Additional Data Points:
User LR: 0x00000000  User SP: 0x00000000
Svc LR: 0x80031f80
```

Then when I add some additional code to determine which line is creating the problem, such as this:

```C
    kprintf("Initializing the butler lists\n");
    ListInit(&butler.stsQueue);
    kprintf("a");
    butler.lockList.lock = {0};
    kprintf("b");
    butler.messages.lock = {0};
    kprintf("c");
    ListInit(&butler.lockList.list);
    kprintf("d");
    ListInit(&butler.messages.list);
```

... I get this result:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
Undefined Instruction:
At address: 0x80031f70
 R0: 0x00005870   R1: 0xfb009c50   R2: 0x00000325
 R3: 0x00000002   R4: 0x00002140   R5: 0x00000000
 R6: 0x00000000   R7: 0x00000000   R8: 0x00000000
 R9: 0x0011c000  R10: 0x0002d207  R11: 0x00028668
R12: 0xfb00a140   SP: 0x800040f4   LR_ret: 0x8000291c
SPSR_ret: 0x600001d3     type: 0x1b

Additional Data Points:
User LR: 0x00000000  User SP: 0x00000000
Svc LR: 0x80031f80
```

... where the line about Initializing the butler lists is missing.  So, it's a qemu log again:

```
IN:
0x8000290c:  e3050870  movw     r0, #0x5870
0x80002910:  e92d41f0  push     {r4, r5, r6, r7, r8, lr}
0x80002914:  e3024140  movw     r4, #0x2140
0x80002918:  ed2d8b02  vpush    {d8}

R00=00000000 R01=fb009c50 R02=00000325 R03=00000002
R04=80032218 R05=00000000 R06=00000000 R07=00000000
R08=00000000 R09=0011c000 R10=0002d207 R11=00028668
R12=fb00a140 R13=80031fd8 R14=800040f4 R15=8000290c
PSR=600001d3 -ZC- A NS svc32
Taking exception 1 [Undefined Instruction]
...from EL1 to EL1
...with ESR 0x7/0x1fe0000a
```

Notice the `vpush` instruction.

---

I was able to determine that this is a cross-compiler problem.  Not only that but the cross-compiler for qemu needs to be configured differently than the cross-compiler for the real rpi2b hardware.  This means I will have to set up a new architecture for qemu.  This task I am going to add to Redmine to address later and move on to get the timer working properly (http://eryjus.ddns.net:3000/issues/373).

At this point, I am past the timer initialization and I am getting a Data Exception as a result:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
Idle Process initialized
Butler Process initialized
Setting the scalar value to 0x25800 for frequency 0xfa
IsrHandler registered
Timer is initialized
Data Exception:
At address: 0x80030f70
 R0: 0x80033000   R1: 0x00000000   R2: 0x00000001
 R3: 0x802000cc   R4: 0x80034000   R5: 0xd0000000
 R6: 0x80043000   R7: 0x80031000   R8: 0x00000000
 R9: 0x0011b000  R10: 0x0002d207  R11: 0x00028668
R12: 0x30200000   SP: 0x80000e10   LR_ret: 0x80001e94
SPSR_ret: 0x80000113     type: 0x17

Additional Data Points:
User LR: 0x00000000  User SP: 0x00000000
Svc LR: 0x80030f80
```

This Data Exception is happening in `MmuUnmapPage()`:

```
80001e88 <_Z12MmuUnmapPagem>:
80001e88:	e1a03620 	lsr	r3, r0, #12
80001e8c:	e1a03103 	lsl	r3, r3, #2
80001e90:	e2833102 	add	r3, r3, #-2147483648	; 0x80000000
80001e94:	e5d30000 	ldrb	r0, [r3]
80001e98:	e2100003 	ands	r0, r0, #3
80001e9c:	13a02000 	movne	r2, #0
80001ea0:	15930000 	ldrne	r0, [r3]
80001ea4:	15832000 	strne	r2, [r3]
80001ea8:	17f30650 	ubfxne	r0, r0, #12, #20
80001eac:	e12fff1e 	bx	lr
```

---

It dawned on me while I was away that the `MmuUnmapPage()` funtion call is from `HeapInit()`, which I am not debugging at the moment.  So, I am going to add a loop and some output to make sure that I am getting a timer emulated from qemu.  But, I am back at my original problem, in qemu the timer will not fire.

Or maybe it is firing and I'm not able to determine the IRQ number.  Nope...  I would see it.

---

I think I am going to have to come to terms with the fact that qemu just does not cut it for an emulator for rpi2b.  I am going to have to work on real hardware.  In order to do that, I am going to have to move my development system to my laptop.  Which means I need to build a number of cross compilers.  I have used `ct-ng` to do this and the config files should be available to migrate over.  I will need to get a commit of those with the code and write some instructions for building each of the cross compilers anyway.

At the same time, I am going to use a tool mrvn has released called `raspbootin`, which will allow a serial connection and load the kernel directly.  This should save a significant number of write cycles on the microSD drive.

---

### 2018-Dec-25

I have spent the better part of 2 days now getting my laptop configured to build the tools I need.  I really need to be able to automate this a bit, since there are several bits that are fragile.  But this is not a task for today.  For this, I created a new project in Redmine: http://eryjus.ddns.net:3000/projects/create-an-system-setup-for-century-os.

I am nearly at the point where I can compile `raspbootin`.  This project was built about 5 years ago and certainly on a compiler version that is older than I am building now.  I am building version 8.2.0 of gcc.  According to this web site, if mrvn was using the latest compiler when this was written, he would have been building with version 4.7.2: https://gcc.gnu.org/releases.html.  I may need a cross-compiler just for this project.  And that is just what needs to happen.

`ct-ng` only has default options available through version 4.9.4.  I'm either going to have to give this a try or manually build the toolchain.  I think I am going to give 4.9.4 a try.  That did not work either....

So, now I am left with a task to clean up and re-write/re-set the raspboot code to C (not C++).  I do have a few things to sort out today as well:
* I need to build the `i686-elf-*` toolchain.
* I need to clean up the 'rpi_in' and 'rpi' versions of the toolchain.
* Backups are not working and I need a good hard cleanup.
* I need to rewrite the raspbootin server portion to C.

Not necessarily in that order....

---

OK, I have a build completing successfully.  And, I the i686 code is still executing in qemu.  So, now I need to change focus to boot loader for the RPi.  The problem here is that I am expecting my own loader to get the multiboot signature and start execiting at `0x100000`.  It would have loaded modules (such as the kernel and the pmm) that will need to be moved over to the loader as well.  At the same time, I need to be able to present the module info to the bootloader so that it can then present the same to OS loader.

All this means is that I will not be able to just implement `raspbootin` as is.  I will need to derive my own solution using `raspbootin` as my roadmap.  This is technically not part of this OS and will be its own project.  Let me get that estiblished and I will come back here.  This will officially put this work on hold until I get the bootloader done.

---

### 2019-Jan-01

Well, I have been working on the `pi-bootloader` application (https://github.com/eryjus/pi-bootloader) for a few days now and I have hit a snag that needs to be addressed in the loader.  In this loader, I have been using the PL011 UART because that is what was emulated to the screen for qemu.  Since qemu is out and I am struggling to get the PL011 UART programmed properly (and the `pi-bootloader` is using the mini-UART), I need to change the output in the loader to be the mini-UART.  I will need to update the kernel as well.

So, I have the loader updated.  It should be outputting to the mini-UART at this point.  I copied the code from `pi-bootloader` so it should work.  Unfortunately, at this juncture, I have no way to test it independently to confirm.

---

At this point, I have everything sent to the rpi except the additional modules.  This includes the MBI structure.  From here, I will have to make modifications to both Century-OS and pi-bootloader to get this debugged.  Right now, it makes the most sense to track my work there in pi-bootloader, even though it will require changes to Century-OS to add debugging code.  The point here is that there should not be any changed to the overall logic in Century-OS, and if there are I will be documenting them here.

---

What the hell was this???

```C
//
// -- A quick MACRO to help determine if a flag is set
//    ------------------------------------------------
#define CHECK_FLAG(f) ((mb1Data->flags != 0) && ((1<<f) != 0))
```

I mean, what the hell was I thinking?  I must have gotten all the flags I every wanted on x86 and therefore never thought twice about the crap in that macro.  I'm cleaning that up for sure.

Now for the next thing.  I am getting something that is not working.  Either the pi stops producing output or the pi repeatedly outputs the same character.  I have put some debugging output in the memory map portion of the hardware discovery and I think I have determined that the location where I am putting the `hwDisc` structure on rpi2b is not real memory (which will also mean that the memory map is wrong).

```C
//            if (entry->mmapType == 1) AddAvailMem(entry->mmapAddr, entry->mmapLength);
            SerialPutChar('a');
            uint64_t newLimit = entry->mmapAddr + entry->mmapLength;
            SerialPutChar('b');
            if (newLimit > GetUpperMemLimit()) SetUpperMemLimit(newLimit);
            SerialPutChar('c');
            size -= (entry->mmapSize + 4);
            SerialPutChar('d');
            entry = (Mb1MmapEntry_t *)(((uint32_t)entry) + entry->mmapSize + 4);
            SerialPutChar('e');
```

However, when I comment the line above I was able to get to checkpoint `b`; checkpoint `c` was never reached.  When I uncomment that line, I never see checkpoint `a`.  Since I do not have exceptions set up a this point in the code, I am likely going off into nowhere.

Now, this all works for x86.  I have put the `hwDisc` structure at frame `0x3000` and limited it to 4K in length.  Both the line before checkpoint `a` and before checkpoint `c` are referencing this `hwDisc` structure.  Well, that's wrong.  This structure is built in local memory as `localHwDisc` and then copied to the target location of `0x3000` right before the kernel is booted.  Now, this might mean I have some initiailized space.

Yeah, there is definitely something with `localHwDisc`, as I cannot even read the number of MMap Entries from the local table.  I will have to sort that out tomorrow -- I'm calling it for the night.

---

### 2019-Jan-02

OK, I have had problem accessing data at address `0x0000` and `0x3000` from within the C programs.  I am not sure if I have had the same problems from asm, but I think I want to see what the resulting asm looks like for accessing the `localHwDisc` structure looks like.  At the point I am doing this, I expect that the stucture is in the `.bss` section and that might mean that I am not getting it cleared out properly.  Not completely sure here on this -- I don't know what I don't know yet.

Hmmmm.... I have added an -O2 flag...  I wonder if that is creating the problem...  Interestingly, when I remove that flag, I get the following results:

```
Sending the Entry point as 10000c
Waiting for the rpi to boot
Bootin
```

The loader never actually launches or it loses the ability to send data to the mini-UART.  THe `-O1` option made no difference versus `-O2`, so that is not it.

---

OK, I do know I have an alignment problem.

```
  101efc:	8581a01d 	strhi	sl, [r1, #29]
  101f00:	8581b021 	strhi	fp, [r1, #33]	; 0x21
```

This might be giving me some trouble.  A quick change without the `__attribute__((packed))` clause on the structure gave me this result (funny how qemu didn't emulate this either!):

```
'pi-bootloader' (hardware component) is loaded
   Waiting for kernel and modules...
Preparing to send century.cfg data
File 1 size is 135168
Notifying the RPi that 135168 bytes will be sent
65536 of 65536 bytes were written
56097 of 56097 bytes were written...
1247 of 1247 bytes were writtennt)...
4 of 4 bytes were writtentes sent)...
12284 of 12284 bytes were written)...
Sending kernel (135168 bytes sent)...
Done
Notifying the RPi that 144 bytes will be sent in the mbi
Sending mbi...
Done
Sending the Entry point as 10000c
Waiting for the rpi to boot
BootinSerial port initialized!
Found the mbi structure at 0x000fe000
  The flags are: 0x00000048
Module information present
Setting memory map data
  iterating in mmap - size is: 0x00000018
    entry address is: 0x000fe078
    entry type is: 0x00000001
    entry base is: 0x00000000:0x00000000
    entry length is: 0x00000000:0x3f000000
    entry size is: 0x00000014
  MMap Entry count is: 0x00000000
abcdeMemory Map is complete
Done parsing MB1 information
PANIC: Unable to determine memory map; Century OS cannot initialize
```

This is better than crashing and locking up, but it is still technically not right since I do have a good memory map.  First I clean up some of my detailed debugging code (oh yeah, and uncomment the one commented line that would cause the memory map to be missing!).

---

Oh holy crap!!!  That actually worked up to the point where I needed to load the kernel.  This looks good for now.

---

### 2019-Jan-03

This morning I have the kernel being recognized and the loader is trying to jump to the kernel.  However, it is not getting very far after that.  I am not completely sure, but I think there is a problem with the paging tables setup.  Once the kernel is mapped, I really should check `MmuDumpTables()` to make sure the address I want is properly set up.

Back on 23-Dec, I was able to get past this point on qemu and get the kernel to take control.  So, there is likely something in the `pi-bootloader` application that is causing me some grief.  I need to dig in a bit to figure out what is going on.

So, some debugging code in `ModuleInit()` and `LoaderMain()` give me this result:

```
Returning kernel entry point: 0x800043a4

MmuDumpTables: Walking the page tables for address 0x800043a4
Level  Tabl-Addr     Index        Entry Addr    Next PAddr    fault
-----  ----------    ----------   ----------    ----------    -----
TTL1   0x3e000000    0x00000800   0x3e002000    0x3e005000     01
TTL2   0x3e005000    0x00000004   0x3e005010    0x00000127     10
Preparing to enable paging
Paging is enabled
The instructions at that location are:
.. 0xf1020013
.. 0xe59fd008
.. 0xebffff26
.. 0xe320f003
Jumping to the kernel
```

Comparing that the the kernel map, at `0x800043a4`, I get this:

```
800043a4 <_start>:
800043a4:	f1020013 	cps	#19
800043a8:	e59fd008 	ldr	sp, [pc, #8]	; 800043b8 <Halt+0x8>
800043ac:	ebffff26 	bl	8000404c <kInit>

800043b0 <Halt>:
800043b0:	e320f003 	wfi
800043b4:	eafffffd 	b	800043b0 <Halt>
800043b8:	80038000 	andhi	r8, r3, r0
```

So these instructions match.  This is not a paging issue related to mapped pages.  The page is mapped and I can read the instructions at that location.  These instructions match what I am expecting.

So, I guess now I am wondering if there are flags that are not set properly in the paging tables.  I cannot imagine this to be the case because the loader continues to execute without any faults.  Also, there might be a barrier that is set (trying to remember the register) that splits the kernel from the user TLB0 which is different from real hardware and qemu (which would not surprise me at all).

Honestly, I suspect this function.  I remember reading something about domains being deprecated in the future and I should set them on a certain way -- I think it was domains anyway.  At any rate, I am setting up the TTLR0/1 registers the way I want and setting the number of bits to 0, so that is not relevant.  But the domains might be.

```
@@
@@ -- Enable the MMU
@@    --------------
MmuEnablePaging:
    mcr     p15,0,r0,c2,c0,0                @@ write the ttl1 table to the TTLR0 register
    mcr     p15,0,r0,c2,c0,1                @@ write the ttl1 table to the TTLR1 register

    mov     r0,#0                           @@ This is the number of bits to use to determine which table
    mcr     p15,0,r0,c2,c0,2                @@ write these to the control register

    mov     r0,#0xffffffff                  @@ All domains can access all things by default
    mcr     p15,0,r0,c3,c0,0                @@ write these to the domain access register

    mrc     p15,0,r0,c1,c0,0                @@ This gets the cp15 register 1 and puts it in r0
    orr     r0,#1                           @@ set bit 0
    mcr     p15,0,r0,c1,c0,0                @@ Put the cp15 register 1 back, with the MMU enabled

    mov     pc,lr
```

---

### 2019-Jan-04

I am not able to find the location where the Domain was supposed to be deprecated.  I must have been mistaken.  In any event all tables are mapped to Domain 0 and that domain allows 'management' level access to the memory.  So this should not be a problem.

So, I need to figure out a way to determine what is wrong.  Right now, I am not sure how I can do this....

Ah hah!!  I think I am onto something...  The following is the line that jumps to the kernel:

```
100528:	e12fff34 	blx	r4
```

In reading the `blx` and the `bx` opcodes, the folling is the what the `blx` instruction will do:

> Branch with Link and Exchange (register) calls a subroutine at an address and instruction set specified by a register.

In particular, the `bx` instruction clarifies a bit more:

> Branch with Link and Exchange Instruction Sets (immediate) calls a subroutine at a PC-relative address, and changes instruction set from ARM to Thumb, or from Thumb to ARM.

So, the instruction set is being changes, meaning it is trying to execute invalid opcodes.  I am not sure what is causing this to happen in the C compiler, but I need to figure out how to change this behavior in the loader.

Well, I changed both architectures to be a static jump to an address in a register and now both architectures break.

---

Well I had to download and build bochs on my laptop.  What a mess!  Anyway, I have a crash report:

```
00193580552i[CPU0  ] CPU is in protected mode (active)
00193580552i[CPU0  ] CS.mode = 32 bit
00193580552i[CPU0  ] SS.mode = 32 bit
00193580552i[CPU0  ] EFER   = 0x00000000
00193580552i[CPU0  ] | EAX=00000000  EBX=0011e498  ECX=00000000  EDX=000003f8
00193580552i[CPU0  ] | ESP=00120f88  EBP=00001c00  ESI=001030ae  EDI=001030ad
00193580552i[CPU0  ] | IOPL=0 ID vip vif ac vm RF nt of df if tf sf zf af pf cf
00193580552i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00193580552i[CPU0  ] |  CS:0038( 0007| 0|  0) 00000000 ffffffff 1 1
00193580552i[CPU0  ] |  DS:0040( 0008| 0|  0) 00000000 ffffffff 1 1
00193580552i[CPU0  ] |  SS:0040( 0008| 0|  0) 00000000 ffffffff 1 1
00193580552i[CPU0  ] |  ES:0040( 0008| 0|  0) 00000000 ffffffff 1 1
00193580552i[CPU0  ] |  FS:0040( 0008| 0|  0) 00000000 ffffffff 1 1
00193580552i[CPU0  ] |  GS:0040( 0008| 0|  0) 00000000 ffffffff 1 1
00193580552i[CPU0  ] | EIP=0010088e (0010088e)
00193580552i[CPU0  ] | CR0=0xe0000011 CR2=0x00001c01
00193580552i[CPU0  ] | CR3=0x00001000 CR4=0x00000000
(0).[193580552] [0x00000010088e] 0038:000000000010088e (unk. ctxt): movzx ecx, byte ptr ss:[ebp+1] ; 0fb64d01
```

Checking the `loader.map`, address `0x10088e` is in `MmuDumpTables()`.  I was able to trace that down to reading the wrong stack position.  i686 works now.

For rpi2b, I am still in the same boat here.

---

### 2019-Jan-05

So I have a couple of thoughts here....  I am most likely getting a fault of some sort, so I really should get an exception handler set up in the loader to be able to identify the exception.  Another thought is that I should be able to emulate what I have written so far on qemu and I can work to get that working there before going back to real hardware.  I have actually taken a step backwards with qemu (meaning the timer was the only thing I am finding is not emulated the way I need).

I think I am going to go with the qemu option first -- I have access to more tools with qemu and will get farther faster.

---

OK with a little effort I was finally able to get qemu working so that I could get an exception printed:

```
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
Data Exception:
At address: 0x80038f8c
 R0: 0x00004208   R1: 0xfb000010   R2: 0x00000000
 R3: 0xfb000002   R4: 0x00003330   R5: 0x800060bc
 R6: 0x800060cb   R7: 0x00003000   R8: 0x00000000
 R9: 0x00122000  R10: 0x000287e7  R11: 0x000254cc
R12: 0xfb000000   SP: 0x00003332   LR_ret: 0x80000234
SPSR_ret: 0x600001d3     type: 0x17

Additional Data Points:
User LR: 0x00000000  User SP: 0x00000000
Svc LR: 0x80038f9c
```

So, the instruction that is generating the fault is `0x80000234`.  This is actually trying to draw on the frame buffer.  Ahhh..  but the other thing is that I am getting serial output in qemu from the kernel and not for the loader.  I should not be getting *any* serial output from qemu.  So, I am really getting farther than I thought on real hardware.  So, I think I know what I need to do:
1. get the frame buffer sorted out in `pi-bootloader`
1. change the serial output to be to the mini-UART in the kernel

Finally a plan of action!!  Sometimes things just don't make sense until you figure them out.

Now, as a note to self: I still do not appear to be able to build an image for rpi on this laptop.

Cleaning up the serial port output reviels the same exception: a Data Abort trying to update the screen.

Now to clean up the frame buffer info....

---

OK, now I have this cleaned up and I am ablt to boot and get into `ProcessInit()`.  I am at least executing my kernel now, which is good.  I want to be able to check the video output as well....  Unfortunately, no video output.  This will need to be the next thing I tackle -- I need to make sure video works.  First to tackle the loader's video output.

---

I'm doing a lot of looking and not a lot of finding.  I think I am going to look at `rpi-boot` to see if I can get the proper output on the screen.  If so, I will use that as a template to pull the info I need.  Actually, `rpi-boot` is not working either.  No screen output.

I think I am going to have to go to the Raspbian source to figure out how this is initialized.

Hmmm... `lk` uses this calculation to convert ARM addresses to VC addresses:

```
(uint32_t)((kvaddr & 0x3fffffff)+0xc0000000)
```

---

Ok, it's time for a purpose-built test to be able to write to the screen.  There are some tutorials from Baking Pi (https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/) that can be used to test.

---

### 2019-Jan-06

OK, I think I have something I can use to move this whole screen thing forward.  I found the following documentation: https://github.com/raspberrypi/firmware/wiki.  This wiki, and in particular this page (https://github.com/raspberrypi/firmware/wiki/Mailbox-framebuffer-interface) states:

> **NOTE: This particular Mailbox call is deprecated, and not guaranteed to work as expected.**

Since I have the latest firmware, I wholely expect that this is broken now and my examples are not yet updated.  Unfortunately, this page just states that the interface is deprecated; it does not grace us with the knowledge that it is deprecated in favors of what other feature.  Genius!

Well, with a little digging, I did manage to come up with the following ARM->VC mailbox which should still work.  It still have the framebuffer interfacing which has not been called out as deprecated (https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface#frame-buffer).  This at least gives me a place to start coding.  Keep in mind, I am working on a purpose-built example to write to the physical screen.

---

I'm still having a hell of a time here.  I looked into lk (https://github.com/littlekernel/lk), and it appears to try to init the framebuffer.  I'm going to try to compile it and test on hardware to see what happens.

---

Well, that is not going to work out like I wanted it to:

```
Discarded input sections

 .text          0x0000000000000000        0x0 ./build-rpi2-test/platform/bcm28xx.mod.o
 .text.gpio_config
                0x0000000000000000       0x48 ./build-rpi2-test/platform/bcm28xx.mod.o
 .text.gpio_set
                0x0000000000000000       0x28 ./build-rpi2-test/platform/bcm28xx.mod.o
 .text.gpio_get
                0x0000000000000000       0x1c ./build-rpi2-test/platform/bcm28xx.mod.o
 .text.mask_interrupt
                0x0000000000000000       0xc0 ./build-rpi2-test/platform/bcm28xx.mod.o
 .text.init_framebuffer
                0x0000000000000000       0xc8 ./build-rpi2-test/platform/bcm28xx.mod.o
 .text.dispflush
                0x0000000000000000        0x4 ./build-rpi2-test/platform/bcm28xx.mod.o
 .text.display_get_framebuffer
                0x0000000000000000       0x3c ./build-rpi2-test/platform/bcm28xx.mod.o
```

... since the function in question never make the final binary!

---

I'm done with this for the night.  I will need to get into raspbian and do a deep trace through that code.  I don't have that in me tonight.

---

### 2019-Jan-07

So I read something today that the BCM2708 and BCM2835 are the same SoC, except the 2835 has RAM on-chip meaning is a complete SoC.  This is significant since I have been ignoring anything with the BCM2708 moniker as not being what I am looking for.  However, when looking through the Raspbian source today, this came up as the only BCM video supported.  A little Googleage and it seems like they are the same.

The defaults for Raspbian for the framebuffer configuration is to ask for 800 bits wide X 400 bits high X 32 bits deep.  It's not my desired config, but I just want *something*.  I will reset to give that a try.

---

OK, progress!!!  With my purpose-built test, I was able to actually get a real value for the framebuffer size:

```
Booting...
Getting the current width/height of the frame buffer
Sending...
Receiving...
.. The width is reported 0x00000720
.. The height is reported 0x000003d8
Getting the current depth of the frame buffer
Sending...
Receiving...
.. The depth is reported 0x00000010
Getting the current pixel order of the frame buffer
Sending...
Receiving...
.. The pixel order is reported 0x00000000
Getting the current alpha mode of the frame buffer
Sending...
Receiving...
.. The alpha mode is reported 0x00000000
Getting the current pitch of the frame buffer
Sending...
Receiving...
.. The pitch is reported 0x00000020
Setting up message to get the virtual FB size
Sending...
Receiving...
.. The frame buffer is reported to be at 0xcf8c3000
.. The frame buffer is 0x00138800 bytes long
```

Anyway, I am still not getting anything on the screen.  I believe that this is because I need to adjust the buffer location.  I believe I need to add `0x40000000` to the frame buffer address to get to the real ARM address. Which works out to be `0xcf8c3000 + 0x40000000 = 0x0f8c3000` in my example.

---

Stupid pointer arithmetic!!

I was doing this:

```
    uint32_t *fb = (uint32_t *)mbBuf[24] + 0x40000000;
```

The real code is this:

```
    uint32_t *fb = (uint32_t *)(mbBuf[24] + 0x40000000);
```

Notice the need for the parenthesis!!  I finally have output on the screen.

First change: can I get the color depth I am looking for?  16 bits....  I think that worked.  The buffer size is half as big:

```
.. The frame buffer is reported to be at 0xcf95f000
.. The frame buffer is 0x0009c400 bytes long
```

Now, my preference is 1024 X 768.  I think I might be pushing my luck there, but I have to try.  Well, I got output.  But I also got some odd results.  The screen appeared to be pinned to the top and clipped on either side.  The geometry works out that there is no partial lines written...  so I cannot be sure.  But the key oddity for me is the buffer location:

```
.. The frame buffer is reported to be at 0x00000000
.. The frame buffer is 0x00180000 bytes long
```

If I add `0x40000000`, I get the location of the timer and BCM2836 extensions....  But I get pixels on the screen....

I think I need to stick with the 800 X 400 screen for now.

Finally, the last change that I had to make which is worth mentioning is that I am putting several tags in the mailbox at once rather than once at a time.  This may have also been a key change since the same settings did not work when I did them individually.  Now to move the changes into the kernel.

OK, with that, I did end up with a problem where clearing the screen from the kernel created a Data Exception before the screen was cleared.  Here's the breakdown of that problem: 800 pixels (bytes) wide by 400 pixels (bytes) high by 2 bytes (16 bits) deep is 800 X 400 X 2 or 640000 bytes or 0x9c400 bytes.  Notice that the ending of the buffer is not frame aligned (like it would be with a 1024 X 768 X 2 display).  I had to change my calculation in `MmuInit()` for the frame buffer to be from `<` to `<=`.  This will create an extra frame mapped for the i686 architecture, so I am going to go fix that.

This now puts me back to a problem with `ProcessInit()`.  I also need to commit my changes since I also tickled the (c) years for the new year.

---

This is the crash report:

```
Initializing the list at 0x800391a9
Data Exception:
At address: 0x80038f90
 R0: 0x00000024   R1: 0x80005bac   R2: 0x800058b8
 R3: 0x00000020   R4: 0x8000589c   R5: 0x800391a9
 R6: 0x800391a9   R7: 0x80039199   R8: 0x80050c0c
 R9: 0x00123000  R10: 0x2411fd81  R11: 0x87b02ffe
R12: 0xfb00c940   SP: 0x80002974   LR_ret: 0x80002974
SPSR_ret: 0x600001d3     type: 0x17

Additional Data Points:
User LR: 0xfff356bf  User SP: 0xdde7aad3
Svc LR: 0x80038fa0
```

In particular, I am looking at the address of the list I am initializing and it is not word-aligned.  This is generating a data exception -- yet another thing in a long line of differences between the qemu emulator that real hardware is not tolerant of.

It turned out that my `Process_t` structure was packed.  I removed that attribute and it works better now -- I am getting the Idle Process structure initialized.  Then an undefined operation.

```
Undefined Instruction:
At address: 0x80038f90
```

The map shows the following:

```
80002a40:	f2c00010 	vmov.i32	d16, #0	; 0x00000000
80002a44:	e30930c0 	movw	r3, #37056	; 0x90c0
```

Which I think means that I am trying to execute the opcode `0xf2c00010` as the disasm does not look valid to me.  When I dump the object file, I see the following:

```
 10c:   ebfffffe        bl      0 <_Z7kprintfPKcz>
 110:   f2c00010        vmov.i32        d16, #0 ; 0x00000000
 114:   e3003000        movw    r3, #0
```

So, the instruction is there as well.  Well, it turned out to be a floating point code problem, which was fixed by adding the following parameter into the compile flags:

```
CFLAGS += -marm -mfpu=vfpv3 -mfloat-abi=softfp
```

Specifically the last 2 flags above.  Since I have 1 flag on one line, I am going to reformat these and then recompile.  Actually, no recompile is necessary -- the reformatting of the lines means that that the file changed on save, but the actual build commands did not.  `tup` evaluated that and realized there was nothing to do.

---

OK, time to take stock in where I am at...  Here is a snip from my `kInit()` function:

```C
    //
    // -- Phase 2: Required OS Structure Initialization
    //    ---------------------------------------------
    ProcessInit();
//    for (int i = 0; i < GetModCount(); i ++) {
//        char *s = GetAvailModuleIdent(i);
//        if (s[0] == 'p' && s[1] == 'm' && s[2] == 'm' && s[3] == 0) {
//            PmmStart(&localHwDisc->mods[i]);
//            break;
//        }
//    }
    TimerInit(250);
    EnableInterrupts();
    ProcessEnabled = true;
    while (1) {}
    HeapInit();
```

I currently have commented out the search for and the initialization of the PMM module (which is not compiling at the moment).  I also have a hard-stop with an infinite loop before I initialize the heap.  I am getting interrupts:

```
OISR Pending: 0x00000000
ERROR: Unable to determine IRQ number: 0x00000000
OISR Pending: 0x00000000
ERROR: Unable to determine IRQ number: 0x00000000
```

... which I expect to be related to the timer firing (well, it is enabled).  So, what to work on next?

I think I am going to work on resolving the ISR Pending and addressing getting the timer right.  I will need that before I can initialize the PMM process, and there is no point is moving on the the heap until I have a properly running PMM.  Dealing with the IRQ now will save me trouble later on.

You know, in order to do that, I'm going to have to go back and revisit my timer implementation.  I do not have that in me tonight.

---

### 2019-Jan-08

So, after some thinking on this, I think I have a spurious interrupt.  However, I am not able to determine from where it came.  What I do need to do is figure out how to trap that and then reset the interrupt flags somehow.

OK, after reviewing the `lk` code, I see now that I am looking at the wrong interrupt controller -- I need to look at the one per CPU.  I was not.  But this also brings up the point that I am setting this all up for a single CPU, not multiple CPUs.  Everything so far is coded to be staticly addressing one CPU.  This will obviously need to be updated when I launch the other CPUs.  I am going to capture this as a Redmine so that I do not forget.  If you are playing along at home, the issue is http://eryjus.ddns.net:3000/issues/380.

So, with that, I now need to update my IRQ number determination code.  That done, I'm back to a Data Exception:

```
Executing interrupt 0xb at 0x80003a38
.Data Exception:
At address: 0x80036f50
 R0: 0x00000000   R1: 0x00000000   R2: 0x800370c0
 R3: 0xffffffff   R4: 0x80003f44   R5: 0x001030c8
 R6: 0x00120000   R7: 0x00000000   R8: 0x00000000
 R9: 0x00123000  R10: 0x440a359f  R11: 0xd7802bfe
R12: 0x00000001   SP: 0x80003a48   LR_ret: 0x80003aa8
SPSR_ret: 0xa00001d3     type: 0x17

Additional Data Points:
User LR: 0xefefcdfe  User SP: 0xa7ff6cf9
Svc LR: 0x80036f60
```

This exception is in the timer callback function.  Hmmm...

```C
void TimerCallBack(UNUSED(isrRegs_t *reg))
{
    kprintf(".");
	if (!ProcessEnabled) {
        timerControl->eoi(0);
        return;
    }
```

I notice I did get the `'.'` on the screen.  It turns out I was not initializing the `timerController` variable for the rpi2b.  Having corrected that, it appears I now have an infinite loop trying to get a spinlock:

```
Timer is initialized
ISR Pending: 0x00000800
Determined IRQ to be 0xb
Executing interrupt 0xb at 0x80003a38
.Looking for a new process
Attempting lock by 0x1 at address 0x80037208
```

And, in short, this is going to be a serious repeat of the scheduler debugging I had to do with the i686 architecture.  But the good news is I have progressed to the point where I am able to focus on this step.  It was definitely rough getting here.

So, it appears that my problem is going to end up being related to the atomic code to obtain a spinlock, found in `SpinlockAtomicLock.s` for the rpi2b arch -- I have a bad implementation.  No kidding, right?  What a damned mess!

```
@@
@@ -- Perform the compare and exchange
@@    --------------------------------
SpinlockAtomicLock:
    push    {r3,fp,lr}                      @@ save the frame pointer and return register, be nice and save r3
    mov     fp,sp                           @@ and create a new frame

loop:
    strex   r3,r2,[r0]                      @@ attempt to lock the spinlock
    cmp     r3,r1                           @@ did we get the lock?
    bne     loop                            @@ if not, try again

    mov     sp,fp                           @@ restore the stack
    pop     {r3,fp,lr}                      @@ and the previous frame and return register
    mov     pc,lr                           @@ return
```

Again, what the hell was I thinking???

OK, I think I have the code straightened out, but I am still not getting the results I want.  Time to look at a reference solution.

---

I had to use the `lk` spinlocks as a template to help me out on this.  But now I am back to a Data Exception:

```
Attempting lock by 0x1 at address 0x8003b208
Data Exception:
At address: 0x8003af30
 R0: 0x8003b208   R1: 0x00000000   R2: 0x00000001
 R3: 0x00000020   R4: 0x8003b200   R5: 0x80072c0c
 R6: 0x8000515c   R7: 0x8003b0c0   R8: 0x8003b208
 R9: 0x00123000  R10: 0x8003b208  R11: 0x8003af80
R12: 0x00000001   SP: 0x80003018   LR_ret: 0x800041c4
SPSR_ret: 0x600001d3     type: 0x17

Additional Data Points:
User LR: 0xafefcdfe  User SP: 0xa7fb6cf9
Svc LR: 0x8003af40
```

... which happens to be in the spinlock code:

```
800041c4 <loop>:
800041c4:	e1901f9f 	ldrex	r1, [r0]
800041c8:	e3510000 	cmp	r1, #0
```

I picked a different solution and I am still getting a Data Abort with the `ldrex` instruction.  I am looking at the ARMv7ARM and the instruction does not indicate *why* it would generate a data abort, just that it can.

---

### 2019-Jan-09

I thought I would give one more try before I gave up completely.  I made changes to the flags for the MMU.  This ended up being the result:

```C
    ttl2Entry->frame = frame;
    ttl2Entry->s = 1;
    ttl2Entry->apx = 0;
    ttl2Entry->ap = 0b11;
    ttl2Entry->tex = (krn?0x001:0b001);
    ttl2Entry->c = 1;
    ttl2Entry->b = (krn?1:1);
    ttl2Entry->nG = 0;
    ttl2Entry->fault = 0b10;
```

The changes were to the `c` flag (made it `1`), the `b` flag (made it `1` for both cases from '0' for `!krn`), and the `tex` flag (made it `0b001` for `krn` instead of `0b000`).  This gets me to the point where the kernel stops processing:

```
    TimerInit(250);
    EnableInterrupts();
    ProcessEnabled = true;
    while (1) {}            // <-- to here
    HeapInit();
 ```

However, I cannot be certain 100% as the last thing I see is:

```
.Looking for a new process
Attempting lock by 0x1 at address 0x8003b208
Lock at 0x8003b208 released
The current process is still the highest priority
```

I am expecting to see more `'.'` characters.  With a number of debugging `kprintf()` calls, I was able to determine that the timer interrupt is never actually returning properly.

Lots of debugging code later, there is something that is happening when I call function from a table -- the `TimerCallBack()` function.  When I comment out this call, the IRQ works:

```C
    kprintf("Determined IRQ to be %x\n", intno);
    if (isrHandlers[intno] != NULL) {
        isrFunc_t handler = isrHandlers[intno];
        kprintf("Executing interrupt %x at %p\n", intno, handler);
//        handler(regs);
    }
    kprintf("handler handled\n");
```

Now, when I comment out the contents of the `TimerCallBack()` function, this works as well.  I wonder if the problem is the `eoi()` function call...??  And that appears to be it -- something is happening in the `eoi()` call...

---

OK, here is the line of code that is creating the problem:

```C
//
// -- Issue an EOI for the timer
//    --------------------------
void TimerEoi(UNUSED(uint32_t irq))
{
    kprintf("eoi start\n");
//    MmioWrite(TMR_BASE + 0x38, (1<<30) | (1<<31));  // clear and reload the timer
    MmioWrite(TMR_BASE + 0x38, (1<<30));  // clear and reload the timer
    kprintf("eoi exit\n");
}
```

The commented line is causing it.  When it is commented out, as above, the timer continues to fire.  The line below it was a test to see if a certain bit created the problem.  This line creates the proble with both `(1<<30)` and `(1<<31)`.  So, now I have to wonder if the reset is needed or not.  I should be able to slow down the timer significantly and check if I get a rapid-fire interrupt or not.

---

### 2019-Jan-16

Well, it's been a week.  I have been away from home and therefore away from my monitor.  Since I have to test my code on real hardware, I was working on `cemu` while I was away and I had time.

Coming back to this problem, I do not think I was able to effect any change in the rate of the interrupts by scaling.  Once the interrupt fired, it was not resetting and when I reset it the pi locks up.

I'm going to try to clear out some of the debugging code that is outputting to the serial port to see if that helps with response time....  This gives me reasonable response time.  So, what I want to do now is to output some character in the `kinit()` loop to see if I can ever get that to output.  If I do not, then I know the CPU is completely consumed in interrupt handling.

Since I never get any of the other characters, I must assume that the CPU is being consumed with interrupts.

---

I may be using the wrong timer interface, or maybe not using the *best* timer interface.  There might be an optional ARM timer on the SoC accessible through the `cp15` registers that Linux appears to use -- so I want to test for its existence....

OK, so it exists and it appears to have a frequency:

```
IsrHandler registered
The ARMv7 System Timer Frequency is: 0x124f800
Timer is initialized
```

`0x124f800` is 19.2 MHz.  This sounds reasonable for a frequency.  I have some reading to do.

---

Finally!!  My EOI routine looks like this:

```C
//
// -- Issue an EOI for the timer
//    --------------------------
void TimerEoi(UNUSED(uint32_t irq))
{
    uint32_t reload = MmioRead(TMR_BASE + 0x34) & 0x0fffffff;
    MmioWrite(TMR_BASE + 0x34, reload); // disable the timer
    MmioWrite(TMR_BASE + 0x38, (1<<30) | (1<<31));  // clear and reload the timer
    MmioWrite(TMR_BASE + 0x34, reload | (1<<28) | (1<<29)); // re-enable the timer
}
```

and I am getting periodic interrupts:

```
############################################################################################################################.eoi start
eoi exit
############################################################################################################################################################
```

Now, to clean up this mess I made a week ago!!

---

I have that code committed.  Now, what is next?

Get the PMM module operational, of course!  ... which is going to need a non-privileged mode and system calls.  This means a whole new set of compile problems, crash problems, and and just general problem with supid problems.  This should be fun.  However, the thing I have going for me is that this really should be architecture independent.  We'll see how this prediction turns out...!!

I'm going to start with libc as that will be completely architecture dependent -- and there are only 2 functions so far.  It will also be needed to link the `pmm.elf`.

---

I managed to get `libc` and `pmm.elf` to both compile tonight.  Tomorrow, I will need to work on the rpi2b kernel syscall components -- which needs to include a determination of what the system call function number really is and the contents of the parameters.  Overall, though, I am happy with the progress today, especially considering I haven't touched this code in a week.

---

### 2019-Jan-19

Actually, I double checked today and the `pmm.elf` was never set up to link.  So I am taking that on first thing.  I find that the `memset()` function is not available to me in `libgcc.a` like it is in i686....  I have the `-lgcc` parameter passed to the linker and the path set for the library and it is still complaining.  So, it looks like I will be providing my own.

---

Extract from http://gcc.gnu.org/onlinedocs/gcc/Standards.html:

> Most of the compiler support routines used by GCC are present in libgcc, but there are a few exceptions.  GCC requires the freestanding environment provide memcpy, memmove, memset and memcmp. Finally, if __builtin_trap is used, and the target does not
implement the trap pattern, then GCC will emit a call to abort.

OK, so I need to provide my own in my `libc`.  This is easy enough and since only `memset()` is being referenced at the moment, I will start with that.  Which addresses the concern.  Now, on to the system calls....

In my system call code, I will need to determine the function number for the call and hang onto that for the jump table.  This is found in the instruction that issued the system call (or `svc` opcode), so I am going to have to read code from the handler to determine this.  I *should* be OK to do this because the original mmu tables for the running process issuing the system call are not changed and I really need the kernel to have access to the user space.

The first thing to deal with is the supervisor handler jump target, which was a cookie-cutter copy from the other handlers.  This is a concern because it tries to put the system into supervisor mode, which it already is!  ARM has some sample code (albeit superceeded) which should help me (http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0203j/Cacdfeci.html).  The challenge is going to be whether or not I change the format of the `isrRegs_t` structure with this change!

---

OK, I have everything compiling again....  I was able to leverage the interrupt type field for the system call function number.  This leaves the `isrRegs_t` structure intact.  On top of that, I was able to create some defines by architecture to handle the system call in a single source set so I do not have to rebuild them for each architecture.

---

Trying to send the `pmm.elf` to the rpi, I am having trouble with the modules and addresses.  In particular, I am getting an invalid ELF signature.

So, here is what I have:

```
Module information present
   Found Module: kernel.elf
    .. Name is at : 0x0x000fffbc
    .. Start: 0x00123000
    .. End: 0x00165ff8
   Found Module: pmm.elf
    .. Name is at : 0x0x000fff9a
    .. Start: 0x00165ff8
    .. End: 0x0018d860
```

Looking at this, the ending address is not correct.  This is from the MB information structure that is passed from `pbl-server`.  I will need to go there to debug this issue.  It ended up being a quick fix.

This now puts me at the point where I need to imnplement the `PmmStart()` function for rpi2b.  This function will start the pmm module in user space.  This is going to take some time to build since I not only need to get processes working but I need to get into a non-privileged mode.

The first thing is to move the pmm to the proper link location (in user space).  When I do this, the loader crashes.  I am not sure why.

---

### 2019-Jan-20

I need to figure out how to link the `pmm.elf` to the proper location and get it to load properly.  Since it is copied over as an uninterpreted binary file, the `pbl-server` should not have anything to do with this.  I also believe I am having trouble in the loader, which it should not be.  However, with the split paging tables, this may have something to do with it and I may have to re-architect how and when I parse the user-space paging tables.

---

Reviewing the output, I observe that I have memory map conflicts.  What bothers me about this is that the pmm should have its own map table -- so it looks like I am updating the main kernel mmu structures which would make sense since the first 1MB is identity mapped.  At the same time, I should have a separate user-space buffer for this code.  I'm going back to my pevious statement that I need to re-architect how I am setting up the user-space paging tables.  It's a complete mess and totally wrong.

First, I need to update the linker script to set aside the right amount of space for the user-space TLB1 table.  This needs to be 8K + 16 bytes.  This structure appears just past the stack, so I might be able to steal from the stack space to get this done.

OK, I have been doing *this* wrong:

```
    .stack : {
        BYTE(4080)
    }
```

They `BYTE` indicates that I need to store a byte and the value is parenthesis is the value to store -- not the number of bytes to store!  Well, all my linker scripts are wrong and I need to clean those up.  I'm going to take care of this before I return to `pmm.ld`.

After cleaning this up, the following is the resulting script section:

```
    .stack : {
        LONG(0)
    }
    . += 4076;
    _stackTop = .;

    _pageTableStart = .;
    .ptable : {
        LONG(0)
        LONG(0)
        LONG(0)
        LONG(0)             /* this is the TTL1 table */
    }
    . += 8192;              /* reserve space for the TTL2 tables */
    . = ALIGN(4096);
```

`readelf -a` confirms I have the data correct:

```
Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .stack            PROGBITS        00001000 001000 000004 00  WA  0   0  1
  [ 2] .ptable           PROGBITS        00001ff0 001ff0 000010 00  WA  0   0  1
  [ 3] .text             PROGBITS        00004000 002000 000298 00  AX  0   0  4
  [ 4] .stab             PROGBITS        00005000 003000 0029c5 00   A  0   0  4
  [ 5] .data             PROGBITS        00008000 006000 020004 00  WA  0   0  4
  [ 6] .ARM.attributes   ARM_ATTRIBUTES  00000000 026004 000035 00      0   0  1
  [ 7] .symtab           SYMTAB          00000000 02603c 0004d0 10      8  56  4
```

But the program load headers are more complicated than I am prepared to handle:

```
Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x00001000 0x00001000 0x01000 0x01000 RW  0x1000
  LOAD           0x002000 0x00004000 0x00004000 0x039c5 0x039c5 R E 0x1000
  LOAD           0x006000 0x00008000 0x00008000 0x20004 0x20004 RW  0x1000
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x10
```

So, getting a little more pedantic, I get:

```
    .stack : {
        LONG(0)
    }
    . += 4076;
    _stackTop = .;

    _pageTableStart = .;
    .ptable : {
        LONG(0)
        LONG(0)
        LONG(0)
        LONG(0)             /* this is the TTL1 table pointers */
        LONG(0)             /* this is the first of the TTL1 tables */
    }
    . = ALIGN(4096);
    .ptable2 : {
        LONG(0)             /* this is the second TTL2 tables */
    }
    . = ALIGN(4096);
```

Which results in:

```
Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .stack            PROGBITS        00001000 001000 000004 00  WA  0   0  1
  [ 2] .ptable           PROGBITS        00001ff0 001ff0 000014 00  WA  0   0  1
  [ 3] .ptable2          PROGBITS        00003000 003000 000004 00  WA  0   0  1
  [ 4] .text             PROGBITS        00004000 004000 000298 00  AX  0   0  4
  [ 5] .stab             PROGBITS        00005000 005000 0029c5 00   A  0   0  4
  [ 6] .data             PROGBITS        00008000 008000 020004 00  WA  0   0  4
  [ 7] .ARM.attributes   ARM_ATTRIBUTES  00000000 028004 000035 00      0   0  1
  [ 8] .symtab           SYMTAB          00000000 02803c 0004e0 10      9  57  4
  [ 9] .strtab           STRTAB          00000000 02851c 00017e 00      0   0  1
  [10] .shstrtab         STRTAB          00000000 02869a 000055 00      0   0  1
```

And the program headers:

```
Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x00001000 0x00001000 0x27004 0x27004 RWE 0x1000
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x10
```

Which are the proper headers for my architecture.  This is cleaned up.

So the next thing is going to be to update the `ModuleInit()` to handle the extra paging tables.  This currently not architecture dependent, so I will need to consider how to make these changes.

---

### 2019-Jan-22

Well, I couldn't bring myself to work on this yesterday -- I was too depressed over the problems to solve.  They are quite a few. I need to start with `ModuleInit()` and focus in particular on the 2 sets of paging tables.  This may require a redo of the loader by architecture.

---

As a digression..., I really have spent a lot of time thinking about how https://github.com/littlekernel/lk is organized.  I really like the structure where the architecture (or `arch`) is separated into one set of folders and the chipset or SoC (or `platform`) is another set of folders.  There are lots of additional things that can be broken out, but that basic structure appeals to me -- more than what I have today for sure.  With this, any given system (or `target`) can be nothing more than the proper combination of `arch` and `platform` -- maybe with optional components enabled or disabled at compile time or even runtime.  So, I have prepared version 0.3.0 in Redmine to support this effort, which is next after rpi2b (see: http://eryjus.ddns.net:3000/versions/7).  At the same time, I may need to eliminate the specific `loader` and `kernel` modules and just take care of everything in one place.

---

OK, that's enough about what is coming down the road.  Time to focus.

The problem at hand here is that rpi2b has 2 top-level ttl1 tables -- one for kernel space and one for user space -- and my `MmuMapToFrame()` function only takes one parameter.  Ultimately, I will need to either figure out a way to determine the correct one to update before calling `MmuMapToFrame()` or I will need to create an architecture-specific structure to hold the MMU top level address(s) for each architecture.  I really think the latter is the best solution to take on at this point since it better aligns with what I would like to do later in `v0.3.0`.  Don't get me wrong: I am not starting yet.  I am specifically focused on getting rpi2b working, but I do not want to knowingly write a bunch of code that will be tossed in a few months.

So, continuing to walk this backward, I need to define what the MMU-critical data is for any given process (including the kernel).  Well, for the i686 arch, this will be the `cr3` value.  For the rpi2b, it is going to be the `ttbr0` and `ttbr1` values.  In the case of rpi2b, the `ttbr1` value is used for the kernel stuff.

Another element to consider for the rpi2b is the virtual addresses of the frames that hold the `ttl1` structure.  There are 4 frames for the `ttbr1` register (since it is also the `ttbr0` structure for the kernel) and another 2 frames for the `ttbr0` register.  One key thing to keep in mind is that for the kernel these addresses will be the same.

My goal with this `v0.2.0` effort is that I do not want to have to severely change the i686 running kernel (I will save that for `v0.3.0`), but just do what I need to do to get the rpi2b target working.

So, starting with rpi2b (since that is my problem at the moment), what does this structure need to look like?

```C
typedef struct {
    ptrsize_t ttbr0;        // this will be the user-space mappings (8K in length)
    ptrsize_t ttbr1;        // kernel-space (global, in case it is not clear; 16K and may == ttbr0)
} MmuData_t;
```

These will need to be the physical addresses of the structures, not the virtual ones.  Now for i686, the following should suffice:

```C
typedef ptrsize_t MmuData_t;
```

These changes should only need to occur in the `mmu` section of code for the loader.  There will be some fall-out in `ModuleInit()` but I believe everything should be isolated to that additional source.  I do not yet believe I have a problem with `kernel`, but that doesn't mean anything -- I have not even looked at it.

---

### 2019-Jan-23

Today I need to get into the nuts and bolts of this change.  First and foremost, I want to get the i686 target changed and fully working again before I take on the rpi2b changes.

---

I was able to modify the i686 code and still get it to run properly.  So, success there....

Now, to change the rpi2b type back to the structure (which I matched the i686 `typedef` to get everything to compile) and the necessary debugging.

---

### 2019-Jan-25

OK, I was able to get both architectures to compile fully again.  I was able to run the i686 architecture and nothing broke in my efforts.  So this is a good thing so far.

rpi2b on the other hand, I expect that I have taken a **huge** step backwards.  And it is as I feared.  I really need to start over from the beginning of the loader to start putting the running components back together.  I am back at the loader's `PmmInit()` function.

I take it back -- it's `MmuInit()` that needs work, which makes more sense.

I have gotten caught up to the point where I am trying to enable paging again.  This is causing a crash and is likely an unmapped page.  I disabled the update to the bit to enable paging and I am able to get past that point.

I am on to something!!  I took `pmm.elf` out of the `cfg-file` and re-enabled the paging bit, and I am getting past where I was -- in fact, the kernel is trying to schedule again.  This is good.

So, something is going on with the mapping of the `pmm.elf` is my suspicion.  Before I take that on, I am going to try to split the tables and make sure I am mapping the correct one in `MmuMapToFrame()`.  Now, with the bit set to 1, I am still getting the scheduler trying to reschedule.  So, now to clean up the `MmuMapToFrame()` function....

All caught back up.  I can now focus on getting `pmm.elf` to load and get its `ttbr0` populated properly.

---

In `ModuleInit()`, the ttbr0 needs 2 frames, not 1.  And, they must be consecutive frames.  And, they need to be 64K aligned.

```C
            modMmu.ttbr0 = PmmFrameToLinear(PmmNewFrame());
            modMmu.ttbr1 = GetMmuTopAddr().ttbr1;
```

... I guess I have a but of clean-up to do with this as well.

---

### 2019-Jan-26

OK, I'm a damned idiot!!!  I finally figured out the reason the system would lock when I was trying to start the PMM!!!

```C
void PmmStart(Module_t *pmmMod)
{
    kprintf("Start the PMM process here in rpi2b\n");
//    Halt();
}
```

I commented the offending line and (shocker of all shockers!!) it worked.

OK, so now back to last night's discovery...  I need to make sure that I allocate 2 consecutive frames for the ttl1 table.

I am going to have to do another MMU virtual address mapping -- but I do not have it in me tonight.  I'm not sure why, but I am just not overly motivated today.

Actually, I have decided that it would be good to keep this documented in a wiki.  My first instinct is to put this in Redmine and have it available through eryjus.ddns.net...  However, I also have a wiki available in github.  Since I am maintaining my bugs and TODOs in Redmine, I am going to use that for the wiki as well.  I may move that at a later date.

---

### 2019-Jan-27

Today I am going to get into the VMM maps for all the different processes -- well for the kernel and the PMM in particular.  I am going to start with the i686 memory map and then I will change to the rpi2b memory map.  The key takeaway (I expect) is going to be that they are very different and will need to be aligned.  I certainly am having trouble keeping track of anything which is generating problems for me in getting the rpi2b pmm operational.

---

As I was writing the rpi2b High-Level Virtual Memory Map wiki, I have a thought cross my mind: What if I did not use the dual memory maps for rpi2b and stuck with just a single map?  It would certainly simplify things and would allow me to keep the memory map more consistent.  I posed the question on `freenode#osdev`:

```
[08:34] <eryjus> for arm, there are 2 sets of paging tables -- one for user processes and one for common kernel pages -- with the intent to improve performance.  in practice does anyone actually use this feature in your own kernels or do you find it too fussy to mess with?
[08:52] <bcos> eryjus: Sounds like a good feature to me (how else would you separate user-space from kernel-space?)
[09:13] <eryjus> bcos: same as you would with x86 -- copy the middle layer tables into the top level for the shared kernel in each user process.  it's an optional feature but creates another layer of complexity I'm wondering if it is worth the effort.
[09:14] <bcos> 80x86 has a "global" flag so that the CPU doesn't throw away all the TLB entries for kernel when you change virtual address space
[09:14] <bcos> (in page table entries)
[09:16] <bcos> I'm guessing (on ARM) there's no "global" flag; so the extra set of paging tables does the same thing (prevents TLB entries being thrown away for no reason), and might also give you more virtual address space to work with
[09:20] <eryjus> there is a not-global flag which is (somewhat) equivalent. IIRC, it looks at that with the buffered and cached flags to make the same determination.
[09:21] <bcos> Hrm. Does the extra/2nd set of page tables give you more virtual address space?
[09:22] <froggey> a whole extra bit
[09:22] <eryjus> no -- same address space.
[09:23] <bcos> Same address space that's twice as large (e.g. highest bit of virtual address selects which set of page tables)?
[09:23] <froggey> that's what I thought, but maybe not?
[09:24] <eryjus> there is a register which indivates how many (MS) bits to compare to 0, and if they are all 0 then the "user" page tables are used else the "kernel" page tables are used.
[09:24] <bcos> I thought ARM had variable page sizes - might depend on which page size you use
[09:24] <eryjus> *indicates
[09:24] <eryjus> AFAIK, that's unrelated to these tables -- or rather independent of them
[09:26] <bcos> "optional feature" means some CPus don't support it?
[09:26] <bcos> (or just means that OS doesn't have to use it?)
[09:28] <eryjus> the OS does not have to use it -- which is my question.  Does anyone actually take advantage of this in their kernel or do they find it too fussy to take advantage of?
[09:31] <bcos> I'm thinking that (if there no other benefits) separate page tables would still make kernel code cleaner and more efficient (easier to update kernel space without having to update every "user+kernel" address spaces)
[09:31] <bcos> ..so I'm figure out why a kernel wouldn't use it (other than "developers were lazy" or ...)
[09:37] <eryjus> "TTBCR can be programmed so that all translations use TTBR0 in a manner compatible with architecture versions before ARMv6."
[09:37] <Brnocrist> separate for user and kernel space, you mean?
[09:37] <eryjus> bcos: ^
[09:38] <bcos> The other thing to consider is whether it helps for melt-down
[09:38] <eryjus> from ARM-ARMv7-A Example B3-1 for the record...
[09:38] <bcos> Not sure if you care about compatiblity with architecture versions before ARMv6
[09:38] <bcos> (half probably don't have MMUs so... ;-)
[09:39] <eryjus> true -- not really concerned about that (yet anyway)
[09:39] <eryjus> bcos: please define melt-down...
[09:40] <eryjus> nvm -- found it
[09:41] <bcos> For Intel; attacker reads kernel data then does something that depends on the value read (e.g. uses value in a table lookup to cause a difference in what is/isn't cached); then the CPU checks if the original access was allowed and triggers an exception after the difference has been caused
[09:41] <bcos> For ARM, not sure (I thought ARM wasn't effected but apparently some ARMs are)
[09:44] <Brnocrist> no MMU no page tables side channel problems!
[09:44] <Brnocrist> :)
[09:45] <bcos> No MMU probably means no way to prevent user-space from doing whatever it likes to kernel = no security at all
[09:46] <bcos> (which is fine for a CPU embedded in a toaster or something, where the software is in ROM and you can't install third-party software, but..)
[09:49] <eryjus> bcos, going back to whether it helps with meltdown, neither the cortex-A7 nor the cortex-A53 are susceptible (according to wikipedia anyway) and these are the 2 I am targeting for now....
[09:51] <bcos> What's the difference between cortex-A53 and cortex-A73 (from perspective of OS)?
[09:52] <Brnocrist> bcos: yeah, I was j/k
[09:53] <eryjus> IDK..  still learning ARM
[09:54] <bcos> Looks like they added a few new features/instructions (half-point floaty point, atomics, virtualisation extensions); but I'd be tempted to suspect that the majority of code written for cortex-A53 would run "as is" on cortex-A75
[09:55] <bcos> *half-precision
[09:56] <bcos> ^ what I'm thinking is that I'd be tempted to target "any ARMv8" eventually; and (because cortex-A75 is vulnerable to meltdown) that means trying to figure out how to mitigate efficiently
[09:58] * eryjus nods
[10:00] <eryjus> a53 is in-order whereas the a73 is out-of-order (which in the limited time I have looked at this seems to be the key distinction for the vulnerability)
```

There are a couple of takeaways from this conversation:
* The RPi is not vulnerable to either meltdown or Spectre, but that does not mean I should not consider it.
* The Cortex-A7 does not require that I use the both `TTBRx` registers (which I knew before but was reinforced today).

I think I am going to revert back to a single `TTBRx` register for ARM at this point.  It will be a good enhancement to add at a later point, but since my goal at this point is a leteral move from i686 to rpi2b, I will stay with the single register (call me lazy...).  However, the first order of business today is to get the Virtual Memory Map properly documented.

Geist also chimed in on `freenode#osdev`, and it was his/her input I was looking for:

```
[13:00] <geist> bcos: microarchitecturally they're quite different
[13:00] <geist> as eryjus points out, a53 is in order, dual issue, whereas a73 is a proper OOO superscalar processor
[13:00] <geist> as far as cpu features, a53 implements v8.0 and a73 is I believe v8.1
[13:01] <geist> as in they're both v8, but there are a set of optional features that are commonly rolled up into the .1, .2 .3 spec
[13:01] <geist> actually, now that i think about it, a72 and a73 are i think v8.0 as wel
[13:01] <geist> since you are intended to be able to pair an a72 or a73 with a a53
[13:01] <geist> a55 pairs nicely with an a75, which both implement v8.1
[13:02] <geist> basically a53 and a73 are more orless identical from a software point of view, though micrroarchitecturally they're quite different, and the a72 or a73 will perform roughly 2 or 2.5x
[13:03] <geist> eryjus: to go back, yes absolutelly everyone uses the split page table thing
[13:03] <geist> i think it's required for 64bit if you want to use the 'high' addresses
[13:05] <geist> re: the larger address space with the split in 64bit, you get 2 more bits than x86-64 specifically
[13:05] <geist> ie, each side is a full 48 bits
[13:06] <geist> 0000.xxxx.xxxx.xxxx and ffff.xxxx.xxxx.xxxx
```

However, for the time being, I am going to stick with a single page table.  v0.3.0 is geared to better separting the architectures and platforms and I may add in the extra page table with that split.  I think I will use that version (or the next one after that) to better align my kernel to the way it should be on ARM.

---

Back to debugging..., I have figured out that the i686 has the following line in `MmuInit()`:

```
    MmuMapToFrame(mmuBase, GDT_ADDRESS, PmmLinearToFrame(0x00000000), true, false);
```

... where `GDT_ADDRESS` is:

```
const ptrsize_t GDT_ADDRESS = 0xff401000;
```

Well, this mapping forces a Page Table to be created at `0xffffd000`.  This way, when I get to the `PmmInit()` function, the Page Table exists in the Page Directory and I do not need to create a new Page Table.

Now, contrasting this with rpi2b, there is nothing in the `MmuInit()` function which would trigger the creation of the ttl2 tables at `0xffffd000`.

I have added a step to create these ttl2 tables:

```
    // -- we also need to "goose" the system into creating this ttl2 table since we will need it later.
    MmuMakeTtl2Table(mmuBase.ttbr1, 0xff400000);
```

So, without the line above, the boot sequence looks like this:

```
Jumping to the kernel, located at address 0x800049d4
#.Mode is: 0x13
Welcome to CenturyOS -- a hobby operating system
    (initializing...)
Initializing the idle process structure
Initializing the list at 0x8003d1ac
Initializing the list at 0x8003d19c
Done
Creating the "magic stack" for the idle process
Idle Process initialized
Butler Process initialized
PmmStart(): installing module for pmm.elf, located at 0x00167000
PmmStart(): calculated pageTtl2Frame to be 0x00000169; tos to be 0x00000168
PmmStart(): Setting up the tables to be managed
PmmStart(): Mapping addr 0xff430000 to frame 0x000001c8
Data Exception:
At address: 0x8003cf98
 R0: 0x00000038   R1: 0x80005c70   R2: 0xffffd0ff
 R3: 0x00000020   R4: 0x00000169   R5: 0x000001c8
 R6: 0x00120000   R7: 0x00103084   R8: 0x00000000
 R9: 0x00123000  R10: 0x2411fd89  R11: 0x87b027fe
R12: 0x8003d078   SP: 0x80001db0   LR_ret: 0x80001db4
SPSR_ret: 0x600001d3     type: 0x17

Additional Data Points:
User LR: 0xdffb56bf  User SP: 0xdde7aad3
Svc LR: 0x8003cfa8
```

With that line in place, I get the following:

```
Jumping to the kernel, located at address 0x800049d4
#.Mode is: 0x13
```

So, I am most likely getting some sort of fault when the kernel gets control.  The most likely culprit is the follwing:

```C
//
// -- This is the location of the exception vector table
//    --------------------------------------------------
const ptrsize_t EXCEPT_VECTOR_TABLE = 0xff401000;
```

This page should be mapped in the Page Tables and I probably overwrote the TTL1 entry.  And I believe that to be the case.

So, back to the drawing board here.

---

With several additional tests, I am rather convinced my problem is not with `MmuInit()` missing something but instead with the `MmuMapToFrame()` not mapping the management tables properly.

---

### 2019-Jan-28

The goal today is to get the `MmuInit()` function working properly, which means making sure `MmuMapToFrame()` is working right.  In particular, the point is to make sure that the management tables are updated properly when I create a new Ttl2 table.  This is going to require that I halt the booting process ahead of launching the kernel.

---

OK, once again, what the hell was I thinking???

```C
//
// -- This is the location of the TTL1/TTL2 Tables
//    --------------------------------------------
const ptrsize_t TTL1_VADDR = 0x80400000;
const ptrsize_t TTL2_VADDR = 0x80000000;
```

This puts the paging tables overwriting the kernel!  From [The High-Level Memory Map](http://eryjus.ddns.net:3000/projects/century-os/wiki/High-Level_Virtual_Memory_Map), I want to have these structures in the `0xff400000` address area.

---

After updating the wiki and the code, I am now getting the following results for the TTL1 table mapping:

```
Set up the TTL1 management table
   location: 0x0003e000
   ttbr0 & ttbr1 are both: 0x3e000000
Mapping address 0xff404000 to frame 0x0003e000
  Ttl1 index is: 0x3e000000[0x00000ff4]
  Ttl2 location is: 0x3e004000[0x00000004]
Mapping address 0xff405000 to frame 0x0003e001
  Ttl1 index is: 0x3e000000[0x00000ff4]
  Ttl2 location is: 0x3e004000[0x00000005]
Mapping address 0xff406000 to frame 0x0003e002
  Ttl1 index is: 0x3e000000[0x00000ff4]
  Ttl2 location is: 0x3e004000[0x00000006]
Mapping address 0xff407000 to frame 0x0003e003
  Ttl1 index is: 0x3e000000[0x00000ff4]
  Ttl2 location is: 0x3e004000[0x00000007]
```

After some debugging code and some additional analysis, I think this is working properly.  Here is the result:

```
Initializing Module: pmm.elf
   Starting Address: 0x00168000
The module is loaded at address: 0x00168000
   FileSize = 0x00026004; MemSize = 0x00026004; FileOffset = 0x00001000
      Attempting to map page 0x00001000 to frame 0x00000169
Mapping address 0x00001000 to frame 0x00000169
.. Ttl1 index is: 0x001c9000[0x00000000]
Creating a new TTL2 table for address 0x00001000
.. The new ttl2 frame is 0x0003e013
.. The base ttl2 1K location is 0x000f804c
.. The ttl1 index is 0x00000000
.. So the address of the ttl1 Entry is: 0xff404000
.. Set the TTL1 table index 0x00000000 to 1K location 0x000f804c
.. Set the TTL1 table index 0x00000001 to 1K location 0x000f804d
.. Set the TTL1 table index 0x00000002 to 1K location 0x000f804e
.. Set the TTL1 table index 0x00000003 to 1K location 0x000f804f
.. The management address for this Ttl2 table is 0xffc00004
.... The base location is 0xffc00000
.... The table offset is  0x00000000
.... The entry offset is  0x00000004
.. The TTL1 management index for this address is 0x00000ffc
<< Completed the table creation for 0x00001000
  Ttl2 location is: 0x3e013000[0x00000001]
      Attempting to map page 0x00002000 to frame 0x0000016a
Mapping address 0x00002000 to frame 0x0000016a
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000002]
      Attempting to map page 0x00003000 to frame 0x0000016b
Mapping address 0x00003000 to frame 0x0000016b
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000003]
      Attempting to map page 0x00004000 to frame 0x0000016c
Mapping address 0x00004000 to frame 0x0000016c
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000004]
      Attempting to map page 0x00005000 to frame 0x0000016d
Mapping address 0x00005000 to frame 0x0000016d
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000005]
      Attempting to map page 0x00006000 to frame 0x0000016e
Mapping address 0x00006000 to frame 0x0000016e
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000006]
      Attempting to map page 0x00007000 to frame 0x0000016f
Mapping address 0x00007000 to frame 0x0000016f
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000007]
      Attempting to map page 0x00008000 to frame 0x00000170
Mapping address 0x00008000 to frame 0x00000170
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000008]
      Attempting to map page 0x00009000 to frame 0x00000171
Mapping address 0x00009000 to frame 0x00000171
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000009]
      Attempting to map page 0x0000a000 to frame 0x00000172
Mapping address 0x0000a000 to frame 0x00000172
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x0000000a]
      Attempting to map page 0x0000b000 to frame 0x00000173
Mapping address 0x0000b000 to frame 0x00000173
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x0000000b]
      Attempting to map page 0x0000c000 to frame 0x00000174
Mapping address 0x0000c000 to frame 0x00000174
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x0000000c]
      Attempting to map page 0x0000d000 to frame 0x00000175
Mapping address 0x0000d000 to frame 0x00000175
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x0000000d]
      Attempting to map page 0x0000e000 to frame 0x00000176
Mapping address 0x0000e000 to frame 0x00000176
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x0000000e]
      Attempting to map page 0x0000f000 to frame 0x00000177
Mapping address 0x0000f000 to frame 0x00000177
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x0000000f]
      Attempting to map page 0x00010000 to frame 0x00000178
Mapping address 0x00010000 to frame 0x00000178
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000010]
      Attempting to map page 0x00011000 to frame 0x00000179
Mapping address 0x00011000 to frame 0x00000179
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000011]
      Attempting to map page 0x00012000 to frame 0x0000017a
Mapping address 0x00012000 to frame 0x0000017a
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000012]
      Attempting to map page 0x00013000 to frame 0x0000017b
Mapping address 0x00013000 to frame 0x0000017b
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000013]
      Attempting to map page 0x00014000 to frame 0x0000017c
Mapping address 0x00014000 to frame 0x0000017c
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000014]
      Attempting to map page 0x00015000 to frame 0x0000017d
Mapping address 0x00015000 to frame 0x0000017d
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000015]
      Attempting to map page 0x00016000 to frame 0x0000017e
Mapping address 0x00016000 to frame 0x0000017e
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000016]
      Attempting to map page 0x00017000 to frame 0x0000017f
Mapping address 0x00017000 to frame 0x0000017f
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000017]
      Attempting to map page 0x00018000 to frame 0x00000180
Mapping address 0x00018000 to frame 0x00000180
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000018]
      Attempting to map page 0x00019000 to frame 0x00000181
Mapping address 0x00019000 to frame 0x00000181
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000019]
      Attempting to map page 0x0001a000 to frame 0x00000182
Mapping address 0x0001a000 to frame 0x00000182
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x0000001a]
      Attempting to map page 0x0001b000 to frame 0x00000183
Mapping address 0x0001b000 to frame 0x00000183
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x0000001b]
      Attempting to map page 0x0001c000 to frame 0x00000184
Mapping address 0x0001c000 to frame 0x00000184
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x0000001c]
      Attempting to map page 0x0001d000 to frame 0x00000185
Mapping address 0x0001d000 to frame 0x00000185
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x0000001d]
      Attempting to map page 0x0001e000 to frame 0x00000186
Mapping address 0x0001e000 to frame 0x00000186
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x0000001e]
      Attempting to map page 0x0001f000 to frame 0x00000187
Mapping address 0x0001f000 to frame 0x00000187
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x0000001f]
      Attempting to map page 0x00020000 to frame 0x00000188
Mapping address 0x00020000 to frame 0x00000188
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000020]
      Attempting to map page 0x00021000 to frame 0x00000189
Mapping address 0x00021000 to frame 0x00000189
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000021]
      Attempting to map page 0x00022000 to frame 0x0000018a
Mapping address 0x00022000 to frame 0x0000018a
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000022]
      Attempting to map page 0x00023000 to frame 0x0000018b
Mapping address 0x00023000 to frame 0x0000018b
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000023]
      Attempting to map page 0x00024000 to frame 0x0000018c
Mapping address 0x00024000 to frame 0x0000018c
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000024]
      Attempting to map page 0x00025000 to frame 0x0000018d
Mapping address 0x00025000 to frame 0x0000018d
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000025]
      Attempting to map page 0x00026000 to frame 0x0000018e
Mapping address 0x00026000 to frame 0x0000018e
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000026]
      Attempting to map page 0x00027000 to frame 0x0000018f
Mapping address 0x00027000 to frame 0x0000018f
.. Ttl1 index is: 0x001c9000[0x00000000]
  Ttl2 location is: 0x3e013000[0x00000027]
   FileSize = 0x00000000; MemSize = 0x00000000; FileOffset = 0x00000000
```

Looking at the first mapping for `pmm.elf`, I notice a few things:

1. This was hard-coded but probably should not be... `.. So the address of the ttl1 Entry is: 0xff404000`
1. This check does not take the split tables into account... `.... The base location is 0xffc00000`
1. This calculation does nto look right either... `.. The TTL1 management index for this address is 0x00000ffc`

I will have to dig into this further tomorrow.

---

### 2019-Jan-29

I think I need some ASCII art....

There are 2 sets of paging tables to maintain -- the user-space tables and the kernel-space tables.


```
User TTL1:
+--------+--------+
|        |        |     * 8K in length (2 frames)
|        |        |     * stored in TTBR0
|        |        |     * virtual address in kernel space (for security reasons)
+--------+--------+


Kernel TTL1:
+--------+--------+--------+--------+
|        |        |        |        |     * 16K in length (4 frames)
|        |        |        |        |     * stored in TTBR1
|        |        |        |        |     * virtual address TTL1_VADDR
+--------+--------+--------+--------+
```

Now, it would seem that the User TTL1 can overlay into the Kernel TTL1, taking up the first 2 frames of the TTL1_VADDR.  However, when I look at a context change, it will require a lot of expensive fussiness when updating this information.  Not a good choice.

On the other hand, if I place this into user space, I may have a security problem where a rogue process can start replacing a page with frames to try to read all the memory in the system.

---

### 2019-Jan-30

As I continue to think about this some more, I am not sure I have much of a choice here....  I will need to define some space in the user paging tables to make the user space so that they are properly handled in a task swap.  The driving problem is simply the need to re-map so much during a task swap (or on demand to service a request) that it is not feasible to keep this information in shared kernel space.

---

### 2019-Feb-01

Well, I think I have my backups sorted out.  The disk I write my daily snapshots to crashed....  But that has been resolved and tomorrow (which is Saturday), I will be able to focus on OS development.

---

### 2019-Feb-02

So, going back to my MMU problem, I think I am going to need to allocate the top 4MB of User Space (`0x7c000000` to `0x7fffffff`) for managing the user paging tables.  The key difference here will be that the top 2MB will be lost -- and I'm not quite sure what I can put in its place (perhaps stacks or some other small elements).

So, with an update to the Virtual Memory Map wiki (http://eryjus.ddns.net:3000/projects/century-os/wiki/High-Level_Virtual_Memory_Map), I am ready to start to make this happen in code.

The first change is going to have to be `MmuMapToFrame()` which is going to need both tables and to make a decision which one will get the mapping.  This function appears to have the all the necessary information as the signature is correct:

```C
void MmuMapToFrame(MmuData_t mmu, ptrsize_t addr, frame_t frame, bool wrt, bool krn);
```

And, in fact I have even tried to get the mapping into the correct table:

```C
    ptrsize_t table = ((addr & 0x80000000) ? mmu.ttbr1 : mmu.ttbr0);
```

But, alas, I'm not convinced this is working properly (or maybe it is the call to `MmuMakeTtl2Table()` that is the problem).  And in fact `MmuMakeTtl2Table()` has the these types of calculations:

```C
    mgmtTtl2Addr = TTL2_VADDR + (ttl2Offset * 1024) + (((addr >> 12) & 0xff) * 4);
```

So, the place to start is there -- and in particular cleaning up the `#define`s and conditionals.

---

This has, of course, made a complete mess of `MmuInit()`.  In fact, I am dealing with a recursive call now.

---

I'm really having a hell of a time with this....  It's frustrating and it's taking away all my motivation to continue.  I need to figure out something before I hit that point where I want to start gutting everything.  At the moment, I am really thinking that I want to start re-architecting for v0.3.0.

I am going to take a look at the objective of v0.2.0:

> The objective of this version is to add support for the Raspberry Pi Model 2b. I am particularly focused on making sure that the architecture abstractions are solid. There is no driving need to add new functionality -- unless the rpi2b drives the need for a parallel move.

... and then compare that to the objective I have documented for v0.3.0:

> This version will better align the architecture abstractions by separating out the arch and platform components into separate groups.
>
> So far the arch components I have written to are:
>
>    * x86
>    * arm
>
> And the platform components are:
>
>    * PC
>    * BCM2835/6
>
> However, as I prepare to get into x86_64- and armv8-related architectures, this work will be important to complete to make the new architectures easier to add in.

Interestingly enough, I think I can justify some arhictecture reorganizaions with this version -- meaning I can start with creating `arch` and `platform` folders and reorganize my build system to accomodate.  But do I want to take this on now and completely stall out v0.2.0?

On the other hand, I should be able to eliminate the user paging table and get there rather quickly, if I ignore @geist's comment (at least for now anyway):

> [13:03] <geist> eryjus: to go back, yes absolutelly everyone uses the split page table thing

I was nearly at that point about a week ago, and then something changed and I felt I should continue to pursue the split page tables (in no small part based on his comment).  However, I think at this point the best path forward is to get this version done.

I really need to think on this decision a bit....  I'm calling it for the night to ponder the possibilities.  At the moment, I am leaning toward killing the split paging table for now....

So, what would that look like?

##### Version 0.2.0
* add rpi2b support wihtout making any big architecture/feature changes
* keep a single paging table for rpi2b, modeling after i686

##### Version 0.3.0
* separate `arch` and `platform` into separate folders
* possibly eliminate the loader and go with a kernel

But this will leave a few things unaccounted for:
1. eliminate the loader
1. split the page tables for rpi2b properly
1. prepare for UEFI boot

It will also makes me wonder if I am approaching this properly....  For example, should I work on writing the same results independently before integratng them into existing common code?  That would allow me to develop for the architecture without being encumbered by the existing code and then the integration effort would be where I spend all my time.

Lots to think about here.  Tomorrow will have some decisions.  And some real progress no matter my decision.

---

### 2019-Feb-03

OK, I have some decisions:
* I am going to back out the split paging table thing
* For the moment, I am *not* going to develop a kernel independently and then add it in
* I am *not* going to kill the loader in this version (but will likely do that in v0.3.0)
* I *will* go back and update the split paging tables in some version after v0.3.0, but *before* I get into 64-bit architectures

So, with this, I start making the paging tables the way I need them for the moment -- which will look very close to the i686 functions.

---

Well, it was relatively quick to clean up the compiles (without looking at logic).

---

Well, I'm back to this:

```C
    // -- Map the area needed to work on the PmmStart() module
    SerialPutS("Create a TTL2 table for the temporary work space needed for PMMInit()\n");
    MmuMakeTtl2Table(mmuBase, 0xff430000);
```

When this call to `MmuMakeTtl2Table()` is commented out, the kernel gets control but the system crashes in `PmmStart()`; when this is enabled, the kernel gets control but goes nowhere.

If I remember correctly, the Interrupt Vector Table is located in those same pages, so there should be no need to re-map.  Verifying (since I did not get my memory map complete...).

* The interrupt exception table is located at `0xff401000`.  This is TTL1 table entry `0xff4` and TTL2 entry `0x01`.
* The Process Initialization tables are located at `0xff430000`.  This is TTL1 table entry `0xff4` and TTL2 entry `0x30`.

So, there is no need to create a new TTL2 table, it already exists for the Interrupt Vector Table.  The problem must be in the `PmmStart()` function.

---

I think I am getting my own brain there...  This is my exception:

```
PmmStart(): Mapping addr 0xff430000 to frame 0x000001c9
  The address of the TTL2 Entry is at: 0xffffd0c0
Data Exception:
At address: 0x8003cf98
 R0: 0x00000032   R1: 0x80005c9c   R2: 0xffffd0ff
 R3: 0x00000020   R4: 0x0000016a   R5: 0x000001c9
 R6: 0x00121000   R7: 0x00103084   R8: 0x00000000
 R9: 0x00124000  R10: 0x2615fd81  R11: 0x87b06ffe
R12: 0x8003d078   SP: 0x80001dd4   LR_ret: 0x80001dd8
SPSR_ret: 0x600001d3     type: 0x17
```

I am trying to add a mapping into the TTL2 table entry at address `0xffffd0c0`.  However, the loader's `MmuInit()` is leaving that mapping in the following state:

```
MmuDumpTables: Walking the page tables for address 0xffffd0c0
Level  Tabl-Addr     Index        Entry Addr    Next PAddr    fault
-----  ----------    ----------   ----------    ----------    -----
TTL1   0x3e000000    0x00000fff   0x3e003ffc    0x3e005c00     01
TTL2   0x3e005c00    0x000000fd   0x3e005ff4    0x00000000     00
```

Meaning it is not mapping the management table properly for the TTL2 table I want to update.  Now, I believe that this is related to the folloing in `MmuMakeTtl2Table()`:

```C
    // If the TTL1 Entry for the management address is faulted; create a new TTL2 table
    if (mgmtTtl1Entry->fault == 0b00) {
        MmuMakeTtl2Table(mmu, mgmtTtl2Addr);
    }
```

In this case, I am only creating a new Ttl2 table, not actually mapping the address.

---

OK, I finally got to the bottom of the `PmmStart()` function and have it now geared for the rpi2b.   The kernel is trying to perform a task swap.  But right now, `ProcessSwitch()` is nothing more than an empty shell, so nothing happens.  I need to get into that next.

In the meantime, it I am very ready for a commit.

---

Well, I'm getting a crash on the call to `ProcessSwitch()`.  I know it is because I am not pushing/popping the `cpsr` and I will have to fix that tomorrow.  For tonight, I am pretty much cooked for the evening.

---

### 2019-Feb-04

Well, I started to clean up `ProcessSwitch()`.  It is much closer to what I am looking for.  However, the kernel is falling off to la-la land.

I am going to add some debugging code to `ProcessSwitch()` so I can track how far I get.

---

Here are the results:

```
OS: Executing process switch: 0x2
0
0x001cc000
2
3
0x80001504
4
▒Prefetch Abort:
At address: 0x8003ff60
 R0: 0x0000000d   R1: 0x37363534   R2: 0xf2215000
 R3: 0x00000020   R4: 0xdde7ead3   R5: 0x001030cc
 R6: 0x00121000   R7: 0x00103084   R8: 0x00000000
 R9: 0x00124000  R10: 0x2611e181  R11: 0x87b0277c
R12: 0x80040114   SP: 0x80004860   LR_ret: 0xdffb56be
SPSR_ret: 0x200001f3     type: 0x17
```

What is interesting is the address between checkpoints 3 and 4.  This address is the entry point of the `pmm.elf`.  This does not look right to me at all -- considering everything above `0x80000000` is kernel space and my `pmm.elf` is loaded at virtual address `0x00001000`.  Well, that is a waste!!!  I never changes the stack.  So the above means nothing.

---

Well, after all my debugging, all I can determine is that the stack is not mapped properly in the new paging tables.  When I update the paging tables, update the stack, and then try to do someting that requires a stack, the CPU faults which tries to push more stuff on the stack, which faults, which...  recursive.

So, the next thing to do is to take a hard look at `PmmStart()` to make sure the paging tables are mapped properly.

---

### 2019-Feb-05

I started by making a `MmuDumpTables()` function for `PmmStart()`.  It has revealed some issues with the management of the paging tables.

*But, more importantly, I now understand why "absolutely everyone uses the split paging tables thing."*  To be able to map the kernel pages into every running process is impossible to do properly -- especially when the kernel pages change.  However, I am not going to change now.  It will be wrong until I get to it properly.  I checked my notes and I still have the split paging table thing unaccounted for.  So, do I take care of that before I organize the arhcitectures and platforms or after.  I'm leaning on after (maybe an interim version like v0.3.1).  I will put that decision off for now, logging it in Redmine: http://eryjus.ddns.net:3000/issues/384.

---

### 2019-Feb-06

I have a few things to sort out today:

1. Interrupts appear to still be firing when they should not be.  I need to get to the root of that.
1. I have scheduling disabled, but it is still trying to schedule.  I need to get to the root of that.
1. I (STILL!!) have a problem with the paging tables, which is pissing me off.

Let me start at the top of the list.  That was a quick fix; I had the flags in the incorrect position (actually enabling the thumb instruction set!).

The second is a misunderstanding by me.  The lock that was attempted was for the heap, not for scheduling.

So, I thought I would get away from paging for a while and I cannot!!  `/me grumbles!`

---

### 2019-Feb-07

Well, I started today by copying and updating the loader's `MmuMapToFrame()` function to the kernel.  With that update, I was able to get really far.  The heap is being initialized and I have actually been able to send the initial message to the PMM process (now, whether the PMM process is actually getting is another story!).  I am reaching the end of the initialization and the final output is being written (well, and interrupted and then an exception).  But I am close!

What is happening is that `r5` is being updated somehow and after the interrupt I am getting some garbage back in the string to be printed -- causing the fault.

If I do not enable interrupts, then the kernel gets to the end cleanly.  If I enable interrupts but do not enable process swaps, I have the same problems.

So, my issue must be in the interrupt handler code -- corrupting some register.

---

OK, I figured out a problem with `SwitchTask()`....  It's likely not related, but at any rate it goes something like this:
* I push `r0`-`r12` on the stack (well, see the next comment)
* I push the `cpsr` on the stack (which also contains the privelege level)
* I save off the paging tables and esp into the process structure
* I then restore the paging tables from the process structure
* I also restore the stack from the process structure
* Then (and this is where the problem is), I pop the `cpsr`, which off the stack if the mode changes will change the stack pointer I just restored.

In short, the cpsr will need to be stored in the `Process_t` structure.

In the meantime, I also realized that I am only pushing `r0` and `r12` on the stack when I am supposed to be pushing `r0` through `r12` on the stack.  Gotta lot at all the handlers since I used the copy-and-paste coding method.

---

OK, even after these changes, I still have a problem.  I thought I would.  I disabled adding the next process into the queue to execute and I get to the end of the execution (well buried in interrupts, so I need to slow that down a bit).  The problem is indeed in the process management.

---

OMG!!!  It is finally working!

I started this back on 11-Nov and it's been a royal pain.  Needless to say, I need to commit this code.

