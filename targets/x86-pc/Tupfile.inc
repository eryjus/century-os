#####################################################################################################################
##
##  Tupfile -- An alternative to 'make` build system -- build the object files for the x86-pc target
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
## -- These are the vars needed to build the rpi2b target
##    ---------------------------------------------------
TARGET = x86-pc
ARCH = x86
PLAT = pc


##
## -- Now, we get all the rules needed for building the target applications
##    ---------------------------------------------------------------------
include_rules
include $(WS)/arch/$(ARCH)/util/Tupfile.inc
include $(WS)/platform/$(PLAT)/Tupfile.inc

