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
## -- These are the vars needed to build the rpi2b target
##    ---------------------------------------------------
TARGET = rpi2b
ARCH = arm
PLAT = bcm2836


##
## -- Now, we get all the rules needed for building the target applications
##    ---------------------------------------------------------------------
include_rules

