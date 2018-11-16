#####################################################################################################################
##
##  Tupfile -- An alternative to the 'make' build system -- i686 kernel objects
##
##        Copyright (c)  2017-2018 -- Adam Clark
##
## ------------------------------------------------------------------------------------------------------------------
##
##     Date      Tracker  Version  Pgmr  Description
##  -----------  -------  -------  ----  ---------------------------------------------------------------------------
##  2017-Nov-11  Initial   0.2.0   ADCL  Initial version
##
#####################################################################################################################


##
## -- Pre-set some variables for the Tuprules.tup file and include the generic rules
##    ------------------------------------------------------------------------------
ARCH = rpi2b
MOD = kernel
CC = arm-eabi-gcc

include_rules

SRC = $(WS)/modules/$(MOD)/src


##
## -- These are the implicit build rules for the i686 kernel object files
##    -------------------------------------------------------------------
: foreach $(SRC)/$(ARCH)/*.s        | $(II) |> !nasm |>

: foreach $(SRC)/*.cc               | $(II) |> !cc |>
: foreach $(SRC)/cpu/*.cc           | $(II) |> !cc |>
: foreach $(SRC)/heap/*.cc          | $(II) |> !cc |>
: foreach $(SRC)/$(ARCH)/*.cc       | $(II) |> !cc |>
: foreach $(SRC)/ipc/*.cc           | $(II) |> !cc |>
: foreach $(SRC)/mmu/*.cc           | $(II) |> !cc |>
: foreach $(SRC)/pmm/*.cc           | $(II) |> !cc |>
: foreach $(SRC)/process/*.cc       | $(II) |> !cc |>
: foreach $(SRC)/syscall/*.cc       | $(II) |> !cc |>
: foreach $(SRC)/timer/*.cc         | $(II) |> !cc |>
