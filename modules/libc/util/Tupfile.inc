#####################################################################################################################
##
##  Tupfile -- An alternative to 'make` build system -- build the object files for the libc library
##
##        Copyright (c)  2017-2020 -- Adam Clark
##
## -----------------------------------------------------------------------------------------------------------------
##
##     Date      Tracker  Version  Pgmr  Description
##  -----------  -------  -------  ----  ---------------------------------------------------------------------------
##  2020-Apr-18  Initial  v0.7.0a  ADCL  Initial version
##
#####################################################################################################################


##
## -- Add the include files needed for compiling the objects
##    ------------------------------------------------------
CFLAGS += -I $(WS)/modules/libc/inc
AFLAGS += -I $(WS)/modules/libc/inc
NFLAGS += -I$(WS)/modules/libc/inc/



##
## -- Now, go compile all the source files for this binary
##    ----------------------------------------------------
: foreach  $(WS)/modules/libc/src/*.cc                      |> !cc |>

: foreach  $(WS)/modules/libc/src/$(ARCH)/*.s               |> !as |>


