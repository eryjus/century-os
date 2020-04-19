#####################################################################################################################
##
##  Tupfile -- An alternative to 'make` build system -- build the object files for the test module
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
CFLAGS += -I $(WS)/targets/$(TARGET)/usr/include
AFLAGS += -I $(WS)/targets/$(TARGET)/usr/include
NFLAGS += -I$(WS)/targets/$(TARGET)/usr/include/



##
## -- Now, go compile all the source files for this binary
##    ----------------------------------------------------
: foreach  $(WS)/modules/test/src/*.cc                      |> !cc |>


