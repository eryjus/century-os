#####################################################################################################################
##
##  Tupfile -- An alternative to 'make` build system -- build the object files for the kernel
##
##        Copyright (c)  2017-2020 -- Adam Clark
##
## -----------------------------------------------------------------------------------------------------------------
##
##     Date      Tracker  Version  Pgmr  Description
##  -----------  -------  -------  ----  ---------------------------------------------------------------------------
##  2019-Mar-03  Initial   0.3.0   ADCL  Initial version
##
#####################################################################################################################


##
## -- Add the include files needed for compiling the objects
##    ------------------------------------------------------
CFLAGS += -I $(WS)/modules/kernel/inc
CFLAGS += -I $(WS)/arch/$(ARCH)/inc
CFLAGS += -I $(WS)/platform/inc
CFLAGS += -I $(WS)/platform/$(PLAT)/inc
CFLAGS += -I $(WS)/include
CFLAGS += -D__CENTURY_KERNEL__

AFLAGS += -I $(WS)/modules/kernel/inc
NFLAGS += -I$(WS)/modules/kernel/inc/



##
## -- Now, go compile all the source files for this binary
##    ----------------------------------------------------
: foreach  $(WS)/modules/kernel/src/*.cc                    |> !cc |>
: foreach  $(WS)/modules/kernel/src/butler/*.cc             |> !cc |>
: foreach  $(WS)/modules/kernel/src/cpu/*.cc                |> !cc |>
: foreach  $(WS)/modules/kernel/src/debugger/*.cc           |> !cc |>
: foreach  $(WS)/modules/kernel/src/elf/*.cc                |> !cc |>
: foreach  $(WS)/modules/kernel/src/frame-buffer/*.cc       |> !cc |>
: foreach  $(WS)/modules/kernel/src/hardware/*.cc           |> !cc |>
: foreach  $(WS)/modules/kernel/src/heap/*.cc               |> !cc |>
: foreach  $(WS)/modules/kernel/src/interrupts/*.cc         |> !cc |>
: foreach  $(WS)/modules/kernel/src/ipi/*.cc                |> !cc |>
: foreach  $(WS)/modules/kernel/src/loader/*.cc             |> !cc |>
: foreach  $(WS)/modules/kernel/src/mmu/*.cc                |> !cc |>
: foreach  $(WS)/modules/kernel/src/msgq/*.cc               |> !cc |>
: foreach  $(WS)/modules/kernel/src/pmm/*.cc                |> !cc |>
: foreach  $(WS)/modules/kernel/src/process/*.cc            |> !cc |>
: foreach  $(WS)/modules/kernel/src/spinlock/*.cc           |> !cc |>
: foreach  $(WS)/modules/kernel/src/stacks/*.cc             |> !cc |>
: foreach  $(WS)/modules/kernel/src/syscall/*.cc            |> !cc |>
: foreach  $(WS)/modules/kernel/src/timer/*.cc              |> !cc |>


##
## -- Now, go compile all the platform-specific source
##    ------------------------------------------------
: foreach  $(WS)/platform/$(PLAT)/framebuffer/*.cc          |> !cc |>
: foreach  $(WS)/platform/$(PLAT)/init/*.cc                 |> !cc |>
: foreach  $(WS)/platform/$(PLAT)/pic/*.cc                  |> !cc |>
: foreach  $(WS)/platform/$(PLAT)/serial/*.cc               |> !cc |>
: foreach  $(WS)/platform/$(PLAT)/timer/*.cc                |> !cc |>

: foreach  $(WS)/platform/$(PLAT)/timer/*.s                 |> !as |>


##
## -- Compiler the architecture-specific source
##    -----------------------------------------
: foreach  $(WS)/arch/$(ARCH)/*.cc                          |> !cc |>
: foreach  $(WS)/arch/$(ARCH)/*.s                           |> !as |>

: foreach  $(WS)/arch/$(ARCH)/common/*.cc                      |> !cc |>
: foreach  $(WS)/arch/$(ARCH)/common/*.s                       |> !as |>

: foreach  $(WS)/arch/$(ARCH)/cpu/*.cc                      |> !cc |>
: foreach  $(WS)/arch/$(ARCH)/cpu/*.s                       |> !as |>

: foreach  $(WS)/arch/$(ARCH)/mmu/*.cc                      |> !cc |>
: foreach  $(WS)/arch/$(ARCH)/mmu/*.s                       |> !as |>


