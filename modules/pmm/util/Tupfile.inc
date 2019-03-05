#####################################################################################################################
##
##  Tupfile -- An alternative to 'make` build system -- build the object files for the pmm manager
##
##        Copyright (c)  2017-2019 -- Adam Clark
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
CFLAGS += -I $(WS)/modules/pmm/inc
CFLAGS += -I $(WS)/modules/libc/inc
CFLAGS += -I $(WS)/modules/kernel/inc
CFLAGS += -I $(WS)/arch/$(ARCH)/inc


##
## -- Now, go compile all the source files for this binary
##    ----------------------------------------------------
: foreach  $(WS)/modules/pmm/src/*.cc |> !cc |>
