#####################################################################################################################
##
##  Tupfile -- An alternative to 'make` build system -- build the object files for the x86-pc target
##
##        Copyright (c)  2017-2019 -- Adam Clark
##
## -----------------------------------------------------------------------------------------------------------------
##
##     Date      Tracker  Version  Pgmr  Description
##  -----------  -------  -------  ----  ---------------------------------------------------------------------------
##  2019-Dec-21  Initial  v0.5.0b  ADCL  Initial version
##
#####################################################################################################################


##
## -- These are the vars needed to build the rpi2b target
##    ---------------------------------------------------
TARGET = x86-pc
ARCH = x86
PLAT = pc


##
## -- Now, we get all the rules needed for building the target applications
##    ---------------------------------------------------------------------
include_rules

